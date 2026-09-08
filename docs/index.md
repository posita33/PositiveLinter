---
title: PositiveLinter ドキュメント
lang: ja
date: 2026-09-08
tags: [unreal-engine, positive-linter, lint]
status: active
---

# PositiveLinter と Gamemakin LLC スタイルガイドのドキュメント

PositiveLinter と [Gamemakin LLC Style Guide](http://ue4.style) の公式ドキュメントです。サポートや質問については、[Gamemakin LLC Community Discord](http://discord.gamemak.in) も参照してください。

## PositiveLinter について

<div style="position: relative; height: 0; overflow: hidden; max-width: 100%; height: auto;">
    <iframe width="640" height="320" src="https://www.youtube.com/embed/An0R9OmULO0" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
</div>

PositiveLinter は、Unreal Engine 5.7.4 向けのアセット検証・命名規約チェック用エディタープラグインです。ルールセットを使ってプロジェクトのコンテンツを自動スキャンし、規約に違反しているアセットをレポートします。Web 開発で一般的な lint の仕組みを、Unreal Engine プロジェクトにも適用できます。

標準で次の 3 種類のルールセットを同梱しています。

1. [Unreal Engine Marketplace Guidelines](https://www.unrealengine.com/marketplace-guidelines) を基にしたルールセット
2. [Gamemakin LLC Style Guide](http://ue4.style) のルールセット
3. [PositiveLinter UE5 Style Guide](ue5rules.md): Niagara、MetaSound、Enhanced Input など 18 クラスに対応した UE5 向けルールセット

導入方法は[はじめに](gettingstarted.md)、出力結果の検索・絞り込み・CSV 出力は [HTML レポートの使い方](report.md)を参照してください。HTML レポートは 1 ファイルで共有でき、ブラウザー上でオフラインで操作できます。

## Gamemakin LLC Style Guide について

PositiveLinter は複数のルールセットに対応していますが、もともとは Gamemakin LLC Style Guide を想定して開発されました。このスタイルガイドは [Michael Allar](http://www.twitter.com/michaelallar) により作成された、Unreal Engine プロジェクト向けの実践的な規約集です。プロジェクトの成長やコミュニティからのフィードバックに合わせ、よりよい整理方法を取り入れて更新されています。

このスタイルガイドは、すべてのプロジェクトに唯一の正解を示すものではありません。独自の規約をまだ持たないチームが運用の出発点として利用し、Unreal Engine 開発における共通認識をつくるためにオープンソースで公開されています。

スタイルガイド本体は [http://ue4.style](http://ue4.style) で確認できます。

## Unreal Engine Marketplace Guidelines について

Unreal Engine Marketplace 向けのコンテンツを作成する場合は、Epic の[ガイドライン](https://www.unrealengine.com/marketplace-guidelines)を確認してください。

PositiveLinter、Gamemakin LLC、および Michael Allar は、これらのガイドラインや審査結果を管理していません。PositiveLinter はプロジェクトをルールセットでスキャンし、規約に適合するための確認を支援します。最終的な要件と審査基準は必ず Epic の最新情報で確認してください。
