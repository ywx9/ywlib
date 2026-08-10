#pragma once
#include <ywx/control.h>

namespace yw::ui {

class grid_layout : public control {
  template<typename T> static constexpr bool is_grid_track_spec =
    convertible_to<T, ui::size_policy> || arithmetic<remove_ref<T>>;

public:
  struct slot : public control::slot {
    struct track {
      ui::size_policy policy = ui::fit;
      float size = 0.0f;
    };

    struct placement {
      uint32_t row = 0;
      uint32_t column = 0;
      uint32_t columns = 1;
      uint32_t rows = 1;
    };

    struct item {
      slotid control_id{};
      placement place{};
    };

    struct pending_failure {
      error::kind kind = errors::invalid_operation;
      string<char> message{};
    };

    static constexpr size_t empty_cell = 0;

    std::vector<track> columns{{ui::fit, 0.0f}};
    std::vector<track> rows{{ui::fit, 0.0f}};
    std::vector<size_t> occupancy{empty_cell};
    std::vector<item> items{};
    std::optional<placement> pending{};
    std::optional<pending_failure> pending_error{};
    float column_gap = 0.0f;
    float row_gap = 0.0f;

    //-- override functions --//

    virtual std::expected<void, error> apply_color_theme(const yw::ui::color_theme& Theme, bool Recursive) override {
      background_color = colors::transparent;
      border_color = colors::transparent;
      if (Recursive)
        for (const auto& it : items)
          if (const auto csp = get_slot<control>(it.control_id)) {
            if (auto res = csp->apply_color_theme(Theme, true); !res) return res.error().relay();
          } else return std::unexpected(error(errors::invalid_slotid));
      make_dirty();
      return {};
    }

    virtual bool attachable() const override { return true; }

    virtual std::expected<void, error> attach(slotid Child) override {
      if (pending_error) {
        auto failure = std::move(*pending_error);
        pending_error = std::nullopt;
        pending = std::nullopt;
        return std::unexpected(error(failure.kind, std::move(failure.message)));
      }
      if (!pending) return std::unexpected(error(errors::invalid_operation, "grid cell is not reserved"));
      if (!placement_in_range(*pending)) {
        pending = std::nullopt;
        return std::unexpected(error(errors::invalid_argument, "grid cell is out of range"));
      }
      if (!placement_available(*pending)) {
        pending = std::nullopt;
        return std::unexpected(error(errors::invalid_operation, "grid cell is already occupied"));
      }

      const auto csp = get_slot<control>(Child);
      if (!csp) {
        pending = std::nullopt;
        return std::unexpected(error(errors::invalid_slotid));
      }

      csp->window_id = window_id;
      items.push_back({Child, *pending});
      mark_placement(*pending, items.size());
      pending = std::nullopt;
      make_messy();
      return {};
    }

    virtual void close_child_controls() override {
      for (const auto& it : items)
        if (const auto csp = get_slot<control>(it.control_id)) {
          csp->close_child_controls();
          interface::slot::slots.erase(it.control_id);
        }
      items.clear();
      std::ranges::fill(occupancy, empty_cell);
      pending = std::nullopt;
      pending_error = std::nullopt;
    }

    virtual std::expected<void, error> detach(slotid Child) override {
      if (const auto csp = get_slot<control>(Child)) csp->clear_window_state();
      const auto old_size = items.size();
      items.erase(
        std::remove_if(items.begin(), items.end(), [&](const item& it) { return it.control_id == Child; }),
        items.end());
      if (old_size == items.size())
        return std::unexpected(error(errors::invalid_operation, "not attached to this control"));
      interface::slot::slots.erase(Child);
      rebuild_occupancy();
      make_messy();
      return {};
    }

    virtual slotid find_next_tabstop(slotid Focused, bool Backward, bool& Found) const override {
      if (Backward) {
        for (const auto& it : items | std::views::reverse)
          if (const auto csp = get_slot<control>(it.control_id))
            if (const auto next = csp->find_next_tabstop(Focused, Backward, Found)) return next;
      } else {
        for (const auto& it : items)
          if (const auto csp = get_slot<control>(it.control_id))
            if (const auto next = csp->find_next_tabstop(Focused, Backward, Found)) return next;
      }
      return {};
    }

    virtual bool2 has_free_size_policy() const override {
      uint2 free{};
      for (const auto& it : items)
        if (const auto csp = get_slot<control>(it.control_id)) free += csp->has_free_size_policy();
      return free * control::slot::has_free_size_policy();
    }

