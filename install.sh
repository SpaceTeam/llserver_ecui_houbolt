#!/bin/bash

cd "$(dirname "$0")"

#llserver ecui
sudo DOCKER_BUILDKIT=0 docker build \
    -t llserver_ecui -f Dockerfile .

sudo docker run \
    -v $CONFIG_DIR:/home/config_ecui/ \
    --privileged \
    --cap-add=ALL \
    -v /dev:/dev \
    -v /lib/modules:/lib/modules \
    -e "ECUI_CONFIG_PATH=/home/config_ecui" \
    -it --name llserver-ecui llserver_ecui
