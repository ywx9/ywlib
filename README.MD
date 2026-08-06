# ywlib / ywxlib Project Template

`ywlib` / `ywxlib` を利用する C++23 プロジェクト向けのテンプレートです。
`project.json` を編集し、`tools/init.py` と `tools/build.py` を実行するだけで、最小構成のプロジェクトを作成できます。

## できること

- `ywlib` を使った通常の C++ プロジェクト作成
- `ywxlib` を使った Windows / DirectX アプリケーション作成

## 必要環境

- Python 3
- Git
- CMake
- C++23 対応コンパイラ
- VS Code（任意）

`ywxlib` を使う場合は、Windows 上で DirectX 関連ライブラリを利用できる環境が必要です。

## クイックスタート

### 1. レポジトリをクローン

```bash
git clone https://github.com/ywx9/ywlib.git PROJECT_NAME
```

以降、生成されたディレクトリをプロジェクトディレクトリと呼ぶ。

### 2. .gitフォルダを削除 (任意)

### 3. プロジェクト初期化

プロジェクトディレクトリに移動し、

```bash
python tools/init.py
```

生成された`project.json`を目的に応じて編集する。
`project.json`の仕様については[以下](#projectjson)を参照すること。

### 4. ソースコードの作成

プロジェクトディレクトリ直下に配置すること。

### 5. ビルド

```bash
python tools/build.py
```

### 6. 再ビルド

`project.json`を変更した場合は`CMakeLists.txt`を削除すること。

```bash
python tools/build.py
```

## `project.json`

`project.json` は、このテンプレートの設定ファイルです。

例:

```json
{
  "project_name": "PROJECT_NAME",
  "source_file": "source.cpp",
  "target_type": "exe",
  "show_console": true,
  "library": "ywlib"
}
```

### 設定項目

- `project_name`: 出力ファイル名に使用
- `source_file`: エントリポイントとなるソースファイル名
- `target_type`: 出力形式 (`exe` or `dll`)
- `show_console`: falseに設定した場合、コンソールを表示しない。
- `library`: 使用するライブラリ (`ywlib` or `ywxlib`)
