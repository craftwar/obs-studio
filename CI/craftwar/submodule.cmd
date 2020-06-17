if "%vc_inc_arch%"=="SSE2" (
	cd ..
	mv obs-studio obs-studio-craftwar
	git clone -q --branch=master https://github.com/obsproject/obs-studio.git C:\projects\obs-studio
	cd obs-studio
	git submodule update --init --recursive
	if exist vlc.zip (curl -kLO https://cdn-fastly.obsproject.com/downloads/vlc.zip -f --retry 5 -z vlc.zip) else (curl -kLO https://cdn-fastly.obsproject.com/downloads/vlc.zip -f --retry 5 -C -)
	if exist cef_binary_%CEF_VERSION%_windows64_minimal.zip (curl -kLO https://cdn-fastly.obsproject.com/downloads/cef_binary_%CEF_VERSION%_windows64_minimal.zip -f --retry 5 -z cef_binary_%CEF_VERSION%_windows64_minimal.zip) else (curl -kLO https://cdn-fastly.obsproject.com/downloads/cef_binary_%CEF_VERSION%_windows64_minimal.zip -f --retry 5 -C -)
	7z x vlc.zip -ovlc
	7z x cef_binary_%CEF_VERSION%_windows64_minimal.zip -oCEF_64
	set VLCPath=%CD%\vlc
	set CEF_64=%CD%\CEF_64\cef_binary_%CEF_VERSION%_windows64_minimal
rem	curl -kLo C:\projects\obs-studio\UI\win-update\win-update.hpp -f --retry 5 https://github.com/obsproject/obs-studio/raw/master/UI\win-update\win-update.hpp
rem	curl -kLo C:\projects\obs-studio\UI\win-update\win-update.hpp -f --retry 5 https://raw.githubusercontent.com/obsproject/obs-studio/master/UI/win-update/win-update.hpp
rem	curl -kLo C:\projects\obs-studio\UI\win-update\win-update.cpp -f --retry 5 https://raw.githubusercontent.com/obsproject/obs-studio/master/UI/win-update/win-update.cpp
rem	curl -kLo C:\projects\obs-studio\plugins\obs-text\gdiplus\obs-text.cpp -f --retry 5 https://raw.githubusercontent.com/craftwar/obs-studio/master/plugins/obs-text/gdiplus/obs-text.cpp
rem enc-amf  error
	move /Y C:\projects\obs-studio-craftwar\CMakeLists.txt C:\projects\obs-studio\CMakeLists.txt
	move /Y C:\projects\obs-studio-craftwar\cmake\Modules\ObsCpack.cmake C:\projects\obs-studio\cmake\Modules\ObsCpack.cmake
	move /Y C:\projects\obs-studio-craftwar\plugins\obs-text\gdiplus\obs-text.cpp C:\projects\obs-studio\plugins\obs-text\gdiplus\obs-text.cpp
	move /Y C:\projects\obs-studio-craftwar\UI\obs-app.* C:\projects\obs-studio\UI
rem -r, --regexp-extended, basic re (BRE) treat () as plain text, use \( if you want grouping
	"C:\Program Files\Git\usr\bin\sed.exe" -i "s/(by craftwar)/(%favor_arch:~0,-2%, by craftwar)/" /C/projects/obs-studio/UI/obs-app.cpp
	"C:\Program Files\Git\usr\bin\sed.exe" -i "/TimedCheckForUpdates();/d" /C/projects/obs-studio/UI/window-basic-main.cpp
	move /Y C:\projects\obs-studio-craftwar\CI\craftwar C:\projects\obs-studio\CI\craftwar
	move /Y C:\projects\obs-studio-craftwar\CI\before-deploy-win.cmd C:\projects\obs-studio\CI\before-deploy-win.cmd
	) else (
	git submodule update --init --recursive --remote plugins\win-dshow
)
