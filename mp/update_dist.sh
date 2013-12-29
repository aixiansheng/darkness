#!/bin/sh

DIST=$1

if [ -z "$BINDIR" ] || [ -z "$GAMEDST" ]
then
	echo "set BINDIR, GAMEDST before running"
	exit 1
fi

if [ -z "$1" ]
then
	echo "specify a directory to place dist files"
	exit 1
fi

check_and_link() {
	if [ ! -e "$DIST/$1" ]
	then
		ln -s $BINDIR/$1 $DIST/$1 || exit 1
	fi
}

mkdir -p $DIST/bin

check_and_link maps
check_and_link models
check_and_link sound
check_and_link particles
check_and_link materials
check_and_link resource
check_and_link shaders
check_and_link scripts
check_and_link cfg

rsync $BINDIR/gameinfo.txt $DIST/gameinfo.txt
rsync $BINDIR/darkness.fgd $DIST/darkness.fgd
