#! /bin/sh

outputDir=stdcxx/

mkdir -p $outputDir

for f in *.h; do

#This line splits the file name on the delimiter "."
baseName=`echo $f | cut -d "." -f 1`
cp $f $outputDir$baseName

done
