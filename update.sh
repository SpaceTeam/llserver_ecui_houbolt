#!/bin/bash

cd "$(dirname "$0")"

git pull

exec ./install.sh
