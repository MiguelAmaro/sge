@echo off

if not exist "build" mkdir "build"


rem REFERENCES:https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=vs-2019

rem ************************************************************
rem ************************************************************
rem **                                                        **
rem **                       DEFINITIONS                      **
rem **                                                        **
rem ************************************************************
rem ************************************************************

rem ==========              BUILD OPTIONS             ==========     
rem ============================================================
set PLATFORM=%1
set OUTPUT=%2


rem ==========              PROJECT/FILES             ==========     
rem ============================================================
set PROJECT_NAME=sge
set SOURCES=^
..\src\win32_%PROJECT_NAME%.c ^
..\src\win32_opengl.c ^
..\src\%PROJECT_NAME%_fileio.c ^
..\src\%PROJECT_NAME%_helpers.c ^
..\lib\glad\src\glad.c

set BUILD_MODES=^
-DSGE_WIN32=1 ^
-DSGE_SLOW=1 ^
-DSGE_INTERNAL=1 ^
-DRION=0

rem ==========              COMPILER(MSVC)            ==========     
rem ============================================================
set MSVC_COMMON=^
-nologo

rem TODO(MIGUEL): ENABLE WARNINGS ONE BY ONE AND RESOLVE
set MSVC_WARNINGS= ^
-W4 ^
-WX ^
-wd4057 ^
-wd4100 ^
-wd4189 ^
-wd4201 ^
-wd4204

rem NOTE(MIGUEL):-MD is using Dynamic CRT Lib which is what is supporting the console
rem TODO(MIGUEL): Figure out a way to get a console that doesnt need to use the MD flag
set MSVC_FLAGS=^
%MSVC_COMMON% ^
-MD ^
-GR ^
-Oi ^
-Od ^
-FC ^
-Z7 ^
-permissive ^
%MSVC_WARNINGS%

set INCLUDE_PATHS=^
-I ..\lib\

rem ==========             LINKER(MSVC)               ==========     
rem ============================================================
set LINKER_FLAGS=^
-incremental:no ^
-opt:ref

set LIBRARIES=^
User32.lib ^
Gdi32.lib ^
Dinput8.lib ^
Dxguid.lib ^
Opengl32.lib ^
Kernel32.lib ^
Ws2_32.lib ^
winmm.lib ^
Shell32.lib


rem ************************************************************
rem ************************************************************
rem **                                                        **
rem **                       START BUILD                      **
rem **                                                        **
rem ************************************************************
rem ************************************************************
rem path=F:\Dev\SimpleGameEngine\build;%path%

pushd "build"


if %PLATFORM% equ --win (call :WINDOWS)
if %PLATFORM% equ --lin (call :LINUX  )
if %PLATFORM% equ --mac (call :MAC    )

popd
exit /B 0


:WINDOWS

if %OUTPUT% equ --exe (call :COMPILE_WIN_EXE)
if %OUTPUT% equ --dll (call :COMPILE_WIN_DLL)

exit


echo ==========                  E X E                 ==========     
echo ============================================================
:COMPILE_WIN_EXE

cl -std:c17 %BUILD_MODES% %MSVC_FLAGS% %INCLUDE_PATHS% %SOURCES% /link %LIBRARIES%

exit


echo ==========                  D L L                 ==========     
echo ============================================================
:COMPILE_WIN_DLL

rem del *.pdb > NUL 2> NUL
rem echo WAITING FOR PDB > lock.tmp

rem -std:c17 ^
cl ^
%MSVC_FLAGS% ^
%BUILD_MODES% ^
%INCLUDE_PATHS% ^
F:\Dev\SimpleGameEngine\src\%PROJECT_NAME%.c ^
-Fmwin32_%PROJECT_NAME%.map ^
glad.obj win32_OpenGL.obj ^
-LD /link -PDB:%PROJECT_NAME%_%random%.pdb ^
-DLL ^
-EXPORT:SGEInit ^
-EXPORT:SGEUpdate ^
-EXPORT:SGEGetSoundSamples

rem del lock.tmp

exit