    virtual std::expected<float2, error> get_necessary_size() const override {
      if (auto tracks = calc_track_min_sizes(false)) {
        const auto inner = float2(sum_tracks(tracks->columns, column_gap), sum_tracks(tracks->rows, row_gap));
        return calc_necessary_size_by_policy(inner + padding.xy() + padding.zw());
      } else return tracks.error().relay();
    }

    virtual slotid hittest(float2 Pt) const override {
      if (!visible) return {};
      if (const auto hit = control::slot::hittest(Pt); !hit) return {};
      for (const auto& it : items | std::views::reverse)
        if (const auto csp = get_slot<control>(it.control_id))
          if (const auto hit = csp->hittest(Pt)) return hit;
      return id;
    }

    virtual std::expected<void, error> draw_backcontent() override {
      for (const auto& it : items)
        if (const auto csp = get_slot<control>(it.control_id)) {
          if (auto res = csp->redraw(); !res) return res.error().relay();
        } else return std::unexpected(error(errors::invalid_slotid));
      return {};
    }

    virtual std::expected<void, error> relocate() override {
      if (auto res = update_geometry(); !res) return res.error().relay();
      auto tracks = calc_track_min_sizes(false);
      if (!tracks) return tracks.error().relay();
      expand_free_tracks(tracks->columns, columns, size.x - padding.x - padding.z, column_gap);
      expand_free_tracks(tracks->rows, rows, size.y - padding.y - padding.w, row_gap);

      const auto column_offsets = calc_offsets(tracks->columns, column_gap);
      const auto row_offsets = calc_offsets(tracks->rows, row_gap);

      for (const auto& it : items) {
        const auto csp = get_slot<control>(it.control_id);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (!csp->visible) continue;
        const auto p = it.place;
        const auto cell_pos = pos + padding.xy() + float2(column_offsets[p.column], row_offsets[p.row]);
        const auto cell_size = float2(
          span_size(tracks->columns, p.column, p.columns, column_gap), span_size(tracks->rows, p.row, p.rows, row_gap));
        if (auto res = csp->relocate(cell_pos, cell_size); !res) return res.error().relay();
      }
      return {};
    }

    virtual std::expected<void, error> set_size_to_necessary() override {
      if (auto tracks = calc_track_min_sizes(true)) {
        const auto inner = float2(sum_tracks(tracks->columns, column_gap), sum_tracks(tracks->rows, row_gap));
        size = calc_necessary_size_by_policy(inner + padding.xy() + padding.zw());
        return {};
      } else return tracks.error().relay();
    }

    //-- functions --//

    size_t column_count() const noexcept { return columns.size(); }
    size_t row_count() const noexcept { return rows.size(); }
    size_t cell_index(size_t Row, size_t Column) const noexcept { return Row * column_count() + Column; }

    bool placement_in_range(const placement& Place) const noexcept {
      if (Place.columns == 0 || Place.rows == 0) return false;
      if (Place.row > row_count() || Place.column > column_count()) return false;
      return Place.row + Place.rows <= row_count() && Place.column + Place.columns <= column_count();
    }

    bool placement_available(const placement& Place) const noexcept {
      for (size_t r = Place.row; r < Place.row + Place.rows; ++r)
        for (size_t c = Place.column; c < Place.column + Place.columns; ++c)
          if (occupancy[cell_index(r, c)] != empty_cell) return false;
      return true;
    }

    void mark_placement(const placement& Place, size_t Value) noexcept {
      for (size_t r = Place.row; r < Place.row + Place.rows; ++r)
        for (size_t c = Place.column; c < Place.column + Place.columns; ++c) occupancy[cell_index(r, c)] = Value;
    }

    void rebuild_occupancy() {
      occupancy.assign(row_count() * column_count(), empty_cell);
      for (size_t i = 0; i < items.size(); ++i) mark_placement(items[i].place, i + 1);
    }

    void reserve(placement Place) noexcept {
      pending = std::nullopt;
      pending_error = std::nullopt;
      if (Place.columns == 0 || Place.rows == 0) {
        pending_error = pending_failure{errors::invalid_argument, "grid cell size must be positive"};
        return;
      }
      if (!placement_in_range(Place)) {
        pending_error = pending_failure{errors::invalid_argument, "grid cell is out of range"};
        return;
      }
      if (!placement_available(Place)) {
        pending_error = pending_failure{errors::invalid_operation, "grid cell is already occupied"};
        return;
      }
      pending = Place;
    }

