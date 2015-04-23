#!/bin/bash
fileList="api-version.c"
for item in $(echo $fileList)
do
	[ -f $item ] && touch $item
	echo touching $item
	find obj_hadcon_2 -name ${item%%\.*}.o -delete
	find obj_hadcon_2 -name ${item%%\.*}.lst -delete
done
