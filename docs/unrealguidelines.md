---
title: Unreal Engine Marketplace ガイドライン
lang: ja
date: 2026-09-06
tags: [unreal-engine, positive-linter, marketplace]
status: active
---

# Unreal Engine Marketplace ガイドライン

このページでは、[Unreal Engine Marketplace Guidelines](https://www.unrealengine.com/marketplace-guidelines) を基にしたルールセットを PositiveLinter がどのように実装しているかを説明します。すべてのガイドラインに対応しているわけではありませんが、Marketplace ガイドラインや広く使われるスタイルガイドへの対応を継続的に改善することを目指しています。

追加のガイドラインに対応するための貢献に興味がある場合は、[Gamemakin LLC Community Discord](http://discord.gamemak.in) の議論へ参加してください。

## 免責事項

このルールセットのすべての検証に合格しても、Unreal Engine Marketplace でのコンテンツ採用が保証されるわけではありません。一方で、ルール違反がある場合は審査で却下される可能性が高くなります。要件は変わる可能性があるため、提出前に Epic の最新のガイドラインを必ず確認してください。

## 2.3.6 パーティクルエフェクト

### 2.3.6.b [パーティクルエミッター名は正確かつ関連性のあるものにし、単一エミッターのシステムを除いて "Particle Emitter" を使用してはならない](https://www.unrealengine.com/en-US/marketplace-guidelines#236b)

PositiveLinter では、`UParticleSystem` アセットに 2 個以上のエミッターがある場合、いずれかのエミッター名が `Particle Emitter` になっていないかを確認します。該当する場合はルール違反です。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Particles_EmitterNames.MPLR_Particles_EmitterNames'` です。

## 2.3.7 テクスチャ

### 2.3.7.a [該当する場合、各テクスチャの両辺は 2 のべき乗サイズにする（例: 1024x512、1024x4096）](https://www.unrealengine.com/en-US/marketplace-guidelines#237a)

PositiveLinter は、すべてのテクスチャの幅と高さに対して 2 のべき乗かどうかを判定します。判定に失敗したテクスチャは、`LODGROUP` がこのルールの例外として設定されているかも確認します。Marketplace ルールセットでは、`UI` `LODGROUP` のテクスチャだけを、2 のべき乗サイズでなくてもよいものとして設定しています。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Texture2D_PowerOfTwo.MPLR_Texture2D_PowerOfTwo'` です。

### 2.3.7.b [テクスチャのいずれかの辺の最大サイズは 8192](https://www.unrealengine.com/en-US/marketplace-guidelines#237b)

PositiveLinter は、テクスチャの幅または高さが 8192 を超えていないことを確認します。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Texture2D_Size_NotTooBig.MPLR_Texture2D_Size_NotTooBig'` です。

## 2.4 オーディオ

### 2.4.c [オーディオファイルのサンプルレートは 22050 Hz または 44100 Hz とし、音声の欠陥があってはならない](https://www.unrealengine.com/en-US/marketplace-guidelines#24c)

PositiveLinter は、`USoundWave` アセットのサンプルレートが 22050 または 44100 であることを確認します。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_SoundWave_SampleRate.MPLR_SoundWave_SampleRate'` です。

## 2.5 Blueprint

### 2.5.d [Blueprint には、例やチュートリアル目的でコメントされている場合を除き、接続されていないノードがあってはならない](https://www.unrealengine.com/en-US/marketplace-guidelines#25d)

PositiveLinter では、`UBlueprint` のいずれかのグラフに、ほかのノードへの接続が 1 本もないノードが存在しないかを確認します。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Blueprint_LooseNodes.MPLR_Blueprint_LooseNodes'` です。

### 2.5.e [Blueprint はエラーまたは重大な警告を発生させてはならない](https://www.unrealengine.com/en-US/marketplace-guidelines#25e)

PositiveLinter は、`UBlueprint` のコンパイル状態が `BS_Error` または `BS_UpToDateWithWarnings` でないことを確認します。つまり、その Blueprint のコンパイルボタンにエラーまたは警告アイコンが表示されない状態である必要があります。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Blueprint_Compiles.MPLR_Blueprint_Compiles'` です。

## 2.7 ファイル構造

### 2.7.1.a [フォルダー名とファイル名は、プロジェクトの文脈において正確かつ一貫した命名規約に従う必要がある](https://www.unrealengine.com/en-US/marketplace-guidelines#271a)

PositiveLinter は、すべてのアセットがルールセットの `NamingConvention` アセットで定義されたパターンに一致することを確認します。

Marketplace の `NamingConvention` アセットは `MarketplaceNamingConvention'/PositiveLinter/MarketplaceLinter/MarketplaceNamingConvention.MarketplaceNamingConvention'` です。この規約を確認するルールは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_IsNamedCorrectly.MPLR_IsNamedCorrectly'` です。

### 2.7.1.b [フォルダー名とファイル名に "Assets"、"NewFolder" などの曖昧な名称を使用してはならない](https://www.unrealengine.com/en-US/marketplace-guidelines#271b)

PositiveLinter は、パスの各要素が `Assets` または `NewFolder` でないことを確認します。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Path_DisallowedPathNames.MPLR_Path_DisallowedPathNames'` です。

### 2.7.1.c [フォルダー名とファイル名には英数字およびアンダースコアのみを使用する](https://www.unrealengine.com/en-US/marketplace-guidelines#271c)

PositiveLinter は、各パス要素を正規表現 `[^a-zA-Z0-9_]` で確認します。英小文字 `a`〜`z`、英大文字 `A`〜`Z`、数字 `0`〜`9`、および `_` 以外の文字を含まない場合だけ有効です。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Path_AlphaNumeric.MPLR_Path_AlphaNumeric'` です。

### 2.7.2.b [Epic Games Launcher からプロジェクトファイルをインポートした後の移行競合を減らすため、プロジェクト固有のアセットは Content 直下の単一の最上位フォルダーに格納し、Content 直下には他のフォルダーやファイルを置かない](https://www.unrealengine.com/en-US/marketplace-guidelines#272b)

PositiveLinter は、アセットの親フォルダーが `/Content/` ではないことを確認します。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Path_NoTopLevelAssets.MPLR_Path_NoTopLevelAssets'` です。

### 2.7.2.d [Content 直下の最上位フォルダー名を含むすべてのアセットファイルパスは、140 文字以下でなければならない](https://www.unrealengine.com/en-US/marketplace-guidelines#272d)

PositiveLinter は、`UObject::GetPathName()` が返すアセットパスから重複するアセット名を除いた長さが 140 文字以下かを確認します。

実装するアセットは `Blueprint'/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Path_IsNotTooLong.MPLR_Path_IsNotTooLong'` です。たとえば、このアセットで使用するパス名は `/PositiveLinter/MarketplaceLinter/LintRules/MPLR_Path_IsNotTooLong` です。
