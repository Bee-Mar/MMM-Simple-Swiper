#/usr/bin/env bash

if [[ -f "./install-dependencies.sh" ]];
then
  source install-dependencies.sh
  cd ../
else
  source scripts/install-dependencies.sh
fi

rm -rf build
mkdir build
cd build
cmake ..
