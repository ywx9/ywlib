#pragma once
#include <ywx/bitmap.h>
#include <ywx/command_manager.h>
#include <ywx/keys.h>
#include <ywx/ui_label.h>

namespace yw {

template<typename T> concept window_options_like = requires(T t) {
  { t.pos } -> convertible_to<optional<int2>>;
  { t.size } -> convertible_to<optional<int2>>;
  { t.get_title() } -> convertible_to<string<wchar_t>>;
  { t.get_style() } -> convertible_to<DWORD>;
  { t.get_exstyle() } -> convertible_to<DWORD>;
};

class window : public interface {
public:
  struct options {
    string<wchar_t> title{};
    optional<int2> pos{};
    optional<int2> size{};
    bool has_border = true;
    bool has_caption = true;
    bool resizable = true;
    bool visible = true;
    bool enabled = true;
    const string<wchar_t>& get_title() const noexcept { return title; }
    DWORD get_style() const noexcept {
      DWORD s = has_caption ? WS_CAPTION | WS_SYSMENU : WS_POPUP;
      if (has_border) s |= WS_BORDER;
      if (resizable) s |= WS_THICKFRAME;
      if (visible) s |= WS_VISIBLE;
      return s;
    }
    DWORD get_exstyle() const noexcept { return WS_EX_ACCEPTFILES; }
  };

  struct slot : interface::slot {
    inline static std::vector<slotid> windows{};
    inline static int2 cursor_pos{}; // in screen coordinates
    slotid parent_id{};
    HWND hwnd{};
    int4 frame_thickness{};
    int2 pos{}, size{};
    DWORD style{}, exstyle{};
    string<wchar_t> title{};
    bitmap controllayer{};
    bitmap rendertarget{};
    comptr<IDXGISwapChain1> swapchain{};
    ui::color_theme color_theme = ui::light_color_theme;
    color background_color = colors::white;

    color tooltip_background_color = color(0.99f, 0.98f, 0.96f, 0.8f);
    color tooltip_border_color = colors::black;
    color tooltip_text_color = colors::black;
    float4 tooltip_padding = float4::fill(arbitrary_value);
    float2 tooltip_radius = float2::fill(arbitrary_value);
    float2 tooltip_offset = float2::fill(arbitrary_value * 2);
    float tooltip_border_thickness = 0.5f;
    font_config tooltip_font = font_config{.size = 12.0f};
    double tooltip_delay = 0.5;
    text tooltip_text{};

    slotid control_id{};
    slotid focused_control_id{};
    slotid hovered_control_id{};
    slotid mouse_capture_control_id{};
    bool window_mouse_capture = false;
    slotid keyboard_capture_control_id{};
    slotid tooltip_control_id{};

    command_manager commands{};
    std::vector<slotid> subwindows{};

    color focus_overlay_color;
    color hover_overlay_color;
    color press_overlay_color;

    optional<float3> caret_pos{};

    TRACKMOUSEEVENT track_mouse_event{sizeof(TRACKMOUSEEVENT), TME_LEAVE};
    int2 last_cursor_pos{}; // updated in mouse_move_event

    function<bool, yw::button_event> button_event{};
    function<bool, yw::drag_event> drag_event{};
    function<bool, yw::focus_event> focus_event{};
    function<bool, yw::key_event> key_event{};
    function<bool, yw::pointer_event> pointer_event{};
    function<bool, yw::wheel_event> wheel_event{};

    function<bool, yw::uint2> resize_event{};

    bool visible = true;
    bool enabled = true;
    bool fit_to_necessary_size = false;
    bool messy = false;
    bool dirty = false;
    bool drawn = false;
    bool resizing = false;
    bool tooltip_visible = false;
    bool tooltip_resolved = false;
    float2 tooltip_anchor_pos{};
    double tooltip_enter_time = 0.0;

    //-- override functions --//

    virtual bool attachable() const noexcept override { return !control_id; }

    virtual std::expected<void, error> attach(slotid Child) override {
      if (control_id) return std::unexpected(error(errors::invalid_operation, "already has a control"));
      const auto csp = get_slot<control>(Child);
      if (!csp) return std::unexpected(error(errors::invalid_slotid));
      csp->window_id = id;
      control_id = Child;
      return {};
    }

    virtual std::expected<void, error> detach(slotid Child) override {
      if (control_id != Child) return std::unexpected(error(errors::invalid_operation, "not attached to this control"));
      if (auto res = slots.erase(Child); !res) return res.error().relay();
      control_id = {};
      return {};
    }

    virtual slotid get_window_id() const noexcept override { return id; }

    //-- functions --//

