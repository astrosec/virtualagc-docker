# virtualagc-docker
Dockerfile and other scripts to dockerize the Apollo Guidance Computer

make sure doit.sh is executable, run the following as root

1. Build the image
docker build -t virtualagc2 .

2. Run it
#seccomp unconfined is needed so the joystick driver in the allegro library won't crash with ASLR...  Use -p to work with EXPOSE directive and make those ports available outside the docker host

docker run --rm -it -p19698:19698 -p19697:19697 --security-opt seccomp=unconfined virtualagc2 unshare --map-rt-user --user /bin/bash

3. Exec into it

Bonus--Forward X display to your host and run yaDSKY2 control panel

For macOS install XQuartz and on Linux it should work by default.  
for XQuartz allow remote connections 
XQuartz->X11 Preferences->Authenticate connections (checked)
XQuartz->X11 Preferences->Allow connections from network clients (checked)
Open an xterm on the host
xhost +        (allow connections from all X clients)

In your host VM (where docker lives)
docker run --rm -it -p19698:19698 -p19697:19697 --security-opt seccomp=unconfined virtualagc2 unshare --map-rt-user --user /bin/bash
docker ps (gather the container id)
docker exec -it <container id like 87917890f14d) /bin/bash

====in the docker container
DISPLAY=10.20.2.86:0.0
export DISPLAY
cd virtualagc/yaDSKY2
./yaDSKY2


