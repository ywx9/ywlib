// #pragma once
// #include "ywx/bitmap.h"
// #include "ywx/svgpath.h"

// namespace yw::ui {

// //////////////////////////////////////// MARK: ui::icon

// class icon : public base {
// public:
//   using icon_variant = std::variant<std::monostate, yw::bitmap, svgpath>;

//   class slot : public base::slot {
//   public:
//     icon_variant image{};
//     color image_color = colors::black;
//     float2 padding{};

//     virtual void draw() const override {
//       // Draw background and border
//       base::slot::draw();

//       // Draw icon
//       if (std::holds_alternative<std::monostate>(image)) {
//         // Empty icon
//       } else if (const auto* bitmap_ptr = std::get_if<yw::bitmap>(&image)) {
//         if (*bitmap_ptr) {
//           draw_bitmap(pos + padding, size - padding * 2.0f, *bitmap_ptr);
//         }
//       } else if (const auto* svgpath_ptr = std::get_if<svgpath>(&image)) {
//         if (*svgpath_ptr) {
//           draw_svgpath(pos + padding, size - padding * 2.0f, *svgpath_ptr, image_color);
//         }
//       }
//     }

//     virtual std::expected<void, error_trace> proc(UINT, WPARAM, LPARAM) override { return {}; }
//   };

// protected:
//   slot* _icon_slot() const noexcept { return dynamic_cast<slot*>(_ui_slot()); }

//   template<typename Mp, typename T> void _icon_set(Mp mp, T&& value) {
//     if (const auto s = _icon_slot()) {
//       s->*mp = static_cast<T&&>(value);
//       if (const auto w = system::windows.get(s->window_id)) w->dirty = true;
//     }
//   }

//   template<typename Mp> const auto* _icon_get(Mp mp) const {
//     if (const auto s = _icon_slot()) return &(s->*mp);
//     else return static_cast<const remove_cvref<decltype(s->*mp)>*>(nullptr);
//   }

// public:
//   using base::base;

//   const auto& image() const { return *_icon_get(&slot::image); }
//   const auto& image_color() const { return *_icon_get(&slot::image_color); }
//   const auto& padding() const { return *_icon_get(&slot::padding); }

//   void image(icon_variant value) { _icon_set(&slot::image, std::move(value)); }
//   void image_color(const color& value) { _icon_set(&slot::image_color, value); }
//   void padding(float2 value) { _icon_set(&slot::padding, value); }

//   static std::expected<icon, error_trace> add(window& w, float2 Pos, float2 Size) {
//     if (auto res = d2d.initialize(); !res) return unexpected_error(res.error());
//     return _add<icon>(w, Pos, Size);
//   }
// };

// } // namespace yw::ui
