#replace the first occurance of line that begins with ORIG_VAL with NEW_VAL

if [ -z "${DEPLOY_ENV}" ]; then
  NEW_VAL="PI/16		2DEC	3.241592653 B-4"
else
  NEW_VAL="PI/16		2DEC	3.${DEPLOY_ENV} B-4"
fi

echo ${NEW_VAL}

ORIG_VAL="3.141592653 B-4"

sed -i "/$ORIG_VAL/c $NEW_VAL" infile.txt 


