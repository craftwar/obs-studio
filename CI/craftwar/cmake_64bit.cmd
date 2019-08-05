if "%vc_inc_arch%"=="SSE2" (
	vcpkg install speexdsp:x64-windows-static
:: delete ffmpeg dlls
::	del /Q C:\projects\obs-studio\old_dep\win64\bin\av*.*
::	del /Q C:\projects\obs-studio\old_dep\win64\bin\swscale-*.*
::	del /Q C:\projects\obs-studio\old_dep\win64\bin\swresample-*.*
::	7z x -y C:\projects\obs-studio\VCdeps-blend.7z -oC:\projects\obs-studio\old_dep
::	set DepsPath64=C:\projects\obs-studio\old_dep\win64
	cmake -E env CFLAGS="%cl_options%"  CXXFLAGS="%cl_options%" LDFLAGS="-LTCG" cmake -G "Visual Studio 15 2017" -A x64 -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE=C:\Tools\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_EXE_LINKER_FLAGS_INIT=/LTCG -DCMAKE_SHARED_LINKER_FLAGS_INIT=/LTCG -DCMAKE_STATIC_LINKER_FLAGS_INIT=/LTCG -DCMAKE_MODULE_LINKER_FLAGS_INIT=/LTCG -DCOPIED_DEPENDENCIES=false -DCOPY_DEPENDENCIES=true -DBUILD_CAPTIONS=true -DCOMPILE_D3D12_HOOK=true -DENABLE_SCRIPTING=false -DBUILD_BROWSER=true -DCRAFTWAR_MIN_BUILD=false -DCEF_ROOT_DIR=%CEF_64% -DTWITCH_CLIENTID="%TWITCH-CLIENTID%" -DTWITCH_HASH="%TWITCH-HASH%" -DMIXER_CLIENTID="%MIXER-CLIENTID%" -DMIXER_HASH="%MIXER-HASH%" ..
	"C:\Program Files\Git\usr\bin\sed.exe" -i "/<LanguageStandard>stdcpplatest<\/LanguageStandard>/d" /C/projects/obs-studio/build64/plugins/enc-amf/enc-amf.vcxproj
) else (
	cmake -E env CFLAGS="%cl_options%"  CXXFLAGS="%cl_options%" LDFLAGS="-LTCG" cmake -G "Visual Studio 15 2017" -A x64 -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_TOOLCHAIN_FILE=C:\Tools\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_EXE_LINKER_FLAGS_INIT=/LTCG -DCMAKE_SHARED_LINKER_FLAGS_INIT=/LTCG -DCMAKE_STATIC_LINKER_FLAGS_INIT=/LTCG -DCMAKE_MODULE_LINKER_FLAGS_INIT=/LTCG -DCOPIED_DEPENDENCIES=false -DCOPY_DEPENDENCIES=true -DBUILD_CAPTIONS=true -DCOMPILE_D3D12_HOOK=true -DENABLE_SCRIPTING=false ..
)
