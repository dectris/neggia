# MIT License
#
# Copyright (c) 2017 DECTRIS Ltd.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#!/usr/bin/env bash

SOURCE_DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DOCKER_DIR=${SOURCE_DIR}
BUILD_DIR=/tmp/neggia-docker-build
mkdir -p ${BUILD_DIR}
docker build -t dectris-neggia-builder ${DOCKER_DIR}
docker run -t -v ${SOURCE_DIR}:/src  -v ${BUILD_DIR}:/build dectris-neggia-builder \
    /bin/bash -c "cd /build && cmake3 /src && make -j4"
cp ${BUILD_DIR}/src/dectris/neggia/plugin/*.so .
