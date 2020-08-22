#!/bin/sh

# カレントディレクトリの設定
cd `dirname $0`/build

# コンパイル
make

# dllを実行ファイルの隣に移動する
mv src/libTLSFAllocator.dylib test

# 実行ファイルのある場所に移動
cd test

# テスト実行
./TLSFAllocatorTest