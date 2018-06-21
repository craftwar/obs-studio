@rem 7zip doesn't support extract some formats (7z...etc) from pipe yet
@rem curl -skL https://zlib.net/zlib1211.zip -f --retry 5 -C - | "C:\Program Files\7-Zip\7z.exe" x -si
@echo off
cd /d %~dp0

:: run in update
if "%1" EQU "move" (
	for /L %%I in (1,1,5) do (
		tasklist /fi "WINDOWTITLE eq craftwar_obs_updater_get" | find /i "cmd.exe" >nul 2>&1
		IF ERRORLEVEL 1 (
			timeout /T 3 /nobreak
		) else (
			goto completeMove
		)
	)
	echo updater error, try again
	exit /B 0
	pause
	:completeMove
	copy /Y *.* .. >NUL
	del /Q *.*
	exit /B 0
	pause
) else (
:: run in obs root
	title craftwar_obs_updater_get
	if exist "update\update updater.cmd" (
		"update\update updater.cmd" move
		exit /B 0
	)
)

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
			%_7z% x %file% -y -oupdate
			"update\update updater.cmd" move
			exit /B 0
		)
	)
) else (
	curl -kLo %file% %file_url% -f --retry 5 -C -
	if %ERRORLEVEL% EQU 0 (
		%_7z% x %file% -y -oupdate
		"update\update updater.cmd" move
		exit /B 0
	)
)
if %ERRORLEVEL% NEQ 0 (echo error!)
pause