# LifeGameDxlibVersion2

DxLib を使用した Conway's Game of Life の Version 2 です。

旧 `LifeGameDxlib` の機能を段階的に移植しつつ、盤面表現を BitBoard ベースに整理していきます。

## 現在の移植範囲

- 1536 x 1536 の盤面
- Conway の Life Game (B3/S23)
- 上下左右が接続するトーラス境界
- ランダム初期配置
- DxLib による 1024 x 1024 描画
- `std::uint64_t` を用いた BitBoard 盤面

初回移植では正しさを優先し、世代更新処理は BitBoard 上のセルを1セルずつ走査しています。後続で 64 セル単位の bit-parallel 更新へ置き換える予定です。

## ビルド

Visual Studio の x64 Debug / Release を用意しています。

DxLib の参照先は MSBuild プロパティ `DxLibDir` で変更できます。未指定時は以下を使用します。

```text
C:\DxLib_VC\プロジェクトに追加すべきファイル_VC用
```
