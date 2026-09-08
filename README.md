---
title: PositiveLinter
lang: ja
date: 2026-09-08
tags: [unreal-engine, ue5, linter, asset-validation]
status: active
---

# PositiveLinter

PositiveLinter は、Unreal Engine 5.7.4 向けのアセット検証・命名規約チェック用エディタープラグインです。UE 4.27 向けに開発されていた Linter を引き継ぎ、Niagara や MetaSound を含む UE5 のアセットを検証できるようにしています。UE 5.7.4 の Win64 Development Editor でビルドと自動テストを実施しました。

既存プロジェクトのルールセットを読み込めるよう、従来の `Linter` および `GamemakinLinter` モジュール名は維持しています。旧 `/Linter/...` パッケージパスはリダイレクトにより `/PositiveLinter/...` へ移行されます。

## 主な機能

- Content Browser のフォルダーを右クリックして、選択範囲をスキャン
- Marketplace Guidelines、Gamemakin Style Guide、UE5 Style Guide のルールセットを選択
- UE5 アセットの命名規約、ネイティブデータ検証、Niagara System のコンパイル検証
- CI 向けの Commandlet 実行と JSON／HTML レポート出力
- 検索、フィルター、ソート、詳細表示、CSV 出力を備えた単一HTMLレポート

### UE5 Style Guide

同梱の **PositiveLinter UE5 Style Guide (UE 5.7.4)** は、次の18種類のアセットに初期の命名規約と検証ルールを用意しています。

| 分類 | 対応アセット |
| --- | --- |
| VFX・オーディオ | Niagara System、Niagara Emitter、MetaSound Source、MetaSound Patch |
| 入力・リグ | Input Action、Input Mapping Context、Control Rig、IK Rig、IK Retargeter |
| プロシージャル・ゲームプレイ | PCG Graph、PCG Graph Instance、PCG Data Asset、StateTree、Data Layer Asset |
| 映像・物理・アニメーション検索 | Level Sequence、Geometry Collection、Pose Search Database、Pose Search Schema |

対象クラスに固有のルールがない場合は、既存の Gamemakin ルールセットをフォールバックとして使用します。プロジェクトの命名規約やクラス別ルールは、同梱プリセットをプロジェクト内へ複製して調整できます。詳しくは[UE5 アセット用ルールの利用と拡張](docs/ue5rules.md)を参照してください。

## 動作環境

- Unreal Engine **5.7.4**
- プラグインを追加・ビルドできる Unreal Engine プロジェクト
- 対応アセットを検証する場合は、そのアセットを提供するUEプラグインをプロジェクト側で有効化

エディターモジュールは Win64、Mac、Linux を対象にしています。Niagara は PositiveLinter の依存プラグインとして有効になります。MetaSound、PCG、Control Rig、Pose Search などは、それぞれを使うプロジェクトで有効化してください。

## インストール

1. 起動中の Unreal Editor を閉じます。
2. このリポジトリの `Plugins/PositiveLinter` を、対象プロジェクトの `Plugins/PositiveLinter` へコピーします。
3. `.uproject` を開きます。C++ プロジェクトでモジュールの再ビルドを求められた場合は実行します。必要に応じてプロジェクトファイルも再生成してください。
4. Unreal Editor の **Edit → Plugins** で `PositiveLinter` が有効になっていることを確認し、求められた場合はエディターを再起動します。

より詳しい画面付きの手順は[はじめに](docs/gettingstarted.md)を参照してください。

## 使い方

1. Content Browser で検証対象のフォルダーを右クリックします。
2. **Scan with PositiveLinter** を選びます。
3. 使用するルールセットを選択して実行します。Niagara、MetaSound などのUE5アセットには **PositiveLinter UE5 Style Guide (UE 5.7.4)** を選びます。
4. スキャン完了後の Lint Report で指摘を確認します。

HTMLレポートは、エラー・警告・情報を明確に表示し、アセット名、パス、種類、ルール、修正案で検索できます。重要度・アセット種類・ルールグループのフィルター、列ソート、25／50／100件ごとのページング、詳細パネル、絞り込み結果のCSV出力、ライト／ダークテーマ切り替えを利用できます。外部ライブラリやネット接続を必要としない単一ファイルです。

実データでは、2,793アセット・4,050件の指摘を保持したまま、旧HTMLレポートを約6.49 MBから約0.60 MBへ削減しました。レポートの利用方法と旧レポートの変換方法は[HTML レポートの使い方](docs/report.md)にまとめています。

## Commandlet で実行する

CIやバッチ処理では、次のように Commandlet を実行できます。エンジンとプロジェクトのパスは実際の環境に置き換えてください。

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Projects\MyProject\MyProject.uproject" -run=Linter "/Game" "-RuleSet=ue5.style" "-html=ue5-lint.html" -unattended -nop4
```

`-html` を単独で指定すると、`Saved/LintReports` に日時付きHTMLレポートを出力します。`"-html=ue5-lint.html"` のようにファイル名を指定した場合は、同フォルダー基準の相対パスとして出力します。JSONも必要な場合は `"-json=quality-report.json"` を追加してください。ログの `Using rule set:` で `UE5LintRuleSet` が選択されたことを確認できます。

終了コードは、正常終了が `0`、処理の失敗が `1`、Lintエラーの検出が `2` です。警告もCIの失敗条件にする場合は `-TreatWarningsAsErrors` を追加します。

## ルールをプロジェクト向けに調整する

Plugin Content の `UE5Linter` フォルダーには、次の初期設定アセットが含まれます。

| アセット | 役割 |
| --- | --- |
| `UE5LintRuleSet` | アセットクラスと実行する検証ルールの対応表 |
| `UE5NamingConvention` | クラス別の命名プレフィックスとサフィックス |

これらを `/Game/Lint/` などプロジェクトの Content 内に複製して編集してください。表示・複製するには Content Browser の **Settings → Show Plugin Content** を有効にします。プラグイン更新後も独自設定を維持できます。BlueprintまたはC++で新しいルールを追加する方法は[UE5 アセット用ルールの利用と拡張](docs/ue5rules.md)に記載しています。

## 検証範囲に関する注意

- 初期プレフィックスはプロジェクト向けの提案です。エンジンが要求する名称ではありません。
- MetaSound はアセット自身のネイティブ `IsDataValid` を実行しますが、グラフ全体のコンパイル検証や Editor Validator／Validator Blueprint の実行は行いません。
- Niagara のコンパイル検証は Niagara System のみを対象とし、現在のエディタープラットフォームとスケーラビリティ設定で確認します。単独Emitterや全ターゲットプラットフォームを網羅するものではありません。
- Niagara Systemを検査するときは、`-NoShaderCompile` や `-PrecompiledShadersOnly` を指定しないでください。コンパイルが無効な場合は検証できない理由をエラーとして報告します。

## ドキュメント

| 内容 | ドキュメント |
| --- | --- |
| 導入と基本操作 | [はじめに](docs/gettingstarted.md) |
| Lintの仕組みとルール実装 | [仕組み](docs/howitworks.md) |
| UE5アセットの対応範囲と拡張 | [UE5 アセット用ルールの利用と拡張](docs/ue5rules.md) |
| HTMLレポート、CSV、旧レポート変換 | [HTML レポートの使い方](docs/report.md) |
| 全ドキュメントの入口 | [ドキュメントトップ](docs/index.md) |

## ライセンス

このプロジェクトは [MIT License](LICENSE) の下で公開されています。
