if "%vc_inc_arch%"=="SSE2" (
	git clone -q --branch=master https://github.com/obsproject/obs-studio.git C:\projects\obs-studio
) else (
	git clone -q --branch=dev https://github.com/craftwar/obs-studio.git C:\projects\obs-studio
	git checkout -qf %APPVEYOR_REPO_COMMIT%
)