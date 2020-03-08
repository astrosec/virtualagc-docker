#replace the first occurance of line that begins with ORIG_VAL with NEW_VAL

if [ -z "${DEPLOY_ENV}" ]; then
  NEW_VAL="PI/16           2DEC    9999.141592653 B-4"
else
  NEW_VAL="PI/16           2DEC    ${DEPLOY_ENV}"
fi

echo ${NEW_VAL}


ORIG_VAL="3.141592653 B-4"
#NEW_VAL="PI/16           2DEC    999.141592653 B-4"

sed -i "/$ORIG_VAL/c $NEW_VAL" infile.txt
