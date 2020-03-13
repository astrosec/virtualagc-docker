#!/bin/sh
#To start the challenge pass in a value for the fractional part of PI you want
# to modify.  This will be passed into the docker container where the AGC ROM
#code is then modified, then recompiled and started with the new value.  The
#contestants will use the DSKY2 to discover the new value of PI.  So instead of
#3.141592654 it will be #3.<fractional value>

#float-conv-agc will add the fractional part of PI which you passed in to 3.0
#and then convert into native AGC octal representation and print it out.  This
#is the value which will be the solution. You can pass the key in via command
#line or use /dev/urandom to generate it.  float-conv will print out the values
#which will be in the ROM

#make the float-conv-agc app
make

if [ -z $1 ]; then
    echo "Generating key"
    key=$(od -An -N4 -tu4 < /dev/urandom | tr -d [:space:])
else
    echo "Key from command line"
    key=$1 
fi

echo $key
#print out the value they will see in the DSKY2
./float-conv-agc $key

docker run --env DEPLOY_ENV=$key --rm -it -p19698:19698 -p19697:19697 --security-opt seccomp=unconfined virtualagc2 unshare --map-rt-user --user /bin/bash
