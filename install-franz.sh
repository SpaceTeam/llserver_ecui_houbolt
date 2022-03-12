#!/bin/bash

cd "$(dirname "$0")"

[ ! -d "./.git/logs" ] && echo "/.git/logs directory doesn't exist! Create it!" && exit 1

sudo apt-get install cmake

sudo cp ecui-llserver-franz.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable ecui-llserver-franz.service
sudo systemctl start ecui-llserver-franz.service
sudo systemctl status ecui-llserver-franz.service

sudo chmod +x update.sh
bash update.sh


