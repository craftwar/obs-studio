@echo off
set file_url=https://ci.appveyor.com/api/projects/craftwar_appveyor/obs-studio/artifacts/obs-text.dll?branch=master
set file=obs-plugins\64bit\obs-text.dll
if exist obs-plugins (
	curl -kLo %file% %file_url% -f --retry 5
) else (
	echo error not OBS studio root
)
pause