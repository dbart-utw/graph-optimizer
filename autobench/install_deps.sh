#!/bin/bash

wget https://github.com/Kitware/CMake/releases/download/v3.29.6/cmake-3.29.6-linux-x86_64.sh
bash cmake-3.29.6-linux-x86_64.sh --skip-license --exclude-subdir --prefix=$HOME/.local
rm cmake-3.29.6-linux-x86_64.sh

cd $HOME
git clone https://github.com/DrTimothyAldenDavis/GraphBLAS.git
cd GraphBLAS/build
cmake -DCMAKE_INSTALL_PREFIX="$HOME/.local" ..
make -j 64
make install

cd $HOME
# git clone https://github.com/GraphBLAS/LAGraph.git
git clone https://github.com/tweska/LAGraph.git
cd LAGraph/build
git checkout knobel  # Contains a bugfix we need!
cmake -DCMAKE_INSTALL_PREFIX="$HOME/.local" ..
make -j 64
make install
