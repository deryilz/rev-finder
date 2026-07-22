#!/bin/bash

echo "||COMPILING||" >&2

(cd cubiomes-viewer-bedrock/cubiomes && make > /dev/null) && \
echo "||DONE WITH CUBIOMES||" >&2 && \
g++ main.cpp -std=c++20 -O3 -march=native -flto=auto \
-L./cubiomes-viewer-bedrock/cubiomes -lcubiomes -lm \
-o program && \
echo "||DONE||" >&2 && \
./program "$@"
