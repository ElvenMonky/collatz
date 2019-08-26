#!/bin/bash

set -e
set -u

export LANG=C

MASTER_BASHPID=${BASHPID}

function die() {
	echo -e "*\n* ERROR: $@\n*"
	[[ ${BASHPID} != ${MASTER_BASHPID} ]] && kill -s SIGTERM ${MASTER_BASHPID}
	exit 1
}

LOCKFILE=/tmp/collatz.lock

function release_lock() {
	rm -f -- "${LOCKFILE}"
}

if ! (set -o noclobber ; echo > "${LOCKFILE}"); then
	die "client already running (or orphaned '${LOCKFILE}')"
fi

THREADS=$(grep processor /proc/cpuinfo | wc -l)
CLIENT=./client

if test -x "${CLIENT}"; then
	echo "executing '${CLIENT}' with ${THREADS} worker threads..."
	if \nice -n 19 "${CLIENT}" "${THREADS}"; then
		echo "shutting down ..."
	else
		release_lock
		die "'${CLIENT}' failure"
	fi
else
	release_lock
	die "cannot execute '${CLIENT}'"
fi

release_lock
echo "successfully finished"