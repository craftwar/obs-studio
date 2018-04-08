@rem 7zip doesn't support extract some formats (7z...etc) from pipe yet
@rem curl -skL https://zlib.net/zlib1211.zip -f --retry 5 -C - | "C:\Program Files\7-Zip\7z.exe" x -si
@echo off
set file_url=https://github.com/craftwar/obs-studio/releases/download/git/craftwar.obs_updater.zip
set file=craftwar.obs_updater.zip
set file-new=%file%-new
set _7z=7za.exe
::set _7z_options=


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
		%_7z% x %file% -y -o. "-x!update updater.cmd" -x!%_7z% -x!CURL.EXE
	)
)
if %ERRORLEVEL% NEQ 0 (echo error!)
pause