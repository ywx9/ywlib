#pragma once
#include "ywx/clipboard.h"
#include "ywx/ui_base.h"
#include "ywx/ui_part_selection.h"

namespace yw::ui {

class edit : public base {
public:
	class slot : public base::slot {
		void _erase_selection() {
			auto& value = text.text();
			const auto start = selection.start();
			value.erase(value.begin() + start, value.begin() + selection.end());
			selection.end(start);
			selection.anchor(start);
		}

		size_t _find_boundary(size_t Pos, bool Forward) const {
			const auto& value = text.text();
			if (Forward) {
				while (Pos < value.size() && !is_alnum(value[Pos])) ++Pos;
				while (Pos < value.size() && is_alnum(value[Pos])) ++Pos;
			} else {
				while (Pos > 0 && !is_alnum(value[Pos - 1])) --Pos;
				while (Pos > 0 && is_alnum(value[Pos - 1])) --Pos;
			}
			return Pos;
		}

	public:
		part::label text;
		part::selection selection;
		// size_t caret_pos{};
		// size_t selection_anchor{};
		// size_t selection_start{};
		// size_t selection_end{};
		// color selection_color = color(0x3399FF, 0.35f);

		slot() { focusable = true; }

		virtual std::expected<void, error_trace> draw() const override {
			if (auto res = base::slot::draw(); !res) return unexpected_error(res.error());
			if (auto res = text.draw(pos); !res) return unexpected_error(res.error());
			if (auto res = selection.draw(pos, text); !res) return unexpected_error(res.error());
			// if (has_selection()) {
			// 	const auto& layout = text.layout();
			// 	const auto pad = text.padding();
			// 	brush.color(selection_color);
			// 	for (size_t i = selection_start; i < selection_end; ++i) {
			// 		if (auto ht = layout.hit_test(i)) {
			// 			const float sel_x = pos.x + pad.x + ht->x;
			// 			const float sel_y = pos.y + pad.y + ht->y;
			// 			if (auto res = fill_rectangle({sel_x, sel_y}, {ht->z, ht->w}); !res)
			// 				return unexpected_error(res.error());
			// 		} else return unexpected_error(ht.error());
			// 	}
			// }

			const auto w_slot_p = system::windows.get(window_id);
			if (w_slot_p && w_slot_p->focused_ui == id) {
				if (auto ht = text.layout().hit_test(selection.anchor())) {
					const float caret_x = pos.x + text.padding().x + ht->x;
					const float caret_y = pos.y + text.padding().y + ht->y;
					brush.color(text.color());
					if (auto res = draw_line({caret_x, caret_y}, {caret_x, caret_y + ht->w}, 1.0f); !res)
						return unexpected_error(res.error());
				}
			}
			return {};
		}

