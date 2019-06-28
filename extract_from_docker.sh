#! /bin/bash

set -o errexit

docker create --name carender-ci carender-ci

rm -rf from_docker
mkdir ./from_docker

docker cp carender-ci:/app/bin/ ./from_docker/bin
docker cp carender-ci:/app/lib/ ./from_docker/lib
docker cp carender-ci:/app/test_coverage/ ./from_docker/test_coverage
docker cp carender-ci:/app/docs-doxygen/ ./from_docker/docs-doxygen

echo Artifacts extracted from docker to ./from_docker/
