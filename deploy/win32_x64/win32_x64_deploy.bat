echo off
REM  This script creates a folder parallel to project folder that holds:
REM  - starvax executable
REM  - QT related DLLs
REM  - USB lib DLLs
REM  - fundamental VLC library (no plug-ins)
REM  - resource files (translations, icons, ...)
REM  - (optionally) a copy of lib VLC plug-ins

REM  INPUTS: to be filled before run
REM  ---------------------------------------

REM : directory that holds starvax binary. 
set BUILD_DIR=..\..\..\build-cmake_starvaxplayer-code-Qt5_15_x64-Release\
REM : directory of VLC library
set VLC_LIB_DIR=..\..\..\VLC_lib\libvlc\x64
REM : QT library folder, containing DLLs for QT modules
set QT_LIB_DIR=c:\Qt\Qt-5.15.2_x64\bin


REM ---------------------------
echo on
set DEST_DIR=..\..\..\starvax-deploy-x64-%date:~-4%_%date:~3,2%_%date:~0,2%
mkdir %DEST_DIR%

xcopy /E /Y /I %BUILD_DIR%\res  %DEST_DIR%\res
copy %BUILD_DIR%\libusb-1.0.dll  %DEST_DIR%
copy %BUILD_DIR%\starvax_player.exe  %DEST_DIR%

REM VLC library and plugins
copy %VLC_LIB_DIR%\libvlc.dll      %DEST_DIR%
copy %VLC_LIB_DIR%\libvlccore.dll  %DEST_DIR%

echo making 'plugins' directory, but it's empty
REM xcopy /E /Y /I %VLC_LIB_DIR%\plugins  %DEST_DIR%\plugins
mkdir %DEST_DIR%\plugins

echo unzipping VLC plugins
7z x plugins_v3.0.8.zip -o%DEST_DIR%\plugins

REM QT libraries

copy %QT_LIB_DIR%\Qt5Svg.dll   %DEST_DIR%
copy %QT_LIB_DIR%\Qt5OpenGL.dll   %DEST_DIR%
copy %QT_LIB_DIR%\Qt5Widgets.dll   %DEST_DIR%
copy %QT_LIB_DIR%\Qt5Gui.dll   %DEST_DIR%
copy %QT_LIB_DIR%\Qt5Network.dll   %DEST_DIR%
copy %QT_LIB_DIR%\Qt5Core.dll   %DEST_DIR%


mkdir %DEST_DIR%\platforms
mkdir %DEST_DIR%\imageformats

copy %QT_LIB_DIR%\..\plugins\platforms\qminimal.dll   %DEST_DIR%\platforms
copy %QT_LIB_DIR%\..\plugins\platforms\qwindows.dll   %DEST_DIR%\platforms

copy %QT_LIB_DIR%\..\plugins\imageformats\qsvg.dll    %DEST_DIR%\imageformats
copy %QT_LIB_DIR%\..\plugins\imageformats\qjpeg.dll    %DEST_DIR%\imageformats

pause 