@ECHO OFF
IF NOT EXIST build MKDIR build


REM REFERENCES:https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=vs-2019

REM ************************************************************
REM ************************************************************
REM **                                                        **
REM **                       DEFINITIONS                      **
REM **                                                        **
REM ************************************************************
REM ************************************************************

REM ==========              PROJECT/FILES             ==========     
REM ============================================================
SET PROJECT_NAME=SGE
SET SOURCES=^
..\src\Win32_%PROJECT_NAME%.c ^
..\src\Win32_OpenGL.c ^
..\src\SGE_FileIO.c ^
..\src\SGE_Helpers.c ^
..\lib\glad\src\glad.c

SET BUILD_MODES=^
-DSGE_WIN32=1 ^
-DSGE_SLOW=1 ^
-DSGE_INTERNAL=1 ^
-DRION=0

REM ==========              COMPILER(MSVC)            ==========     
REM ============================================================
SET MSVC_COMMON=^
-nologo

REM TODO(MIGUEL): ENABLE WARNINGS ONE BY ONE AND RESOLVE
SET MSVC_WARNINGS= ^
-wd4057 ^
-wd4013 ^
-wd4057 ^
-wd4100 ^
-wd4101 ^
-wd4189 ^
-wd4201 ^
-wd4204 ^
-wd4218 ^
-wd4244 ^
-wd4431 ^
-wd4456 ^
-wd4700 ^
-wd4706 ^
-wd4996

REM NOTE(MIGUEL):-MD is using Dynamic CRT Lib which is what is supporting the console
REM TODO(MIGUEL): Figure out a way to get a console that doesnt need to use the MD flag
SET MSVC_FLAGS=^
%MSVC_COMMON% ^
-MD ^
-GR ^
-Oi ^
-Od ^
-WX ^
-W4 ^
-FC ^
-Z7 ^
-permissive ^
%MSVC_WARNINGS%

SET INCLUDE_PATHS=^
-I ..\lib\

REM ==========             LINKER(MSVC)               ==========     
REM ============================================================
SET LINKER_FLAGS=^
-incremental:no ^
-opt:ref

SET LIBRARIES=^
User32.lib ^
Gdi32.lib ^
Dinput8.lib ^
Dxguid.lib ^
Opengl32.lib ^
Kernel32.lib ^
Ws2_32.lib ^
winmm.lib ^
Shell32.lib ^
..\lib\bin\cglm.lib


REM ************************************************************
REM ************************************************************
REM **                                                        **
REM **                       START BUILD                      **
REM **                                                        **
REM ************************************************************
REM ************************************************************
PUSHD build
REM path=F:\Dev\SimpleGameEngine\build;%path%

CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 10.0.20292.0

ECHO ==========             COMPILE AND LINK           ==========     
ECHO ============================================================
CALL cl -std:c17 %BUILD_MODES% %MSVC_FLAGS% %INCLUDE_PATHS% %SOURCES% /link %LIBRARIES%


ECHO ==========              CREATE A DLL              ==========     
ECHO ============================================================
CALL cl ^
-std:c17 ^
-nologo ^
-MD ^
-Zi ^
%INCLUDE_PATHS% ^
..\src\SGE.c ^
-FmWin32_SGE.map ^
glad.obj Win32_OpenGL.obj ^
-LD /link -PDB:SGE_%random% ^
-DLL -EXPORT:SGEInit -EXPORT:SGEUpdate


POPD