    std::expected<void, error> apply_color_theme(const ui::color_theme& Theme, bool Recursive) {
      color_theme = Theme;
      background_color = Theme.canvas;
      tooltip_background_color = color(Theme.surface_popup, 0.95f);
      tooltip_border_color = Theme.outline;
      tooltip_text_color = Theme.text;
      focus_overlay_color = Theme.accent;
      hover_overlay_color = color(Theme.accent, ui::default_overlay_opacity.hover);
      press_overlay_color = color(Theme.accent, ui::default_overlay_opacity.press);
      if (Recursive) {
        if (const auto csp = get_slot<control>(control_id)) {
          if (auto res = csp->apply_color_theme(Theme, true); !res) return res.error().relay();
        } else if (control_id) return std::unexpected(error(errors::invalid_slotid));
      }
      dirty = true;
      return {};
    }

    int2 get_bounds() const noexcept { return size + frame_thickness.xy() + frame_thickness.zw(); }
    int2 get_client_origin() const noexcept { return pos + frame_thickness.xy(); }
    int2 get_local_pointer_pos() const noexcept { return cursor_pos - get_client_origin(); }

    std::expected<uint2, error> get_necessary_size() const {
      const auto csp = get_slot<control>(control_id);
      if (!csp) return uint2::fill(arbitrary_value);
      const auto margin = csp->margin.xy() + csp->margin.zw();
      if (auto res = csp->get_necessary_size(); !res) return res.error().relay();
      else return vapply_r<uint2>(yw::max, vapply_r<uint2>(yw::ceil, *res + margin), uint2::fill(arbitrary_value));
    }

    void hide_tooltip() noexcept {
      tooltip_control_id = {};
      tooltip_visible = false;
      tooltip_resolved = false;
      tooltip_enter_time = 0.0;
      tooltip_anchor_pos = {};
      tooltip_text = {};
    }

    void clear_window_state() noexcept {
      if (const auto csp = get_slot<control>(focused_control_id)) csp->handle_focus_event({false});
      focused_control_id = {};
      caret_pos = std::nullopt;

      if (const auto csp = get_slot<control>(hovered_control_id)) {
        csp->handle_hover_event(yw::hover_event::create::leave(get_local_pointer_pos()));
        dirty = true;
      }
      hovered_control_id = {};

      if (const auto csp = get_slot<control>(mouse_capture_control_id)) csp->reset_state();
      mouse_capture_control_id = {};
      window_mouse_capture = false;
      ::ReleaseCapture();

      keyboard_capture_control_id = {};
      track_mouse_event.hwndTrack = nullptr;
      hide_tooltip();
      update_ime_window();
    }

    void update_ime_window() noexcept {
      const HIMC himc = ::ImmGetContext(hwnd);
      if (!himc) return;
      if (caret_pos) {
        const auto x = LONG(caret_pos->x);
        const auto y = LONG(caret_pos->y);
        const auto h = LONG(yw::max(caret_pos->z, 1.0f));

        CANDIDATEFORM candidate{};
        candidate.dwIndex = 0;
        candidate.dwStyle = CFS_EXCLUDE;
        candidate.ptCurrentPos = POINT{x, y + h};
        candidate.rcArea = RECT{x, y, x + 1, y + h};
        ::ImmSetCandidateWindow(himc, &candidate);

        COMPOSITIONFORM composition{};
        composition.dwStyle = CFS_POINT;
        composition.ptCurrentPos = POINT{x, y};
        ::ImmSetCompositionWindow(himc, &composition);
      }
      ::ImmReleaseContext(hwnd, himc);
    }

    std::expected<void, error> update_caret_pos() {
      if (const auto csp = get_slot<control>(focused_control_id)) caret_pos = csp->get_caret_pos();
      else caret_pos = std::nullopt;
      update_ime_window();
      return {};
    }

