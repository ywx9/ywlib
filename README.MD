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

確認しました。python tools/build.py は通っています。対象は ywlib/ywxlib で有効なヘッダだけに絞りました。
問題がありそうな場所:
[control.h (line 204)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/control.h:204)
draw_foreground() の境界線描画条件が逆に見えます。
border_color.a >= 0.0f で return しているため、通常の alpha 値では常に border が描画されません。たぶん <= 0.0f のつもり。

[window.h (line 419)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/window.h:419)
マウスキャプチャ対象が hit_id、つまり is_focusable() な control に限定されています。
そのため button_event は持つが focus_event を持たない control は、down は届いても up/click/drag が届かない可能性があります。現在有効な label はまさにこの影響を受けやすいです。

[window.h (line 416)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/window.h:416)
button 処理中に hovered_control_id を直接書き換えていますが、hover_event::leave/enter や tooltip reset を通していません。
update_hovered_control() と状態更新経路が分岐しているので、hover 状態・tooltip・dirty がずれる可能性があります。

[window.h (line 331)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/window.h:331) / [wndproc.h (line 48)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/wndproc.h:48)
cursor_pos - pos は client 座標ではなく、frame 分ずれそうです。
button/wheel は client 座標を作っていますが、dirty 時の hover 再判定や mouse leave の座標は frame_thickness.xy() を引いていないように見えます。

[window.h (line 503)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/window.h:503) / [window.h (line 511)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/window.h:511)
window::pointer_event() と window::wheel_event() の setter/getter はありますが、control が処理しなかった場合に window 側 callback へ fallback していません。
button_event と key_event には fallback があるので、ここだけ機能していない可能性が高いです。

[wndproc.h (line 191)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/wndproc.h:191)
pointer_event 型と handle_pointer_event() はありますが、WM_POINTER* 系の dispatch がありません。現状 pointer_event API は実質未使用に見えます。

[control.h (line 152)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/control.h:152) / [control.h (line 160)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/control.h:160)
hittest() と is_focusable() が enabled を見ていません。無効化した control に hover/click/focus が入る可能性があります。find_next_tabstop() も visible/enabled を見ないので、focus_event を付けた非表示 control が Tab 対象になる可能性があります。

[control.h (line 124)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/control.h:124) / [ui_label.h (line 19)](C:/Users/AA1W017.KOBELCO/workspace/ywlib/ywlib/ywx/ui_label.h:19)
hover_overlay_color は設定されていますが、有効範囲の label/layout では draw_overlay() が未実装なので hover 表示は出ません。イベントだけのテストなら問題なしですが、見た目で hover を確認する用途では機能していません。

ビルドは成功しているので、まず直すなら 1、2、4、5 が優先度高そうです。
