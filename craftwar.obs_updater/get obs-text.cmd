@echo off
call check_cpu.cmd
set file_url=https://github.com/craftwar/obs-studio/releases/download/git/obs-text-%favor_arch%.dll
set file=obs-plugins\64bit\obs-text.dll
cd /d %~dp0
if exist obs-plugins (
	curl -kLo %file% %file_url% -f --retry 5
) else (
	echo error not OBS studio root
)
pause