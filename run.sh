#!/bin/bash

echo "||COMPILING CUBIOMES||" >&2

(cd cubiomes-viewer-bedrock/cubiomes && make > /dev/null) && \
echo "||DONE||" >&2 && \
g++ main.cpp -std=c++20 -O3 -march=native -flto \
-L./cubiomes-viewer-bedrock/cubiomes -lcubiomes -lm \
-o program && ./program "$@"
