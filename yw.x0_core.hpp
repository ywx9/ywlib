#pragma once
namespace yw {

  using nullptr_t = decltype(nullptr);
  using cat1 = char;
  using cat2 = wchar_t;
  using uct1 = char8_t;
  using uct2 = char16_t;
  using uct4 = char32_t;
  using int1 = signed char;
  using int2 = signed short;
  using int4 = signed int;
  using int8 = signed long long;
  using nat1 = unsigned char;
  using nat2 = unsigned short;
  using nat4 = unsigned int;
  using nat8 = unsigned long long;
  using fat4 = float;
  using fat8 = double;

  namespace _zyx {
    template<typename T> struct _type_identity { using t = T; };
    template<bool, typename> struct _type_breaker {};
    template<typename T> struct _type_breaker<true, T> { using t = T; };
    template<bool, typename, typename T2> struct _type_switch { using t = T2; };
    template<typename T1, typename T2> struct _type_switch<true, T1, T2> { using t = T1; };
  }
  template<typename...> using void_t = void;
  template<typename T> using type_identity = _zyx::_type_identity<T>::t;
  template<bool B, typename T> using type_breaker = _zyx::_type_breaker<B, T>::t;
  template<bool B, typename T1, typename T2> using type_switch = _zyx::_type_switch<B, T1, T2>::t;

  using catt = cat2;
  using intt = type_switch<sizeof nullptr == 4, int4, int8>;
  using natt = type_switch<sizeof nullptr == 4, nat4, nat8>;
  using fatt = type_switch<sizeof nullptr == 4, fat4, fat8>;

  template<typename T, T Val> struct agent_constant {
    static constexpr T value = Val;
    constexpr operator T (void)noexcept { return value; }
    [[nodiscard]] constexpr T operator()(void)const noexcept { return value; }
  };

  namespace _zyx {
    template<natt N> struct _xxx_type {};
    template<> struct _xxx_type<1> { using tc = cat1; using tu = uct1; using ti = int1; using tn = nat1; };
    template<> struct _xxx_type<2> { using tc = cat2; using tu = uct2; using ti = int2; using tn = nat2; };
    template<> struct _xxx_type<4> { using tu = uct4; using ti = int4; using tn = nat4; using tf = fat4; };
    template<> struct _xxx_type<8> { using ti = int8; using tn = nat8; using tf = fat8; };
  }
  namespace type {
    template<natt N> requires requires { typename _zyx::_xxx_type<N>::tc; } using cat_type = _zyx::_xxx_type<N>::tc;
    template<natt N> requires requires { typename _zyx::_xxx_type<N>::tu; } using uct_type = _zyx::_xxx_type<N>::tu;
    template<natt N> requires requires { typename _zyx::_xxx_type<N>::ti; } using int_type = _zyx::_xxx_type<N>::ti;
    template<natt N> requires requires { typename _zyx::_xxx_type<N>::tn; } using nat_type = _zyx::_xxx_type<N>::tn;
    template<natt N> requires requires { typename _zyx::_xxx_type<N>::tf; } using fat_type = _zyx::_xxx_type<N>::tf;
  }
  namespace _zyx {
    template<typename T> struct _is_cv { using tc = T; using tv = T; using tcv = T; static constexpr bool vc = false, vv = false; };
    template<typename T> struct _is_cv<const T> { using tc = T; using tv = const T; using tcv = T; static constexpr bool vc = true, vv = false; };
    template<typename T> struct _is_cv<volatile T> { using tc = volatile T; using tv = T; using tcv = T; static constexpr bool vc = false, vv = true; };
    template<typename T> struct _is_cv<const volatile T> { using tc = volatile T; using tv = const T; using tcv = T; static constexpr bool vc = true, vv = true; };
    template<typename T> struct _is_ref { using t = T; static constexpr bool vl = false, vr = false; };
    template<typename T> struct _is_ref<T&> { using t = T; static constexpr bool vl = true, vr = false; };
    template<typename T> struct _is_ref<T&&> { using t = T; static constexpr bool vl = false, vr = true; };
    template<typename T> struct _is_ptr { using t = T; static constexpr bool v = false, vc = false, vv = false; };
    template<typename T> struct _is_ptr<T*> { using t = T; static constexpr bool v = true, vc = false, vv = false; };
    template<typename T> struct _is_ptr<T* const> { using t = T; static constexpr bool v = true, vc = true, vv = false; };
    template<typename T> struct _is_ptr<T* volatile> { using t = T; static constexpr bool v = true, vc = false, vv = true; };
    template<typename T> struct _is_ptr<T* const volatile> { using t = T; static constexpr bool v = true, vc = true, vv = true; };
    template<typename T> struct _is_array { using t = T; using ta = T; static constexpr bool vb = false, vu = false; };
    template<typename T> struct _is_array<T[]> { using t = T; using ta = _is_array<T>::ta; static constexpr bool vb = false, vu = true; };
    template<typename T, natt N> struct _is_array<T[N]> { using t = T; using ta = _is_array<T>::ta; static constexpr bool vb = true, vu = false };
  }
  namespace type {
    template<typename T> using rem_const = _zyx::_is_cv<T>::tc;
    template<typename T> using rem_volat = _zyx::_is_cv<T>::tv;
    template<typename T> using rem_cv = _zyx::_is_cv<T>::tcv;
    template<typename T> using rem_ref = _zyx::_is_ref<T>::t;
    template<typename T> using rem_ptr = _zyx::_is_ptr<T>::t;
    template<typename T> using rem_extent = _zyx::_is_array<T>::t;
    template<typename T> using rem_extent_all = _zyx::_is_array<T>::ta;

