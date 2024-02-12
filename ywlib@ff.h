#pragma once

#include "ywlib@core.h"
#include "ywlib@xv.h"

namespace yw::ff {

// stl

class stl {
  array<nat1> _{};
public:
#pragma pack(2)
  struct facet {
    array<fat4, 3> normal;
    array<fat4, 3> vertex[3];
    nat2 attribute;
  };
#pragma pack()
  stl() noexcept = default;
  stl(natt Facets) : _(80 + 4 + 50 * Facets) { *(nat4*)(_.data() + 80) = nat4(Facets); }
  stl(const path& File) : _() { file::read(File, _); }
  nat4 size() const noexcept { return _.empty() ? nat4{} : *(nat4*)(_.data() + 80); }
  bool empty() const noexcept { return _.empty(); }
  bool valid() const noexcept { return _.size() >= 84 && (_.size() >= 84 + (*(nat4*)(_.data() + 80) * 50)); }
  stv1 header() const noexcept { return valid() ? stv1((cat1*)(_.data())) : stv1(); }
  void header(cnt_range_of<cat1> auto&& Text) { valid() ? void(std::ranges::copy_n(Text, _.data(), min(80, yw::size(Text)))) : void(); }
  facet* data() { return valid() ? (facet*)(_.data() + 84) : nullptr; }
  const facet* data() const { return valid() ? (facet*)(_.data() + 84) : nullptr; }
  facet* begin() { return valid() ? (facet*)(_.data() + 84) : nullptr; }
  const facet* begin() const { return valid() ? (facet*)(_.data() + 84) : nullptr; }
  facet* end() { return valid() ? (facet*)(_.data() + 84 + (*(nat4*)(_.data() + 80) * 50)) : nullptr; }
  const facet* end() const { return valid() ? (facet*)(_.data() + 84 + (*(nat4*)(_.data() + 80) * 50)) : nullptr; }
  facet& operator[](natt i) { return *(data() + i); }
  const facet& operator[](natt i) const { return *(data() + i); }
  void write(const path& File) const { valid() ? void(file::write(File, _)) : void(); }
};

// csv

class csv {
  array<array<str1>> _{};
public:
  csv() noexcept = default;
  csv(natt Rows, natt Columns) : _(Rows) {
    auto a = str1("");
    for (auto& e : _) e.resize(Columns, a);
  }
  csv(const path& File, cat1 Delimiter = ',') {
    std::ifstream ifs(File);
    natt m{}, n{};
    for (std::string line, cell; std::getline(ifs, line);) {
      _.emplace_back(array<str1>{});
      for (std::stringstream ss(line); std::getline(ss, cell, Delimiter); ++m) _.back().emplace_back(cell);
      n = max(n, exchange(m, 0));
    }
    for (auto& r : _) r.resize(n, "");
  }
  natt rows() const { return _.size(); }
  natt columns() const { return _.empty() ? natt{} : _.front().size(); }
  str1& operator()(natt Row, natt Column) { return _[Row][Column]; }
  const str1& operator()(natt Row, natt Column) const { return _[Row][Column]; }
  csv& erase_row(natt i) { return _.erase(_.begin() + i), *this; }
  csv& erase_column(natt i) {
    return std::ranges::for_each(_, [&](auto& a) { a.erase(a.begin() + i); }), *this;
  }
  csv& insert_row(natt i) { return _.insert(_.begin() + i, array<str1>{}), *this; }
  csv& insert_column(natt i) {
    return std::ranges::for_each(_, [&](auto& a) { a.insert(a.begin() + i, str1{}); }), *this;
  }
};
}
