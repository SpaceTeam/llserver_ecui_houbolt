#!/bin/bash

echo "hello world!\n";
echo $1;

pushd $1;

ls -la;

popd;
