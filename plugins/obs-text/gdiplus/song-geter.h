#pragma once
#include "song-geter-cfg.h"
#include <wchar.h>

#define STRCMP_CONST(str, const_str) memcmp(str, const_str, sizeof(const_str))
#define WCSCMP_CONST(str, const_str) \
	wmemcmp(str, const_str, sizeof(const_str) / sizeof(*(const_str)))
// this applys to L"12345" too
#define WSTRLEN_CONST(str) (sizeof(str) / sizeof(*(str)) - 1)
#define WCSCPY_CONST(str, const_str) \
	wmemcpy(str, const_str, sizeof(const_str) / sizeof(*(const_str)))

class SongGeter {

public:
	// Microsoft Edge, the white spaces are 0x200b 0x0020
	static constexpr wchar_t const *browsers[] = {L" — Mozilla Firefox",
						      L" - Microsoft"
						      L"\x200b\x0020"
						      L"Edge",
						      //L"Edge",
						      L" - Google Chrome"};
	static constexpr wchar_t browser_app[] = L"- YouTube";
	static char isBrowser(wchar_t *const __restrict title,
			      size_t title_len);
	static bool isSpotify(wchar_t *exeName, wchar_t *className);
	static bool isFoobar2000(wchar_t *exeName, wchar_t *className);
	static bool isOsu(wchar_t *exeName, wchar_t *className);
	// A Desktop App for YouTube Music https://github.com/ytmdesktop/ytmdesktop
	// you need YouTube Music Premium to prevent set advertising as song name potentially
	static bool isYTMDesktop(wchar_t* exeName, wchar_t* className);
	static const wchar_t *
	get_song_browser_youtube(wchar_t *const __restrict title,
				 size_t str_len);
	static const wchar_t *get_title_song(wchar_t *const __restrict title,
					     size_t str_len);
	static const wchar_t *get_song_Spotify(wchar_t *const __restrict title,
					       size_t str_len);
	static const wchar_t *
	get_song_foobar2000(wchar_t *const __restrict title, size_t str_len);
	static const wchar_t *get_song_osu(wchar_t *const __restrict title,
					   size_t str_len);
};

static bool wcs_endWith(wchar_t* __restrict str,
	const wchar_t* __restrict suffixStr, size_t str_len,
	size_t suffix_len)
{
	return (str_len > suffix_len) && (wmemcmp(str + str_len - suffix_len,
		suffixStr, suffix_len) == 0);
}
