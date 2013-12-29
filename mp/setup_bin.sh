#!/bin/sh

if [ -z "$BINDIR" ] || [ -z "$GAMEDST" ]
then
	echo "set BINDIR, GAMEDST before running"
	exit 1
fi

rm -rf $GAMEDST/maps
rm -rf $GAMEDST/models
rm -rf $GAMEDST/sound
rm -rf $GAMEDST/particles
rm -rf $GAMEDST/materials
rm -rf $GAMEDST/resources
rm -rf $GAMEDST/shaders

ln -s $BINDIR/maps $GAMEDST/maps
ln -s $BINDIR/models $GAMEDST/models
ln -s $BINDIR/sound $GAMEDST/sound
ln -s $BINDIR/particles $GAMEDST/particles
ln -s $BINDIR/materials $GAMEDST/materials
ln -s $BINDIR/resources $GAMEDST/resources
ln -s $BINDIR/shaders $GAMEDST/shaders
