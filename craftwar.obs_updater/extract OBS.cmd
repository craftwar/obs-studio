@set file=OBS-git-craftwar.7z
@set _7z=7za.exe
@cd /d %~dp0
@%_7z% x %file% -y -o. %_7z_options%
@pause