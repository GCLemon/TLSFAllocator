rem カレントディレクトリの設定
cd %~dp0

rem buildディレクトリを作成し、移動
mkdir build
cd build

rem プロジェクトの作成
cmake ..