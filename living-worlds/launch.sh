#!/bin/sh
echo $0 $*
progdir=`dirname "$0"`
cd $progdir
export LD_LIBRARY_PATH="$progdir/lib:$LD_LIBRARY_PATH"
SDL_HIDE_BATTERY=1 HOME=$progdir $progdir/living-worlds
