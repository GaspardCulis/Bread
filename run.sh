#!/bin/bash

RED=`tput setaf 1`
GREEN=`tput setaf 2`
RESET=`tput sgr0`

cd build
cmake -DGAME_VERSION=latest -DCMAKE_BUILD_TYPE=Release -DBOTCRAFT_BUILD_EXAMPLES=OFF -DBOTCRAFT_COMPRESSION=ON -DBOTCRAFT_ENCRYPTION=ON -DBOTCRAFT_USE_OPENGL_GUI=OFF ..
if [ $? -ne 0 ]; then
echo "${RED}CMake build failed${RESET}"
exit 1
fi
make -j8 all
if [ $? -ne 0 ]; then
echo "${RED}Build failed${RESET}"
exit 1
fi
echo "${GREEN}Build successful ! Running the binary...${RESET}"
cd ../bin
./Bread
