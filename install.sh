#!/bin/bash

cd "$(dirname "$0")"

[ ! -d "./.git/logs" ] && echo "/.git/logs directory doesn't exist! Create it!" && exit 1

sudo apt install -y cmake

sudo cp ecui-llserver.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable ecui-llserver.service
sudo systemctl start ecui-llserver.service
sudo systemctl status ecui-llserver.service

sudo chmod +x update.sh
bash update.sh

