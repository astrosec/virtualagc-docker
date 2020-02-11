# virtualagc-docker
Dockerfile and other scripts to dockerize the Apollo Guidance Computer

make sure doit.sh is executable, run the following as root

1. Build the image
docker build -t virtualagc2 .

2. Run it
#seccomp unconfined is needed so the joystick driver in the allegro library won't crash with ASLR...  Use -p to work with EXPOSE directive and make those ports available outside the docker host

docker run --rm -it -p19698:19698 -p19697:19697 --security-opt seccomp=unconfined virtualagc2 unshare --map-rt-user --user /bin/bash



3. Exec into it

