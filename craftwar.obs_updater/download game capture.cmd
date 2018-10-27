@echo off
setlocal
set cdn_root=https://cdn-fastly.obsproject.com/update_studio
set gc_dir=data/obs-plugins/win-capture
set cdn_gc_url=%cdn_root%/core/%gc_dir%
set curl_bin=%~dp0\curl

pushd data\obs-plugins\win-capture\

%curl_bin% -kLO %cdn_gc_url%/get-graphics-offsets32.exe -f --retry 5
%curl_bin% -kLO %cdn_gc_url%/get-graphics-offsets64.exe -f --retry 5
%curl_bin% -kLO %cdn_gc_url%/graphics-hook32.dll -f --retry 5
%curl_bin% -kLO %cdn_gc_url%/graphics-hook64.dll -f --retry 5
%curl_bin% -kLO %cdn_gc_url%/inject-helper32.exe -f --retry 5
%curl_bin% -kLO %cdn_gc_url%/inject-helper64.exe -f --retry 5
popd
endlocal