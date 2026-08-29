#!/bin/bash
# Hack-a-Sat AGC challenge startup (modernized; original preserved at tag
# hackasat-final).
#
# Substitutes the key in $DEPLOY_ENV for the fractional part of PI in
# Comanche 055's TIME_OF_FREE_FALL.agc, pads PI's address with a random
# number of decoy constants, reassembles the rope with yaYUL, and starts
# yaAGC serving the DSKY channel protocol on port 19697.
#
# Run with CHALLENGE_SHELL=1 for an interactive shell instead of starting
# the AGC (the original behavior).
set -e
cd /virtualagc/Comanche055

# Put a random number of variables/padding so the address of PI is not
# always the same.
RANGE=5
FLOOR=1
number=0
while [ "$number" -le $FLOOR ]
do
  number=$RANDOM
  let "number %= $RANGE"
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

ORIG_VAL="3.141592653 B-4"
sed -i "/$ORIG_VAL/c $NEW_VAL" TIME_OF_FREE_FALL.agc

# Reassemble the patched rope and produce HTML symbol listings.
../yaYUL/yaYUL --html MAIN.agc > /tmp/yaYUL.log
if [ ! -s MAIN.agc.bin ]; then
  echo "yaYUL failed to produce MAIN.agc.bin; see /tmp/yaYUL.log" >&2
  exit 1
fi
cp MAIN.agc.bin Comanche055.bin
echo "Comanche 055 reassembled with challenge key in place (padding=$number)."

if [ -n "${CHALLENGE_SHELL:-}" ]; then
  cd /virtualagc/yaAGC
  exec bash
fi

exec /virtualagc/yaAGC/yaAGC --exec=/virtualagc/Comanche055/Comanche055.bin \
    --port=19697 --nodebug --no-resume --quiet