    template<typename T> concept is_const = _zyx::_is_cv<T>::vc;
    template<typename T> concept is_volat = _zyx::_is_cv<T>::vv;
    template<typename T> concept is_cv = is_const<T> && is_volat<T>;
    template<typename T> concept is_ref_lv = _zyx::_is_ref<T>::vl;
    template<typename T> concept is_ref_rv = _zyx::_is_ref<T>::vr;
    template<typename T> concept is_ref = is_ref_lv<T> || is_ref_rv<T>;
    template<typename T> concept is_ptr = _zyx::_is_ptr<T>::v;
    template<typename T> concept is_ptr_const = is_ptr<T> && _zyx::_is_ptr<T>::vc;
    template<typename T> concept is_ptr_volat = is_ptr<T> && _zyx::_is_ptr<T>::vv;
    template<typename T> concept is_ptr_cv = is_ptr_const<T> && is_ptr_volat<T>;
    template<typename T> concept is_array_bounded = _zyx::_is_array<T>::vb;
    template<typename T> concept is_array_unbounded = _zyx::_is_array<T>::vu;
    template<typename T> concept is_array = is_array_bounded<T> || is_array_unbounded<T>;
    template<typename T> concept is_function = !is_const<const T> && !is_ref<T>;
  }
  namespace _zyx {
    template<typename, typename> inline constexpr bool is_same = false;
    template<typename T> inline constexpr bool is_same<T, T> = true;
    template<typename T, typename U> concept half_same_as = is_same<T, U>;
  }
  namespace type {
    template<typename T, typename U> concept same_as = _zyx::half_same_as<T, U> && _zyx::half_same_as<U, T>;
    template<typename T, typename...Us> concept included_in = (same_as<T, Us> || ...);
    template<typename T> concept is_nullptr = same_as<rem_cv<T>, nullptr_t>;
    template<typename T> concept is_void = same_as<rem_cv<T>, void>;
    template<typename T> concept is_bool = same_as<rem_cv<T>, bool>;
    template<typename T> concept is_cat = included_in<rem_cv<T>, cat1, cat2>;
    template<typename T> concept is_uct = included_in<rem_cv<T>, uct1, uct2, uct4>;
    template<typename T> concept is_int = included_in<rem_cv<T>, int1, int2, int4, int8, signed long>;
    template<typename T> concept is_nat = included_in<rem_cv<T>, nat1, nat2, nat4, nat8, unsigned long>;
    template<typename T> concept is_fat = included_in<rem_cv<T>, fat4, fat8, long double>;
    template<typename T> concept character = is_cat<T> || is_uct<T>;
    template<typename T> concept countable = is_int<T> || is_nat<T>;
    template<typename T> concept quantable = is_fat<T> || countable<T>;
    template<typename T> concept integral = is_bool<T> || character<T> || countable<T>;
    template<typename T> concept arithmetic = integral<T> || is_fat<T>;
    template<typename T> concept fundamental = is_nullptr<T> || is_void<T> || arithmetic<T>;
  }

  using ::yw::type::character;
  using ::yw::type::countable;
  using ::yw::type::quantable;

