FROM ubuntu:22.10 as base

WORKDIR /app

LABEL maintainer=christofer.held@spaceteam.at

ENV TZ="Europe/Vienna"

RUN apt-get update -y && apt-get upgrade -y && apt-get install tzdata -y

RUN apt-get install git make cmake g++23 build-essential libcurl4-openssl-dev -y

RUN git clone https://github.com/offa/influxdb-cxx  \
    && cd influxdb-cxx  \
    && mkdir build; cd build \
    && cmake -D INFLUXCXX_TESTING:BOOL=OFF -D INFLUXCXX_WITH_BOOST:BOOL=OFF .. \
    && make install \
    && cd ..; cd .. \
    && rm -r influxdb-cxx

COPY . ./

RUN cmake .; make; make install

RUN ./test_llserver_ecui_houbolt

CMD ["./llserver_ecui_houbolt"]

