#!/bin/bash

cd "$(dirname "$0")"

[ ! -d "./.git/logs" ] && echo "/.git/logs directory doesn't exist! Create it!" && exit 1

sudo apt-get install cmake

sudo cp ecui-llserver-small-teststand.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable ecui-llserver-small-teststand.service
sudo systemctl start ecui-llserver-small-teststand.service
sudo systemctl status ecui-llserver-small-teststand.service

sudo chmod +x update.sh
bash update.sh


