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

REM ==========              BUILD OPTIONS             ==========     
REM ============================================================
SET PLATFORM=%1
SET OUTPUT=%2


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
REM path=F:\Dev\SimpleGameEngine\build;%path%

PUSHD build

IF %PLATFORM%==--win (CALL :WINDOWS)
IF %PLATFORM%==--lin (CALL :LINUX  )
IF %PLATFORM%==--mac (CALL :MAC    )

POPD
EXIT /B 0


:WINDOWS

IF %OUTPUT%==--exe       (CALL :COMPILE_WIN_EXE)

IF %OUTPUT%==--dll       (CALL :COMPILE_WIN_DLL)

EXIT


ECHO ==========                  E X E                 ==========     
ECHO ============================================================
:COMPILE_WIN_EXE

CALL cl -std:c17 %BUILD_MODES% %MSVC_FLAGS% %INCLUDE_PATHS% %SOURCES% /link %LIBRARIES%

EXIT


ECHO ==========                  D L L                 ==========     
ECHO ============================================================
:COMPILE_WIN_DLL

CALL cl ^
-std:c17 ^
-nologo ^
-MD ^
-Zi ^
%BUILD_MODES% ^
%INCLUDE_PATHS% ^
F:\Dev\SimpleGameEngine\src\SGE.c ^
-FmWin32_SGE.map ^
glad.obj Win32_OpenGL.obj ^
-LD /link -PDB:SGE_%random%.pdb ^
-DLL ^
-EXPORT:SGEInit ^
-EXPORT:SGEUpdate ^
-EXPORT:SGEGetSoundSamples ^

EXIT

POPD