#! /bin/bash

set -o errexit

CXX=${1:-g++-9}
CSTD=${2:-c++14}
COVER=${3}

docker rm -f carender-ci || true

echo docker build -t carender-ci --build-arg CXX="$CXX" --build-arg CSTD="$CSTD" --build-arg COVER="$COVER" --build-arg CI="$CI" .
docker build -t carender-ci --build-arg CXX="$CXX" --build-arg CSTD="$CSTD" --build-arg COVER="$COVER" --build-arg CI="$CI" .
