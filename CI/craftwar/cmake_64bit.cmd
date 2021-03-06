if "%vc_inc_arch%"=="SSE2" (
	vcpkg install speexdsp:x64-windows-static
rem delete ffmpeg dlls
rem	del /Q C:\projects\obs-studio\old_dep\win64\bin\av*.*
rem	del /Q C:\projects\obs-studio\old_dep\win64\bin\swscale-*.*
rem	del /Q C:\projects\obs-studio\old_dep\win64\bin\swresample-*.*
rem	7z x -y C:\projects\obs-studio\VCdeps-blend.7z -oC:\projects\obs-studio\old_dep
rem	set DepsPath64=C:\projects\obs-studio\old_dep\win64
	cmake -E env CFLAGS="%cl_options%"  CXXFLAGS="%cl_options%" LDFLAGS="-LTCG" cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_SYSTEM_VERSION=10.0 -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE=C:\Tools\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_EXE_LINKER_FLAGS_INIT=/LTCG -DCMAKE_SHARED_LINKER_FLAGS_INIT=/LTCG -DCMAKE_STATIC_LINKER_FLAGS_INIT=/LTCG -DCMAKE_MODULE_LINKER_FLAGS_INIT=/LTCG -DCOPIED_DEPENDENCIES=false -DCOPY_DEPENDENCIES=true -DCOMPILE_D3D12_HOOK=true -DENABLE_SCRIPTING=false -DBUILD_BROWSER=true -DCRAFTWAR_MIN_BUILD=false -DCEF_ROOT_DIR=%CEF_64% -DTWITCH_CLIENTID="%TWITCH-CLIENTID%" -DTWITCH_HASH="%TWITCH-HASH%" -DVIRTUALCAM_GUID="A3FCE0F5-3493-419F-958A-ABA1250EC20B" ..
) else (
	cmake -E env CFLAGS="%cl_options%"  CXXFLAGS="%cl_options%" LDFLAGS="-LTCG" cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_SYSTEM_VERSION=10.0 -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE=C:\Tools\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_EXE_LINKER_FLAGS_INIT=/LTCG -DCMAKE_SHARED_LINKER_FLAGS_INIT=/LTCG -DCMAKE_STATIC_LINKER_FLAGS_INIT=/LTCG -DCMAKE_MODULE_LINKER_FLAGS_INIT=/LTCG -DCOPIED_DEPENDENCIES=false -DCOPY_DEPENDENCIES=true -DCOMPILE_D3D12_HOOK=true -DENABLE_SCRIPTING=false -DBUILD_BROWSER=OFF -DBUILD_VST=OFF ..
)
rem patch project setting
rem remove /GL to incompatible project
rem "C:\Program Files\Git\usr\bin\sed.exe" -i "/<WholeProgramOptimization>true<\/WholeProgramOptimization>/d" /C/projects/obs-studio/build64/UI/obs.vcxproj
"C:\Program Files\Git\usr\bin\sed.exe" -i "s/<LanguageStandard>stdcpp17</<LanguageStandard>stdcpplatest</" /C/projects/obs-studio/build64/plugins/obs-text/obs-text.vcxproj
