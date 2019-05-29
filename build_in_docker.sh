#! /bin/bash

docker rm -f carender-ci

docker build -t carender-ci .
docker create --name carender-ci carender-ci

rm -rf from_docker
mkdir -p ./from_docker

docker cp carender-ci:/app/bin/ ./from_docker/bin
docker cp carender-ci:/app/lib/ ./from_docker/lib
docker cp carender-ci:/app/test_coverage/ ./from_docker/test_coverage

echo Artifacts extracted from docker to ./from_docker/