    std::expected<void, error> update_focused_control(is_bool auto Backward) {
      const auto csp = get_slot<control>(control_id);
      if (!csp) return {};
      const auto old_fc_id = focused_control_id;
      bool found = !slot::slots.contains(old_fc_id);
      focused_control_id = csp->find_next_tabstop(old_fc_id, Backward, found);
      if (const auto old_fcsp = get_slot<control>(old_fc_id)) old_fcsp->handle_focus_event({false});
      if (const auto new_fcsp = get_slot<control>(focused_control_id)) new_fcsp->handle_focus_event({true});
      if (auto res = update_caret_pos(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> update_hovered_control(slotid New, float2 Pos, bool Moved, double Time) {
      if (New != hovered_control_id) {
        if (const auto csp = get_slot<control>(hovered_control_id)) {
          csp->handle_hover_event(yw::hover_event::create::leave(Pos));
          dirty = true;
        }
        hovered_control_id = New;
        hide_tooltip();
        tooltip_control_id = New;
        tooltip_enter_time = Time;
        tooltip_anchor_pos = Pos;
        if (const auto csp = get_slot<control>(hovered_control_id)) {
          csp->handle_hover_event(yw::hover_event::create::enter(Pos));
          dirty = true;
        }
      } else if (Moved) {
        tooltip_anchor_pos = Pos;
        if (const auto csp = get_slot<control>(hovered_control_id))
          csp->handle_hover_event(yw::hover_event::create::move(Pos));
      }
      return {};
    }

    std::expected<void, error> update_tooltip(double Time) {
      if (!tooltip_control_id || tooltip_visible || tooltip_resolved) return {};
      if (Time - tooltip_enter_time < tooltip_delay) return {};
      tooltip_resolved = true;
      const auto csp = get_slot<control>(tooltip_control_id);
      if (!csp || csp->tooltip.empty()) return {};
      if (auto res = text::create(csp->tooltip, tooltip_font)) tooltip_text = std::move(*res);
      else return res.error().relay();
      tooltip_visible = true;
      return {};
    }

    std::expected<void, error> draw_tooltip() const {
      if (!tooltip_visible || !tooltip_text) return {};
      auto pos = tooltip_anchor_pos + tooltip_offset;
      const auto size = tooltip_text.size() + tooltip_padding.xy() + tooltip_padding.zw();
      pos.x = yw::min(pos.x, float(this->size.x) - size.x);
      pos.y = yw::min(pos.y, float(this->size.y) - size.y);
      if (tooltip_background_color.a > 0.0f) {
        brush::color(tooltip_background_color);
        if (auto res = fill_round_rectangle(pos, size, tooltip_radius); !res) return res.error().relay();
      }
      brush::color(tooltip_text_color);
      if (auto res = draw_text(pos + tooltip_padding.xy(), tooltip_text); !res) return res.error().relay();
      if (tooltip_border_color.a > 0.0f && tooltip_border_thickness > 0.0f) {
        brush::color(tooltip_border_color);
        if (auto res = stroke_round_rectangle(pos, size, tooltip_radius, tooltip_border_thickness); !res)
          return res.error().relay();
      }
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
      if (resize_event) resize_event(size);
      return {};
    }

    std::expected<void, error> update_layout() {
      if (auto res = get_necessary_size(); !res) return res.error().relay();
      else if (fit_to_necessary_size) size = *res;
      else size = vapply_r<uint2>(yw::max, size, *res);
      const bool size_changed = size != rendertarget.size();
      if (size_changed) {
        const auto b = get_bounds();
        win32_bool_test(::SetWindowPos, hwnd, 0, 0, 0, b.x, b.y, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
        if (auto res = update_rendertarget(); !res) return res.error().relay();
        messy = true;
      }
      const auto csp = get_slot<control>(control_id);
      if (messy) {
        if (csp) {
          if (auto res = csp->relocate({}, float2(float(size.x), float(size.y))); !res) return res.error().relay();
        }
        dirty = true;
      }
      return {};
    }

    std::expected<void, error> update(double Time) {
      if (!visible) return {};
      if (auto res = update_layout(); !res) return res.error().relay();
      const auto csp = get_slot<control>(control_id);
      if (dirty) {
        if (auto d = controllayer.begin_draw(colors::transparent)) {
          if (csp) {
            if (auto rr = csp->redraw(); !rr) return rr.error().relay();
            if (auto res = update_caret_pos(); !res) return res.error().relay();
            if (track_mouse_event.hwndTrack != nullptr) {
              const auto hit = csp->hittest(get_local_pointer_pos());
              if (auto res = update_hovered_control(hit, get_local_pointer_pos(), false, Time); !res)
                return res.error().relay();
            }
          }
          if (auto res = d->close(); !res) return res.error().relay();
        } else return d.error().relay();
      }
      drawing d{};
      if (drawn) {
        if (auto res = rendertarget.begin_draw()) d = std::move(*res);
        else return res.error().relay();
      } else if (auto res = rendertarget.begin_draw(background_color)) d = std::move(*res);
      else return res.error().relay();
      if (auto rr = draw_bitmap({}, controllayer); !rr) return rr.error().relay();
      if (auto tr = update_tooltip(Time); !tr) return tr.error().relay();
      if (auto tt = draw_tooltip(); !tt) return tt.error().relay();
      if (auto res = d.close(); !res) return res.error().relay();
      hresult_test(swapchain->Present, 1, 0);
      messy = false, dirty = false, drawn = false;
      for (const auto& subwin_id : subwindows)
        if (const auto swsp = get_slot<window>(subwin_id))
          if (auto res = swsp->update(Time); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> sync_redraw() {
      if (auto res = update_layout(); !res) return res.error().relay();
      const auto csp = get_slot<control>(control_id);
      if (auto d = controllayer.begin_draw(colors::transparent)) {
        if (csp) {
          if (auto rr = csp->redraw(); !rr) return rr.error().relay();
          if (auto res = update_caret_pos(); !res) return res.error().relay();
        }
        if (auto res = d->close(); !res) return res.error().relay();
      } else return d.error().relay();
      dirty = false;
      messy = false;
      return {};
    }

    std::expected<void, error> close_subwindows() {
      const auto ids = std::exchange(subwindows, {});
      for (const auto& id : ids)
        if (const auto swsp = get_slot<window>(id); swsp && swsp->hwnd)
          if (auto res = swsp->close(); !res) return res.error().relay();
      return {};
    }

    std::expected<void, error> close() {
      if (hwnd) win32_bool_test(::DestroyWindow, hwnd);
      return {};
    }

    //-- create function --//

    template<typename T> static std::expected<slot*, error> create(T&& op, slotid Parent = {}) {
      static_assert(is_rvref<T&&>, "Unreachable");
      const auto temp_id = make_slot<window>();
      const auto sp = get_slot<window>(temp_id);
      if (!sp) return std::unexpected(error(errors::slot_creation_failed));
      sp->id = temp_id;
      sp->parent_id = Parent;
      sp->title = op.get_title();
      sp->style = op.get_style();
      sp->exstyle = op.get_exstyle();
      if (auto res = sp->apply_color_theme(sp->color_theme, false); !res) return res.error().relay();
      sp->hwnd = ::CreateWindowExW(
        sp->exstyle, wclass::name(), sp->title.c_str(), sp->style & ~WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        int(arbitrary_value), int(arbitrary_value), nullptr, nullptr, wclass::hinstance(), nullptr);
      if (!sp->hwnd) return std::unexpected(error(errors::operation_failed, "CreateWindowExW failed"));
      ::SetWindowLongPtrW(sp->hwnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(sp->id));
      if (!Parent) windows.push_back(sp->id);
      else if (const auto psp = get_slot<window>(Parent)) psp->subwindows.push_back(sp->id);
      else return std::unexpected(error(errors::invalid_slotid));
      RECT wr{}, cr{};
      win32_bool_test(::GetWindowRect, sp->hwnd, &wr);
      win32_bool_test(::GetClientRect, sp->hwnd, &cr);
      const auto left = (wr.right - wr.left - cr.right) / 2;
      sp->frame_thickness = int4(left, wr.bottom - wr.top - cr.bottom - left, left, left);
      sp->pos = op.pos.value_or(int2(wr.left, wr.top));
      sp->visible = static_cast<bool>(sp->style & WS_VISIBLE);
      if constexpr (requires { op.enabled; }) sp->enabled = static_cast<bool>(op.enabled);
      else sp->enabled = true;
      sp->fit_to_necessary_size = !(sp->style & WS_THICKFRAME) && !op.size.has_value();
      sp->size = vapply_r<uint2>(yw::max, op.size.value_or(int2(cr.right, cr.bottom)), uint2::fill(arbitrary_value));
      const auto b = sp->get_bounds();
      win32_bool_test(::SetWindowPos, sp->hwnd, 0, sp->pos.x, sp->pos.y, b.x, b.y, SWP_NOZORDER | SWP_NOACTIVATE);
      if (!sp->enabled) win32_bool_test(::EnableWindow, sp->hwnd, false);
      if (auto res = sp->update_rendertarget(); !res) return res.error().relay();
      if (sp->style & WS_VISIBLE) ::ShowWindow(sp->hwnd, SW_SHOW);
      return sp;
    }

    //-- event functions --//

    std::expected<bool, error> handle_button_event(yw::button_event e) {
      bool control_handled = false;
      last_cursor_pos = pos + frame_thickness.xy() + e.pos;
      if (const auto csp = get_slot<control>(control_id)) {
        const auto old_fc_id = focused_control_id;
        const auto hit_id = csp->hittest(e.pos);
        const auto hcsp = get_slot<control>(hit_id);
        const auto focus_id = (hcsp && hcsp->is_focusable()) ? hit_id : slotid{};
        if (e.key == keys::lbutton)
          if (e.down && focused_control_id != focus_id) {
            focused_control_id = focus_id;
            dirty = true;
            if (const auto old_fcsp = get_slot<control>(old_fc_id)) old_fcsp->handle_focus_event({false});
            if (const auto new_fcsp = get_slot<control>(focused_control_id)) new_fcsp->handle_focus_event({true});
            if (auto res = update_caret_pos(); !res) return res.error().relay();
          }
        if (e.down) {
          if (hcsp) control_handled = hcsp->handle_button_event(e);
          mouse_capture_control_id = hit_id;
          dirty = true;
          if (hit_id) ::SetCapture(hwnd);
        } else if (const auto ccsp = get_slot<control>(mouse_capture_control_id)) {
          control_handled = ccsp->handle_button_event(e);
          if (hit_id == mouse_capture_control_id) control_handled |= ccsp->handle_click_event(e);
          mouse_capture_control_id = {};
          dirty = true;
          ::ReleaseCapture();
        }
        if (control_handled) {
          if (auto res = update_caret_pos(); !res) return res.error().relay();
          return true;
        }
      }
      if (e.down && drag_event && !mouse_capture_control_id) {
        window_mouse_capture = true;
        ::SetCapture(hwnd);
      } else if (!e.down && window_mouse_capture) {
        window_mouse_capture = false;
        ::ReleaseCapture();
      }
      if (button_event) return button_event(e);
      return false;
    }

    std::expected<bool, error> handle_char_event(wchar_t c) {
      if (const auto csp = get_slot<control>(focused_control_id))
        if (csp->handle_char_event(c)) return true;
      return false;
    }

    std::expected<bool, error> handle_double_click_event(yw::button_event e) {
      if (const auto csp = get_slot<control>(control_id)) {
        const auto hit_id = csp->hittest(e.pos);
        if (const auto hcsp = get_slot<control>(hit_id); hcsp && hcsp->handle_double_click_event(e)) {
          if (auto res = update_caret_pos(); !res) return res.error().relay();
          return true;
        }
      }
      return false;
    }

    std::expected<bool, error> handle_drag_event(yw::drag_event e) {
      if (const auto ccsp = get_slot<control>(mouse_capture_control_id); ccsp && ccsp->handle_drag_event(e)) {
        if (auto res = update_caret_pos(); !res) return res.error().relay();
        return true;
      }
      if (drag_event) return drag_event(e);
      return false;
    }

    std::expected<bool, error> handle_focus_event(yw::focus_event e) {
      if (focus_event) return focus_event(e);
      return false;
    }

    /// \note (en) To avoid swallowing system-handled events such as Alt+F4, this function returns a bool.
    std::expected<bool, error> handle_key_event(yw::key_event e) {
      if (const auto csp = get_slot<control>(control_id)) {
        if (e.down && e.key == keys::tab && !e.mods.ctrl) {
          const auto old_fc_id = focused_control_id;
          bool found = !slot::slots.contains(old_fc_id);
          focused_control_id = csp->find_next_tabstop(old_fc_id, e.mods.shift, found);
          if (const auto old_fcsp = get_slot<control>(old_fc_id)) old_fcsp->handle_focus_event({false});
          if (const auto new_fcsp = get_slot<control>(focused_control_id)) new_fcsp->handle_focus_event({true});
          if (auto res = update_caret_pos(); !res) return res.error().relay();
          return true;
        } else if (e.down && e.key == keys::escape && focused_control_id && !e.mods.ctrl && !e.mods.shift &&
                   !e.mods.alt) {
          const auto old_fc_id = focused_control_id;
          focused_control_id = {};
          if (const auto old_fcsp = get_slot<control>(old_fc_id)) old_fcsp->handle_focus_event({false});
          if (auto res = update_caret_pos(); !res) return res.error().relay();
          return true;
        } else if (const auto fcsp = get_slot<control>(focused_control_id); fcsp && fcsp->handle_key_event(e)) {
          if (auto res = update_caret_pos(); !res) return res.error().relay();
          return true;
        }
      }
      if (key_event) return key_event(e);
      return false;
    }

    std::expected<bool, error> handle_pointer_event(yw::pointer_event e) {
      if (const auto csp = get_slot<control>(control_id)) {
        const auto hit = csp->hittest(e.pos);
        if (const auto hcsp = get_slot<control>(hit); hcsp && hcsp->handle_pointer_event(e)) return true;
      }
      if (pointer_event) return pointer_event(e);
      return false;
    }

    std::expected<bool, error> handle_wheel_event(yw::wheel_event e) {
      if (const auto csp = get_slot<control>(control_id)) {
        const auto hit = csp->hittest(e.pos);
        if (const auto hcsp = get_slot<control>(hit); hcsp && hcsp->handle_wheel_event(e)) { return true; }
      }
      if (wheel_event) return wheel_event(e);
      return false;
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
    if (auto res = slot::create(std::move(Options))) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  window(window_options_like auto Options, const source_line& sl = here()) {
    if (auto res = slot::create(std::move(Options))) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  /// creates subwindow
  window(same_as<window> auto& Parent, options Options, const source_line& sl = here()) {
    if (auto res = slot::create(std::move(Options), Parent.id())) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  /// creates subwindow
  window(same_as<window> auto& Parent, window_options_like auto Options, const source_line& sl = here()) {
    if (auto res = slot::create(std::move(Options), Parent.id())) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<window, error> create(options Options) {
    window w;
    if (auto res = slot::create(std::move(Options))) w._id = (*res)->id;
    else return res.error().relay();
    return w;
  }

  static std::expected<window, error> create(window_options_like auto Options) {
    window w;
    if (auto res = slot::create(std::move(Options))) w._id = (*res)->id;
    else return res.error().relay();
    return w;
  }

  /// creates subwindow
  static std::expected<window, error> create(same_as<window> auto& Parent, options Options) {
    window w;
    if (auto res = slot::create(std::move(Options), Parent.id())) w._id = (*res)->id;
    else return res.error().relay();
    return w;
  }

  /// creates subwindow
  static std::expected<window, error> create(same_as<window> auto& Parent, window_options_like auto Options) {
    window w;
    if (auto res = slot::create(std::move(Options), Parent.id())) w._id = (*res)->id;
    else return res.error().relay();
    return w;
  }

  std::expected<void, error> close() {
    if (const auto sp = get_slot<window>(_id); sp) return sp->close();
    return {};
  }

  std::expected<void, error> show() {
    if (const auto sp = get_slot<window>(_id); sp) {
      if (sp->visible) return {};
      sp->visible = true;
      ::ShowWindow(sp->hwnd, SW_SHOW);
      sp->messy = true;
    }
    return {};
  }

  std::expected<void, error> hide() {
    if (const auto sp = get_slot<window>(_id); sp) {
      if (!sp->visible) return {};
      sp->clear_window_state();
      sp->visible = false;
      ::ShowWindow(sp->hwnd, SW_HIDE);
    }
    return {};
  }

  std::expected<drawing, error> begin_draw() {
    const auto sp = get_slot<window>(_id);
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if (!sp->rendertarget) return std::unexpected(error(errors::invalid_operation, "rendertarget not initialized"));
    if (auto res = sp->rendertarget.begin_draw(sp->background_color)) {
      sp->drawn = true;
      return std::move(*res);
    } else return res.error().relay();
  }

  //-- getter --//

  HWND hwnd() const noexcept {
    if (const auto sp = get_slot(this)) return sp->hwnd;
    error(errors::invalid_slotid).fizzle_out();
    return {};
  }

  const auto& frame_thickness() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->frame_thickness;
  }

  int2 window_pos() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    } else return sp->pos;
  }

  int2 client_origin() const noexcept {
    if (const auto sp = get_slot(this); !sp) {
      error(errors::invalid_slotid).fizzle_out();
      return {};
    } else return sp->pos + sp->frame_thickness.xy();
  }

  int2 size() const noexcept {
    if (const auto sp = get_slot(this)) return sp->size;
    error(errors::invalid_slotid).fizzle_out();
    return {};
  }

  DWORD style() const noexcept {
    if (const auto sp = get_slot(this)) return sp->style;
    error(errors::invalid_slotid).fizzle_out();
    return {};
  }

  DWORD exstyle() const noexcept {
    if (const auto sp = get_slot(this)) return sp->exstyle;
    error(errors::invalid_slotid).fizzle_out();
    return {};
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

  bool visible() const noexcept {
    if (const auto sp = get_slot(this)) return sp->visible;
    error(errors::invalid_slotid).fizzle_out();
    return false;
  }

  bool enabled() const noexcept {
    if (const auto sp = get_slot(this)) return sp->enabled;
    error(errors::invalid_slotid).fizzle_out();
    return false;
  }

  const auto& color_theme() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->color_theme;
  }

  const auto& tooltip_background_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->tooltip_background_color;
  }

  const auto& tooltip_border_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->tooltip_border_color;
  }

  const auto& tooltip_text_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->tooltip_text_color;
  }

  const auto& tooltip_padding() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->tooltip_padding;
  }

  const auto& tooltip_radius() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->tooltip_radius;
  }

  const auto& tooltip_offset() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->tooltip_offset;
  }

  float tooltip_border_thickness() const noexcept {
    if (const auto sp = get_slot(this)) return sp->tooltip_border_thickness;
    error(errors::invalid_slotid).fizzle_out();
    return {};
  }

  const auto& tooltip_font() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->tooltip_font;
  }

