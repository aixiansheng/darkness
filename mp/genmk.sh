#!/bin/bash

STR=$1
if [ -z "$STR" ]
then
	STR=client
fi

if [ "$STR" != "client" ] && [ "$STR" != "server" ]
then
	echo "Usage: genmk.sh [client|server] < input_paths"
	exit 1
fi

while read LINE
do

DIRN=`dirname $LINE`
FILE=`basename $LINE`
SHORT=`basename $LINE .cpp`

echo "

ifneq (clean, \$(findstring clean, \$(MAKECMDGOALS)))
-include \$(OBJ_DIR)/$SHORT.P
endif

\$(OBJ_DIR)/$SHORT.o : \$(PWD)/$LINE \$(PWD)/${STR}_linux32_hl2mp.mak \$(SRCROOT)/devtools/makefile_base_posix.mak
	\$(PRE_COMPILE_FILE)
	\$(COMPILE_FILE) \$(POST_COMPILE_FILE)
"

done
