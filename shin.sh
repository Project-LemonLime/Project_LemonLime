#!/bin/bash
cd "source"
for file in ./*/*.*
do
	mulu=${file%/*}
	name=${file##*/}
	sourcename=${name%.*}
	if [ -a "$mulu/$sourcename" ]; then
		if [ ! -d "$mulu/$sourcename" ];then
			rm "$mulu/$sourcename"
			mkdir "$mulu/$sourcename"
		fi
	else mkdir "$mulu/$sourcename"
	fi
	if [ ! -a "$mulu/$sourcename/$name" ];then
		cp "$file" "$mulu/$sourcename/$name"
	fi
done
