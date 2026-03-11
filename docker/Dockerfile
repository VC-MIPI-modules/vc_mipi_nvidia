FROM ubuntu:18.04

RUN apt-get update && \
    apt-get -y install sudo wget bc xxd python3 usbutils nano cpio python iputils-ping python-pip liblz4-tool dosfstools flex bison libssl-dev python3-pip libxml2-utils kmod && \
    apt-get clean

RUN useradd -m dockervc && echo "dockervc:dockervc" | chpasswd && adduser dockervc sudo

RUN sudo -H pip install pyyaml

RUN sudo -H pip3 install pyyaml

USER dockervc

WORKDIR /src
