# specify the node base image with your desired version node:<version>
FROM ubuntu


WORKDIR /home


### KVASER Driver

RUN apt-get update
RUN apt-get install -y build-essential 
RUN apt-get install -y linux-headers-`uname -r` 
RUN apt-get install -y cmake make
RUN apt-get install -y wget

RUN wget --content-disposition "https://www.kvaser.com/downloads-kvaser/?utm_source=software&utm_ean=7330130980754&utm_status=latest"
RUN tar xvzf linuxcan.tar.gz
WORKDIR /home/linuxcan/canlib
RUN make
RUN make install
RUN /usr/doc/canlib/examples/listChannels

WORKDIR /home/

# Clone the conf files into the docker container
ADD ./ /home/llserver_ecui_houbolt
WORKDIR /home/llserver_ecui_houbolt

RUN git submodule init
RUN git submodule update

RUN mkdir -p build
WORKDIR /home/llserver_ecui_houbolt/build

RUN cmake -D NO_PYTHON=true -S ../ -B  ./
RUN make -j

ENV ECUI_CONFIG_PATH=/home/config_ecui

ENTRYPOINT ./llserver_ecui_houbolt