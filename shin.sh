#!/bin/bash
cd "source"
for file in ./*/*.*
do
	coder=${file%/*}
	name=${file##*/}
	sourcename=${name%.*}
	if [ -a "$coder/$sourcename" ]; then
		if [ ! -d "$coder/$sourcename" ];then
			rm "$coder/$sourcename"
			mkdir "$coder/$sourcename"
		fi
	else mkdir "$coder/$sourcename"
	fi
	if [ ! -a "$coder/$sourcename/$name" ];then
		cp "$file" "$coder/$sourcename/$name"
	fi
done
