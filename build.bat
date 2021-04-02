@ECHO OFF
IF NOT EXIST build MKDIR build


REM REFERENCES:https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=vs-2019

REM ************************************************************
REM *********               DEFINITIONS               **********
REM ************************************************************

REM ====================    PROJECT/FILES      ====================
SET Project_Name=SGE
SET Sources=^
..\src\Win32_%Project_Name%.c ^
..\src\Win32_OpenGL.c ^
..\src\SGE_FileIO.c ^
..\src\SGE_Helpers.c ^
..\lib\glad\src\glad.c

REM ====================    COMPILER(MSVC)     ====================
SET Compiler_Common=^
-nologo

REM TODO(MIGUEL): ENABLE WARNINGS ONE BY ONE AND RESOLVE
SET Warnings= ^
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
SET Compiler_Flags=^
%Compiler_Common% ^
-MD ^
-GR ^
-Od ^
-Oi ^
-WX ^
-W4 ^
-FC ^
-Z7 ^
-permissive ^
%Warnings%

SET Include_Directories=^
-I ..\lib\

REM ==================    LINKER(MSVC)   ====================
SET Common_Linker_Flags=^
-incremental:no ^
-opt:ref

SET Libraries=^
User32.lib ^
Gdi32.lib ^
Dinput8.lib ^
Dxguid.lib ^
Opengl32.lib ^
Kernel32.lib ^
Ws2_32.lib ^
Shell32.lib ^
..\lib\bin\cglm.lib

REM ************************************************************
REM **********              START BUILD               **********
REM ************************************************************
PUSHD build
REM path=F:\Dev\SimpleGameEngine\build;%path%

CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 10.0.20292.0

ECHO ====================      WINDOWS       ====================

ECHO --------------------  COMPILE AND LINK  --------------------
CALL cl -std:c17 %Compiler_Flags% %Include_Directories% %Sources% /link %Libraries%


ECHO --------------------    CREATE A DLL    --------------------
CALL cl -std:c17 -nologo -MD -Zi %Include_Directories% ..\src\SGE.c -FmWin32_SGE.map glad.obj Win32_OpenGL.obj -LD /link -PDB:SGE_%random% -DLL -EXPORT:SGEInit -EXPORT:SGEUpdate


POPD