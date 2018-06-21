@echo off
cd /d %~dp0
echo Checking OBS update
call "update OBS.cmd" noPause
start /D %cd%\bin\64bit\ obs64.exe
echo Checking updater update
"update updater.cmd"