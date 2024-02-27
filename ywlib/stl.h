/// @file ywlib/stl.h

#pragma once

#include "core.h"

namespace yw {
/// namespace for file formats
namespace ff {
/// class for handling .stl files
class stl;
}
}

class yw::ff::stl {
protected:
  array<cat1> _{};
public:
#pragma pack(2)
  /// representing a normal vector, three vertices, and an attribute of each triangle
  struct facet {
    array<fat4, 3> normal;
    array<array<fat4, 3>, 3> vertex;
    nat2 attribute;
  };
#pragma pack()

  /// default constructor
  stl() = default;

  /// constructor from .stl file
  stl(const path& p) : _(p.file_size()) { p.read(_); }

  /// constructs a stl data with n facets
  stl(natt n) : _(84 + 50 * n) { *reinterpret_cast<nat4*>(std::memset(_.data(), ' ', 80)) = nat4(n); }

  /// returns the number of facets
  nat4 size() const { return *reinterpret_cast<const nat4*>(_.data() + 80); }

  /// checks if the data is empty
  bool empty() const { return size() == 0 || _.size() != 84 + 50 * size(); }

  /// returns the string view of the file header
  stv1 header() const { return {_.data(), 80}; }

  /// sets the file header
  void header(const stv1& s) { std::char_traits<cat1>::copy(_.data(), s.data(), min(s.size(), 80)); }

  /// returns the pointer to the first facet
  facet* data() { return reinterpret_cast<facet*>(_.data() + 84); }
  const facet* data() const { return reinterpret_cast<const facet*>(_.data() + 84); }

  /// returns the iterator to the first facet
  facet* begin() { return data(); }
  const facet* begin() const { return data(); }

  /// returns the iterator to the end of the facets
  facet* end() { return data() + size(); }
  const facet* end() const { return data() + size(); }

  /// returns the reference to the i-th facet
  facet& operator[](natt i) { return data()[i]; }
  const facet& operator[](natt i) const { return data()[i]; }

  /// writes the data to a file
  void write(const path& p) const { p.write(_); }
};
