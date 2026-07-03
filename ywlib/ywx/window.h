#pragma once
#include <ywx/bitmap.h>
#include <ywx/keys.h>
#include <ywx/ui_label.h>

namespace yw {

template<typename T> concept window_options_like = requires(T t) {
  { t.title } -> convertible_to<string<wchar_t>>;
  { t.pos } -> convertible_to<std::optional<int2>>;
  { t.size } -> convertible_to<std::optional<int2>>;
  { t.get_style() } -> convertible_to<DWORD>;
  { t.get_exstyle() } -> convertible_to<DWORD>;
};

class window : public interface {
public:
  struct options {
    string<wchar_t> title{};
    std::optional<int2> pos{};
    std::optional<int2> size{};
    bool has_border = true;
    bool has_caption = true;
    bool resizable = true;
    bool visible = true;
    DWORD get_style() const noexcept {
      DWORD s = has_caption ? WS_CAPTION | WS_SYSMENU : WS_POPUP;
      if (has_border) s |= WS_BORDER;
      if (resizable) s |= WS_THICKFRAME;
      if (visible) s |= WS_VISIBLE;
      return s;
    }
    DWORD get_exstyle() const noexcept { return WS_EX_ACCEPTFILES; }
  };

  struct custom_options {
    string<wchar_t> title{};
    std::optional<int2> pos{};
    std::optional<int2> size{};
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exstyle = WS_EX_ACCEPTFILES;
    DWORD get_style() const noexcept { return style; }
    DWORD get_exstyle() const noexcept { return exstyle; }
  };

  struct tooltip {
    yw::text text{};
    color text_color = colors::white;
    color background_color = colors::black;
    float2 offset = float2::fill(arbitrary_value);
    float2 padding = float2::fill(arbitrary_value);
    double delay = 0.5;
  };

  struct slot : interface::slot {
    inline static std::vector<slotid> windows{};
    inline static int2 cursor_pos{}; // in screen coordinates
    HWND hwnd{};
    int4 frame_thickness{};
    int2 pos{}, size{};
    DWORD style{}, exstyle{};
    string<wchar_t> title{};
    bitmap controllayer{};
    bitmap rendertarget{};
    comptr<IDXGISwapChain1> swapchain{};
    color background_color = colors::white;

    slotid control_id{};
    slotid focused_control_id{};
    slotid hovered_control_id{};
    slotid mouse_capture_control_id{};
    slotid keyboard_capture_control_id{};

    TRACKMOUSEEVENT track_mouse_event{sizeof(TRACKMOUSEEVENT), TME_LEAVE};



    function<bool, button_event> on_button_down{};
    function<bool, button_event> on_button_up{};

    bool messy = false;
    bool dirty = false;
    bool drawn = false;
    bool resizing = false;

    //-- override functions --//

    virtual bool attachable() const override { return !control_id; }

    virtual std::expected<void, error> attach(slotid Child) override {
      if (control_id) return std::unexpected(error(errors::invalid_operation, "already has a control"));
      const auto csp = slot::get<control>(Child);
      if (!csp) return std::unexpected(error(errors::invalid_slotid));
      csp->window_id = id;
      csp->layout_id = id;
      control_id = Child;
      return {};
    }

    virtual std::expected<void, error> detach(slotid Child) override {
      if (control_id != Child) return std::unexpected(error(errors::invalid_operation, "not attached to this control"));
      if (auto res = slots.erase(Child); !res) return res.error().relay();
      control_id = {};
      return {};
    }

    virtual std::expected<void, error> make_dirty() override {
      dirty = true;
      return {};
    }

    virtual std::expected<void, error> make_messy() override {
      messy = true;
      return {};
    }

    virtual void button_event(yw::button_event e) {
      const auto csp = slot::get<control>(control_id);
      if (csp && csp->button_event(e)) return;
      if (e.down) {
        if (on_button_down) on_button_down(e);
      } else if (on_button_up) on_button_up(e);
    }

    //-- functions --//

    std::expected<uint2, error> get_necessary_size() const {
      const auto csp = slot::get<control>(control_id);
      if (!csp) return uint2::fill(arbitrary_value);
      const auto margin = csp->margin.xy() + csp->margin.zw();
      if (auto res = csp->get_necessary_size(); !res) return res.error().relay();
      else return vapply_r<uint2>(yw::max, vapply_r<uint2>(yw::ceil, *res + margin), uint2::fill(arbitrary_value));
    }

