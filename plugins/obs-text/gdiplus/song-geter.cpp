#include "song-geter.h"
#include <wchar.h>
#include <regex>

static bool wcs_endWith(wchar_t *__restrict str,
			const wchar_t *__restrict suffixStr, size_t str_len,
			size_t suffix_len)
{
	return (str_len > suffix_len) && (wmemcmp(str + str_len - suffix_len,
						  suffixStr, suffix_len) == 0);
}

// size_t is unsigned, so +1 to suffix_len, use 0 represent false
size_t SongGeter::isBrowser(wchar_t *const __restrict title, size_t title_len)
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

	// use -1 instead if browser suffix can be length 0
	return 0;
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

bool SongGeter::isSpotify(wchar_t *exeName, wchar_t *className)
{
	return !WCSCMP_CONST(exeName, L"Spotify.exe") &&
	       !WCSCMP_CONST(className, L"Chrome_WidgetWin_0");
}

bool SongGeter::isVLC(wchar_t *exeName, wchar_t *className)
{
	return !WCSCMP_CONST(exeName, L"vlc.exe") &&
	       !WCSCMP_CONST(className, L"Qt5QWindowIcon");
}

bool SongGeter::isYTMDesktop(wchar_t *exeName, wchar_t *className)
{

	return !WCSCMP_CONST(exeName, L"YouTube Music Desktop App.exe") &&
	       !WCSCMP_CONST(className, L"Chrome_WidgetWin_1");
}

const wchar_t *SongGeter::getBrowserYoutube(wchar_t *const __restrict title,
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
			title[pos] = 0; // remove app suffix
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

// title case
const wchar_t *SongGeter::getTitle(wchar_t *const __restrict title,
				   [[maybe_unused]] size_t str_len)
{
	return title;
}

const wchar_t *SongGeter::getSpotify(wchar_t *const __restrict title,
				     [[maybe_unused]] size_t str_len)
{
	// not playing
	if (!WCSCMP_CONST(title, L"Spotify Free") ||
	    !WCSCMP_CONST(title, L"Spotify Premium")) [[unlikely]]
		return L"";
	return title;
}

// endWith case
const wchar_t *SongGeter::getFoobar2000(wchar_t *const __restrict title,
					size_t str_len)
{
	// when not playing, title is "foobar2000 v1.5.3"
	// title suffixes with "[foobar2000]" only when playing

	// decide only by tile, no exeName and className
	static constexpr wchar_t app[] = L"[foobar2000]";
	constexpr size_t app_len = WSTRLEN_CONST(app);
	if (wcs_endWith(title, app, str_len, app_len)) [[likely]] {
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

const wchar_t *SongGeter::getVLC(wchar_t *const __restrict title,
				 size_t str_len)
{
	// when not playing, title is "VLC media player"
	// title suffixes with "- VLC media player" only when playing and pause
	// ex: - Track01 - VLC media player

	static constexpr wchar_t app[] = L"VLC media player";
	constexpr size_t app_len = WSTRLEN_CONST(app);
	if (str_len == app_len) {
		*title = 0; // not playing, clear song name
		return title;
	} else if (str_len > app_len) [[likely]] {
		title[str_len - app_len - 3] =
			0; // remove 3 character before suffix
		return title;
	}

	return nullptr;
}

// startWith case
const wchar_t *SongGeter::getOsu(wchar_t *const __restrict title,
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
