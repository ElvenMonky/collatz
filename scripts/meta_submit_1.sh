#!/bin/bash
#PBS -N collatz
#PBS -S /bin/bash
#PBS -M ibarina@fit.vutbr.cz
#PBS -m a
#
#PBS -l select=1:ncpus=1:mem=1gb:scratch_local=1gb:vnode=^zelda3,walltime=4:00:00
#PBS -R eo

HOME=/storage/brno11-elixir/home/ibarina

TMPDIR=$SCRATCHDIR

export LANG=C

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
echo "SCRATCHDIR=$SCRATCHDIR"

type 'clean_scratch' || :
trap 'clean_scratch' TERM EXIT

module load clang-9.0
module load gmp

set -u
set -e

# check the connection
if ! ping -c1 -q "${SERVER_NAME}"; then
	echo "No connection!"
	exit
fi

umask 077

CC=gcc
if type clang > /dev/null 2> /dev/null && clang --version | grep -q "version [89]"; then
        echo "INFO: clang available"
        CC=clang
fi

# don't forget git clone git@github.com:xbarin02/collatz.git into $HOME
SRCDIR=$HOME/collatz/
MAPDIR=$HOME/collatz-sieve/
TMP=$(mktemp -d collatz.XXXXXXXX --tmpdir)

echo "SRCDIR=$SRCDIR"
echo "TMP=$TMP"

mkdir -p -- "$TMP"
pushd -- "$TMP"

cp -r "${SRCDIR}" .

cd collatz/src

# build mclient & worker
make -C worker clean all USE_LIBGMP=1 CC=$CC USE_SIEVE=1 USE_PRECALC=1 USE_SIEVE3=1 USE_ESIEVE=1
make -C mclient clean all

pushd $MAPDIR
./unpack.sh esieve-32 "$TMP"/collatz/src/worker
popd

cd mclient

# limit 2 hours for the mclient; 3 hours for worker, not one-shot run
stdbuf -o0 -e0 ./mclient -a 10800 -b 7200 1

popd
rm -rf -- "$TMP"

clean_scratch
