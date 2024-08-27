/// \file xmatrix.hpp

#pragma once

#include "xvector.hpp"

export namespace yw {


/// extended matrix type
struct xmatrix {

  /// first row
  xvector x;

  /// second row
  xvector y;

  /// third row
  xvector z;

  /// fourth row
  xvector w;

  /// transposes the matrix
  xmatrix t() const noexcept {
    xmatrix r;
    this->transpose(r);
    return r;
  }

  /// transposes the matrix
  void transpose(xmatrix& m) const noexcept {
    auto a = xvpermute<0, 1, 4, 5>(x, y);
    m.w = xvpermute<0, 1, 4, 5>(z, w);
    m.x = xvpermute<0, 2, 4, 6>(a, m.w);
    m.y = xvpermute<1, 3, 5, 7>(a, m.w);
    a = xvpermute<2, 3, 6, 7>(x, y);
    m.w = xvpermute<2, 3, 6, 7>(z, w);
    m.z = xvpermute<0, 2, 4, 6>(a, m.w);
    m.w = xvpermute<1, 3, 5, 7>(a, m.w);
  }

  /// adds two matrices
  friend xmatrix operator+(const xmatrix& a, const xmatrix& b) noexcept {
    xmatrix r;
    a.add(b, r);
    return r;
  }

  /// adds two matrices
  void add(const xmatrix& m, xmatrix& r) const noexcept {
    r.x = xvadd(x, m.x);
    r.y = xvadd(y, m.y);
    r.z = xvadd(z, m.z);
    r.w = xvadd(w, m.w);
  }

  /// subtracts two matrices
  friend xmatrix operator-(const xmatrix& a, const xmatrix& b) noexcept {
    xmatrix r;
    a.sub(b, r);
    return r;
  }

  /// subtracts two matrices
  void sub(const xmatrix& m, xmatrix& r) const noexcept {
    r.x = xvsub(x, m.x);
    r.y = xvsub(y, m.y);
    r.z = xvsub(z, m.z);
    r.w = xvsub(w, m.w);
  }

  /// performs `matrix * scalar`
  friend xmatrix operator*(const xmatrix& a, numeric auto&& s) noexcept {
    xmatrix r;
    a.mul(s, r);
    return r;
  }

  /// performs `result = matrix * scalar`
  void mul(numeric auto&& s, xmatrix& r) const noexcept {
    r.w = xvfill(fat4(s));
    r.x = xvmul(x, r.w);
    r.y = xvmul(y, r.w);
    r.z = xvmul(z, r.w);
    r.w = xvmul(w, r.w);
  }

  /// performs `scalar * matrix`
  friend xmatrix operator*(numeric auto&& s, const xmatrix& m) noexcept {
    xmatrix r;
    m.mul(s, r);
    return r;
  }

  /// performs `matrix * vector`
  friend xvector operator*(const xmatrix& m, const xvector& v) noexcept {
    xvector r;
    m.mul(v, r);
    return r;
  }

  /// performs `result = matrix * vector`
  void mul(const xvector& v, xvector& r) const noexcept {
    r = xvdot<4, 0b0001>(x, v);
    r = xvadd(xvdot<4, 0b0010>(y, v), r);
    r = xvadd(xvdot<4, 0b0100>(z, v), r);
    r = xvadd(xvdot<4, 0b1000>(w, v), r);
  }

  /// performs `vector * matrix`
  friend xvector operator*(const xvector& v, const xmatrix& m) noexcept {
    //////////////
  }

  /// multiplies two matrices
  friend xmatrix operator*(const xmatrix& a, const xmatrix& b) noexcept {
    xmatrix r;
    a.mul(b, r);
    return r;
  }

  /// multiplies two matrices (NOT hadamard product)
  void mul(const xmatrix& m, xmatrix& r) const noexcept {

  }
};


}
