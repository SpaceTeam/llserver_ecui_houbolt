FROM ubuntu:22.10 as base

WORKDIR /app

LABEL maintainer=christofer.held@spaceteam.at

ENV TZ="Europe/Vienna"

RUN apt-get update -y && apt-get upgrade -y && apt-get install tzdata -y

RUN apt-get install git make cmake g++23 build-essential -y

RUN git clone -q https://github.com/google/googletest.git /googletest \
  && mkdir -p /googletest/build \
  && cd /googletest/build \
  && cmake .. && make && make install \
  && cd / && rm -rf /googletest

COPY . ./

RUN cmake .; make; make install

RUN ./test_llserver_ecui_houbolt

CMD ["./llserver_ecui_houbolt"]