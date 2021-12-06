del *.exe

set FLAGS=-Wall -Werror

set DEFINES=-D _DEVEL

gcc %DEFINES% %FLAGS% mkstorage.c -o mkstorage

pause