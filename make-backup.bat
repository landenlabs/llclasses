@echo off
setlocal enableextensions

rem
rem  Backup to Dropbox using 7z
rem

set DSTNAME=llclasses

if exist  C:\Users\dlang_local\Dropbox  set DROPBOX=C:\Users\dlang_local\Dropbox
if exist  C:\opt\Dropbox                set DROPBOX=C:\opt\Dropbox
if exist  F:\ssd-dropbox\Dropboxset     set DROPBOX=F:\ssd-dropbox\Dropbox
set DSTDIR=%DROPBOX%\backups\cpp\_ll\

rem Use environment set on Windows 7Z
if exist  "%7Z%"                  set Z7="%7Z%"
if exist  c:\opt\disk\7zip\7z.exe set Z7=c:\opt\disk\7zip\7z.exe
if exist  d:\opt\disk\7zip\7z.exe set Z7=d:\opt\disk\7zip\7z.exe

set DT=%date:~-4,4%-%date:~-10,2%-%date:~-7,2%_%time:~0,2%-%time:~3,2%

for /F "tokens=* USEBACKQ" %%F IN (`pdate`) DO (SET DT=%%F)
set OUTZIP=%DSTNAME%_%DT%.zip
set DST=%DSTDIR%%OUTZIP%

rem - remove spaces in file name replace with 0 or _
rem lm "* *" "*1_*1"

echo %Z7% a "%DST%" .\ -mx0 -xr!bin -xr!obj -xr!Debug -xr!x64 -xr!.git -xr!.vs
%Z7% a "%DST%" .\ -mx0 -xr!bin -xr!obj -xr!Debug -xr!x64 -xr!.git -xr!.vs -xr!*.bak

echo ---- Backups ----
dir %DSTDIR%%DSTNAME%*