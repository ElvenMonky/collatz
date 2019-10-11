#!/bin/bash

export SERVER_NAME=pcbarina.fit.vutbr.cz

cd ~/collatz-$(hostname -s)/src

make -C worker clean all
make -C gpuworker clean all || echo "gpuworker cannot be build :("
make -C mclient clean all

cd mclient

screen -d -m ./spawn.sh
