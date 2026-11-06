@echo off
gcc -Wall -Wextra -O2 -o demo.exe demo.c modsystem.c
if %errorlevel%==0 echo Built demo.exe
