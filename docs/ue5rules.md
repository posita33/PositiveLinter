---
title: UE5 アセット用ルールの利用と拡張
lang: ja
date: 2026-09-07
tags: [unreal-engine, positive-linter, ue5, niagara, metasound, ruleset]
status: active
---

# UE5 アセット用ルールの利用と拡張

`UE5Linter` は、UE 5.7.4 向けのアセット検証を追加する Editor モジュールです。Niagara、MetaSound、Enhanced Input などの命名規約と、アセットクラス自身が提供するデータ検証を利用できます。Niagara System にはコンパイル検証も追加しています。

UE4 スタイルガイド由来の `GamemakinLinter` は引き続き利用できます。新しいルールセットは UE5 向けに登録したクラスを検証し、該当しないクラスには既存の Gamemakin ルールを適用します。UE 5.7.4 の全機能や、すべての配布先プラットフォームへの適合を保証するものではありません。

## 利用を開始する

1. プロジェクトで PositiveLinter を有効にし、UE 5.7.4 向けにビルドしてエディターを起動します。Niagara は PositiveLinter の依存プラグインとして有効になります。
2. Content Browser の **Settings → Show Plugin Content** を有効にします。
3. 検証対象フォルダーを右クリックし、**Scan with PositiveLinter** を選択します。
4. ランチャーで初期選択されている **PositiveLinter UE5 Style Guide (UE 5.7.4)** を使い、スキャンを実行します。

同梱プリセットはプラグインコンテンツの `UE5Linter` フォルダーにあります。

| アセット | 用途 |
| --- | --- |
| `UE5LintRuleSet` | アセットクラスと検証ルールの対応表 |
| `UE5NamingConvention` | クラス別のプレフィックス・サフィックス |

MetaSound、Enhanced Input、Control Rig、IK Rig、PCG、StateTree、Pose Search などを利用する場合は、プロジェクト側でも対応するプラグインを有効にしてください。命名規約とルールの対応表はソフトクラス参照を使うため、これらすべてをビルド依存として要求することはありません。未導入プラグインのアセットを検証できるようにする機能ではありません。

基本操作は[導入手順](gettingstarted.md)、レポートの見方は[Lint の仕組み](howitworks.md)も参照してください。

## 対応するアセットと命名規約

次の 18 クラスに、命名規約とネイティブデータ検証を登録しています。ネイティブ検証の実装がないクラスでは、データ検証をスキップします。

| アセット | UE クラス名 | 初期プレフィックス |
| --- | --- | --- |
| Niagara System | `NiagaraSystem` | `FXS_` |
| Niagara Emitter | `NiagaraEmitter` | `FXE_` |
| MetaSound Source | `MetaSoundSource` | `MSS_` |
| MetaSound Patch | `MetaSoundPatch` | `MSP_` |
| Input Action | `InputAction` | `IA_` |
| Input Mapping Context | `InputMappingContext` | `IMC_` |
| Control Rig | `ControlRigBlueprint` | `CR_` |
| IK Rig | `IKRigDefinition` | `IKR_` |
| IK Retargeter | `IKRetargeter` | `RTG_` |
| PCG Graph | `PCGGraph` | `PCG_` |
| PCG Graph Instance | `PCGGraphInstance` | `PCGI_` |
| PCG Data Asset | `PCGDataAsset` | `PCGD_` |
| StateTree | `StateTree` | `ST_` |
| Data Layer Asset | `DataLayerAsset` | `DLA_` |
| Level Sequence | `LevelSequence` | `LS_` |
| Geometry Collection | `GeometryCollection` | `GC_` |
| Pose Search Database | `PoseSearchDatabase` | `PSD_` |
| Pose Search Schema | `PoseSearchSchema` | `PSS_` |

