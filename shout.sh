#!/bin/bash
cd "source"
for file in ./*/*/*.*
do
	name=${file##*/}
	fa=${file#*/}
	fa=${fa#*/}
	fa=${fa%/*}
	filename=${name%.*}
	coder=${file#*/}
	coder=${coder%%/*}
	if test "$filename" = "$fa"
	then
		cp "$file" "./$coder/$name"
	fi
done
