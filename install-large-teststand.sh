#!/bin/bash

cd "$(dirname "$0")"

sudo apt-get install cmake

sudo cp ecui-llserver-large-teststand.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable ecui-llserver-large-teststand.service
sudo systemctl start ecui-llserver-large-teststand.service
sudo systemctl status ecui-llserver-large-teststand.service

sudo chmod +x update.sh
bash update.sh


