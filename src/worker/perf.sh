#!/bin/bash

for t in test/test*.sh; do
	echo $t
	\time ./$t > $t.txt 2>&1
done
