#!/bin/bash
fileList="api-version.c"
for item in $(echo $fileList)
do
	[ -f $item ] && touch $item
	echo touching $item
done
