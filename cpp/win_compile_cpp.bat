@echo off

pushd cpp

if "%1"=="" goto rebuild

if "%1"=="quick" goto quick else goto end

:rebuild
node-gyp rebuild --debug -j 4 --target=1.6.2 --arch=x64 --dist-url=https://atom.io/download/electron

:quick
node-gyp build --debug -j 4 --target=1.6.2 --arch=x64 --dist-url=https://atom.io/download/electron

:end

popd

echo on
