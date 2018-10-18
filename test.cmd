@echo on
REM Add Qt bin dir to path, using build.cmd envvar, to run tests
set PATH=%PATH%;%prefixPath%\bin
pushd build
ninja test
popd
