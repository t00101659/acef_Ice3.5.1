@echo off
cd  D:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin
call "D:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin\vcvars32.bat"
REM E:
REM nmake /f Makefile.mak clean

nmake /f Makefile.mak

pause