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
