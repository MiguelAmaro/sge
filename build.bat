@ECHO OFF
IF NOT EXIST build MKDIR build

REM REFERENCES:https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically?view=vs-2019

REM ************************************************************
REM *********               DEFINITIONS               **********
REM ************************************************************

REM ==================   COMPILER(MSVC)  ================== 
SET Sources= ..\src\Win32_graphics.c ..\src\graphics.c ..\src\helpers.c ..\lib\glad\src\glad.c
SET Compiler_Common= -nologo
SET Warning= -wd4700
SET Compiler_Flags= %Compiler_Common% -MD -FC -Z7 %Warning%
SET Include_Directories= -I ..\lib\ -I ..\src\

REM ==================    LINKER(MSVC)   ====================
SET Common_Linker_Flags= -incremental:no -opt:ref
SET Libraries= gdi32.lib opengl32.lib kernel32.lib user32.lib shell32.lib ..\lib\bin\glfw3.lib ..\lib\bin\cglm.lib



ECHO ************************************************************
ECHO **********              START BUILD               **********
ECHO ************************************************************
PUSHD build

CALL "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
SET path=p:\4coder\Projects\SimpleGameEngine\build;%path%

REM CL [option...] file... [option | file]... [lib...] [@command-file] [/link link-opt...]

REM CREATE A DLL
REM cl %CommonCompilerFlags%  %IncludeDirectories% %Sources% /link %CommonLinkerFlags%  %Libraries% 
 
REM COMPILE & LINK
cl %Compiler_Flags% %Include_Directories% %Sources% /link %Common_Linker_Flags%  %Libraries%  

POPD



