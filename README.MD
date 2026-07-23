# ywlib / ywxlib Project Template

`ywlib` / `ywxlib` を利用する C++23 プロジェクト向けのテンプレートです。
`project.json` を編集し、`tools/init.py` と `tools/build.py` を実行するだけで、最小構成のプロジェクトを作成できます。

## できること

- `ywlib` を使った通常の C++ プロジェクト作成
- `ywxlib` を使った Windows / DirectX アプリケーション作成
- `exe` / `dll` ターゲットの切り替え
- CMake / VS Code 設定の自動生成
- umbrella header (`ywlib`, `ywxlib`) の自動生成
- `compile_commands.json` を使った IntelliSense 対応

## 必要環境

- Python 3
- Git
- CMake
- C++23 対応コンパイラ
- VS Code（任意）

`ywxlib` を使う場合は、Windows 上で DirectX 関連ライブラリを利用できる環境が必要です。

## クイックスタート

### 1. テンプレートから新しいプロジェクトを作成

[`bootstrap.py`](/tools/bootstrap.py)をローカルに用意してください。

```bash
python bootstrap.py my_project
cd my_project
```

### 2. 設定を反映

```bash
python tools/init.py
```

### 3. ビルド

```bash
python tools/build.py
```

## 日常的に使うコマンド

### 設定を再生成する

```bash
python tools/init.py
```

次のような場合に実行します。

- 初回セットアップ時
- `project.json` を変更した後
- `CMakeLists.txt` や `.vscode/settings.json` を再生成したいとき
- `build/` を作り直したいとき

### ビルドする

```bash
python tools/build.py
```

`build/compile_commands.json` が存在しない場合は、内部で `tools/init.py` が実行されます。

## `project.json`

`project.json` は、このテンプレートの設定ファイルです。

例:

```json
{
  "project_name": "my_project",
  "source_file": "source.cpp",
  "target_type": "exe",
  "library": "ywlib"
}
```

### 設定項目

#### `project_name`

出力ファイル名や CMake のターゲット名に使われるプロジェクト名です。

#### `source_file`

エントリポイントとなるソースファイル名です。
既定値は `source.cpp` です。

#### `target_type`

出力形式を指定します。

- `exe`
- `dll`

#### `library`

使用するライブラリを指定します。

- `ywlib`
- `ywxlib`

## ディレクトリ構成

```text
.
├─ .clang-format
├─ .gitignore
├─ project.json
├─ source.cpp
├─ CMakeLists.txt
├─ .vscode/
│  └─ settings.json
├─ build/
├─ tools/
│  ├─ init.py
│  ├─ build.py
│  ├─ _common.py
│  ├─ _project.py
│  ├─ _gen_umbrella.py
│  └─ templates/
│     ├─ project.json.in
│     ├─ CMakeLists.txt.in
│     ├─ settings.json.in
│     ├─ source_ywlib.cpp.in
│     └─ source_ywxlib.cpp.in
└─ ywlib/
   ├─ yw/
   ├─ ywx/
   ├─ ywlib
   └─ ywxlib
```


## To Do

確認しました。`python tools/build.py` は成功しています。表示崩れにつながる `PushLayer/PopLayer` 不均衡は直っていました。

残っていそうな問題はこのあたりです。

1. [window.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/window.h:409)
   `handle_button_event()` で `focused_control_id` / `mouse_capture_control_id` を変更しているのに `dirty = true` していません。
   そのため focus overlay / press overlay が、control 側が `make_dirty()` しない限り即時反映されない可能性があります。down/up、focus 変更、capture release 時は window を dirty にした方がよさそうです。

2. [wndproc.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/wndproc.h:48) / [wndproc.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/wndproc.h:63)
   `WM_MOUSELEAVE` と focus loss 時の hover leave 座標がまだ `cursor_pos - wsp->pos` です。
   他の箇所では `cursor_pos - pos - frame_thickness.xy()` に直っているので、ここも client 座標に合わせるべきです。

3. [window.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/window.h:449)
   `handle_double_click_event()` が `hovered_control_id = csp->hittest(e.pos);` を直接書き換えています。
   通常の hover 更新経路 `update_hovered_control()` を迂回するので、tooltip / hover enter/leave / dirty とずれる可能性があります。ここは local `hit_id` で処理するだけにした方がいいです。

4. [window.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/window.h:1018)
   `draw_foreground()` の focus overlay が `fill_geometry()` なので、focus された control の文字・内容・枠線を上から塗ります。しかも `focus_overlay_color = Theme.accent` で alpha が強そうです。
   focus は `stroke_geometry()` にするか、薄い alpha の overlay にするか、content 前の `draw_overlay()` 側へ移す方が安全です。

5. [window.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/window.h:1031)
   `draw_overlay()` は window の `hover_overlay_color` / `press_overlay_color` だけを使っていて、`control::slot::hover_overlay_color` が実質使われていません。
   [control.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/control.h:425) の setter や [ui_label.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/ui_label.h:19) の設定が効かない状態です。window グローバルに寄せるなら control 側の色は消す、control 個別色を残すなら `draw_overlay()` でそちらを見る、のどちらかに寄せたいです。

6. [ui_layout.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/ui_layout.h:33)
   `detach()` が子 control を `clear_window_state()` せずに erase しています。
   detach 対象が focused / hovered / captured / tooltip 対象だった場合、window 側に古い slotid が残ります。世代チェックで致命傷は避けられますが、leave/reset が飛ばない・tooltip 状態が残るなどのズレが出ます。

7. [wndproc.h](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/wndproc.h:234)
   `WM_CAPTURECHANGED` で `mouse_capture_control_id` を消しますが、`dirty = true` していません。base `reset_state()` は何もしないので、press overlay を window 側で描く設計ならここも dirty にした方がよさそうです。

優先度は 1、2、4、5 が高めです。今の最低限構成なら 1 と 4 は後で操作系 control を戻した瞬間に目立ちそうです。
