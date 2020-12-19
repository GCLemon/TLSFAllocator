#!/bin/sh

# カレントディレクトリの設定
cd `dirname $0`/build

# コンパイル
make

# dllを実行ファイルの隣に移動する
cd RELEASE

# テスト実行
./TLSFAllocatorTest