  const auto& focus_overlay_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->focus_overlay_color;
  }

  const auto& hover_overlay_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->hover_overlay_color;
  }

  const auto& press_overlay_color() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->press_overlay_color;
  }

  double tooltip_delay() const noexcept {
    if (const auto sp = get_slot(this)) return sp->tooltip_delay;
    error(errors::invalid_slotid).fizzle_out();
    return {};
  }

  const auto& button_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->button_event;
  }

  const auto& drag_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->drag_event;
  }

  const auto& focus_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->focus_event;
  }

  const auto& key_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->key_event;
  }

  const auto& pointer_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->pointer_event;
  }

  const auto& wheel_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->wheel_event;
  }

  const auto& resize_event() const noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    return sp->resize_event;
  }

  //-- setter --//

  auto& window_pos(int2 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->pos = v;
    if (!::SetWindowPos(sp->hwnd, 0, v.x, v.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE))
      error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError())).go_off();
    return *this;
  }

  auto& client_origin(int2 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->pos = v - sp->frame_thickness.xy();
    if (!::SetWindowPos(sp->hwnd, 0, sp->pos.x, sp->pos.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE))
      error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError())).go_off();
    return *this;
  }

  auto& size(int2 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    sp->fit_to_necessary_size = false;
    if (auto res = sp->get_necessary_size(); !res) res.error().go_off();
    else sp->size = vapply_r<uint2>(yw::max, v, *res);
    const auto b = sp->get_bounds();
    if (!::SetWindowPos(sp->hwnd, 0, sp->pos.x, sp->pos.y, b.x, b.y, SWP_NOZORDER | SWP_NOACTIVATE))
      error(errors::operation_failed, "SetWindowPos failed", int32_t(::GetLastError())).go_off();
    sp->messy = true;
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
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->background_color = c;
    sp->dirty = true;
    return *this;
  }

  auto& visible(bool b) noexcept {
    if (b) {
      if (auto res = show(); !res) res.error().go_off();
    } else {
      if (auto res = hide(); !res) res.error().go_off();
    }
    return *this;
  }

  auto& enabled(bool b) noexcept {
    const auto sp = get_slot(this);
    if (!sp) error(errors::invalid_slotid).go_off();
    if (sp->enabled == b) return *this;
    if (!b) sp->clear_window_state();
    sp->enabled = b;
    if (!::EnableWindow(sp->hwnd, b)) {
      const auto last_error = ::GetLastError();
      if (last_error != ERROR_SUCCESS)
        error(errors::operation_failed, "EnableWindow failed", int32_t(last_error)).go_off();
    }
    sp->dirty = true;
    return *this;
  }

  auto& color_theme(const ui::color_theme& Theme) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    if (auto res = sp->apply_color_theme(Theme, true); !res) res.error().fizzle_out();
    return *this;
  }

  auto& tooltip_background_color(const color& c) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->tooltip_background_color = c;
    return *this;
  }

  auto& tooltip_border_color(const color& c) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->tooltip_border_color = c;
    return *this;
  }

  auto& tooltip_text_color(const color& c) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->tooltip_text_color = c;
    return *this;
  }

  auto& tooltip_padding(float4 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->tooltip_padding = v;
    return *this;
  }

  auto& tooltip_radius(float2 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->tooltip_radius = v;
    return *this;
  }

  auto& tooltip_offset(float2 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->tooltip_offset = v;
    return *this;
  }

  auto& tooltip_border_thickness(float1 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->tooltip_border_thickness = yw::max(0.0f, v.x);
    return *this;
  }

  auto& tooltip_font(font_config v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->tooltip_font = std::move(v);
    return *this;
  }

  auto& focus_overlay_color(const color& c) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->focus_overlay_color = c;
    return *this;
  }

  auto& hover_overlay_color(const color& c) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->hover_overlay_color = c;
    return *this;
  }

  auto& press_overlay_color(const color& c) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->press_overlay_color = c;
    return *this;
  }

  auto& tooltip_delay(double v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->tooltip_delay = yw::max(0.0, v);
    return *this;
  }

  auto& button_event(function<bool, yw::button_event> f) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->button_event = std::move(f);
    return *this;
  }

  auto& drag_event(function<bool, yw::drag_event> f) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->drag_event = std::move(f);
    return *this;
  }

  auto& focus_event(function<bool, yw::focus_event> f) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->focus_event = std::move(f);
    return *this;
  }

  auto& key_event(function<bool, yw::key_event> f) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->key_event = std::move(f);
    return *this;
  }

  auto& pointer_event(function<bool, yw::pointer_event> f) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->pointer_event = std::move(f);
    return *this;
  }

  auto& wheel_event(function<bool, yw::wheel_event> f) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->wheel_event = std::move(f);
    return *this;
  }

  auto& resize_event(function<bool, uint2> f) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->resize_event = std::move(f);
    return *this;
  }

  auto& sync_layout() noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    if (auto res = sp->update_layout(); !res) res.error().fizzle_out();
    return *this;
  }

  auto& sync_redraw() noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    if (auto res = sp->sync_redraw(); !res) res.error().fizzle_out();
    return *this;
  }
};