		virtual void key_event(event::key e) override {
			if (!focusable || !e.down) return;
      auto& value = text.text();
      if (selection.anchor() > value.size()) selection.anchor(value.size());
      const size_t old_caret = selection.anchor();
      // bool moved = false;

			if (e.ctrl) {
				if (e.code == key::a) {
					selection.start(0);
					selection.end(value.size());
					selection.anchor(value.size());
					make_window_dirty();
				} else if (e.code == key::c) {
					if (has_selection())
						if (auto s = selected_text(value)) clipboard.text(*s);
				} else if (e.code == key::x) {
					if (has_selection()) {
						if (auto s = selected_text(value)) clipboard.text(*s);
						_erase_selection();
						make_window_dirty();
					}
				} else if (e.code == key::v) {
					if (auto s = clipboard.text()) {
						if (has_selection()) _erase_selection();
						value.insert(selection.anchor(), *s);
						selection.anchor(selection.anchor() + s->size());
						make_window_dirty();
					}
				} else if (e.code == key::backspace) {
					if (!has_selection()) {
						selection.start(_find_boundary(selection.anchor(), false));
						selection.end(selection.anchor());
					}
					if (has_selection()) {
						_erase_selection();
						make_window_dirty();
					}
				} else if (e.code == key::delete_) {
					if (!has_selection()) {
						selection.start(selection.anchor());
						selection.end(_find_boundary(selection.anchor(), true));
					}
					if (has_selection()) {
						_erase_selection();
						make_window_dirty();
					}
				} else if (e.code == key::left) {
					selection.anchor(_find_boundary(selection.anchor(), false));
					_reset_selection();
					make_window_dirty();
				} else if (e.code == key::right) {
					selection.anchor(_find_boundary(selection.anchor(), true));
					_reset_selection();
					make_window_dirty();
				} else if (e.code == key::home) {
					selection.anchor(0);
					_reset_selection();
					make_window_dirty();
				} else if (e.code == key::end) {
					selection.anchor(value.size());
					_reset_selection();
					make_window_dirty();
				}
				return;
			}
			if (e.code == key::backspace) {
				if (!has_selection()) {
					if (selection.anchor() == 0) return;
					selection.start(selection.anchor() - 1);
					selection.end(selection.anchor());
				}
				_erase_selection();
				make_window_dirty();
			} else if (e.code == key::delete_) {
				if (!has_selection()) {
					if (selection.anchor() >= value.size()) return;
					selection.start(selection.anchor());
					selection.end(selection.anchor() + 1);
				}
				_erase_selection();
				make_window_dirty();
			} else if (e.code == key::left) {
				if ()
				if (selection.anchor() > 0) selection.anchor(selection.anchor() - 1);
				_reset_selection();
				make_window_dirty();
			} else if (e.code == key::right) {
				if (caret_pos < value.size()) {
					if (e.ctrl) {
						size_t pos = caret_pos;
						const bool cur = is_alnum(value[pos]);
						while (++pos < value.size() && is_alnum(value[pos]) == cur) continue;
						caret_pos = pos;
					} else ++caret_pos;
          moved = true;
				}
			} else if (e.code == key::home) {
				caret_pos = 0;
        moved = true;
			} else if (e.code == key::end) {
				caret_pos = value.size();
        moved = true;
			}

      if (moved) {
        update_selection(e.shift, old_caret);
        make_window_dirty();
      }
		}

		virtual void char_event(wchar_t c) override {
			if (!focusable) return;
			if (is_ascii(c)) {
        auto& value = text.text();
				if (caret_pos > value.size()) caret_pos = value.size();
        if (has_selection()) erase_selection(value);
				value.insert(value.begin() + caret_pos, c);
				++caret_pos;
        clear_selection();
				make_window_dirty();
			}
		}

	private:
		bool has_selection() const noexcept { return selection_start != selection_end; }

		void clear_selection() noexcept {
			selection_anchor = caret_pos;
			selection_start = caret_pos;
			selection_end = caret_pos;
		}

		void update_selection(bool shift, size_t old_caret) {
			if (shift) {
				if (!has_selection()) selection_anchor = old_caret;
				selection_start = std::min(selection_anchor, caret_pos);
				selection_end = std::max(selection_anchor, caret_pos);
			} else clear_selection();
		}

		std::expected<std::wstring, error_trace> selected_text(const std::wstring& value) const {
			if (!has_selection()) return unexpected_error(errors::invalid_operation, "no selection");
			const auto start = std::min(selection_start, value.size());
			const auto end = std::min(selection_end, value.size());
			if (start >= end) return unexpected_error(errors::invalid_operation, "empty selection");
			return value.substr(start, end - start);
		}

		void erase_selection(std::wstring& value) {
			if (!has_selection()) return;
			const auto start = std::min(selection_start, value.size());
			const auto end = std::min(selection_end, value.size());
			if (start < end) value.erase(value.begin() + start, value.begin() + end);
			caret_pos = start;
			clear_selection();
		}
	};

public:
	using base::operator bool;
	slot* slot_address() const noexcept { return dynamic_cast<slot*>(system::uis.get(_id)); }

	auto* edit_text() noexcept { return safe_get(&slot::text); }
	const auto& text() const { return unsafe_get(&slot::text); }

  const auto& caret_pos() const { return unsafe_get(&slot::caret_pos); }
  void caret_pos(size_t value) {
    if (const auto s = slot_address()) {
      s->caret_pos = value;
      make_window_dirty();
    }
  }

	template<included_in<window&, none> Window>
  static std::expected<edit, error_trace> add(Window&& w, float2 Pos, float2 Size) {
		if (auto res = base::add<edit>(w, Pos, Size)) {
			const auto slot_p = res->second;
			slot_p->text.size(Size);
			slot_p->text.layout().text_alignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			slot_p->make_window_dirty();
			return edit{std::move(res->first)};
		} else return unexpected_error(res.error());
	}
};
} // namespace yw::ui
