@echo off
cd /d %~dp0
"update updater.cmd"
"update OBS.cmd"
start /D %cd%\bin\64bit\ obs64.exe
