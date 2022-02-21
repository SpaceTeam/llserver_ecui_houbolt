#!/bin/bash

cd "$(dirname "$0")"

git pull

cmake . -DCMAKE_BUILD_TYPE:STRING=Release -DNO_CANLIB:BOOL=True

make -j 3

sudo systemctl restart ecui-llserver-large-teststand.service
sudo systemctl restart ecui-llserver-small-teststand.service
sudo systemctl restart ecui-llserver-small-oxfill.service
sudo systemctl restart ecui-llserver-gss.service

