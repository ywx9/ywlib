#pragma once
#include "ywx/bitmap.h"
#include "ywx/svgpath.h"

namespace yw {

class icon {
  std::variant<std::monostate, bitmap, svgpath> _icon;

public:
  icon() noexcept = default;
  icon(bitmap Icon) : _icon(std::move(Icon)) {}
  icon(svgpath Icon) : _icon(std::move(Icon)) {}

  explicit operator bool() const noexcept {
    return std::visit(
      []<typename T>(T&& a) -> bool {
        if constexpr (same_as<remove_cvref<T>, std::monostate>) return false;
        else return static_cast<bool>(a);
      },
      _icon);
  }

  float2 size() const {
    if (std::holds_alternative<bitmap>(_icon)) return std::get<bitmap>(_icon).size();
    else if (std::holds_alternative<svgpath>(_icon)) return std::get<svgpath>(_icon).size();
    else return {0, 0};
  }

  bool is_bitmap() const { return std::holds_alternative<bitmap>(_icon); }
  bool is_svgpath() const { return std::holds_alternative<svgpath>(_icon); }

  const bitmap& get_bitmap() const { return std::get<bitmap>(_icon); }
  const svgpath& get_svgpath() const { return std::get<svgpath>(_icon); }
};

//////////////////////////////////////// MARK: draw/stroke_icon

/// アイコンを描画する。SVGパス保持なら塗潰しを行う。
inline std::expected<void, error_trace> draw_icon(float2 Pos, float2 Size, const icon& Icon) {
  if (Icon.is_bitmap()) {
    if (auto res = draw_bitmap(Pos, Size, Icon.get_bitmap()); !res) return unexpected_error(res.error());
  } else if (Icon.is_svgpath()) {
    if (auto res = fill_svgpath(Pos, Size, Icon.get_svgpath()); !res) return unexpected_error(res.error());
  }
  return {};
}

/// アイコンを描画する。SVGパス保持なら塗潰しを行う。
inline std::expected<void, error_trace> draw_icon(float2 Pos, float1 Scale, const icon& Icon) {
  const auto sz = Icon.size() * Scale.x;
  if (auto res = draw_icon(Pos, sz, Icon)) return {};
  else return unexpected_error(res.error());
}

/// アイコンを描画する。SVGパス保持なら塗潰しを行う。
inline std::expected<void, error_trace> draw_icon(float2 Pos, const icon& Icon) {
  if (Icon.is_bitmap()) {
    if (auto res = draw_bitmap(Pos, Icon.get_bitmap()); !res) return unexpected_error(res.error());
  } else if (Icon.is_svgpath()) {
    if (auto res = fill_svgpath(Pos, Icon.get_svgpath()); !res) return unexpected_error(res.error());
  }
  return {};
}

/// アイコンの輪郭を描画する。ビットマップ保持なら矩形を描く。
inline std::expected<void, error_trace> stroke_icon(float2 Pos, float2 Size, const icon& Icon, float1 width = 1.0f) {
  if (Icon.is_bitmap()) {
    if (auto res = draw_rectangle(Pos, Size, width); !res) return unexpected_error(res.error());
  } else if (Icon.is_svgpath()) {
    if (auto res = stroke_svgpath(Pos, Size, Icon.get_svgpath(), width); !res) return unexpected_error(res.error());
  }
  return {};
}

/// アイコンの輪郭を描画する。ビットマップ保持なら矩形を描く。
inline std::expected<void, error_trace> stroke_icon(float2 Pos, float1 Scale, const icon& Icon, float1 width = 1.0f) {
  const auto sz = Icon.size() * Scale.x;
  if (auto res = stroke_icon(Pos, sz, Icon, width)) return {};
  else return unexpected_error(res.error());
}

/// アイコンの輪郭を描画する。ビットマップ保持なら矩形を描く。
inline std::expected<void, error_trace> stroke_icon(float2 Pos, const icon& Icon, float1 width = 1.0f) {
  if (Icon.is_bitmap()) {
    if (auto res = draw_rectangle(Pos, Icon.get_bitmap().size(), width); !res) return unexpected_error(res.error());
  } else if (Icon.is_svgpath()) {
    if (auto res = stroke_svgpath(Pos, Icon.get_svgpath(), width); !res) return unexpected_error(res.error());
  }
  return {};
}
}; // namespace yw
