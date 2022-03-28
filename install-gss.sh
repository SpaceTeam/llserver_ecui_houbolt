#!/bin/bash

cd "$(dirname "$0")"

[ ! -d "./.git/logs" ] && echo "/.git/logs directory doesn't exist! Create it!" && exit 1

sudo apt install -y cmake

sudo cp ecui-llserver-gss.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable ecui-llserver-gss.service
sudo systemctl start ecui-llserver-gss.service
sudo systemctl status ecui-llserver-gss.service

sudo chmod +x update.sh
bash update.sh

