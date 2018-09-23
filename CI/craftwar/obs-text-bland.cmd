if "%favor_arch%"=="INTEL64" (
	"C:\Program Files\Git\usr\bin\sed.exe" -i "/<EnableEnhancedInstructionSet>AdvancedVectorExtensions2<\/EnableEnhancedInstructionSet>/d" /C/projects/obs-studio/build64/plugins/obs-text/obs-text.vcxproj
	"C:\Program Files\Git\usr\bin\sed.exe" -i "s/-favor:INTEL64//" /C/projects/obs-studio/build64/plugins/obs-text/obs-text.vcxproj
	msbuild /m /p:Configuration=Release C:\projects\obs-studio\build64\plugins\obs-text\obs-text.vcxproj /t:Clean;Rebuild /logger:"C:\Program Files\AppVeyor\BuildAgent\Appveyor.MSBuildLogger.dll"
	PowerShell -Command "& {Push-AppveyorArtifact "C:\projects\obs-studio\build64\plugins\obs-text\Release\obs-text.dll" -FileName "obs-text.dll"}"
)
