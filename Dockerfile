FROM debian:buster

ARG CXX=g++-9
ARG CSTD=c++14
ARG COVER=1
ARG CI=0

RUN echo "deb http://deb.debian.org/debian experimental main" | tee -a /etc/apt/sources.list
RUN apt-get update && apt-get install -y make lcov doxygen $CXX

RUN mkdir /app
ADD . /app
WORKDIR /app

ENV CXX $CXX

RUN make clean && SANITIZE="-fsanitize=undefined -fsanitize=address -fsanitize=leak" make -j`nproc` all

RUN if [ "${CI}" = "1" ] ; then ASAN_OPTIONS=detect_leaks=0 ./bin/test ; else ./bin/test ; fi

RUN if [ "${COVER}" = "1" ] ; then make clean && make -j`nproc` cover ; fi

RUN make clean && make -j`nproc` lib cmd

RUN make docs
