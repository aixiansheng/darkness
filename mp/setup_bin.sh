#!/bin/sh

if [ -z "$BINDIR" ]
then
	echo "set BINDIR before running"
	exit 1
fi

GAMEDIR=game/game_src

ln -s $BINDIR/mapsrc $GAMEDIR/mapsrc
ln -s $BINDIR/modelsrc $GAMEDIR/modelsrc
ln -s $BINDIR/sound $GAMEDIR/sound
ln -s $BINDIR/particles $GAMEDIR/particles
ln -s $BINDIR/materialsrc $GAMEDIR/materialsrc