/// MARK: Other functions

inline std::expected<void, error> control::slot::draw_foreground() {
  if (border_color.a > 0.0f && border_thickness > 0.0f) {
    brush::color(border_color);
    if (auto res = stroke_geometry(geometry.get(), border_thickness); !res) return res.error().relay();
  }
  if (const auto wsp = get_slot<window>(window_id); !wsp) return std::unexpected(error(errors::invalid_slotid));
  else if (id == wsp->focused_control_id && wsp->focus_overlay_color.a > 0.0f) {
    brush::color(wsp->focus_overlay_color);
    if (auto res = stroke_geometry(geometry.get(), border_thickness + 1.0f); !res) return res.error().relay();
  }
  return {};
}

inline std::expected<void, error> control::slot::draw_overlay() {
  const auto wsp = get_slot<window>(window_id);
  if (!wsp) return std::unexpected(error(errors::invalid_slotid));
  if (id == wsp->mouse_capture_control_id && is_interactive() && wsp->press_overlay_color.a > 0.0f) {
    brush::color(wsp->press_overlay_color);
    if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
  } else if (id == wsp->hovered_control_id && is_interactive() && wsp->hover_overlay_color.a > 0.0f) {
    brush::color(wsp->hover_overlay_color);
    if (auto res = fill_geometry(geometry.get()); !res) return res.error().relay();
  }
  return {};
}

