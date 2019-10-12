#!/bin/bash

cd "$(dirname "$0")"

git pull

cmake . -DCMAKE_BUILD_TYPE:STRING=Release

make -j 3

sudo systemctl restart ecui-llserver.service


