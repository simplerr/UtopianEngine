@echo off

echo Copying assets to build directory...
robocopy data bin\Debug\data\ /MIR /NFL /NDL /NJH /NJS /nc /ns /np > NUL

echo Copying DLLs to build directory...
xcopy "libs\assimp\assimp-vc140-mt.dll" "bin\Debug\"     /I /Q /y > NUL

echo Copying imgui.ini to build directory...
xcopy "imgui.ini" "bin\Debug\"     /I /Q /y > NUL

echo Generating workspace...
tools\premake5.exe --file=premake.lua vs2019

exit /b