    std::expected<void, error> update_hovered_control(slotid New, float2 Pos, bool Moved) {
      if (New != hovered_control_id) {
        if (const auto csp = slot::get<control>(hovered_control_id)) csp->hover_event({Pos, hover_event::type::leave});
        hovered_control_id = New;
        if (const auto csp = slot::get<control>(hovered_control_id)) csp->hover_event({Pos, hover_event::type::enter});
      } else if (Moved)
        if (const auto csp = slot::get<control>(hovered_control_id)) csp->hover_event({Pos, hover_event::type::move});
      return {};
    }

    std::expected<void, error> update_rendertarget() {
      if (size == rendertarget.size()) return {};
      rendertarget = {};
      if (!swapchain) {
        auto desc = DXGI_SWAP_CHAIN_DESC1(size.x, size.y, bitmap::dxgiformat, false, DXGI_SAMPLE_DESC(1, 0), {}, 2);
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT, desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        hresult_test(dxgi::factory()->CreateSwapChainForHwnd, d3d::device(), hwnd, &desc, 0, 0, &swapchain.get());
      } else hresult_test(swapchain->ResizeBuffers, 0, size.x, size.y, bitmap::dxgiformat, 0);
      if (auto res = bitmap::create(swapchain.get())) rendertarget = std::move(*res);
      else return res.error().relay();
      if (auto res = bitmap::create(size)) controllayer = std::move(*res);
      else return res.error().relay();
      return {};
    }

    std::expected<void, error> update() {
      if (auto res = get_necessary_size(); !res) return res.error().relay();
      else size = vapply_r<uint2>(yw::max, size, *res);
      const bool size_changed = size != rendertarget.size();
      if (size_changed) {
        const auto area = size + frame_thickness.xy() + frame_thickness.zw();
        win32_bool_test(::SetWindowPos, hwnd, 0, 0, 0, area.x, area.y, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
        if (auto res = update_rendertarget(); !res) return res.error().relay();
        messy = true;
      }
      const auto csp = slot::get<control>(control_id);
      if (messy) {
        if (csp)
          if (auto res = csp->relocate({}, float2(float(size.x), float(size.y))); !res) return res.error().relay();
        dirty = true;
      }
      if (dirty) {
        if (auto d = controllayer.begin_draw(colors::transparent)) {
          if (csp) {
            if (auto rr = csp->redraw(); !rr) return rr.error().relay();
            if (track_mouse_event.hwndTrack != nullptr) {
              const auto hit = csp->hittest(cursor_pos - pos);
              if (auto res = update_hovered_control(hit, cursor_pos - pos, false); !res) return res.error().relay();
            }
          }
          if (auto res = d->close(); !res) return res.error().relay();
        } else return d.error().relay();
      }
      if (drawn) {
        if (auto res = rendertarget.begin_draw()) {
          auto d = std::move(*res);
          if (auto rr = draw_bitmap({}, controllayer); !rr) return rr.error().relay();
        } else return res.error().relay();
      } else {
        if (auto res = rendertarget.begin_draw(background_color)) {
          auto d = std::move(*res);
          if (auto rr = draw_bitmap({}, controllayer); !rr) return rr.error().relay();
        } else return res.error().relay();
      }
      hresult_test(swapchain->Present, 1, 0);
      messy = false;
      dirty = false;
      drawn = false;
      return {};
    }


    //-- create function --//

    template<typename T> static std::expected<slot*, error> create(T&& op, const source_line& sl) {
      static_assert(is_rvref<T&&>, "Unreachable");
      const auto temp_id = slot::add<window>();
      const auto sp = slot::get<window>(temp_id);
      if (!sp) return std::unexpected(error(errors::slot_creation_failed));
      sp->id = temp_id;
      sp->source = sl;
      sp->title = std::move(op.title);
      sp->style = op.get_style();
      sp->exstyle = op.get_exstyle();
      sp->hwnd = ::CreateWindowExW(
        sp->exstyle, wclass::name(), sp->title.c_str(), sp->style & ~WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        int(arbitrary_value), int(arbitrary_value), nullptr, nullptr, wclass::hinstance(), nullptr);
      if (!sp->hwnd) return std::unexpected(error(errors::operation_failed, "CreateWindowExW failed"));
      ::SetWindowLongPtrW(sp->hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(sp->id));
      windows.push_back(sp->id);
      RECT wr{}, cr{};
      win32_bool_test(::GetWindowRect, sp->hwnd, &wr);
      win32_bool_test(::GetClientRect, sp->hwnd, &cr);
      const auto left = (wr.right - wr.left - cr.right) / 2;
      sp->frame_thickness = int4(left, wr.bottom - wr.top - cr.bottom - left, left, left);
      sp->pos = op.pos.value_or(int2(wr.left, wr.top));
      sp->size = vapply_r<uint2>(yw::max, op.size.value_or(int2(cr.right, cr.bottom)), uint2::fill(arbitrary_value));
      const auto area = sp->size + sp->frame_thickness.xy() + sp->frame_thickness.zw();
      win32_bool_test(::SetWindowPos, sp->hwnd, 0, sp->pos.x, sp->pos.y, area.x, area.y, SWP_NOZORDER | SWP_NOACTIVATE);
      if (auto res = sp->update_rendertarget(); !res) return res.error().relay();
      if (sp->style & WS_VISIBLE) ::ShowWindow(sp->hwnd, SW_SHOW);
      return sp;
    }
  };

