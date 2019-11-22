#!/bin/bash
#PBS -N collatz
#PBS -S /bin/bash
#PBS -M ibarina@fit.vutbr.cz
#PBS -m a
#
#PBS -l select=1:ncpus=2,walltime=4:00:00

HOME=/storage/brno11-elixir/home/ibarina

export SERVER_NAME=pcbarina.fit.vutbr.cz

echo "hostname=$(hostname)"
echo "pwd=$(pwd)"
echo "HOME=$HOME"
echo "cpu model name=$(cat /proc/cpuinfo | grep "model name" | head -n1)"
echo "cpus=$(cat /proc/cpuinfo | grep processor | wc -l)"
echo "TMPDIR=$TMPDIR"
echo "PBS_JOBID=${PBS_JOBID}"
# this is the cwd where qsub was executed, not cwd of the script itself
echo "PBS_O_WORKDIR=${PBS_O_WORKDIR}"

module load clang-9.0
module load gmp

set -u
set -e

export LANG=C

# check the connection
if ! ping -c1 -q "${SERVER_NAME}"; then
	echo "No connection!"
	exit
fi

umask 077

# don't forget git clone git@github.com:xbarin02/collatz.git into $HOME
SRCDIR=$HOME/collatz/
TMP=$(mktemp -d collatz.XXXXXXXX --tmpdir)

echo "SRCDIR=$SRCDIR"
echo "TMP=$TMP"

mkdir -p -- "$TMP"
pushd -- "$TMP"

cp -r "${SRCDIR}" .

cd collatz/src

# build mclient & worker
make -C worker clean all USE_LIBGMP=1 CC=clang
make -C mclient clean all

cd mclient

# no limit for mclient; 4 hours minus 100 secs for the worker
stdbuf -o0 -e0 ./mclient -a 14300 -1 2

popd
rm -rf -- "$TMP"
