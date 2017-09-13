#! /bin/bash

# This is a placeholder only!

templateFile=$1
nDriftVolumes=$[$2-1]

outputFile=`echo ${templateFile} | sed -e s/\.xml//`

for i in `seq 0 ${nDriftVolumes}`
    do sed -e s/VOLUMEID/${i}/ ${templateFile} > ${outputFile}${i}.xml
done
