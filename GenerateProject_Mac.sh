#!/bin/sh

# カレントディレクトリの設定
cd `dirname $0`

# ディレクトリの移動
if [ ! -e build ]; then
    mkdir build
fi
cd build

# プロジェクトの作成
cmake ..