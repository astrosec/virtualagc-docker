#!/bin/sh
# Start the Hack-a-Sat AGC challenge.
#
# Pass a value for the fractional part of PI, or let this script generate
# one. The key is baked into the AGC ROM inside the container at startup
# (PI becomes 3.<key> instead of 3.141592653); contestants recover it by
# reading AGC memory through a DSKY connected to port 19697.
#
# float-conv-agc prints the AGC double-precision octal representation the
# contestants will find in memory — i.e. the expected solution.
set -e

make float-conv-agc

if [ -z "$1" ]; then
    echo "Generating key"
    key=$(od -An -N4 -tu4 < /dev/urandom | tr -d '[:space:]')
else
    echo "Key from command line"
    key=$1
fi

echo "key: $key"
./float-conv-agc "$key"

docker build -t hackasat-agc .
exec docker run --env DEPLOY_ENV="$key" --rm -p 19697:19697 hackasat-agc
