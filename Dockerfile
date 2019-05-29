FROM debian:stretch

RUN apt-get update && apt-get install -y build-essential lcov

RUN mkdir /app
ADD . /app
WORKDIR /app

ENV CC gcc

RUN make clean && SANITIZE="-fsanitize=undefined -fsanitize=address -fsanitize=leak" make -j`nproc` all
RUN ./bin/test

RUN make clean && make -j`nproc` cover

RUN make clean && make -j`nproc` lib cmd