    static std::expected<track, error> make_track(auto Spec) {
      using spec_type = std::remove_cvref_t<decltype(Spec)>;
      if constexpr (convertible_to<spec_type, ui::size_policy>) {
        if (const auto p = static_cast<ui::size_policy>(Spec); p != ui::fixed) return track{p, 0.0f};
        else return std::unexpected(error(errors::invalid_argument, "fixed size_policy is not allowed"));
      } else if (const auto size = float(Spec); size >= 0.0f) return track{ui::fixed, size};
      else return std::unexpected(error(errors::invalid_argument, "track size must not be negative"));
    }

    std::expected<void, error> set_columns(std::vector<track> Tracks) {
      if (Tracks.size() < column_count())
        return std::unexpected(error(errors::invalid_operation, "columns cannot shrink"));
      columns = std::move(Tracks);
      rebuild_occupancy();
      make_messy();
      return {};
    }

    std::expected<void, error> set_rows(std::vector<track> Tracks) {
      if (Tracks.size() < row_count()) return std::unexpected(error(errors::invalid_operation, "rows cannot shrink"));
      rows = std::move(Tracks);
      rebuild_occupancy();
      make_messy();
      return {};
    }

    struct track_sizes {
      std::vector<float> columns;
      std::vector<float> rows;
    };

    std::expected<track_sizes, error> calc_track_min_sizes(bool SetChildren) const {
      track_sizes result;
      result.columns.resize(column_count());
      result.rows.resize(row_count());
      for (size_t i = 0; i < column_count(); ++i)
        if (columns[i].policy == ui::fixed) result.columns[i] = columns[i].size;
      for (size_t i = 0; i < row_count(); ++i)
        if (rows[i].policy == ui::fixed) result.rows[i] = rows[i].size;

      for (const auto& it : items) {
        const auto csp = get_slot<control>(it.control_id);
        if (!csp) return std::unexpected(error(errors::invalid_slotid));
        if (!csp->visible) continue;
        float2 child_size{};
        if (SetChildren) {
          if (auto res = csp->set_size_to_necessary(); !res) return res.error().relay();
          child_size = csp->get_bounds();
        } else if (auto res = csp->get_necessary_size()) child_size = *res + csp->margin.xy() + csp->margin.zw();
        else return res.error().relay();
        ensure_span_size(result.columns, columns, it.place.column, it.place.columns, column_gap, child_size.x);
        ensure_span_size(result.rows, rows, it.place.row, it.place.rows, row_gap, child_size.y);
      }
      return result;
    }

    static void ensure_span_size(
      std::vector<float>& Sizes, const std::vector<track>& Tracks, size_t Begin, size_t Count, float Gap,
      float Required) {
      const auto current = span_size(Sizes, Begin, Count, Gap);
      if (current >= Required) return;
      size_t grow_count = 0;
      for (size_t i = Begin; i < Begin + Count; ++i)
        if (Tracks[i].policy != ui::fixed) ++grow_count;
      if (grow_count == 0) return;
      const auto each = (Required - current) / float(grow_count);
      for (size_t i = Begin; i < Begin + Count; ++i)
        if (Tracks[i].policy != ui::fixed) Sizes[i] += each;
    }

    static float span_size(const std::vector<float>& Sizes, size_t Begin, size_t Count, float Gap) noexcept {
      float result = 0.0f;
      for (size_t i = Begin; i < Begin + Count; ++i) result += Sizes[i];
      if (Count > 1) result += Gap * float(Count - 1);
      return result;
    }

    static float sum_tracks(const std::vector<float>& Sizes, float Gap) noexcept {
      float result = 0.0f;
      for (const auto s : Sizes) result += s;
      if (Sizes.size() > 1) result += Gap * float(Sizes.size() - 1);
      return result;
    }

    static void expand_free_tracks(
      std::vector<float>& Sizes, const std::vector<track>& Tracks, float Area, float Gap) noexcept {
      const auto current = sum_tracks(Sizes, Gap);
      if (current >= Area) return;
      const auto extra = Area - current;
      size_t free_count = 0;
      for (const auto& t : Tracks)
        if (t.policy == ui::free) ++free_count;
      if (free_count == 0) return;
      const auto each = extra / float(free_count);
      for (size_t i = 0; i < Sizes.size(); ++i)
        if (Tracks[i].policy == ui::free) Sizes[i] += each;
    }

    static std::vector<float> calc_offsets(const std::vector<float>& Sizes, float Gap) {
      std::vector<float> result(Sizes.size());
      float offset = 0.0f;
      for (size_t i = 0; i < Sizes.size(); ++i) {
        result[i] = offset;
        offset += Sizes[i] + Gap;
      }
      return result;
    }
  };

  grid_layout() noexcept = default;

