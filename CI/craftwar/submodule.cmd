if "%vc_inc_arch%"=="SSE2" (
	git clone -q --branch=master https://github.com/obsproject/obs-studio.git C:\projects\obs-studio
	git submodule update --init --recursive
	if exist vlc.zip (curl -kLO https://cdn-fastly.obsproject.com/downloads/vlc.zip -f --retry 5 -z vlc.zip) else (curl -kLO https://cdn-fastly.obsproject.com/downloads/vlc.zip -f --retry 5 -C -)
	if exist cef_binary_%CEF_VERSION%_windows64_minimal.zip (curl -kLO https://cdn-fastly.obsproject.com/downloads/cef_binary_%CEF_VERSION%_windows64_minimal.zip -f --retry 5 -z cef_binary_%CEF_VERSION%_windows64_minimal.zip) else (curl -kLO https://cdn-fastly.obsproject.com/downloads/cef_binary_%CEF_VERSION%_windows64_minimal.zip -f --retry 5 -C -)
	7z x vlc.zip -ovlc
	7z x cef_binary_%CEF_VERSION%_windows64_minimal.zip -oCEF_64
rem	set VLCPath=%CD%\vlc
rem	set CEF_64=%CD%\CEF_64\cef_binary_%CEF_VERSION%_windows64_minimal
	set VLCPath=C:\projects\%APPVEYOR_PROJECT_NAME%\vlc
	set CEF_64=C:\projects\%APPVEYOR_PROJECT_NAME%\CEF_64\cef_binary_%CEF_VERSION%_windows64_minimal
) else (
	git submodule update --init --recursive --remote plugins\win-dshow
)
