#pragma once

#define ENABLE_ALL 0

#define ENABLE_OSU (0 || ENABLE_ALL)
#define ENABLE_Spotify (0 || ENABLE_ALL)
#define ENABLE_YTMDesktop (0 || ENABLE_ALL)

#if ENABLE_Spotify || ENABLE_OSU || ENABLE_YTMDesktop
#define ENABLE_PLAYING_SONG_LONG_DESC 1
#else
#define ENABLE_PLAYING_SONG_LONG_DESC 0
#endif
