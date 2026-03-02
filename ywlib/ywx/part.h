// #pragma once
// #include "ywx/core.h"

// namespace yw {

// class part {
//   slotid _id{};
//   part(const part&) = delete;
//   part& operator=(const part&) = delete;

// public:
//   class slot {
//   public:
//     slotid id{}, owner{};
//     float2 size{};
//     color bg_color = colors::white;
//     bool visible = true;

//     virtual bool focusable() const noexcept { return false; }
//     virtual bool hit_test(float2 local_pt) const noexcept {
//       return local_pt.x >= 0 && local_pt.y >= 0 && local_pt.x < size.x && local_pt.y < size.y;
//     }
//     virtual std::expected<void, error_trace> draw() { return {}; }
//   };

//   virtual ~part() noexcept = default;
//   part() noexcept = default;

//   part(part&& p) noexcept : _id(std::exchange(p._id, {})) {}
//   part& operator=(part&& p) noexcept { return _id = std::exchange(p._id, {}), *this; }

//   const slotid& id() const noexcept { return _id; }
//   const auto& size() const { return get_member(&slot::size); }
//   const auto& bg_color() const { return get_member(&slot::bg_color); }
//   const auto& visible() const { return get_member(&slot::visible); }

// protected:
//   part(slotid id) noexcept : _id(id) {}
//   slot* get_slot() const noexcept;

//   template<typename Mp> const auto& get_member(Mp mp) const {
//     if (const auto s = dynamic_cast<class_type<Mp>*>(get_slot())) return s->*mp;
//     else throw std::runtime_error("unaccessible part");
//   }

//   template<typename Mp, typename T> void set_member(Mp mp, T&& value) const noexcept {
//     if (auto s = dynamic_cast<class_type<Mp>*>(get_slot())) s->*mp = std::forward<T>(value);
//   }

//   template<typename P>
//   static std::expected<tuple<typename P::slot*, P>, error_trace> add(const slotid& Owner, float2 Size);
// };

// namespace system {
// slotlist<part::slot> parts;
// }

// inline part::slot* part::get_slot() const noexcept {
//   if (const auto s = system::parts.get(_id)) return s;
//   else return nullptr;
// }

// template<typename P>
// std::expected<tuple<typename P::slot*, P>, error_trace> part::add(const slotid& Owner, float2 Size) {
//   auto s = std::make_unique<typename P::slot>();
//   s->owner = Owner;
//   s->size = Size;
//   const auto id = system::parts.push(std::move(s));
//   if (auto ps = system::parts.get(id)) return tuple(ps, P(ps->id = id));
//   else return unexpected_error(errors::operation_failed, "failed to add part");
// }
// } // namespace yw