これらは命名の提案であり、エンジンが要求する名前ではありません。`FXS_`、`FXE_`、`LS_` は [Epic Games の UE 5.7 推奨命名規約](https://dev.epicgames.com/documentation/en-us/unreal-engine/recommended-asset-naming-conventions-in-unreal-engine-projects?application_version=5.7)に合わせています。それ以外は PositiveLinter のプロジェクト向け初期設定です。チームの既存規約に合わせて変更してください。

## 追加された検証ルール

| ルールクラス | 初期重大度 | 検証内容 |
| --- | --- | --- |
| `LintRule_UE5_IsNamedCorrectly` | Warning | 設定したプレフィックスとサフィックスを、大文字・小文字を区別して検証 |
| `LintRule_UE5_DataValid` | Error | アセットの `UObject::IsDataValid` が返すエラーを報告 |
| `LintRule_NiagaraSystem_Compiles` | Error | Niagara System の未更新スクリプトをコンパイルし、完了とシステムの有効性を確認 |

命名規則はアセット自身のクラスを優先します。たとえば MetaSound Source を通常の音声アセットとして、Control Rig を一般の Blueprint として誤判定することを防ぎます。名前の変更は自動で行いません。

データ検証はゲームスレッド上で、アセットクラスのネイティブ検証を直接呼び出します。診断のない `NotValidated` はスキップし、エラーメッセージはレポートに表示します。初期設定ではネイティブ検証の警告を違反にしません。警告も対象にするには、ルールの Blueprint 子クラスで **Fail On Warnings** を有効にします。この場合、その警告もルールの重大度（初期値は Error）で報告されます。

`LintRule_UE5_DataValid` は、登録済みの Editor Validator や Editor Validator Blueprint を実行しません。MetaSound ではドキュメントの識別情報やページ設定など、UE が `IsDataValid` に実装した範囲を確認します。MetaSound グラフ全体のコンパイル検証ではありません。

Niagara のコンパイル検証は **Niagara System のみ**が対象です。VM と GPU のコンパイル完了を待ち、現在のエディタープラットフォームとスケーラビリティ設定に対する有効性を確認します。単独の Niagara Emitter や全ターゲットプラットフォームを網羅する検証ではありません。コンパイルが必要なアセットではスキャンに時間がかかることがあります。

`-NoShaderCompile` や `-PrecompiledShadersOnly` によりコンパイルが無効の場合は、検証できない理由をエラーとして報告します。Niagara System を検査する際は、これらのオプションを外してください。

## プロジェクト用に変更する

1. `UE5LintRuleSet` と `UE5NamingConvention` を、プロジェクトの Content 内（例: `/Game/Lint/`）へ複製します。
2. 複製したルールセットの **Naming Convention** に、複製した命名規約アセットを指定します。
3. 命名規約の **Class Naming Conventions** で、対象クラスの **Prefix** と **Suffix** を編集します。既存の行を編集すると、そのクラスで受け入れる命名を置き換えられます。
4. ルールセットの **Rule Set Description** を分かりやすい名前へ変更します。Commandlet でも使う場合は、**Name For Commandlet** を `project.ue5` などの一意な名前にします。
5. ランチャーで複製したルールセットを選択して実行します。

プラグインの更新後も設定を保つため、プロジェクト側の複製を編集してください。命名規約が未設定の場合、`UE5LintRuleSet` は C++ の `UUE5NamingConvention` クラスデフォルトを使用します。独自の命名規約を指定した場合、そのアセットが使用されます。

### 適用ルールの優先順位

対象クラスから親クラスへ順に調べ、最も近いクラスの設定を採用します。同じクラスに両方の設定がある場合は **Class Lint Rules Map** が **UE5 Class Lint Rules Map** より優先されます。後者は任意プラグインのクラスもソフト参照で登録できる対応表です。

どのクラスにも一致しない場合は、**Class Lint Rules Map** の `AnyObject_LinterDummyClass`、続いて **Legacy Rule Set** を参照します。Legacy の初期値は既存の `GamemakinLinterRuleSet` です。循環参照を避けるため、Legacy に別の `UE5LintRuleSet` を指定することはできません。

採用した一覧だけを実行し、親クラスや Legacy のルールを自動で追加実行することはありません。たとえば Niagara System に独自ルールを登録した場合、既存の命名・データ・コンパイル検証も必要なら、同じ一覧に含めてください。空のルール一覧を明示すると、その一致クラスの検証を無効にできます。

### Blueprint で独自ルールを追加する

1. `LintRule`、または追加済みルールを親とする Blueprint クラスをプロジェクト内に作成します。
2. **Class Defaults** で表示名・説明・重大度を設定します。UObject やエディター API に触れるルールでは、詳細設定の **Requires Game Thread** を有効にします。同梱の UE5 ルールは有効になっています。
3. 検証内容を追加する場合は **Passes Rule Internal** をオーバーライドします。違反時は `LintRuleViolation` に対象アセット・ルールクラス・推奨対応を設定し、`Out Rule Violations` に追加して `false` を返します。違反がない場合は `true` を返します。
4. 複製したルールセットの対象クラスの **Lint Rules** に、その Blueprint クラスを登録します。
5. 正常なアセットと意図的に違反させたアセットで、適用先・重大度・レポートの文言を確認します。

Blueprint の公開範囲にない UE API が必要な検証は C++ で実装し、調整値を Blueprint に公開します。詳細は[LintRule の実装](howitworks.md#lintrule-の実装)を参照してください。

## Commandlet から実行する

Windows の PowerShell で実行する例です。プロジェクトのパスを実際の `.uproject` に置き換えてください。

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Projects\MyProject\MyProject.uproject" -run=Linter "/Game" "-RuleSet=ue5.style" "-json=ue5-lint.json" -unattended
```

JSON はプロジェクトの `Saved/LintReports/ue5-lint.json` に出力します。ログの `Using rule set:` で `UE5LintRuleSet` が選択されていることを確認してください。独自プリセットには設定した名前（例: `-RuleSet=project.ue5`）を指定します。

終了コードは正常終了が `0`、処理自体の失敗が `1`、Lint エラーの検出が `2` です。命名警告も CI の失敗条件にする場合は `-TreatWarningsAsErrors` を追加します。

## C++ で拡張する際の参照先

- [UE5AssetTypes.cpp](../Plugins/PositiveLinter/Source/UE5Linter/Private/UE5AssetTypes.cpp): クラスパスと命名の初期値。ここへ追加すると、UE5 ルールセットと命名規約のネイティブ初期値に反映されます。
- [UE5LintRuleSet.cpp](../Plugins/PositiveLinter/Source/UE5Linter/Private/UE5LintRuleSet.cpp): クラス別ルールの登録、優先順位、Legacy へのフォールバック。
- [Rules](../Plugins/PositiveLinter/Source/UE5Linter/Private/Rules): 命名・データ検証・Niagara コンパイル規則の実装。
- [UE5Linter.Build.cs](../Plugins/PositiveLinter/Source/UE5Linter/UE5Linter.Build.cs): モジュール依存。別プラグインの C++ API を直接利用する際は、その依存宣言も追加します。

クラスパスや検証 API は、利用する UE 5.7.4 のエンジンソースと照合してください。ネイティブ初期値を変更した後は、保存済みのルールセット・命名規約アセットにも必要な項目が入っていることを確認します。

## 検証結果と自動テスト

UE 5.7.4（Win64 / Development Editor）でモジュールをビルドし、`PositiveLinter.UE5` の自動テスト 13 件が合格しました。テストは、命名、Blueprint 派生アセットのクラス判定、クラス別ルールの上書き、従来の Gamemakin ルールへのフォールバック、ネイティブデータ検証の正常・異常ケースを対象にしています。

実アセットでも、正常な Niagara System テンプレートの合格、Emitter を持たない空の Niagara System の不合格、Input Action の null Trigger の検出を確認しました。MetaSound Source / Patch と Input Action を新しいプリセットでスキャンし、意図的に不正な名前を付けた Input Action だけが命名警告になることも確認しています。画面表示や全アセット種別・全ターゲットプラットフォームを網羅するテストではありません。

プラグインをビルドしたプロジェクトで、自動テストを再実行できます。

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Projects\MyProject\MyProject.uproject" "-ExecCmds=Automation RunTests PositiveLinter.UE5" "-TestExit=Automation Test Queue Empty" "-ReportExportPath=D:\Projects\MyProject\Saved\UE5LinterTests" -unattended -nop4 -NullRHI -RenderOffscreen
```

テストの成否は出力先の `index.json` で確認してください。プロセスの終了コードだけでなく、`failed` が `0`、`succeeded` が `13` になっていることを確認します。
