This is craftwar's customized OBS Studio. (improve performance, added features ex: display playing song, VNR translation)
I remove some plugins. You need more features, you need install them yourself or use official OBS Studio version instead.
It share the same settings with official OBS, no extra setup.
Don't install under system folder (ex: C:\Program Files (x86)\), updater can't write. Install in C:\craftwar_OBS or D:\craftwar_OBS ...etc
Don't delete downloaded .7z/zip files, it's used for update check. (delete it only when you encounter download problem)

Microsoft Visual C++ Redistributable for Visual Studio 2017 (x64 and x86) is required.
2015 Redistributable is installed and 2017 is not installed, OBS may work abnormally. Install 2017 please. (don't uninstall old 2015/2017)
https://support.microsoft.com/help/2977003/the-latest-supported-visual-c-downloads

"update and run OBS-blend.cmd"	to update and start OBS blend for general users
	include most OBS plugins (ex: obs-browser), suitable for 90%+ usage
"update and run OBS.cmd"	to update and start OBS CPU-specific/debloated version, more optimizations, most funtions is removed.
	It detects INTEL/AMD CPU automatically and downloads CPU-specific version, crash or close on old computers
"get obs-text.cmd"
	download obs-text.dll only (put updater in OBS root directory)
"get obs-text -CPU.cmd"
	more optimized obs-text.dll (may not run on old computers)

Updater updates craftwar's OBS studio only. (not including plugins you install)
Share updater only, not OBS you get from updating. (CPU specific version is slower in incorrect CPU)


Q:	How do I know I need update plugin?
A:	When OBS version is changed, you may need update it.
	Put newer plugin into OBS to overwrite it or download official OBS and overwite with my updater.

more details (in Traditional Chinese)
http://craftwarblog.blogspot.com/2017/09/chatty-obs-by-craftwar.html


These files should not be used in most cases
"update OBS.cmd"
	to update OBS (auto check AMD/CPU. Pick it manually if it can't detect)
"extract OBS.cmd"
	extract OBS (no download, extract only. use when extract error happens)
"download game capture.cmd"
	download game capture from official OBS
"update updater.cmd"
	to update updater (including doc, change to auto update updater now)
"update and run OBS-AMD.cmd"
	for AMD CPU, higher performance (use only when CPU-specific version can't detect your CPU. lower performance to Intel CPU, use "update and run OBS-INTEL.cmd")
"update OBS -Dev.cmd"
	update OBS Dev (don't use if you are not pro. test only, not always lastest/stable)