  virtual ~window() { close(); }
  explicit window() noexcept = default;
  window(window&&) noexcept = default;

  window& operator=(window&& Other) noexcept {
    if (this == &Other) return *this;
    close();
    _id = std::exchange(Other._id, {});
    return *this;
  }

  window(options Options, const source_line& sl = here()) {
    if (auto res = slot::create(std::move(Options), sl)) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  window(window_options_like auto Options, const source_line& sl = here()) {
    if (auto res = slot::create(std::move(Options), sl)) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<window, error> create(options Options, const source_line& sl = here()) {
    window w;
    if (auto res = slot::create(std::move(Options), sl)) w._id = (*res)->id;
    else return res.error().relay();
    return w;
  }

  static std::expected<window, error> create(window_options_like auto Options, const source_line& sl = here()) {
    window w;
    if (auto res = slot::create(std::move(Options), sl)) w._id = (*res)->id;
    else return res.error().relay();
    return w;
  }

  std::expected<void, error> close() {
    if (const auto sp = slot::get<window>(_id); sp) win32_bool_test(::DestroyWindow, sp->hwnd);
    return {};
  }

  std::expected<void, error> show() {
    if (const auto sp = slot::get<window>(_id); sp) win32_bool_test(::ShowWindow, sp->hwnd, SW_SHOW);
    return {};
  }

  std::expected<void, error> hide() {
    if (const auto sp = slot::get<window>(_id); sp) win32_bool_test(::ShowWindow, sp->hwnd, SW_HIDE);
    return {};
  }

  std::expected<drawing, error> begin_draw() {
    const auto sp = slot::get<window>(_id);
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (!sp->rendertarget) return std::unexpected(error(errors::invalid_operation, "rendertarget not initialized"));
    if (auto res = sp->rendertarget.begin_draw(sp->background_color)) {
      sp->drawn = true;
      return std::move(*res);
    } else return res.error().relay();
  }

  //-- getter --//

  HWND hwnd() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->hwnd;
  }

  const auto& frame_thickness() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->frame_thickness;
  }

  const auto& pos() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->pos;
  }

  const auto& size() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->size;
  }

  const auto& style() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->style;
  }

  const auto& exstyle() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->exstyle;
  }

  const auto& title() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->title;
  }

  const auto& background_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->background_color;
  }

  const auto& on_button_down() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->on_button_down;
  }

  const auto& on_button_up() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->on_button_up;
  }

  //-- setter --//

  auto& pos(int2 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->pos = v;
    const auto area = sp->size + sp->frame_thickness.xy() + sp->frame_thickness.zw();
    if (!::SetWindowPos(sp->hwnd, 0, v.x, v.y, area.x, area.y, SWP_NOZORDER | SWP_NOACTIVATE))
      error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError())).go_off();
    return *this;
  }

  auto& size(int2 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (auto res = sp->get_necessary_size(); !res) res.error().go_off();
    else sp->size = vapply_r<uint2>(yw::max, v, *res);
    const auto area = sp->size + sp->frame_thickness.xy() + sp->frame_thickness.zw();
    if (!::SetWindowPos(sp->hwnd, 0, sp->pos.x, sp->pos.y, area.x, area.y, SWP_NOZORDER | SWP_NOACTIVATE))
      error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError())).go_off();
    if (auto res = sp->make_messy(); !res) res.error().go_off();
    return *this;
  }

  auto& title(string<wchar_t> v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->title = std::move(v);
    if (!::SetWindowTextW(sp->hwnd, sp->title.c_str()))
      error(errors::operation_failed, "SetWindowTextW failed", int32_t(::GetLastError())).go_off();
    return *this;
  }

  auto& background_color(const color& c) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->background_color = c;
    if (auto res = sp->make_dirty(); !res) res.error().go_off();
    return *this;
  }

  auto& on_button_down(function<bool, button_event> f) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_button_down = std::move(f);
    return *this;
  }

  auto& on_button_up(function<bool, button_event> f) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->on_button_up = std::move(f);
    return *this;
  }
};
} // namespace yw
