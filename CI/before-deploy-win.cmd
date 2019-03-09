rem robocopy C:\projects\obs-studio\build32\rundir\RelWithDebInfo C:\projects\obs-studio\build\ /E /XF .gitignore
robocopy .\build32\rundir\Release\data\obs-plugins\win-capture C:\projects\obs-studio\build\data\obs-plugins\win-capture /E /XF .gitignore
robocopy .\build64\rundir\Release C:\projects\obs-studio\build\ /E /XC /XN /XO /XF .gitignore
call "C:\projects\obs-studio\CI\craftwar\download game capture.cmd"
7z a build.7z -mx=9 -myx=9 C:\projects\obs-studio\build\*
