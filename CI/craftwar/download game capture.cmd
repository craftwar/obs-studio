::@echo off
setlocal
set cdn_root=https://cdn-fastly.obsproject.com/update_studio
set gc_dir=data/obs-plugins/win-capture
set cdn_gc_url=%cdn_root%/core/%gc_dir%

if "%favor_arch%"=="blend" (
	mkdir Jim_OBS\data\obs-plugins\win-capture\
	pushd Jim_OBS\data\obs-plugins\win-capture\

	if exist get-graphics-offsets32.exe (curl -kLO %cdn_gc_url%/get-graphics-offsets32.exe -f --retry 5 -z get-graphics-offsets32.exe) else	(curl -kLO %cdn_gc_url%/get-graphics-offsets32.exe -f --retry 5 -C -)
	if exist get-graphics-offsets64.exe (curl -kLO %cdn_gc_url%/get-graphics-offsets64.exe -f --retry 5 -z get-graphics-offsets64.exe) else	(curl -kLO %cdn_gc_url%/get-graphics-offsets64.exe -f --retry 5 -C -)
	if exist graphics-hook32.dll (curl -kLO %cdn_gc_url%/graphics-hook32.dll -f --retry 5 -z graphics-hook32.dll) else	(curl -kLO %cdn_gc_url%/graphics-hook32.dll -f --retry 5 -C -)
	if exist graphics-hook64.dll (curl -kLO %cdn_gc_url%/graphics-hook64.dll -f --retry 5 -z graphics-hook64.dll) else	(curl -kLO %cdn_gc_url%/graphics-hook32.dll -f --retry 5 -C -)
	if exist inject-helper32.exe (curl -kLO %cdn_gc_url%/inject-helper32.exe -f --retry 5 -z inject-helper32.exe) else (curl -kLO %cdn_gc_url%/inject-helper32.exe -f --retry 5 -C -)
	if exist inject-helper64.exe (curl -kLO %cdn_gc_url%/inject-helper64.exe -f --retry 5 -z inject-helper64.exe) else (curl -kLO %cdn_gc_url%/inject-helper64.exe -f --retry 5 -C -)
	popd
	xcopy /Y /E Jim_OBS C:\projects\obs-studio\build
)
endlocal