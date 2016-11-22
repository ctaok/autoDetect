#!/bin/bash
source conf/autoDetect.conf

tmp_fifofile=".tmp"
mkfifo $tmp_fifofile
exec 6<>$tmp_fifofile
rm $tmp_fifofile
THREAD_NUM=threadNum
for ((i=0; i<${THREAD_NUM}; i++)); do
	echo
done >&6

if [ ! -f $videoFile ]; then
	echo "video file is not exist!"
	exit -1;
fi
if [ ! -d $outputPath ]; then
	mkdir $outputPath
else
	mv $outputPath ${outputPath}.bak
	mkdir $outputPath
fi
echo "video file:  $videoFile"
echo "output path: $outputPath"

while read -r line; do
read -u6
{
	echo "begin to deal ${line}!"
	name=${line%.*}
	res=${name##*/}
	./bin/autoDetect $line ${outputPath}/${res}
	echo "finish ${line}!"
	echo >&6
} &
done < $videoFile

wait
exec 6>&-
