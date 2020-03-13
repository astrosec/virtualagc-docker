#!/bin/bash
#This script gets kicked off upon docker container startup and will use the
#key which is within the $DEPLOY_ENV variable as a substitute for the value
#of PI in the TIME_OF_FREE_FALL.agc ROM file.  We are using the Comanche055
#Version of the AGC ROM which flew/ran on the Apollo 11 Command Module.  
# 
cd virtualagc/Comanche055 

#Put a random number of variables/padding so the address of PI is not always the same
RANGE=5
FLOOR=1
number=0

number=0   #initialize
while [ "$number" -le $FLOOR ]
do
  number=$RANDOM
  let "number %= $RANGE"  # Scales $number down within $RANGE.
done

for (( c=1; c<=$number; c++ ))
do

sed -i   '/RTMUM/ {a\
PI'"$c"'		2DEC	3.0 B-4
}' TIME_OF_FREE_FALL.agc

done


if [ -z "${DEPLOY_ENV}" ]; then
  NEW_VAL="PI/16		2DEC	3.241592653 B-4"
else
  NEW_VAL="PI/16		2DEC	3.${DEPLOY_ENV} B-4"
fi

echo ${NEW_VAL}

ORIG_VAL="3.141592653 B-4"

sed -i "/$ORIG_VAL/c $NEW_VAL" TIME_OF_FREE_FALL.agc 

#compile the new code and produce and html symbol file
../yaYUL/yaYUL --html MAIN.agc
#replace the AGC ROM
cp MAIN.agc.bin Comanche055.bin
#no idea why it needs to be copied here, but this allows us to avoid a complete rebuild
cp MAIN.agc.bin ../VirtualAGC/temp/lVirtualAGC/Resources/source/Comanche055/Comanche055.bin

cd ..
#re-make everything
#make


cd yaAGC
bash

#./yaAGC --core="../Comanche055/Comanche055.bin" --port=19697 --cfg="../yaDSKY/src/LM.ini" --cfg=../yaDSKY2/CM.ini