inline void control::slot::clear_window_state() noexcept {
  const auto wsp = get_slot<window>(window_id);
  if (!wsp) return;
  if (wsp->focused_control_id == id) {
    handle_focus_event({false});
    wsp->focused_control_id = {};
    wsp->caret_pos = std::nullopt;
  }
  if (wsp->hovered_control_id == id) wsp->hovered_control_id = {};
  if (wsp->mouse_capture_control_id == id) {
    wsp->mouse_capture_control_id = {};
    ::ReleaseCapture();
  }
  if (wsp->keyboard_capture_control_id == id) wsp->keyboard_capture_control_id = {};
  if (wsp->tooltip_control_id == id) wsp->hide_tooltip();
}

inline std::expected<const ui::color_theme*, error> control::slot::get_color_theme() const noexcept {
  if (const auto wsp = get_slot<window>(window_id)) return &wsp->color_theme;
  else return std::unexpected(error(errors::invalid_slotid));
}

inline std::expected<command_manager*, error> control::slot::get_command_manager() const noexcept {
  if (const auto wsp = get_slot<window>(window_id); !wsp) {
    error(errors::invalid_slotid).fizzle_out();
    return nullptr;
  } else return &wsp->commands;
}

inline bool control::slot::is_captured() const noexcept {
  if (const auto wsp = get_slot<window>(window_id); !wsp) {
    error(errors::invalid_slotid).fizzle_out();
    return false;
  } else return wsp->mouse_capture_control_id == id;
}