  enum class byte : nat1 {};
  constexpr byte operator"" ywb(nat8 val)noexcept { return static_cast<byte>(val & 0xFF); }
  constexpr byte operator<<(byte a, countable auto b)noexcept { return static_cast<byte>((static_cast<natt>(a) << b) & 0xFF); }
  constexpr byte operator>>(byte a, countable auto b)noexcept { return static_cast<byte>((static_cast<natt>(a) >> b) & 0xFF); }
  constexpr byte& operator<<=(byte& a, countable auto b)noexcept { return a = (a << b); }
  constexpr byte& operator>>=(byte& a, countable auto b)noexcept { return a = (a >> b); }
  constexpr byte operator&(byte a, byte b)noexcept { return static_cast<byte>(static_cast<nat1>(a) & static_cast<nat1>(b)); }
  constexpr byte operator|(byte a, byte b)noexcept { return static_cast<byte>(static_cast<nat1>(a) | static_cast<nat1>(b)); }
  constexpr byte operator^(byte a, byte b)noexcept { return static_cast<byte>(static_cast<nat1>(a) ^ static_cast<nat1>(b)); }
  constexpr byte& operator&=(byte& a, byte b)noexcept { return a = (a & b); }
  constexpr byte& operator|=(byte& a, byte b)noexcept { return a = (a | b); }
  constexpr byte& operator^=(byte& a, byte b)noexcept { return a = (a ^ b); }
  constexpr byte operator~(byte a)noexcept { return static_cast<byte>(~static_cast<nat1>(a)); }
  constexpr byte operator+(byte a, byte b)noexcept { return static_cast<byte>((static_cast<natt>(a) + static_cast<natt>(b)) & 0xFF); }
  constexpr byte operator-(byte a, byte b)noexcept { return static_cast<byte>((static_cast<natt>(a) - static_cast<natt>(b)) & 0xFF); }
  constexpr byte operator*(byte a, quantable auto b)noexcept { return static_cast<byte>(static_cast<natt>(static_cast<intt>(static_cast<natt>(a) * b)) & 0xFF); }
  constexpr byte operator/(byte a, quantable auto b)noexcept { return static_cast<byte>(static_cast<natt>(static_cast<intt>(static_cast<fat4>(static_cast<natt>(a)) / b)) & 0xFF); }
  constexpr byte operator+=(byte& a, byte b)noexcept { return a = (a + b); }
  constexpr byte operator-=(byte& a, byte b)noexcept { return a = (a - b); }
  constexpr byte operator*=(byte& a, quantable auto b)noexcept { return a = (a * b); }
  constexpr byte operator/=(byte& a, quantable auto b)noexcept { return a = (a / b); }

  namespace _zyx {
    template<typename T> struct _add_cv { using tc = const T; using tv = volatile T; using tcv = const volatile T; };
    template<typename T, typename = void> struct _add_ref { using tl = T; using tr = T; };
    template<typename T> struct _add_ref<T, void_t<T&>> { using tl = T&; using tr = T&&; };
    template<typename T, typename = void> struct _add_ptr { using t = T; using tc = T; using tv = T; using tcv = T; };
    template<typename T> struct _add_ptr<T, void_t<T*>> { using t = T*; using tc = T* const; using tv = T* volatile; using tcv = T* const volatile; };
    template<typename T, natt N, typename = void> struct _add_ext { using t = T; };
    template<typename T, natt N> struct _add_ext<T, N, void_t<T[N]>> { using t = T[N]; };
    template<typename T> struct _add_ext<T, 0, void_t<T[]>> { using t = T[]; };
  }
  namespace type {
    template<typename T> using add_const = _zyx::_add_cv<T>::tc;
    template<typename T> using add_volat = _zyx::_add_cv<T>::tv;
    template<typename T> using add_cv = _zyx::_add_cv<T>::tcv;
    template<typename T> using add_ref_lv = _zyx::_add_ref<T>::tl;
    template<typename T> using add_ref_rv = _zyx::_add_ref<T>::tr;
    template<typename T> using add_ptr = _zyx::_add_ptr<T>::t;
    template<typename T> using add_ptr_const = _zyx::_add_ptr<T>::tc;
    template<typename T> using add_ptr_volat = _zyx::_add_ptr<T>::tv;
    template<typename T> using add_ptr_cv = _zyx::_add_ptr<T>::tcv;
    template<typename T, natt N> using add_extent = _zyx::_add_ext<T, N>::t;
  }

  [[nodiscard]] constexpr bool is_constant_evaluated(void)noexcept { return __builtin_is_constant_evaluated(); }
  template<typename T> [[noreturn]] type::add_ref_rv<T> declval(void)noexcept { static_assert(false); }
  template<typename T> [[nodiscard]] constexpr T* addressof(T& arg)noexcept { return __builtin_addressof(arg); }
  template<typename T> const T* addressof(const T&&) = delete;
  template<typename T> [[nodiscard]] constexpr type::rem_ref<T>&& move(T&& arg)noexcept { return static_cast<type::rem_ref<T>&&>(arg); }
  template<typename T> [[nodiscard]] constexpr T&& forward(type::rem_ref<T>& arg)noexcept { return static_cast<T&&>(arg); }
  template<typename T> [[nodiscard]] constexpr T&& forward(type::rem_ref<T>&& arg)noexcept requires (!type::is_ref_lv<T>) { return static_cast<T&&>(arg); }

}
