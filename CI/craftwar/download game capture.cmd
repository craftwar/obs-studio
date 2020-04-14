@rem echo off
@rem https://cdn-fastly.obsproject.com/update_studio/core/obs-plugins/64bit/win-capture.dll
setlocal
set cdn_root=https://cdn-fastly.obsproject.com/update_studio
set gc_dir=data/obs-plugins/win-capture
set cdn_gc_url=%cdn_root%/core/%gc_dir%
set file_dir=obs-plugins/64bit
set cdn_wc_url=%cdn_root%/core/%file_dir%/win-capture.dll
if "%favor_arch%"=="INTEL64" (
	if "%vc_inc_arch%"=="SSE2" (
		mkdir Jim_OBS\data\obs-plugins\win-capture\locale
		pushd Jim_OBS\data\obs-plugins\win-capture\
		if exist get-graphics-offsets32.exe (curl -kLO %cdn_gc_url%/get-graphics-offsets32.exe -f --retry 5 -z get-graphics-offsets32.exe) else	(curl -kLO %cdn_gc_url%/get-graphics-offsets32.exe -f --retry 5 -C -)
		if exist get-graphics-offsets64.exe (curl -kLO %cdn_gc_url%/get-graphics-offsets64.exe -f --retry 5 -z get-graphics-offsets64.exe) else	(curl -kLO %cdn_gc_url%/get-graphics-offsets64.exe -f --retry 5 -C -)
		if exist graphics-hook32.dll (curl -kLO %cdn_gc_url%/graphics-hook32.dll -f --retry 5 -z graphics-hook32.dll) else	(curl -kLO %cdn_gc_url%/graphics-hook32.dll -f --retry 5 -C -)
		if exist graphics-hook64.dll (curl -kLO %cdn_gc_url%/graphics-hook64.dll -f --retry 5 -z graphics-hook64.dll) else	(curl -kLO %cdn_gc_url%/graphics-hook32.dll -f --retry 5 -C -)
		if exist inject-helper32.exe (curl -kLO %cdn_gc_url%/inject-helper32.exe -f --retry 5 -z inject-helper32.exe) else (curl -kLO %cdn_gc_url%/inject-helper32.exe -f --retry 5 -C -)
		if exist inject-helper64.exe (curl -kLO %cdn_gc_url%/inject-helper64.exe -f --retry 5 -z inject-helper64.exe) else (curl -kLO %cdn_gc_url%/inject-helper64.exe -f --retry 5 -C -)
		if exist obs-vulkan32.json (curl -kLO %cdn_gc_url%/obs-vulkan32.json -f --retry 5 -z obs-vulkan32.json) else (curl -kLO %cdn_gc_url%/obs-vulkan32.json -f --retry 5 -C -)
		if exist obs-vulkan64.json (curl -kLO %cdn_gc_url%/obs-vulkan64.json -f --retry 5 -z obs-vulkan64.json) else (curl -kLO %cdn_gc_url%/obs-vulkan64.json -f --retry 5 -C -)
		cd locale
		if exist en-US.ini (curl -kLO %cdn_gc_url%/locale/en-US.ini -f --retry 5 -z locale\en-US.ini) else (curl -kLO %cdn_gc_url%/locale/en-US.ini -f --retry 5 -C -)
		popd

		mkdir Jim_OBS\obs-plugins\64bit
		pushd Jim_OBS\obs-plugins\64bit
		set file_dir=obs-plugins/64bit
		set file_url=%cdn_root%/core/%file_dir%
		@rem if exist win-capture.dll (curl -kLO %file_url%/win-capture.dll -f --retry 5 -z win-capture.dll) else	(curl -kLO %file_url%/win-capture.dll -f --retry 5 -C -)
		echo %file_dir%
		echo %file_url%
		if exist win-capture.dll (curl -kLO %cdn_wc_url% -f --retry 5 -z win-capture.dll) else	(curl -kLO %cdn_wc_url% -f --retry 5 -C -)
		popd

		7z a gc.7z -mx=9 -myx=9 %APPVEYOR_BUILD_FOLDER%\Jim_OBS\*
		@rem	xcopy /Y /E Jim_OBS C:\projects\obs-studio\build
	)
)
endlocal
