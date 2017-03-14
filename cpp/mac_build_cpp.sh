#!/bin/bash

pushd cpp
node-gyp rebuild --target=1.6.2 --arch=x64 --dist-url=https://atom.io/download/electron
popd
