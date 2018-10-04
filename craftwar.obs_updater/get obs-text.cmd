@echo off
cd /d %~dp0
set file_url=https://github.com/craftwar/obs-studio/releases/download/git/obs-text-blend.dll
set file=obs-plugins\64bit\obs-text.dll
if exist obs-plugins (
	taskkill /F /IM obs64.exe
	curl -kLo %file% %file_url% -f --retry 5
) else (
	echo error not OBS studio root
)
pause