inline bool control::slot::is_focused() const noexcept {
  if (const auto wsp = get_slot<window>(window_id); !wsp) {
    error(errors::invalid_slotid).fizzle_out();
    return false;
  } else return wsp->focused_control_id == id;
}

inline bool control::slot::is_hovered() const noexcept {
  if (const auto wsp = get_slot<window>(window_id); !wsp) {
    error(errors::invalid_slotid).fizzle_out();
    return false;
  } else return wsp->hovered_control_id == id;
}

inline void control::slot::make_dirty() const noexcept {
  if (const auto wsp = get_slot<window>(window_id); wsp) wsp->dirty = true;
}

inline void control::slot::make_messy() const noexcept {
  if (const auto wsp = get_slot<window>(window_id); wsp) wsp->messy = true;
}

inline void control::slot::sync_layout() noexcept {
  const auto wsp = get_slot<window>(window_id);
  if (!wsp) return;
  if (auto res = wsp->update_layout(); !res) res.error().fizzle_out();
}

struct custom_window_options {
  string<wchar_t> title{};
  optional<int2> pos{};
  optional<int2> size{};
  DWORD style = WS_OVERLAPPEDWINDOW;
  DWORD exstyle = WS_EX_ACCEPTFILES;
  const string<wchar_t>& get_title() const noexcept { return title; }
  DWORD get_style() const noexcept { return style; }
  DWORD get_exstyle() const noexcept { return exstyle; }
};
} // namespace yw
