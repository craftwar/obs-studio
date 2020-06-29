$srcDir = 'C:\projects\obs-studio-craftwar'
$dstDir = 'C:\projects\obs-studio'

$files = @(
    'CMakeLists.txt'
    'cmake\Modules\ObsCpack.cmake'
    'plugins\obs-text\gdiplus\obs-text.cpp'
    'UI\obs-app.hpp'
    'UI\obs-app.cpp'
    'UI\obs.manifest'
    'CI\craftwar'
    'CI\before-deploy-win.cmd'
)

foreach ($file in $files) {
    Move-Item -Path $srcDir\$file -Destination $dstDir\$file -Force
}

$cpu_arch = ${env:favor_arch}.substring(0, $env:favor_arch.length - 2)
& "C:\Program Files\Git\usr\bin\sed.exe" -i "s/(by craftwar)/($cpu_arch, by craftwar)/" /C/projects/obs-studio/UI/obs-app.cpp
& "C:\Program Files\Git\usr\bin\sed.exe" -i "/TimedCheckForUpdates();/d" /C/projects/obs-studio/UI/window-basic-main.cpp
