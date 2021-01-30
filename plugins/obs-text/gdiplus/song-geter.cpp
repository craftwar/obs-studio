#include "song-geter.h"
#include <wchar.h>
#include <regex>

#define STRCMP_CONST(str, const_str) memcmp(str, const_str, sizeof(const_str))
#define WCSCMP_CONST(str, const_str) \
	wmemcmp(str, const_str, sizeof(const_str) / sizeof(*(const_str)))
// this applys to L"12345" too
#define WSTRLEN_CONST(str) (sizeof(str) / sizeof(*(str)) - 1)
#define WCSCPY_CONST(str, const_str) \
	wmemcpy(str, const_str, sizeof(const_str) / sizeof(*(const_str)))

char SongGeter::isBrowser(wchar_t *const __restrict title, size_t title_len)
{
	// msedge.exe
	for (auto &__restrict brower : SongGeter::browsers) {
		const size_t suffix_len = wcslen(brower);
		if (wcs_endWith(title, brower, title_len, suffix_len))
			return suffix_len;
	}

	// for browser doesn't suffix window title with its name (ex: M$ Edge based on Chromium < 81.0?)
	//constexpr unsigned char app_len = WSTRLEN_CONST(browser_app);
	//if (wcs_endWith(title, browser_app, title_len, app_len))
	//	return 0;

	return -1;
}

bool SongGeter::isSpotify(wchar_t *exeName, wchar_t *className)
{
	return !WCSCMP_CONST(exeName, L"Spotify.exe") &&
	       !WCSCMP_CONST(className, L"Chrome_WidgetWin_0");
}

bool SongGeter::isFoobar2000(wchar_t *exeName, wchar_t *className)
{
	return !WCSCMP_CONST(exeName, L"foobar2000.exe");
	//return !WCSCMP_CONST(exeName, L"foobar2000.exe") &&
	//       !WCSCMP_CONST(className,
	//		    L"{97E27FAA-C0B3-4b8e-A693-ED7881E99FC1}");
}

bool SongGeter::isOsu(wchar_t *exeName, wchar_t *className)
{
	return !WCSCMP_CONST(exeName, L"osu!.exe");
}

const wchar_t *
SongGeter::get_song_browser_youtube(wchar_t *const __restrict title,
				    size_t str_len)
{
	// these title suffixes are locale specific in M$ Edge
	// song name - YouTube - Personal - Microsoft Edge
	// song name - YouTube and 13 more pages - Personal - Microsoft Edge
	constexpr size_t app_len = WSTRLEN_CONST(SongGeter::browser_app);
	static const std::wregex const youtube_regex(
		L".+( - YouTube).*?",
		std::regex::optimize | std::regex::ECMAScript);
	if (str_len > app_len) {
		std::wcmatch match;
		title[str_len] = 0; // don't compare browser_suffix part
		std::regex_match(title, match, youtube_regex);
		if (!match.empty()) {
			const auto pos = match.position(1);
			title[pos] = 0;
			return title;
		}
	}

	// old string match when I don't handle special Edge behavior
	//if (wcs_endWith(title, browser_app, str_len, app_len)) {
	//	title[str_len - app_len - 1] =
	//		'\0'; // remove 1 space before suffix
	//	return title;
	//}
	return nullptr;
}

const wchar_t *SongGeter::get_title_song(wchar_t *const __restrict title,
					 [[maybe_unused]] size_t str_len)
{
	return title;
}

const wchar_t *SongGeter::get_song_Spotify(wchar_t *const __restrict title,
					   [[maybe_unused]] size_t str_len)
{
	// not playing
	if (!WCSCMP_CONST(title, L"Spotify Free") ||
	    !WCSCMP_CONST(title, L"Spotify Premium")) [[unlikely]]
		return L"";
	return title;
}

// endWith case
const wchar_t *SongGeter::get_song_foobar2000(wchar_t *const __restrict title,
					      size_t str_len)
{
	// when not playing, title is "foobar2000 v1.5.3"
	// title suffixes with "[foobar2000]" only when playing

	// decide only by tile, no exeName and className
	static constexpr wchar_t app[] = L"[foobar2000]";
	constexpr size_t app_len = WSTRLEN_CONST(app);
	if (wcs_endWith(title, app, str_len, app_len)) {
		title[str_len - app_len - 1] =
			'\0'; // remove 1 space before suffix
		return title;
	}
	return L"";

	// seems ok but not abosultely safe
	//static constexpr wchar_t app_playing[] = L"[foobar2000]";
	//constexpr unsigned char app_len = WSTRLEN_CONST(app_playing);
	//// not end with ']'
	//if (title[str_len - 1] == L']') {
	//	title[str_len - app_len - 1] =
	//		'\0'; // remove 1 space before suffix
	//	return title;
	//}
	//return nullptr;
}

// startWith case
const wchar_t *SongGeter::get_song_osu(wchar_t *const __restrict title,
				       size_t str_len)
{
	// when not playing, title is "osu!"
	// title starts with "osu!  -" only when playing

	static constexpr wchar_t app[] = L"osu!  -";
	constexpr size_t app_len = WSTRLEN_CONST(app);
	if (str_len > app_len && (wmemcmp(title, app, app_len) == 0))
		return title + app_len + 1; // skip 1 space after prefix

	return nullptr;

	// osu create many windows, use this will do more filtering
	//constexpr unsigned char app_notPlaying_len = WSTRLEN_CONST(L"osu!");
	//static constexpr wchar_t app[] = L"osu!  -";
	//constexpr unsigned char app_len = WSTRLEN_CONST(app);
	//+title	0x0000026b68801ae0 L"__wglDummyWindowFodder"	wchar_t* const
	//if (title[app_notPlaying_len])
	//if (title[app_notPlaying_len] && wmemcmp(title, L"__wglDummyWindowFodder", WSTRLEN_CONST(L"__wglDummyWindowFodder")))
	//	return title + app_len + 1; // skip 1 space after prefix
	//return nullptr;
}