  grid_layout(derived_from<interface> auto& Parent, const source_line& sl = here()) {
    if (auto res = create(Parent)) *this = std::move(*res);
    else res.error().add_footprint().go_off(sl);
  }

  static std::expected<grid_layout, error> create(derived_from<interface> auto& Parent) {
    grid_layout g;
    const auto temp_id = make_slot<grid_layout>();
    const auto sp = get_slot<grid_layout>(temp_id);
    if (!sp) return std::unexpected(error(errors::slot_creation_failed));
    const auto psp = get_slot<control>(Parent.id());
    if (!psp) return std::unexpected(error(errors::invalid_slotid));
    if (auto res = psp->attach(temp_id); !res) {
      slot::slots.erase(temp_id);
      return res.error().relay();
    }
    g._id = temp_id;
    sp->id = temp_id;
    sp->window_id = psp->get_window_id();
    sp->margin = {};
    sp->padding = {};
    sp->radius = {};
    if (auto theme = sp->get_color_theme(); !theme) return theme.error().relay();
    else if (auto res = sp->apply_color_theme(*(*theme), false); !res) return res.error().relay();
    return g;
  }

  //-- getter --//

  float column_gap() const noexcept {
    if (const auto sp = get_slot(this)) return sp->column_gap;
    error(errors::invalid_slotid).fizzle_out();
    return {};
  }

  float row_gap() const noexcept {
    if (const auto sp = get_slot(this)) return sp->row_gap;
    error(errors::invalid_slotid).fizzle_out();
    return {};
  }

  //-- setter --//

  template<typename... Specs> requires((is_grid_track_spec<Specs> && ...))
  std::expected<void, error> columns(Specs... Specs_) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if constexpr (sizeof...(Specs_) == 0)
      return std::unexpected(error(errors::invalid_argument, "columns must not be empty"));
    std::vector<slot::track> tracks;
    tracks.reserve(sizeof...(Specs_));
    auto append = [&](auto Spec) -> std::expected<void, error> {
      if (auto res = slot::make_track(Spec)) {
        tracks.push_back(*res);
        return {};
      } else return res.error().relay();
    };
    std::expected<void, error> parse_result{};
    auto parse = [&](auto Spec) {
      if (!parse_result) return;
      if (auto res = append(Spec); !res) parse_result = res.error().relay();
    };
    (parse(Specs_), ...);
    if (!parse_result) return parse_result.error().relay();
    return sp->set_columns(std::move(tracks));
  }

  template<typename... Specs> requires((is_grid_track_spec<Specs> && ...))
  std::expected<void, error> rows(Specs... Specs_) {
    const auto sp = get_slot(this);
    if (!sp) return std::unexpected(error(errors::invalid_slotid));
    if constexpr (sizeof...(Specs_) == 0)
      return std::unexpected(error(errors::invalid_argument, "rows must not be empty"));
    std::vector<slot::track> tracks;
    tracks.reserve(sizeof...(Specs_));
    auto append = [&](auto Spec) -> std::expected<void, error> {
      if (auto res = slot::make_track(Spec)) {
        tracks.push_back(*res);
        return {};
      } else return res.error().relay();
    };
    std::expected<void, error> parse_result{};
    auto parse = [&](auto Spec) {
      if (!parse_result) return;
      if (auto res = append(Spec); !res) parse_result = res.error().relay();
    };
    (parse(Specs_), ...);
    if (!parse_result) return parse_result.error().relay();
    return sp->set_rows(std::move(tracks));
  }

  grid_layout& at(uint2 Pos, uint2 Size = {1, 1}) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->reserve({Pos.x, Pos.y, Size.x, Size.y});
    return *this;
  }

  grid_layout& cell(uint2 Pos, uint2 Size = {1, 1}) noexcept { return at(Pos, Size); }
  grid_layout& grid(uint2 Pos, uint2 Size = {1, 1}) noexcept { return at(Pos, Size); }

  grid_layout& column_gap(float v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->column_gap = yw::max(0.0f, v);
    sp->make_messy();
    return *this;
  }

  grid_layout& row_gap(float v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->row_gap = yw::max(0.0f, v);
    sp->make_messy();
    return *this;
  }

  grid_layout& gap(float2 v) noexcept {
    const auto sp = get_slot(this);
    if (!sp) {
      error(errors::invalid_slotid).fizzle_out();
      return *this;
    }
    sp->column_gap = yw::max(0.0f, v.x);
    sp->row_gap = yw::max(0.0f, v.y);
    sp->make_messy();
    return *this;
  }
};
} // namespace yw::ui
