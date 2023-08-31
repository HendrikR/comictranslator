#!/bin/bash

IMAGE=comictranslator
docker build --build-arg UID=$UID --build-arg GROUPS=$GROUPS --build-arg USER=$USER . -t $IMAGE
if [ $? != 0 ]; then echo "build faled"; exit; fi

echo "running image $IMAGE"


# quite hacky/cool way to solve the xauth problem: make xauth ignore the hostname.
# See https://stackoverflow.com/questions/16296753/can-you-run-gui-applications-in-a-linux-docker-container
XSOCK=/tmp/.X11-unix
XAUTH=/tmp/.docker.xauth
xauth nlist $DISPLAY | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -
XFORWARDING_OPTS="--net=host -e DISPLAY=$DISPLAY -v $XSOCK -v $XAUTH:$HOME/.Xauthority"
docker run -t -v .:/comictranslator $XFORWARDING_OPTS $IMAGE
