rem echo off
rem $Id: build.bat 3449 2005-04-25 12:07:40Z silke $
rem *** build the binaries, libs etc. for test
rem


pushd coll
call build.bat %1
popd

pushd coll
call build.bat %1
popd

pushd context
call build.bat %1
popd

pushd env
call build.bat %1
popd

pushd pt2pt
call build.bat %1
popd

pushd resources
call build.bat %1
popd

pushd topol
call build.bat %1
popd