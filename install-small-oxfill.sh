#!/bin/bash

cd "$(dirname "$0")"

sudo apt-get install cmake

sudo cp ecui-llserver-small-oxfill.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable ecui-llserver-small-oxfill.service
sudo systemctl start ecui-llserver-small-oxfill.service
sudo systemctl status ecui-llserver-small-oxfill.service

sudo chmod +x update.sh
bash update.sh


