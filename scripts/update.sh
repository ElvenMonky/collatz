#!/bin/bash

cd ~/collatz-$(hostname -s)/

git pull

cd

killall mclient

sleep 1

killall worker gpuworker

sleep 1

~/collatz-$(hostname -s)/scripts/bootstrap.sh
