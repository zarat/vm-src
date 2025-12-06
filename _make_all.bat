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

echo Done


pause
