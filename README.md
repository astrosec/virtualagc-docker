# virtualagc-docker
Dockerfile and other scripts to dockerize the Apollo Guidance Computer

make sure doit.sh is executable, run the following as root

1. Build the image
docker build -t virtualagc2 .

2. Run it
docker run --rm -it --security-opt seccomp=unconfined virtualagc2 unshare --map-root-user --user /bin/bash
#seccomp unconfined is needed so the joystick driver in the allegro library won't crash with ASLR...yes this entire project is a horrible security mess

3. Exec into it

