#pragma once
#include <variant>

#include <ywx/bitmap.h>
#include <ywx/control.h>

namespace yw::ui {

class image : public control {
public:
  struct scaled_content {
    float2 value = float2::fill(1.0f);
  };

  struct sized_content {
    float2 value{};
  };

  struct slot : control::slot {
    bitmap content{};
    std::variant<scaled_content, sized_content> content_transform = scaled_content{};

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

    float2 _original_content_size() const noexcept {
      if (!content) return {};
      return float2(float(content.size().x), float(content.size().y));
    }

    float2 _content_scale() const noexcept {
      if (!content) return float2::fill(1.0f);
      if (const auto* scaled = std::get_if<scaled_content>(&content_transform)) return scaled->value;
      const auto original = _original_content_size();
      if (original.x <= 0.0f || original.y <= 0.0f) return float2::fill(1.0f);
      return std::get<sized_content>(content_transform).value / original;
    }

    float2 _content_size() const noexcept {
      if (!content) return {};
      if (const auto* sized = std::get_if<sized_content>(&content_transform)) return sized->value;
      return _original_content_size() * std::get<scaled_content>(content_transform).value;
    }

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
      const auto draw_size = _content_size();
      const auto draw_pos = pos + _content_offset(draw_size);
      d2d::push_layer(geometry.get());
      if (auto res = draw_bitmap(draw_pos, draw_size, content); !res) {
        d2d::pop_layer();
        return res.error().relay();
      }
      d2d::pop_layer();
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
  float2 content_size() const noexcept {
    const auto s = get_slot(this);
    if (!s) error(errors::invalid_slotid).go_off();
    return s->_content_size();
  }
  float2 content_scale() const noexcept {
    const auto s = get_slot(this);
    if (!s) error(errors::invalid_slotid).go_off();
    return s->_content_scale();
  }

  auto& content(this auto& self, bitmap b) noexcept { ywlib_control_set(content, std::move(b), messy); }
  auto& content_size(this auto& self, float2 v) noexcept {
    const auto s = get_slot(&self);
    if (!s) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f || v.y <= 0.0f) {
      error(errors::invalid_argument, format("content_size must be positive: ", v)).go_off();
      return self;
    }
    s->content_transform = sized_content{v};
    if (auto res = s->make_messy(); !res) res.error().go_off();
    return self;
  }
  auto& content_scale(this auto& self, float2 v) noexcept {
    const auto s = get_slot(&self);
    if (!s) error(errors::invalid_slotid).go_off();
    if (v.x <= 0.0f || v.y <= 0.0f) {
      error(errors::invalid_argument, format("content_scale must be positive: ", v)).go_off();
      return self;
    }
    s->content_transform = scaled_content{v};
    if (auto res = s->make_messy(); !res) res.error().go_off();
    return self;
  }
  auto& content_scale(this auto& self, arithmetic auto v) noexcept { return self.content_scale(float2::fill(float(v))); }
};
} // namespace yw::ui
