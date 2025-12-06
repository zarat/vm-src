@echo off

del *.exe 

echo Compiling zlang.exe
cd zlang
call _make.bat
cd ..

echo Compiling as.exe
cd assembler
call _make.bat
cd ..

echo Compiling vm.exe
cd vm
call _make.bat
cd ..

zlang test.script > test.asm
as test.asm test.bin
vm test.bin

pause

