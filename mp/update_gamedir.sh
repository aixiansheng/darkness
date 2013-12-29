#!/bin/sh

rsync -r $BINDIR/maps/ $GAMEDST/maps
rsync -r $BINDIR/models/ $GAMEDST/models
rsync -r $BINDIR/sound/ $GAMEDST/sound
rsync -r $BINDIR/particles/ $GAMEDST/particles
rsync -r $BINDIR/materials/ $GAMEDST/materials
rsync -r $BINDIR/shaders/ $GAMEDST/shaders
rsync -r $BINDIR/resource/ $GAMEDST/resource
