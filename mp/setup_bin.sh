#!/bin/sh

BINDIR=$DEVEL/bin/darkness
GAMEDIR=game/game_src

ln -s $BINDIR/mapsrc $GAMEDIR/mapsrc
ln -s $BINDIR/modelsrc $GAMEDIR/modelsrc
ln -s $BINDIR/sound $GAMEDIR/sound
ln -s $BINDIR/particles $GAMEDIR/particles
ln -s $BINDIR/materialsrc $GAMEDIR/materialsrc
