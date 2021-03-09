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
	static bool isFoobar2000(wchar_t *exeName, wchar_t *className);
	static bool isOsu(wchar_t *exeName, wchar_t *className);
	static bool isSpotify(wchar_t *exeName, wchar_t *className);
	// VLC media player
	static bool isVLC(wchar_t *exeName, wchar_t *className);
	// A Desktop App for YouTube Music https://github.com/ytmdesktop/ytmdesktop
	// you need YouTube Music Premium to prevent set advertising as song name potentially
	static bool isYTMDesktop(wchar_t *exeName, wchar_t *className);
	static const wchar_t *getBrowserYoutube(wchar_t *const __restrict title,
						size_t str_len);
	static const wchar_t *getTitle(wchar_t *const __restrict title,
				       size_t str_len);
	static const wchar_t *getSpotify(wchar_t *const __restrict title,
					 size_t str_len);
	static const wchar_t *getFoobar2000(wchar_t *const __restrict title,
					    size_t str_len);
	static const wchar_t *getOsu(wchar_t *const __restrict title,
				     size_t str_len);
	static const wchar_t *getVLC(wchar_t *const __restrict title,
				     size_t str_len);
};

static bool wcs_endWith(wchar_t *__restrict str,
			const wchar_t *__restrict suffixStr, size_t str_len,
			size_t suffix_len);
