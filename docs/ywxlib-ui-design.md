# ywxlib UI Design: `window` and `ui::control`

## Table of Contents

1. [Overview](#1-overview)
2. [Object Management: The Slot Pattern](#2-object-management-the-slot-pattern)
3. [Window (`yw::window`)](#3-window-ywwindow)
4. [Control Base Class (`ui::control`)](#4-control-base-class-uicontrol)
5. [Layout System](#5-layout-system)
6. [Leaf Controls](#6-leaf-controls)
7. [Event System](#7-event-system)
8. [Rendering Pipeline](#8-rendering-pipeline)
9. [Focus and Tab Navigation](#9-focus-and-tab-navigation)
10. [Tooltip](#10-tooltip)

---

## 1. Overview

ywxlib is a Win32/Direct2D-based UI framework written in C++. The UI subsystem
is organized around two main concepts:

- **`yw::window`** — a top-level OS window with its own swap chain and render target.
- **`yw::ui::control`** — the base class for all widgets placed inside a window.

Controls are arranged in a tree. Interior nodes are *layout controls* that size
and position their children. Leaf nodes are the actual interactive widgets
(button, checkbox, slider, …). The window holds a reference to the root layout
and drives the rendering and event dispatch.

---

## 2. Object Management: The Slot Pattern

All UI objects share a common ownership and identification mechanism defined in
`ui::unknown`.

```
ui::unknown          (handle — lightweight, movable)
  └── _id : slotid  (index into system::uis)

system::uis : slotset<ui::unknown::slot>
  └── owns  ui::unknown::slot  (polymorphic, heap-allocated)
```

### Key types

| Type | Role |
|------|------|
| `ui::slotid` | Stable opaque ID for a slot entry |
| `ui::unknown::slot` | Base runtime state; contains `id`, `attach()`, `detach()` |
| `system::uis` | Global `slotset` that owns all slot objects |
| `system::slot_address<T>(id)` | `dynamic_cast` helper to get a typed pointer from an ID |

Every concrete control (e.g. `ui::button`) defines its own `slot` class
inheriting from `ui::control::slot`, which in turn inherits from
`ui::unknown::slot`.

The **handle** class (e.g. `ui::button`) is a thin wrapper around a `slotid`.
It provides a property-accessor API and does *not* own the slot — the slot lives
in `system::uis`. Handles can be moved; the moved-from handle becomes invalid
(`operator bool()` returns `false`).

---

## 3. Window (`yw::window`)

Declared in `ywx/window.h`. Inherits `ui::unknown`.

### 3.1 Window Styles

```cpp
enum class style : uint32_t {
    regular    // WS_OVERLAPPEDWINDOW — resizable title-bar window
    fixed      // non-resizable title-bar window
    borderless // WS_POPUP — no decoration
};
```

### 3.2 Slot State

| Field | Type | Description |
|-------|------|-------------|
| `hwnd` | `HWND` | Win32 window handle |
| `pos` | `int2` | Screen position of the window |
| `size` | `uint2` | Client area size in pixels |
| `margin` | `int4` | Non-client border insets (computed at creation) |
| `style` | `window::style` | Window decoration style |
| `title` | `wstring` | Window title |
| `rendertarget` | `bitmap` | DXGI back-buffer render target |
| `layout_bitmap` | `bitmap` | Off-screen bitmap for UI layer |
| `swapchain` | `IDXGISwapChain1*` | Swap chain for presentation |
| `background` | `yw::background` | Window background fill |
| `layout_id` | `ui::slotid` | Root layout control |
| `focused_control` | `ui::slotid` | Currently focused control |
| `hovered_control` | `ui::slotid` | Control under the cursor |
| `captured_control` | `ui::slotid` | Control with mouse capture |
| `dirty` | `bool` | Visual content changed; redraw needed |
| `messy` | `bool` | Layout changed; full recalculation needed |
| `focus_ring` | struct | `offset`, `width`, `color` for the focus ring |

### 3.3 Construction

```cpp
window(std::optional<int2> Pos, uint2 Size,
       style Style = style::regular, bool Show = true);
```

If `Pos` is `std::nullopt` the window is centered on the screen.
On construction the window registers itself in `system::primal_windows`.

### 3.4 Key Public API

| Member | Description |
|--------|-------------|
| `pos()` / `pos(int2)` | Get / set screen position |
| `size()` / `size(uint2)` | Get / set client size |
| `title()` / `title(S&&)` | Get / set title string |
| `visible()` / `visible(bool)` | Show / hide window |
| `enabled()` / `enabled(bool)` | Enable / disable input |
| `background()` / `background(bg)` | Window background |
| `on_close(fn)` | Callback — return `false` to cancel close |
| `on_keydown(fn)` / `on_keyup(fn)` | Raw keyboard events |
| `commands()` | `yw::command_manager` for keyboard shortcuts |
| `begin_draw()` | Open a direct drawing context onto the render target |
| `screenshot(path, writeUI)` | Save window contents to PNG |
| `destroy()` | Explicitly destroy the OS window |

### 3.5 Layout Attachment

`window::slot` delegates `attach()` / `detach()` to the root layout control.
When controls are added to a layout that is inside a window, their
`window_id` field is set so they can propagate dirty/messy flags to the window.

---

## 4. Control Base Class (`ui::control`)

Declared in `ywx/ui_control.h`. Inherits `ui::unknown`.

### 4.1 Slot State

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `layout_id` | `slotid` | — | Parent layout |
| `window_id` | `slotid` | — | Owning window |
| `margin` | `float4` | 5 px all sides | External spacing (left, top, right, bottom) |
| `pos` | `float2` | — | Computed render position |
| `size` | `float2` | — | Computed render size |
| `radius` | `float2` | 5 px | Corner radius |
| `min_size` | `float2` | 10 px | Minimum size |
| `alignment` | `ui::alignment` | `center` | Placement within surplus area |
| `constrained` | `vector<bool,2>` | `{false,false}` | Whether width / height are user-fixed |
| `visible` | `bool` | `true` | Whether the control is drawn |
| `enabled` | `bool` | `true` | Whether the control accepts input |
| `tooltip` | `wstring` | `""` | Tooltip text (empty = no tooltip) |
| `on_hover` | `function<void, event::hover>` | — | Hover callback |

### 4.2 Alignment Enum

```cpp
enum class alignment {
    center,                      // centered in both axes
    left,  right,                // horizontal alignment
    top,   bottom,               // vertical alignment
    left_top,  left_bottom,
    right_top, right_bottom
};
```

When the available area is larger than the control, `align(Extra)` shifts `pos`
so that the control sits at the requested corner/edge.
`ui::unconstrained` (`-1.0f`) is a sentinel value that disables size fixing.

### 4.3 Virtual Layout Protocol

These three virtual methods form the layout protocol called by the parent layout
and ultimately triggered by the window's rendering loop.

| Method | When called | What it does |
|--------|-------------|--------------|
| `calculate_size() → float2` | Size-query pass | Returns the minimum bounding size the control needs *without* side-effects |
| `update_size()` | Size-commit pass | Writes the computed size into `slot::size` |
| `update_layout(Pos, Area)` | Position pass | Writes `pos` and final `size`; calls `align()` if area > size |

**Constrained dimensions**: if `constrained.x` is `true`, the user has fixed
the width; `update_layout` preserves that dimension and only stretches
unconstrained ones.

### 4.4 Dirty / Messy Flags

Controls propagate change notifications up to the owning window:

- `make_dirty()` — only a repaint is needed (e.g. color change).
- `make_messy()` — the layout must be recalculated (e.g. text or size change).

Setting a property through the handle's setter chooses the appropriate flag
automatically.

### 4.5 Virtual Event Handlers

| Handler | Triggered by |
|---------|--------------|
| `click_event(event::button)` | Mouse button released over the same control that received `button_event` |
| `button_event(event::button)` | Mouse button press / release |
| `drag_event(event::drag)` | Mouse moved while a button is held |
| `focus_event(bool)` | Focus gained (`true`) / lost (`false`). Returns `true` if the control accepts focus |
| `hover_event(event::hover)` | Cursor entered / left / moved over the control. Default impl shows tooltip |
| `key_event(event::key) → bool` | Keyboard key while the control is focused. Returns `true` if consumed |
| `move_event(event::move)` | Mouse moved |
| `wheel_event(event::wheel)` | Mouse wheel |

### 4.6 Focus Ring

```cpp
virtual void draw_focus_ring(float offset, float width) const;
```

Default implementation draws a rounded rectangle at `pos - offset` with
`size + 2*offset` and `radius + offset`.
`radiobutton` overrides this to draw the ring around the individual focused item.

### 4.7 Hit Testing

```cpp
virtual slotid hit_test(float2 Pt) const noexcept;
```

Returns the slot's own ID if `Pt` is inside `[pos, pos+size]`, otherwise an
invalid ID. Layout controls override this to delegate to children.

### 4.8 Handle-Level Setters and Getters

The handle class exposes typed getters and setters. Setters that affect size use
`safe_set_size` (calls `make_messy`); purely visual setters use `safe_set`
(calls `make_dirty`).

Notable setters:

```cpp
void size(float2 size);    // negative component = unconstrained
void width(float1 width);
void height(float1 height);
```

---

## 5. Layout System

Layout controls are containers that implement `calculate_size`, `update_size`,
`update_layout`, `hit_test`, `next_tab_stop`, and `draw` to manage child
controls.

### 5.1 `ui::layout` / `ui::vertical_layout`

Defined in `ywx/ui_layout.h`.

Children are stacked **vertically** (top to bottom).
Extra vertical space is distributed equally among children whose height is
*unconstrained*.
The cross (horizontal) dimension of each child is offered the full layout width.

Key properties (slot):

| Property | Default | Description |
|----------|---------|-------------|
| `background` | transparent | Fill for the layout area |
| `border_color` | transparent | Outline color |
| `border_width` | 1 px | Outline width |
| `padding` | 5 px | Inner spacing between border and children |

`controls` is a `std::vector<slotid>` of child slots in display order.

On `attach(cid)`, the child's `layout_id` and `window_id` are set.
On `~slot()`, all children are destroyed via `system::uis.erase`.

### 5.2 `ui::horizontal_layout`

Inherits `ui::layout::slot`. Overrides the three layout virtuals with the
horizontal variant (template parameter `V = false`). Children are stacked
**left to right**; extra horizontal space is distributed among unconstrained
children.

### 5.3 `ui::grid_layout<Columns>`

Fixed number of columns supplied as a template argument.

Children are added left-to-right, top-to-bottom (row-major).
Column widths are the maximum of all cells in that column.
Row heights are the maximum of all cells in that row.
Extra space is distributed to columns / rows that contain only unconstrained
cells.

```cpp
ui::grid_layout<3> grid(parentLayout);
// adds 9 controls → 3 rows × 3 columns
```

---

## 6. Leaf Controls

All leaf controls share a common appearance model:

- `background` — fill (can be a solid color, gradient, or bitmap)
- `border_color` / `border_width` — outline drawn as a rounded rectangle
- `padding` — inner spacing between border and content
- `radius` (inherited) — corner rounding

### 6.1 `ui::label`

Declared in `ywx/ui_label.h`.

A non-interactive text display. Size is determined by the text extent plus
padding.

| Property | Type | Description |
|----------|------|-------------|
| `text` | `yw::text` | Displayed text object |
| `text_color` | `color` | Text color |
| `text_alignment` | `ui::alignment` | Horizontal/vertical placement of text within the label area |

`text_alignment` also propagates to the DirectWrite text format (left/center/right).

Events: `on_button`, `on_wheel`.

### 6.2 `ui::button`

Declared in `ywx/ui_button.h`.

A clickable button. Click is triggered by:
- Left mouse button (press + release on same control)
- **Enter** or **Space** key (while focused: press + release)

| Property | Type | Description |
|----------|------|-------------|
| `text` | `yw::text` | Button label |
| `text_color` | `color` | Label color |

| Callback | Signature | When |
|----------|-----------|------|
| `on_click` | `void(key)` | Click confirmed (key code = trigger) |
| `on_button` | `void(event::button)` | Any mouse button press / release |
| `on_focus` | `void(bool)` | Focus gained / lost |
| `on_keydown` | `void(event::key)` | Key pressed while focused |
| `on_keyup` | `void(event::key)` | Key released while focused |
| `on_move` | `void(event::move)` | Mouse move |
| `on_wheel` | `void(event::wheel)` | Scroll wheel |

### 6.3 `ui::checkbox`

Declared in `ywx/ui_checkbox.h`.

A toggle button rendered as an icon + label. The icon pair (box / mark) is
defined as `yw::icon` (SVG path geometry). Default shape: circle with inner
circle mark.

| Property | Type | Description |
|----------|------|-------------|
| `checked` | `bool` | Current state |
| `box` | `yw::icon` | Outer icon |
| `mark` | `yw::icon` | Inner check mark icon |
| `box_fill_color`, `box_stroke_color`, `box_stroke_width` | — | Box rendering |
| `mark_fill_color`, `mark_stroke_color`, `mark_stroke_width` | — | Mark rendering |
| `icon_size` | `float2` | Rendered icon size |
| `icon_offset` | `float` | Gap between icon and text |
| `text` | `yw::text` | Label |
| `text_color` | `color` | Label color |

| Callback | Signature | When |
|----------|-----------|------|
| `on_change` | `void(bool)` | State toggled |
| `on_click` | `void(key)` | Click confirmed |
| *(same as button)* | | |

Click toggles `checked` and fires `on_change`.

### 6.4 `ui::radiobutton`

Declared in `ywx/ui_radiobutton.h`.

A group of mutually exclusive radio items within a single control.
Each item has its own `text` and `text_color`; all items share icon geometry.

```cpp
radiobutton rb(layout);
rb.append_item(L"Option A");
rb.append_item(L"Option B");
rb.append_item(L"Option C");
rb.checked(0); // selects first item
```

| Property | Type | Description |
|----------|------|-------------|
| `checked` | `unsigned` | Index of selected item |
| `item_gap` | `float` | Vertical gap between rows |
| *(box / mark same as checkbox)* | | |

Focus navigation:
- **Up / Down arrow** moves the *focus highlight* within the group.
- **Enter / Space** selects the focused item.
- `draw_focus_ring` is overridden to draw the ring around the focused row only.

| Callback | Signature | When |
|----------|-----------|------|
| `on_change` | `void(unsigned)` | Selected index changed |
| `on_click` | `void(key)` | Selection confirmed |
| *(same as button)* | | |

### 6.5 `ui::slider`

Declared in `ywx/ui_slider.h`.

A draggable range slider. Can be **horizontal** (default) or **vertical**.

Visual structure:

```
 [ track (gray) ──────[fill (blue)──●thumb]─── ]
```

| Property | Type | Description |
|----------|------|-------------|
| `min_value`, `max_value` | `double` | Value range |
| `value` | `double` | Current value |
| `step` | `double` | Arrow key increment |
| `page_step` | `double` | Page Up/Down increment |
| `vertical` | `bool` | Orientation |
| `wrap` | `bool` | Wrap around at endpoints |
| `track_color` | `color` | Empty track color |
| `fill_color` | `color` | Filled portion color |
| `thumb_color` / `thumb_border_color` / `thumb_border_width` | — | Thumb appearance |
| `track_thickness` | `float` | Track bar height/width |
| `thumb_size` | `float2` | Thumb ellipse size |

Keyboard bindings (when focused):

| Key | Action |
|-----|--------|
| ←/→ (horizontal) or ↑/↓ (vertical) | ± `step` |
| Page Up / Page Down | ± `page_step` |
| Home / End | `min_value` / `max_value` |

Mouse wheel steps by `step` (or `page_step` with Shift held).
Click + drag directly positions the thumb.

| Callback | Signature |
|----------|-----------|
| `on_change` | `void(double)` |
| *(standard set)* | |

### 6.6 `ui::spinbox`

Declared in `ywx/ui_spinbox.h`.

A numeric text display with ▲/▼ increment buttons on the right.

```
┌──────────────┬──┐
│   42         │▲ │
│              │▼ │
└──────────────┴──┘
```

| Property | Type | Description |
|----------|------|-------------|
| `value` | `double` | Current value |
| `min_value`, `max_value` | `double` | Clamping range |
| `step` | `double` | Arrow key / button increment |
| `page_step` | `double` | Page Up/Down increment |
| `precision` | `unsigned` | Decimal digits displayed |
| `wrap` | `bool` | Wrap around at endpoints |
| `button_width` | `float` | Width of the ▲▼ button area |
| `button_color` / `pressed_button_color` | `color` | Button state colors |

The displayed text is formatted as a fixed-point string with `precision` digits.
Value is always rounded to `precision` decimal places.

Keyboard: ↑/↓ (±`step`), Page Up/Down (±`page_step`), Home/End.
Wheel: ±`step` (or ±`page_step` with Shift).

| Callback | Signature |
|----------|-----------|
| `on_change` | `void(double)` |
| *(standard set)* | |

### 6.7 `ui::progressbar`

Declared in `ywx/ui_progressbar.h`.

A read-only progress indicator. No user interaction.

```
┌────────────────────────────────┐
│ ████████████░░░░░░░  42%       │
└────────────────────────────────┘
```

| Property | Type | Description |
|----------|------|-------------|
| `value` | `double` | Current progress |
| `min_value`, `max_value` | `double` | Range |
| `track_color` | `color` | Background bar color |
| `fill_color` | `color` | Filled portion color |
| `show_text` | `bool` | Whether to display percentage text |
| `precision` | `unsigned` | Decimal digits in percentage |
| `text_color` | `color` | Percentage text color |

| Callback | Signature |
|----------|-----------|
| `on_change` | `void(double)` |

---

## 7. Event System

Events are plain aggregates defined in `ywx/event.h` (namespace `yw::event`).
Each fits in ≤ 8 bytes.

### `event::button`
```cpp
struct button {
    short2 pos;   // cursor position
    yw::key code; // which button
    bool down;    // true = press, false = release
    bool ctrl, shift, alt;
};
```
Fired for mouse button press and release.

### `event::drag`
```cpp
struct drag {
    short2 delta; // accumulated cursor delta since capture
    yw::key code; // button being held
    bool ctrl, shift, alt;
};
```
Fired while the mouse moves with a button held.

### `event::hover`
```cpp
struct hover {
    short2 pos;
    enum class type { enter, leave, move } type;
};
```
`enter` fires when the cursor first moves over a control;
`leave` fires when it moves away;
`move` fires on subsequent moves within the control.

### `event::key`
```cpp
struct key {
    yw::key code;
    bool down;   // true = press, false = release
    bool first;  // true = first repeat (no auto-repeat on press)
    bool ctrl, shift, alt;
};
```

### `event::move`
```cpp
struct move {
    short2 pos;   // current cursor position
    short2 delta; // movement since last event
};
```

### `event::wheel`
```cpp
struct wheel {
    short2 pos;
    short delta;       // positive = forward/up
    bool horizontal;   // true for horizontal scroll
    bool ctrl, shift, alt;
};
```

---

## 8. Rendering Pipeline

```
window::draw()
 │
 ├─ [messy || dirty] → draw_layout_bitmap()
 │    │
 │    ├─ [messy] → root_layout->update_size()
 │    │             ├─ window auto-resizes if layout exceeds client area
 │    │             └─ root_layout->update_layout({0,0}, float2(size))
 │    │
 │    └─ [messy || dirty] → root_layout->draw()
 │         └─ recursively draws all visible children
 │
 ├─ rendertarget.begin_draw()
 │    ├─ draw_bitmap(layout_bitmap)          — composite UI layer
 │    └─ focused_control->draw_focus_ring()  — focus overlay
 │
 └─ swapchain->Present()
```

### Dirty vs Messy

| Flag | Set when | Effect |
|------|----------|--------|
| `dirty` | Visual property changed (color, value, …) | Re-draw; no re-layout |
| `messy` | Structural property changed (size, text, padding, …) | Full re-layout + re-draw |

Setting a property through the handle automatically chooses the correct flag
(`safe_set` → dirty, `safe_set_size` → messy).

### Off-screen Layout Bitmap

Controls are drawn into `layout_bitmap` (an off-screen `bitmap`).
The window `rendertarget` may contain application-drawn content (via
`begin_draw()`); the UI layer is composited on top.
`screenshot(path, writeUI=true)` composites both bitmaps before saving.

---

## 9. Focus and Tab Navigation

### Focus Ring

The window stores `focused_control : slotid`.
After compositing the layout bitmap the window calls:

```cpp
focused_control_slot->draw_focus_ring(focus_ring.offset, focus_ring.width);
```

The ring is drawn directly on `rendertarget`, not in the layout bitmap, so it
always appears above all controls.

Default ring style: semi-transparent blue (`rgba(0, 0, 1, 0.5)`), offset 3 px,
width 1 px.

### Tab Traversal

```cpp
window::slot::next_tab_stop(bool Forward);
```

Calls `root_layout->next_tab_stop(focused_control, Forward, found)`.
Layouts walk their children in order (or reverse for backward traversal) and
recursively ask each child.
A leaf control returns its own ID from `next_tab_stop` if:
- `Found` is already `true` (a previous control was the current focus, so this one is next), **and**
- `visible && enabled && focus_event(true)` succeeds.

When `focused_control` is invalid (no focus), `Found` starts as `true` and the
first focusable control is returned.

---

## 10. Tooltip

`control::slot::hover_event` provides built-in tooltip support:

- On `hover::enter` — `system::tooltip.show(pos, size, tooltip)` is called with
  the control's screen-space position (computed from `window.pos + window.margin`
  plus the control's client `pos`).
- On `hover::leave` — `system::tooltip.hide()` is called.
- If `tooltip` is empty, no tooltip is shown.

The tooltip text is set via `control::tooltip(S&&)` (accepts any `stringable`).

```cpp
btn.tooltip(L"Click to apply changes");
```
