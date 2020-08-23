#!/bin/sh

# カレントディレクトリの設定
cd `dirname $0`

# buildディレクトリを作成し、移動
mkdir build
cd build

# プロジェクトの作成
cmake -DCMAKE_BUILD_TYPE=Release ..