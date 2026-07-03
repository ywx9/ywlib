#pragma once
#include <ywx/bitmap.h>
#include <ywx/control.h>

namespace yw::ui {

class image : public control {
public:
  enum class image_size_mode : unsigned char {
    none,
    stretch
  };

  struct slot : control::slot {
    bitmap content{};
    image_size_mode content_mode = image_size_mode::none;
    bool crop_content = true;

    static std::expected<slot*, error> create(derived_from<interface> auto& Parent, const source_line& sl) {
      const auto psp = interface::slot::get<interface>(Parent.id());
      if (!psp) return std::unexpected(error(errors::invalid_slotid));
      if (!psp->attachable()) return std::unexpected(error(errors::invalid_operation, "not attachable"));
      const auto temp_id = interface::slot::add<image>();
      const auto sp = interface::slot::get<image>(temp_id);
      if (!sp) return std::unexpected(error(errors::slot_creation_failed));
      sp->id = temp_id;
      sp->source = sl;
      if (auto res = psp->attach(temp_id); !res) {
        interface::slot::slots.erase(temp_id);
        return res.error().relay();
      }
      return sp;
    }

    float2 _content_size() const noexcept { return float2(float(content.size().x), float(content.size().y)); }

    float2 _content_offset(float2 DrawSize) const noexcept {
      constexpr float c[]{0.5f, 0.0f, 1.0f};
      const float2 cc{c[unsigned(align) % 3], c[unsigned(align) / 3 % 3]};
      return (size - DrawSize) * cc;
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      return vapply_r<float2>(_necessary_size, policy, minimum_size, required_size, _content_size());
    }

    virtual std::expected<void, error> redraw() override {
      if (!visible || !content) return {};
      const auto draw_size = content_mode == image_size_mode::stretch ? size : _content_size();
      const auto draw_pos = pos + _content_offset(draw_size);
      if (crop_content) d2d::push_layer(geometry.get());
      if (auto res = draw_bitmap(draw_pos, draw_size, content); !res) {
        if (crop_content) d2d::pop_layer();
        return res.error().relay();
      }
      if (crop_content) d2d::pop_layer();
      return {};
    }
  };

  using control::operator bool;
  image() noexcept = default;

  image(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = slot::create(Parent, sl)) _id = (*res)->id;
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<image, error> create(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    image i;
    if (auto res = slot::create(Parent, sl)) {
      i._id = (*res)->id;
      return i;
    } else return res.error().relay();
  }

  const auto& content() const noexcept { ywlib_control_get(content); }
  const auto& content_mode() const noexcept { ywlib_control_get(content_mode); }
  const auto& crop_content() const noexcept { ywlib_control_get(crop_content); }

  auto& content(this auto& self, bitmap b) noexcept { ywlib_control_set(content, std::move(b), messy); }
  auto& content_mode(this auto& self, image_size_mode m) noexcept { ywlib_control_set(content_mode, m, dirty); }
  auto& crop_content(this auto& self, bool b) noexcept { ywlib_control_set(crop_content, b, dirty); }
};
} // namespace yw::ui
