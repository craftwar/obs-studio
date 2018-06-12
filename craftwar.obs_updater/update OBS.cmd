@echo off
if NOT DEFINED file_url (
	set file_url=https://github.com/craftwar/obs-studio/releases/download/git/OBS-git-craftwar.7z
)
set file=OBS-git-craftwar.7z
set file-new=%file%-new
set _7z=7za.exe
::set _7z_options=

taskkill /F /FI "WINDOWTITLE eq OBS *" /IM obs64.exe
if exist %file% (
	curl -kLo %file-new% %file_url% -f --retry 5 -z %file%
	if %ERRORLEVEL% EQU 0 (
		if exist %file%-new (
			move /y %file-new% %file%
			%_7z% x %file% -y -o. %_7z_options%
		)
	)
) else (
	curl -kLo %file% %file_url% -f --retry 5 -C -
	if %ERRORLEVEL% EQU 0 (
		%_7z% x %file% -y -o. %_7z_options%
	)
)
if %ERRORLEVEL% NEQ 0 (echo error!)
pause