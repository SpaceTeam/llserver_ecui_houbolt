#!/bin/bash

cd "$(dirname "$0")"

sudo apt-get install cmake

sudo cp ecui-llserver.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable ecui-llserver.service
sudo systemctl start ecui-llserver.service
sudo systemctl status ecui-llserver.service

sudo chmod +x update.sh
./update.sh


