#!/bin/sh

olddir=$PWD
scriptdir=$(dirname "$(readlink -f -- "$0")")
if [ -z "$scriptdir" ]; then
	echo Unable to get script dir, exiting!
	exit
fi
cd $scriptdir
./libs/build_all.sh
./libaroma.sh
./app.sh test
cd $olddir
