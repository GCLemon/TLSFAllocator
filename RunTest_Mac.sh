#!/bin/sh

# カレントディレクトリの設定
cd `dirname $0`/build

# コンパイル
make

# 出力ディレクトリに移動
cd RELEASE

# テスト実行
./TLSFAllocatorTest