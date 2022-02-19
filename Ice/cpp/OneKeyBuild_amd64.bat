@echo off
cd D:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\amd64
call "D:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\amd64\vcvars64.bat"
REM cd E:\Ice-3.5.1\cpp
REM E:
nmake /f Makefile.mak clean

nmake /f Makefile.mak

pause