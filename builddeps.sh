#!/usr/bin/env sh

set -e

CGLM_VER=0.9.0
CGLM_PACKAGE=v${CGLM_VER}.tar.gz

mkdir -p lib/
cd lib/
curl -L -O https://github.com/recp/cglm/archive/refs/tags/${CGLM_PACKAGE}
tar -zxvf ${CGLM_PACKAGE}
mv cglm-${CGLM_VER} cglm
cd cglm
cmake . -DCGLM_STATIC=ON
make
