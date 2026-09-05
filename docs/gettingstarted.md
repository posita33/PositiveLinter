---
title: はじめに
lang: ja
date: 2026-09-06
tags: [unreal-engine, positive-linter, installation]
status: active
---

# はじめに

## 必要な環境

* Unreal Engine **5.7.4**
* プロジェクトを編集できる権限

## インストール

1. 起動中の Unreal Editor をすべて閉じます。
2. [`Plugins/PositiveLinter`](../Plugins/PositiveLinter) フォルダーを、対象プロジェクトの `Plugins` フォルダーへコピーします。
3. 必要に応じてプロジェクトファイルを再生成します。
4. プロジェクトを Unreal Editor で開きます。

![](img/LinterLauncher.png)

> この画像は旧 Linter 版のものです。現在のソース版はプロジェクトへ直接コピーして導入します。

## PositiveLinter を有効にする

1. 対象プロジェクトを開きます。
2. メインツールバーで **Edit** を開き、**Plugins** を選択します。
3. `PositiveLinter` を検索します。
4. **Enabled** をオンにします。
5. エディターを再起動します。

## PositiveLinter を使う

導入後の操作は次のとおりです。

1. Content Browser でスキャン対象のフォルダーを右クリックします。
2. **Scan with PositiveLinter** を選択します。
3. 使用するルールセットを選択します。
4. スキャンが完了するまで待ちます。

![](img/ScanWithLinter.png)

> この画像のメニュー名は旧版の **Scan with Linter** です。UE 5.7.4 対応版では **Scan with PositiveLinter** と表示されます。

## Lint Report

スキャンが完了すると、プロジェクト全体の状態をまとめた **Lint Report** が表示されます。

![](img/LintReport.png)
