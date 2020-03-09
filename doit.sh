#!/bin/sh

cd virtualagc/Comanche055 

if [ -z "${DEPLOY_ENV}" ]; then
  NEW_VAL="PI/16		2DEC	3.241592653 B-4"
else
  NEW_VAL="PI/16		2DEC	3.${DEPLOY_ENV} B-4"
fi

echo ${NEW_VAL}

ORIG_VAL="3.141592653 B-4"

sed -i "/$ORIG_VAL/c $NEW_VAL" TIME_OF_FREE_FALL.agc 

../yaYUL/yaYUL MAIN.agc
cp MAIN.agc.bin Comanche055.bin
cd ..
make

cd ../yaAGC
bash

#./yaAGC --core="../Comanche055/Comanche055.bin" --port=19697 --cfg="../yaDSKY/src/LM.ini" --cfg=../yaDSKY2/CM.ini
