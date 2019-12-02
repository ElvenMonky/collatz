#!/bin/bash

# assigned.map complete.map
test -e assigned.000.map && mv -f -- assigned.000.map assigned.001.map
test -e assigned.map && cp -- assigned.map assigned.000.map

test -e complete.000.map && mv -f -- complete.000.map complete.001.map
test -e complete.map && cp -- complete.map complete.000.map

# clientids.dat cycleoffs.dat checksums.dat mxoffsets.dat overflows.dat usertimes.dat
test -e clientids.000.dat && mv -f -- clientids.000.dat clientids.001.dat
test -e clientids.dat && cp -- clientids.dat clientids.000.dat

test -e checksums.000.dat && mv -f -- checksums.000.dat checksums.001.dat
test -e checksums.dat && cp -- checksums.dat checksums.000.dat

test -e overflows.000.dat && mv -f -- overflows.000.dat overflows.001.dat
test -e overflows.dat && cp -- overflows.dat overflows.000.dat

test -e usertimes.000.dat && mv -f -- usertimes.000.dat usertimes.001.dat
test -e usertimes.dat && cp -- usertimes.dat usertimes.000.dat
