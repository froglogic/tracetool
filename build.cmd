@echo on
if "%~1" == "" goto usage
if "%~2" == "" goto usage

if "%~2" == "true" (
    set buildType=Release
    set buildTarget=package
) else (
    if "%~2" == "false" (
        set buildType=Debug
        set buildTarget=
    ) else goto usage
)

if "%~1" == "x86" goto 32bit else (
    if "%~1" == "x64" goto 64bit else goto usage
)

:32bit
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
@echo on
set prefixPath=C:\Qt\5.9.5\msvc2015
set archSuffix="x86"
goto build

:64bit
call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86_amd64
@echo on
set prefixPath=C:\Qt\5.9.5\msvc2015_64
set archSuffix="x64"
goto build

:build
mkdir build
pushd build
cmake -DARCH_LIB_SUFFIX=%archSuffix% -DCMAKE_PREFIX_PATH=%prefixPath% -DCMAKE_BUILD_TYPE=%buildType% -G "Ninja" ..
ninja %buildTarget%
goto exit

:usage
echo "Usage build.cmd [x86|x64] [true|false]"
goto exit

:exit
popd
