@echo off
cd /d %~dp0
echo Checking OBS update
set file_url=https://github.com/craftwar/obs-studio/releases/download/git/OBS-git-craftwar-blend.7z
set file=OBS-git-craftwar-blend.7z
call "update OBS.cmd" noPause
start /D %cd%\bin\64bit\ obs64.exe
echo Checking updater update
"update updater.cmd"