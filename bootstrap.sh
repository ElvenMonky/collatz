#!/bin/bash

cd src

make -C task
make -C client

cd client

screen -d -m ./spawn.sh
