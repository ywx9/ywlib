#pragma once
#include <yw/vector.h>

namespace yw::phy {

template<typename T> struct circle {
  vector2<T> center{};
  T radius{};
};

template<typename T> struct aabb {
  vector2<T> min{};
  vector2<T> max{};

  constexpr vector2<T> size() const noexcept { return max - min; }
  constexpr vector2<T> center() const noexcept { return (min + max) * T(0.5); }
};

template<typename T> struct circle_body {
  vector2<T> position{};
  vector2<T> velocity{};
  T radius{};
  T mass{1};
};

template<typename T> struct contact2 {
  vector2<T> normal{T(1), T(0)};
  T penetration{};
};

template<typename T> constexpr T cross(vector2<T> a, vector2<T> b) noexcept { return a.x * b.y - a.y * b.x; }

template<typename T> constexpr vector2<T> perp(vector2<T> v) noexcept { return {-v.y, v.x}; }

template<typename T> constexpr vector2<T> project(vector2<T> v, vector2<T> onto) noexcept {
  const auto len2 = onto.squared_length();
  if (len2 <= T(0)) return {};
  return onto * (dot(v, onto) / len2);
}

template<typename T> constexpr vector2<T> reflect(vector2<T> v, vector2<T> normal, T restitution = T(1)) noexcept {
  return v - normal * ((T(1) + restitution) * dot(v, normal));
}

template<typename T> constexpr vector2<T> closest_point(vector2<T> point, const aabb<T>& box) noexcept {
  return {
    yw::clamp(point.x, box.min.x, box.max.x),
    yw::clamp(point.y, box.min.y, box.max.y),
  };
}

template<typename T> constexpr bool overlaps(const circle<T>& a, const circle<T>& b) noexcept {
  const auto r = a.radius + b.radius;
  return (a.center - b.center).squared_length() <= r * r;
}

template<typename T> constexpr bool overlaps(const circle<T>& c, const aabb<T>& box) noexcept {
  const auto p = closest_point(c.center, box);
  return (c.center - p).squared_length() <= c.radius * c.radius;
}

template<typename T> constexpr vector2<T> integrate(vector2<T> position, vector2<T> velocity, T dt) noexcept {
  return position + velocity * dt;
}

template<typename T> constexpr vector2<T> damp(vector2<T> velocity, T factor_per_second, T dt) noexcept {
  return velocity * yw::pow(factor_per_second, dt);
}

template<typename T> constexpr contact2<T> circle_contact(const circle<T>& a, const circle<T>& b) noexcept {
  const auto delta = b.center - a.center;
  const auto distance2 = delta.squared_length();
  const auto radius = a.radius + b.radius;
  if (distance2 <= T(0)) return {{T(1), T(0)}, radius};
  const auto distance = yw::sqrt(distance2);
  return {delta / distance, radius - distance};
}

template<typename T> constexpr void resolve_collision(circle_body<T>& a, circle_body<T>& b, T restitution = T(1)) noexcept {
  const auto contact = circle_contact(circle<T>{a.position, a.radius}, circle<T>{b.position, b.radius});
  if (contact.penetration <= T(0)) return;

  const auto inv_mass_a = a.mass > T(0) ? T(1) / a.mass : T(0);
  const auto inv_mass_b = b.mass > T(0) ? T(1) / b.mass : T(0);
  const auto inv_mass_sum = inv_mass_a + inv_mass_b;
  if (inv_mass_sum <= T(0)) return;

  const auto correction = contact.normal * (contact.penetration / inv_mass_sum);
  a.position -= correction * inv_mass_a;
  b.position += correction * inv_mass_b;

  const auto relative_velocity = b.velocity - a.velocity;
  const auto separating_speed = dot(relative_velocity, contact.normal);
  if (separating_speed >= T(0)) return;

  const auto impulse = contact.normal * (-(T(1) + restitution) * separating_speed / inv_mass_sum);
  a.velocity -= impulse * inv_mass_a;
  b.velocity += impulse * inv_mass_b;
}

template<typename T> constexpr void resolve_bounds(circle_body<T>& body, const aabb<T>& bounds, T restitution = T(1)) noexcept {
  if (body.position.x - body.radius < bounds.min.x) {
    body.position.x = bounds.min.x + body.radius;
    if (body.velocity.x < T(0)) body.velocity.x = -body.velocity.x * restitution;
  } else if (body.position.x + body.radius > bounds.max.x) {
    body.position.x = bounds.max.x - body.radius;
    if (body.velocity.x > T(0)) body.velocity.x = -body.velocity.x * restitution;
  }

  if (body.position.y - body.radius < bounds.min.y) {
    body.position.y = bounds.min.y + body.radius;
    if (body.velocity.y < T(0)) body.velocity.y = -body.velocity.y * restitution;
  } else if (body.position.y + body.radius > bounds.max.y) {
    body.position.y = bounds.max.y - body.radius;
    if (body.velocity.y > T(0)) body.velocity.y = -body.velocity.y * restitution;
  }
}

} // namespace yw::phy
