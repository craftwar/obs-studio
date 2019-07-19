#include <graphics/math-defs.h>
#include <util/platform.h>
#include <util/util.hpp>
#include <obs-module.h>
#include <sys/stat.h>
#include <windows.h>
#include <gdiplus.h>
#include <algorithm>
#include <string>
#include <memory>

using namespace std;
using namespace Gdiplus;

#define warning(format, ...)                                           \
	blog(LOG_WARNING, "[%s] " format, obs_source_get_name(source), \
	     ##__VA_ARGS__)

#define warn_stat(call)                                                   \
	do {                                                              \
		if (stat != Ok)                                           \
			warning("%s: %s failed (%d)", __FUNCTION__, call, \
				(int)stat);                               \
	} while (false)

#ifndef clamp
#define clamp(val, min_val, max_val) \
	if (val < min_val)           \
		val = min_val;       \
	else if (val > max_val)      \
		val = max_val;
#endif

#define MIN_SIZE_CX 2
#define MIN_SIZE_CY 2
#define MAX_SIZE_CX 16384
#define MAX_SIZE_CY 16384

#define MAX_AREA (4096LL * 4096LL)

#define VNR_SHM_SIZE 1024
#define VNR_SHM TEXT("Local\\VNR_PlotText")
#define VNR_SHM_MUTEX TEXT("Local\\VNR_SHM_MUTEX")
#define VNR_SHM_EVENT TEXT("Local\\VNR_SHM_EVENT")
#define VNR_kyob1010_MultipleStream 0
#define song_thread_version

/* ------------------------------------------------------------------------- */

#define S_FONT "font"
#define S_USE_FILE "read_from_file"
#define S_USE_SONG "get_playing_song"
#define S_USE_VNR "read_from_vnr"
#if VNR_kyob1010_MultipleStream
#define S_VNR_MODE "vnr_mode"
#endif
#define S_FILE "file"
#define S_TEXT "text"
#define S_COLOR "color"
#define S_GRADIENT "gradient"
#define S_GRADIENT_COLOR "gradient_color"
#define S_GRADIENT_DIR "gradient_dir"
#define S_GRADIENT_OPACITY "gradient_opacity"
#define S_ALIGN "align"
#define S_VALIGN "valign"
#define S_OPACITY "opacity"
#define S_BKCOLOR "bk_color"
#define S_BKOPACITY "bk_opacity"
#define S_VERTICAL "vertical"
#define S_OUTLINE "outline"
#define S_OUTLINE_SIZE "outline_size"
#define S_OUTLINE_COLOR "outline_color"
#define S_OUTLINE_OPACITY "outline_opacity"
#define S_CHATLOG_MODE "chatlog"
#define S_CHATLOG_LINES "chatlog_lines"
#define S_EXTENTS "extents"
#define S_EXTENTS_WRAP "extents_wrap"
#define S_EXTENTS_CX "extents_cx"
#define S_EXTENTS_CY "extents_cy"
#define S_TRANSFORM "transform"

#define S_ALIGN_LEFT "left"
#define S_ALIGN_CENTER "center"
#define S_ALIGN_RIGHT "right"

#define S_VALIGN_TOP "top"
#define S_VALIGN_CENTER S_ALIGN_CENTER
#define S_VALIGN_BOTTOM "bottom"

#define S_TRANSFORM_NONE 0
#define S_TRANSFORM_UPPERCASE 1
#define S_TRANSFORM_LOWERCASE 2

#define T_(v) obs_module_text(v)
#define T_FONT T_("Font")
#define T_USE_FILE T_("ReadFromFile")
#define T_FILE T_("TextFile")
#define T_USE_SONG "GetPlayingSong"
#define T_USE_VNR "ReadFromVNR"
#define T_VNR_MODE "VNR text mode"
#define T_TEXT T_("Text")
#define T_COLOR T_("Color")
#define T_GRADIENT T_("Gradient")
#define T_GRADIENT_COLOR T_("Gradient.Color")
#define T_GRADIENT_DIR T_("Gradient.Direction")
#define T_GRADIENT_OPACITY T_("Gradient.Opacity")
#define T_ALIGN T_("Alignment")
#define T_VALIGN T_("VerticalAlignment")
#define T_OPACITY T_("Opacity")
#define T_BKCOLOR T_("BkColor")
#define T_BKOPACITY T_("BkOpacity")
#define T_VERTICAL T_("Vertical")
#define T_OUTLINE T_("Outline")
#define T_OUTLINE_SIZE T_("Outline.Size")
#define T_OUTLINE_COLOR T_("Outline.Color")
#define T_OUTLINE_OPACITY T_("Outline.Opacity")
#define T_CHATLOG_MODE T_("ChatlogMode")
#define T_CHATLOG_LINES T_("ChatlogMode.Lines")
#define T_EXTENTS T_("UseCustomExtents")
#define T_EXTENTS_WRAP T_("UseCustomExtents.Wrap")
#define T_EXTENTS_CX T_("Width")
#define T_EXTENTS_CY T_("Height")
#define T_TRANSFORM T_("Transform")

#define T_FILTER_TEXT_FILES T_("Filter.TextFiles")
#define T_FILTER_ALL_FILES T_("Filter.AllFiles")

#define T_ALIGN_LEFT T_("Alignment.Left")
#define T_ALIGN_CENTER T_("Alignment.Center")
#define T_ALIGN_RIGHT T_("Alignment.Right")

#define T_VALIGN_TOP T_("VerticalAlignment.Top")
#define T_VALIGN_CENTER T_ALIGN_CENTER
#define T_VALIGN_BOTTOM T_("VerticalAlignment.Bottom")

#define T_TRANSFORM_NONE T_("Transform.None")
#define T_TRANSFORM_UPPERCASE T_("Transform.Uppercase")
#define T_TRANSFORM_LOWERCASE T_("Transform.Lowercase")

/* ------------------------------------------------------------------------- */

static inline DWORD get_alpha_val(uint32_t opacity)
{
	return ((opacity * 255 / 100) & 0xFF) << 24;
}

static inline DWORD calc_color(uint32_t color, uint32_t opacity)
{
	return color & 0xFFFFFF | get_alpha_val(opacity);
}

static inline wstring to_wide(const char *utf8)
{
	wstring text;

	size_t len = os_utf8_to_wcs(utf8, 0, nullptr, 0);
	text.resize(len);
	if (len)
		os_utf8_to_wcs(utf8, 0, &text[0], len + 1);

	return text;
}

static inline uint32_t rgb_to_bgr(uint32_t rgb)
{
	return ((rgb & 0xFF) << 16) | (rgb & 0xFF00) | ((rgb & 0xFF0000) >> 16);
}

/* ------------------------------------------------------------------------- */

template<typename T, typename T2, BOOL WINAPI deleter(T2)> class GDIObj {
	T obj = nullptr;

	inline GDIObj &Replace(T obj_)
	{
		if (obj)
			deleter(obj);
		obj = obj_;
		return *this;
	}

public:
	inline GDIObj() {}
	inline GDIObj(T obj_) : obj(obj_) {}
	inline ~GDIObj() { deleter(obj); }

	inline T operator=(T obj_)
	{
		Replace(obj_);
		return obj;
	}

	inline operator T() const { return obj; }

	inline bool operator==(T obj_) const { return obj == obj_; }
	inline bool operator!=(T obj_) const { return obj != obj_; }
};

using HDCObj = GDIObj<HDC, HDC, DeleteDC>;
using HFONTObj = GDIObj<HFONT, HGDIOBJ, DeleteObject>;
using HBITMAPObj = GDIObj<HBITMAP, HGDIOBJ, DeleteObject>;

/* ------------------------------------------------------------------------- */

enum class Align : unsigned char { Left, Center, Right };

enum class VAlign : unsigned char { Top, Center, Bottom };

struct TextSource {
	obs_source_t *source = nullptr;

	gs_texture_t *tex = nullptr;
	uint32_t cx = 0;
	uint32_t cy = 0;

	HDCObj hdc;
	Graphics graphics;

	HFONTObj hfont;
	unique_ptr<Font> font;

	const char *file = nullptr;
	time_t file_timestamp = 0;
	float update_time_elapsed = 0.0f;

	enum class Mode : unsigned char {
		text,
		file,
		song,
		vnr
	} mode = Mode::text;

	static struct SONG {
		HWND hWnd;
		unsigned char browser_suffix_len;
		typedef wchar_t *(*pFn)(wchar_t *const, size_t);
		pFn pFunc;
		HANDLE hThread;
		DWORD thread_id;
		TextSource *thread_owner;
	} song;

	static unsigned char vnr_count;
	static struct SHM {
		HANDLE hMapFile;
		HANDLE hMutex;
		HANDLE hEvent;
		HANDLE hThread;
		TextSource *thread_owner;
		wchar_t *data;
	} shm;

	wstring text;
	wstring face;
	int face_size = 0;
	uint32_t color = 0xFFFFFF;
	uint32_t color2 = 0xFFFFFF;
	float gradient_dir = 0;
	uint32_t opacity = 100;
	uint32_t opacity2 = 100;
	uint32_t bk_color = 0;
	uint32_t bk_opacity = 0;
	Align align = Align::Left;
	VAlign valign = VAlign::Top;
	bool gradient = false;
	bool bold = false;
	bool italic = false;
	bool underline = false;
	bool strikeout = false;
	bool vertical = false;

	bool use_outline = false;
	float outline_size = 0.0f;
	uint32_t outline_color = 0;
	uint32_t outline_opacity = 100;

	bool use_extents = false;
	bool wrap = false;
	uint32_t extents_cx = 0;
	uint32_t extents_cy = 0;

	int text_transform = S_TRANSFORM_NONE;

	bool chatlog_mode = false;
	int chatlog_lines = 6;

	/* --------------------------- */

	inline TextSource(obs_source_t *source_, obs_data_t *settings)
		: source(source_),
		  hdc(CreateCompatibleDC(nullptr)),
		  graphics(hdc)
	{
		obs_source_update(source, settings);
		connect_signal_handler();
	}

	inline ~TextSource()
	{
		if (tex) {
			obs_enter_graphics();
			gs_texture_destroy(tex);
			obs_leave_graphics();
		}
		if (mode == Mode::vnr) {
			--TextSource::vnr_count;
			TextSource::VNR_cleanup();
		} else if (mode == Mode::vnr && song.thread_owner != nullptr) {
			TextSource::song_close_thread();
		}
	}

	void UpdateFont();
	void GetStringFormat(StringFormat &format);
	void RemoveNewlinePadding(const StringFormat &format, RectF &box);
	void CalculateTextSizes(const StringFormat &format, RectF &bounding_box,
				SIZE &text_size);
	void RenderOutlineText(Graphics &graphics, const GraphicsPath &path,
			       const Brush &brush);
	void RenderText();
	void LoadFileText();

	const char *GetMainString(const char *str);

	inline void Update(obs_data_t *settings);
	inline void Tick(float seconds);
	inline void Render();

	static BOOL CALLBACK find_target(const HWND hwnd, const LPARAM lParam);
	BOOL get_song_name(const HWND hwnd);
	// song players
	static constexpr wchar_t *browsers[] = {L" - Mozilla Firefox",
						L" - Google Chrome"};
	inline static unsigned char isBrowser(wchar_t *const title,
					      size_t str_len);
	static wchar_t *get_song_browser_youtube(wchar_t *const title,
						 size_t str_len);
	static wchar_t *get_song_foobar2000(wchar_t *const title,
					    size_t str_len);
	static wchar_t *get_song_osu(wchar_t *const title, size_t str_len);
	void set_song_name(const wchar_t *const name);
	static DWORD WINAPI song_thread(LPVOID lpParam);
	static void Wineventproc(HWINEVENTHOOK hWinEventHook, DWORD event,
				 HWND hwnd, LONG idObject, LONG idChild,
				 DWORD idEventThread, DWORD dwmsEventTime);
	static void song_close_thread();

	void connect_signal_handler();
	static void show_handler(void *data, calldata_t *cd);
	static void hide_handler(void *data, calldata_t *cd);

	inline bool VNR_initial();
	static void VNR_cleanup();
	static void VNR_close_thread();
	static DWORD WINAPI VNR_thread(LPVOID lpParam);
	void ReadFromVNR();
};
struct TextSource::SONG TextSource::song = {};
unsigned char TextSource::vnr_count = 0;
struct TextSource::SHM TextSource::shm = {};

static time_t get_modified_timestamp(const char *filename)
{
	struct stat stats;
	if (os_stat(filename, &stats) != 0)
		return -1;
	return stats.st_mtime;
}

void TextSource::UpdateFont()
{
	hfont = nullptr;
	font.reset(nullptr);

	LOGFONT lf = {};
	lf.lfHeight = face_size;
	lf.lfWeight = bold ? FW_BOLD : FW_DONTCARE;
	lf.lfItalic = italic;
	lf.lfUnderline = underline;
	lf.lfStrikeOut = strikeout;
	lf.lfQuality = ANTIALIASED_QUALITY;
	lf.lfCharSet = DEFAULT_CHARSET;

	if (!face.empty()) {
		const std::size_t length = face.copy(
			lf.lfFaceName,
			sizeof(lf.lfFaceName) / sizeof(lf.lfFaceName[0]));
		lf.lfFaceName[length] = '\0';
		hfont = CreateFontIndirect(&lf);
	}

	if (!hfont) {
		wcscpy(lf.lfFaceName, L"Arial");
		hfont = CreateFontIndirect(&lf);
	}

	if (hfont)
		font.reset(new Font(hdc, hfont));
}

void TextSource::GetStringFormat(StringFormat &format)
{
	UINT flags = StringFormatFlagsNoFitBlackBox |
		     StringFormatFlagsMeasureTrailingSpaces;

	if (vertical)
		flags |= StringFormatFlagsDirectionVertical |
			 StringFormatFlagsDirectionRightToLeft;

	format.SetFormatFlags(flags);
	format.SetTrimming(StringTrimmingWord);

	switch (align) {
	case Align::Left:
		if (vertical)
			format.SetLineAlignment(StringAlignmentFar);
		else
			format.SetAlignment(StringAlignmentNear);
		break;
	case Align::Center:
		if (vertical)
			format.SetLineAlignment(StringAlignmentCenter);
		else
			format.SetAlignment(StringAlignmentCenter);
		break;
	case Align::Right:
		if (vertical)
			format.SetLineAlignment(StringAlignmentNear);
		else
			format.SetAlignment(StringAlignmentFar);
	}

	switch (valign) {
	case VAlign::Top:
		if (vertical)
			format.SetAlignment(StringAlignmentNear);
		else
			format.SetLineAlignment(StringAlignmentNear);
		break;
	case VAlign::Center:
		if (vertical)
			format.SetAlignment(StringAlignmentCenter);
		else
			format.SetLineAlignment(StringAlignmentCenter);
		break;
	case VAlign::Bottom:
		if (vertical)
			format.SetAlignment(StringAlignmentFar);
		else
			format.SetLineAlignment(StringAlignmentFar);
	}
}

/* GDI+ treats '\n' as an extra character with an actual render size when
 * calculating the texture size, so we have to calculate the size of '\n' to
 * remove the padding.  Because we always add a newline to the string, we
 * also remove the extra unused newline. */
void TextSource::RemoveNewlinePadding(const StringFormat &format, RectF &box)
{
	RectF before;
	RectF after;
	Status stat;

	stat = graphics.MeasureString(L"W", 2, font.get(), PointF(0.0f, 0.0f),
				      &format, &before);
	warn_stat("MeasureString (without newline)");

	stat = graphics.MeasureString(L"W\n", 3, font.get(), PointF(0.0f, 0.0f),
				      &format, &after);
	warn_stat("MeasureString (with newline)");

	float offset_cx = after.Width - before.Width;
	float offset_cy = after.Height - before.Height;

	if (!vertical) {
		if (offset_cx >= 1.0f)
			offset_cx -= 1.0f;

		if (valign == VAlign::Center)
			box.Y -= offset_cy * 0.5f;
		else if (valign == VAlign::Bottom)
			box.Y -= offset_cy;
	} else {
		if (offset_cy >= 1.0f)
			offset_cy -= 1.0f;

		if (align == Align::Center)
			box.X -= offset_cx * 0.5f;
		else if (align == Align::Right)
			box.X -= offset_cx;
	}

	box.Width -= offset_cx;
	box.Height -= offset_cy;
}

void TextSource::CalculateTextSizes(const StringFormat &format,
				    RectF &bounding_box, SIZE &text_size)
{
	RectF layout_box;
	RectF temp_box;
	Status stat;

	if (!text.empty()) {
		if (use_extents && wrap) {
			layout_box.X = layout_box.Y = 0;
			layout_box.Width = float(extents_cx);
			layout_box.Height = float(extents_cy);

			if (use_outline) {
				layout_box.Width -= outline_size;
				layout_box.Height -= outline_size;
			}

			stat = graphics.MeasureString(text.c_str(),
						      (int)text.size() + 1,
						      font.get(), layout_box,
						      &format, &bounding_box);
			warn_stat("MeasureString (wrapped)");

			temp_box = bounding_box;
		} else {
			stat = graphics.MeasureString(
				text.c_str(), (int)text.size() + 1, font.get(),
				PointF(0.0f, 0.0f), &format, &bounding_box);
			warn_stat("MeasureString (non-wrapped)");

			temp_box = bounding_box;

			bounding_box.X = 0.0f;
			bounding_box.Y = 0.0f;

			RemoveNewlinePadding(format, bounding_box);

			if (use_outline) {
				bounding_box.Width += outline_size;
				bounding_box.Height += outline_size;
			}
		}
	}

	if (vertical) {
		if (bounding_box.Width < face_size) {
			text_size.cx = face_size;
			bounding_box.Width = float(face_size);
		} else {
			text_size.cx = LONG(bounding_box.Width + EPSILON);
		}

		text_size.cy = LONG(bounding_box.Height + EPSILON);
	} else {
		if (bounding_box.Height < face_size) {
			text_size.cy = face_size;
			bounding_box.Height = float(face_size);
		} else {
			text_size.cy = LONG(bounding_box.Height + EPSILON);
		}

		text_size.cx = LONG(bounding_box.Width + EPSILON);
	}

	if (use_extents) {
		text_size.cx = extents_cx;
		text_size.cy = extents_cy;
	}

	text_size.cx += text_size.cx % 2;
	text_size.cy += text_size.cy % 2;

	int64_t total_size = int64_t(text_size.cx) * int64_t(text_size.cy);

	/* GPUs typically have texture size limitations */
	clamp(text_size.cx, MIN_SIZE_CX, MAX_SIZE_CX);
	clamp(text_size.cy, MIN_SIZE_CY, MAX_SIZE_CY);

	/* avoid taking up too much VRAM */
	if (total_size > MAX_AREA) {
		if (text_size.cx > text_size.cy)
			text_size.cx = (LONG)MAX_AREA / text_size.cy;
		else
			text_size.cy = (LONG)MAX_AREA / text_size.cx;
	}

	/* the internal text-rendering bounding box for is reset to
	 * its internal value in case the texture gets cut off */
	bounding_box.Width = temp_box.Width;
	bounding_box.Height = temp_box.Height;
}

void TextSource::RenderOutlineText(Graphics &graphics, const GraphicsPath &path,
				   const Brush &brush)
{
	DWORD outline_rgba = calc_color(outline_color, outline_opacity);
	Status stat;

	Pen pen(Color(outline_rgba), outline_size);
	stat = pen.SetLineJoin(LineJoinRound);
	warn_stat("pen.SetLineJoin");

	stat = graphics.DrawPath(&pen, &path);
	warn_stat("graphics.DrawPath");

	stat = graphics.FillPath(&brush, &path);
	warn_stat("graphics.FillPath");
}

void TextSource::RenderText()
{
	if (text_transform == S_TRANSFORM_UPPERCASE)
		transform(text.begin(), text.end(), text.begin(), towupper);
	else if (text_transform == S_TRANSFORM_LOWERCASE)
		transform(text.begin(), text.end(), text.begin(), towlower);

	StringFormat format(StringFormat::GenericTypographic());
	Status stat;

	RectF box;
	SIZE size;

	GetStringFormat(format);
	CalculateTextSizes(format, box, size);

	unique_ptr<uint8_t> bits(new uint8_t[size.cx * size.cy * 4]);
	Bitmap bitmap(size.cx, size.cy, 4 * size.cx, PixelFormat32bppARGB,
		      bits.get());

	Graphics graphics_bitmap(&bitmap);
	LinearGradientBrush brush(RectF(0, 0, (float)size.cx, (float)size.cy),
				  Color(calc_color(color, opacity)),
				  Color(calc_color(color2, opacity2)),
				  gradient_dir, 1);
	DWORD full_bk_color = bk_color & 0xFFFFFF;

	if (!text.empty() || use_extents)
		full_bk_color |= get_alpha_val(bk_opacity);

	if ((size.cx > box.Width || size.cy > box.Height) && !use_extents) {
		stat = graphics_bitmap.Clear(Color(0));
		warn_stat("graphics_bitmap.Clear");

		SolidBrush bk_brush = Color(full_bk_color);
		stat = graphics_bitmap.FillRectangle(&bk_brush, box);
		warn_stat("graphics_bitmap.FillRectangle");
	} else {
		stat = graphics_bitmap.Clear(Color(full_bk_color));
		warn_stat("graphics_bitmap.Clear");
	}

	graphics_bitmap.SetTextRenderingHint(TextRenderingHintAntiAlias);
	graphics_bitmap.SetCompositingMode(CompositingModeSourceOver);
	graphics_bitmap.SetSmoothingMode(SmoothingModeAntiAlias);

	if (!text.empty()) {
		if (use_outline) {
			box.Offset(outline_size / 2, outline_size / 2);

			FontFamily family;
			GraphicsPath path;

			font->GetFamily(&family);
			stat = path.AddString(text.c_str(), (int)text.size(),
					      &family, font->GetStyle(),
					      font->GetSize(), box, &format);
			warn_stat("path.AddString");

			RenderOutlineText(graphics_bitmap, path, brush);
		} else {
			stat = graphics_bitmap.DrawString(text.c_str(),
							  (int)text.size(),
							  font.get(), box,
							  &format, &brush);
			warn_stat("graphics_bitmap.DrawString");
		}
	}

	if (!tex || (LONG)cx != size.cx || (LONG)cy != size.cy) {
		obs_enter_graphics();
		if (tex)
			gs_texture_destroy(tex);

		const uint8_t *data = (uint8_t *)bits.get();
		tex = gs_texture_create(size.cx, size.cy, GS_BGRA, 1, &data,
					GS_DYNAMIC);

		obs_leave_graphics();

		cx = (uint32_t)size.cx;
		cy = (uint32_t)size.cy;

	} else if (tex) {
		obs_enter_graphics();
		gs_texture_set_image(tex, bits.get(), size.cx * 4, false);
		obs_leave_graphics();
	}
}

const char *TextSource::GetMainString(const char *str)
{
	if (!str)
		return "";
	if (!chatlog_mode || !chatlog_lines)
		return str;

	int lines = chatlog_lines;
	size_t len = strlen(str);
	if (!len)
		return str;

	const char *temp = str + len;

	while (temp != str) {
		temp--;

		if (temp[0] == '\n' && temp[1] != 0) {
			if (!--lines)
				break;
		}
	}

	return *temp == '\n' ? temp + 1 : temp;
}

void TextSource::LoadFileText()
{
	BPtr<char> file_text = os_quick_read_utf8_file(file);
	text = to_wide(GetMainString(file_text));

	if (!text.empty() && text.back() != '\n')
		text.push_back('\n');
	RenderText();
}

#define obs_data_get_uint32 (uint32_t) obs_data_get_int

#pragma optimize("s", on)
inline void TextSource::Update(obs_data_t *s)
{
	obs_data_t *font_obj = obs_data_get_obj(s, S_FONT);
	const char *align_str = obs_data_get_string(s, S_ALIGN);
	const char *valign_str = obs_data_get_string(s, S_VALIGN);
	color = rgb_to_bgr(obs_data_get_uint32(s, S_COLOR));
	opacity = obs_data_get_uint32(s, S_OPACITY);
	bool gradient = obs_data_get_bool(s, S_GRADIENT);
	color2 = rgb_to_bgr(obs_data_get_uint32(s, S_GRADIENT_COLOR));
	opacity2 = obs_data_get_uint32(s, S_GRADIENT_OPACITY);
	gradient_dir = (float)obs_data_get_double(s, S_GRADIENT_DIR);
	vertical = obs_data_get_bool(s, S_VERTICAL);
	use_outline = obs_data_get_bool(s, S_OUTLINE);
	outline_color = rgb_to_bgr(obs_data_get_uint32(s, S_OUTLINE_COLOR));
	outline_opacity = obs_data_get_uint32(s, S_OUTLINE_OPACITY);
	outline_size = roundf(obs_data_get_uint32(s, S_OUTLINE_SIZE));
#if VNR_kyob1010_MultipleStream
	vnr_mode = obs_data_get_string(s, S_VNR_MODE);
#endif
	file = obs_data_get_string(s, S_FILE);
	chatlog_mode = obs_data_get_bool(s, S_CHATLOG_MODE);
	chatlog_lines = (int)obs_data_get_int(s, S_CHATLOG_LINES);
	use_extents = obs_data_get_bool(s, S_EXTENTS);
	wrap = obs_data_get_bool(s, S_EXTENTS_WRAP);
	extents_cx = obs_data_get_uint32(s, S_EXTENTS_CX);
	extents_cy = obs_data_get_uint32(s, S_EXTENTS_CY);
	text_transform = (int)obs_data_get_int(s, S_TRANSFORM);

	wstring new_face = to_wide(obs_data_get_string(font_obj, "face"));
	int font_size = (int)obs_data_get_int(font_obj, "size");
	int64_t font_flags = obs_data_get_int(font_obj, "flags");
	bool new_bold = (font_flags & OBS_FONT_BOLD) != 0;
	bool new_italic = (font_flags & OBS_FONT_ITALIC) != 0;
	bool new_underline = (font_flags & OBS_FONT_UNDERLINE) != 0;
	bool new_strikeout = (font_flags & OBS_FONT_STRIKEOUT) != 0;

	bk_color = rgb_to_bgr(obs_data_get_uint32(s, S_BKCOLOR));
	bk_opacity = obs_data_get_uint32(s, S_BKOPACITY);

	/* ----------------------------- */

	if (new_face != face || face_size != font_size || new_bold != bold ||
	    new_italic != italic || new_underline != underline ||
	    new_strikeout != strikeout) {

		face = new_face;
		face_size = font_size;
		bold = new_bold;
		italic = new_italic;
		underline = new_underline;
		strikeout = new_strikeout;

		UpdateFont();
	}

	/* ----------------------------- */

	if (!gradient) {
		color2 = color;
		opacity2 = opacity;
	}

	if (strcmp(align_str, S_ALIGN_CENTER) == 0)
		align = Align::Center;
	else if (strcmp(align_str, S_ALIGN_RIGHT) == 0)
		align = Align::Right;
	else
		align = Align::Left;

	if (strcmp(valign_str, S_VALIGN_CENTER) == 0)
		valign = VAlign::Center;
	else if (strcmp(valign_str, S_VALIGN_BOTTOM) == 0)
		valign = VAlign::Bottom;
	else
		valign = VAlign::Top;

	TextSource::Mode old_mode = mode;
	if (obs_data_get_bool(s, S_USE_VNR)) {
		if (VNR_initial()) {
			if (old_mode != Mode::vnr) {
				++TextSource::vnr_count;
				mode = Mode::vnr;
			}
		} else { // fallback to text mode
			obs_data_set_bool(s, S_USE_VNR, false);
			goto fallback_to_text_mode;
		}
	} else if (obs_data_get_bool(s, S_USE_SONG)) {
		mode = Mode::song;
		get_song_name(song.hWnd);
	} else if (obs_data_get_bool(s, S_USE_FILE)) {
		mode = Mode::file;
		file_timestamp = get_modified_timestamp(file);
		LoadFileText();
	} else {
	fallback_to_text_mode:
		mode = Mode::text;
		const char *const new_text = obs_data_get_string(s, S_TEXT);
		text = to_wide(GetMainString(new_text));

		/* all text should end with newlines due to the fact that GDI+
			* treats strings without newlines differently in terms of
			* render size */
		if (!text.empty())
			text.push_back('\n');
		RenderText();
	}
	// close resource if mode is unused
	if (old_mode != mode) {
		if (old_mode == Mode::vnr) { // change from vnr mode
			--TextSource::vnr_count;
			TextSource::VNR_cleanup();
		} else if (old_mode == Mode::song) {
			song_close_thread();
		}
	}

	update_time_elapsed = 0.0f;

	/* ----------------------------- */

	obs_data_release(font_obj);
}

inline void TextSource::Tick(float seconds)
{
	if (!obs_source_showing(source))
		return;

	update_time_elapsed += seconds;
	if (update_time_elapsed < 2.0f)
		return;
	else
		update_time_elapsed = 0.0f;

	// no difference in using case 0 or default (same asm)
	// threaded code, gcc Labels as Values is not supported in MSVC
	// https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
	switch (mode) {
	case Mode::file: {
		time_t t = get_modified_timestamp(file);

		if (file_timestamp != t) {
			file_timestamp = t;
			LoadFileText();
		}
	} break;
	case Mode::song:
		if (song.thread_owner)
			return;
		if (!get_song_name(song.hWnd)) {
			::EnumWindows(&TextSource::find_target,
				      reinterpret_cast<LPARAM>(this));
		}
		// lambda can't be used (will be this call, not callback)
		//::EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
		//{
		//	TextSource* _TextSource = reinterpret_cast<TextSource *>(lParam);
		//	return !_TextSource->get_song_name(hwnd);
		//}
		//, reinterpret_cast<LPARAM>(this));
		break;
	default: // don't produce code for default
		// https://docs.microsoft.com/en-us/cpp/intrinsics/assume
		//__assume(0);
		break;
	}
}

inline void TextSource::Render()
{
	if (!tex)
		return;

	gs_effect_t *effect = obs_get_base_effect(OBS_EFFECT_DEFAULT);
	gs_technique_t *tech = gs_effect_get_technique(effect, "Draw");

	gs_technique_begin(tech);
	gs_technique_begin_pass(tech, 0);

	gs_effect_set_texture(gs_effect_get_param_by_name(effect, "image"),
			      tex);
	gs_draw_sprite(tex, 0, cx, cy);

	gs_technique_end_pass(tech);
	gs_technique_end(tech);
}

BOOL TextSource::get_song_name(const HWND hwnd)
{
	const int len = GetWindowTextLengthW(hwnd);
	if (!len)
		return FALSE;
	std::unique_ptr<wchar_t> title(new wchar_t[len + 1]);
	if (!title || !GetWindowTextW(hwnd, title.get(), len + 1))
		return FALSE;

#ifndef song_thread_version
	if (len > song.browser_suffix_len && song.pFunc) {
		wchar_t *const song_name = (*song.pFunc)(
			title.get(), len - song.browser_suffix_len);
		if (!song_name) {
			song.hWnd = NULL;
			song.pFunc = nullptr;
			text = L"";
			RenderText();
		} else {
			set_song_name(song_name);
			return TRUE;
		}
	}
#endif

	// Using "else if"s produces bigger binary (why?)
	song.browser_suffix_len = isBrowser(title.get(), len);
	wchar_t *song_name =
		song.browser_suffix_len
			? get_song_browser_youtube(
				  title.get(), len - song.browser_suffix_len)
			: nullptr;
	if (song_name) {
		song.pFunc = &TextSource::get_song_browser_youtube;
		goto song_found;
	}
	song_name = get_song_foobar2000(title.get(), len);
	if (song_name) {
		song.pFunc = &TextSource::get_song_foobar2000;
		goto song_found;
	}
	song_name = get_song_osu(title.get(), len);
	if (song_name)
		song.pFunc = &TextSource::get_song_osu;
	else
		goto song_not_found;

song_found:
	set_song_name(song_name);
	// If you wana switch player, disable then enable song mode again
	song.hWnd = hwnd;
	if (song.thread_owner == NULL) {
		TextSource::song.hThread = CreateThread(
			NULL, 0, song_thread, this, 0, &song.thread_id);
		if (TextSource::song.hThread) {
			// I don't need this handle, close it early
			CloseHandle(song.hThread);
			song.thread_owner = this;
		}
	} else
		song_close_thread();

song_not_found:
	return !!song_name;
}

// reutrn endString length or 0
unsigned char wcs_endWith(wchar_t *str, wchar_t *strSearch, size_t str_len)
{
	const size_t suffix_len = wcslen(strSearch);
	if ((str_len > suffix_len) &&
	    (wmemcmp(str + str_len - suffix_len, strSearch, suffix_len) == 0))
		return suffix_len;

	return 0;
}

unsigned char TextSource::isBrowser(wchar_t *const title, size_t str_len)
{
	for (auto &brower : TextSource::browsers) {
		const unsigned char len = wcs_endWith(title, brower, str_len);
		if (len)
			return len;
	}
	return 0;
}

wchar_t *TextSource::get_song_browser_youtube(wchar_t *const title,
					      size_t str_len)
{
	const unsigned char len = wcs_endWith(title, L"- YouTube", str_len);
	if (len) {
		*(title + str_len - len - 1) =
			'\0'; // remove 1 space before suffix
		return title;
	}
	return nullptr;
}

wchar_t *TextSource::get_song_foobar2000(wchar_t *const title, size_t str_len)
{
	const unsigned char len = wcs_endWith(title, L"[foobar2000]", str_len);
	if (len) {
		*(title + str_len - len - 1) =
			'\0'; // remove 1 space before suffix
		return title;
	}
	return nullptr;
}

wchar_t *TextSource::get_song_osu(wchar_t *const title, size_t str_len)
{
	static constexpr wchar_t app[] = L"osu!  -";
	constexpr unsigned char app_len = sizeof(app) / sizeof(*app) - 1;
	if (str_len > app_len && (wmemcmp(title, app, app_len) == 0))
		return title + app_len + 1; // skip 1 space after prefix

	return nullptr;
}

void TextSource::set_song_name(const wchar_t *const name)
{
	// unnecessary in song_thread version
	//if (text.compare(name)) {
	text = name;
	text.push_back('\n');
	RenderText();
	//}
}

DWORD __stdcall TextSource::song_thread(LPVOID lpParam)
{
	TextSource *s = reinterpret_cast<TextSource *>(lpParam);

	DWORD error;
	//WNDCLASSW wndClass = {0};
	//wndClass.lpfnWndProc = DefWindowProcW;
	//wndClass.hInstance = GetModuleHandleA("User32.dll");
	//wndClass.lpszClassName = L"a";
	//ATOM atom = RegisterClassW(&wndClass);
	//ATOM atom = RegisterClassExW(&wndClass);
	// use System registered class "Message"
	// https://docs.microsoft.com/en-us/windows/win32/winmsg/about-window-classes#system
	CreateWindowW(L"Message", L"", 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
	error = GetLastError();
	MSG Msg;
	DWORD process_id;
	DWORD thread_id = GetWindowThreadProcessId(song.hWnd, &process_id);
	HWINEVENTHOOK hHook = SetWinEventHook(EVENT_OBJECT_NAMECHANGE,
					      EVENT_OBJECT_NAMECHANGE, NULL,
					      Wineventproc, process_id,
					      thread_id, WINEVENT_OUTOFCONTEXT);
	while (GetMessage(&Msg, NULL, 0, 0))
		;
	UnhookWinEvent(hHook);
	//CloseHandle(song.hThread);
	song.hWnd = NULL;
	song.hThread = NULL;
	song.thread_owner = nullptr;

	return 0;
}

void TextSource::Wineventproc(HWINEVENTHOOK hWinEventHook, DWORD event,
			      HWND hwnd, LONG idObject, LONG idChild,
			      DWORD idEventThread, DWORD dwmsEventTime)
{
	if (idObject == OBJID_WINDOW) {

		const int len = GetWindowTextLengthW(hwnd);
		if (!len)
			return;
		std::unique_ptr<wchar_t> title(new wchar_t[len + 1]);
		if (!title || !GetWindowTextW(hwnd, title.get(), len + 1))
			return;
		wchar_t *song_name = (song.thread_owner->song.pFunc)(
			title.get(),
			len - song.thread_owner->song.browser_suffix_len);
		// if not found, close thread?
		// can't handle window close
		if (song_name == nullptr)
			song_name = L"";
		song.thread_owner->set_song_name(song_name);
	}
}

void TextSource::song_close_thread()
{
	PostThreadMessageW(song.thread_id, WM_QUIT, 0, 0);
}

void TextSource::connect_signal_handler()
{
	// trigger oder: show, hide (fixed, not random, defined in OBS internal)
	signal_handler_t *handler = obs_source_get_signal_handler(this->source);
	signal_handler_connect(handler, "hide", hide_handler, this);
	signal_handler_connect(handler, "show", show_handler, this);
}

void TextSource::show_handler(void *data, [[maybe_unused]] calldata_t *cd)
{
	//void *a = calldata_ptr(cd, "source");
	TextSource *const s = reinterpret_cast<TextSource *>(data);
	switch (s->mode) {
	case TextSource::Mode::vnr:
		if (shm.thread_owner != s) {
			TextSource::VNR_close_thread();
			TextSource::shm.hThread =
				CreateThread(NULL, 0, VNR_thread, s, 0, NULL);
			if (shm.hThread)
				shm.thread_owner = s;
		}
		break;
	case TextSource::Mode::song:
		if (song.thread_owner != s)
			TextSource::song_close_thread();
		break;
	default:
		break;
	}
}

void TextSource::hide_handler(void *data, calldata_t *cd)
{
	TextSource *const s = reinterpret_cast<TextSource *>(data);

	// maybe if can be deleted, use switch is faster (use if for safty)
	if (shm.thread_owner == s)
		TextSource::VNR_close_thread();
	else if (song.thread_owner == s)
		TextSource::song_close_thread();

	/* 
	switch (s->mode) {
	case TextSource::Mode::vnr:
		if (shm.thread_owner == s)
			TextSource::VNR_close_thread();
		break;
	case TextSource::Mode::song:
		if (song.thread_owner == s)
			TextSource::song_close_thread();
		break;
	default:
		break;
	}
	*/
}

#pragma optimize("s", on)
bool TextSource::VNR_initial()
{
	if (shm.hThread == NULL) {
		shm.hMapFile = OpenFileMapping(
			FILE_MAP_ALL_ACCESS, // read/write access
			FALSE,               // do not inherit the name
			VNR_SHM);            // name of mapping object
		if (shm.hMapFile == NULL) {
			shm.hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE,
							 NULL, PAGE_READWRITE,
							 0, VNR_SHM_SIZE,
							 VNR_SHM);
			if (shm.hMapFile == NULL)
				goto SHM_fail;
		}

		TextSource::shm.data = static_cast<wchar_t *>(MapViewOfFile(
			shm.hMapFile,        // handle to map object
			FILE_MAP_ALL_ACCESS, // read/write permission
			0, 0, VNR_SHM_SIZE));
		if (TextSource::shm.data == nullptr)
			goto SHM_error_clean;

		// No initialization is required https://docs.microsoft.com/en-us/windows/desktop/api/winbase/nf-winbase-createfilemappinga
		// The initial contents of the pages in a file mapping object backed by
		// the operating system paging file are 0 (zero).

		TextSource::shm.hMutex =
			OpenMutex(SYNCHRONIZE, FALSE, VNR_SHM_MUTEX);
		if (shm.hMutex == NULL) {
			shm.hMutex = CreateMutex(NULL, false, VNR_SHM_MUTEX);
			if (shm.hMutex == NULL)
				goto SHM_error_clean;
		}
		TextSource::shm.hEvent = OpenEvent(
			SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, VNR_SHM_EVENT);
		if (shm.hEvent == NULL) {
			shm.hEvent =
				CreateEvent(NULL, true, false, VNR_SHM_EVENT);
			if (shm.hEvent == NULL)
				goto SHM_error_clean;
		}
		// don't use mutiple obs-text in vnr mode
		// I am lazy
		// void signal_handler_connect
		// source_show (ptr source), source_hide (ptr source)
		TextSource::shm.hThread =
			CreateThread(NULL, 0, VNR_thread, this, 0, NULL);
		if (shm.hThread == NULL)
			goto SHM_error_clean;
		else
			shm.thread_owner = this;
	}
	return true;

SHM_error_clean:
	TextSource::VNR_cleanup();
SHM_fail:
	return false;
}

BOOL CALLBACK TextSource::find_target(const HWND hwnd, const LPARAM lParam)
{
	return !reinterpret_cast<TextSource *>(lParam)->get_song_name(hwnd);
}

#pragma optimize("s", on)
void TextSource::VNR_cleanup()
{
	if (TextSource::vnr_count != 0)
		return;

	if (TextSource::shm.hThread)
		goto close_thread;
	if (TextSource::shm.hEvent)
		goto close_event;
	if (TextSource::shm.hMutex)
		goto close_mutex;
	if (TextSource::shm.data)
		goto unmap_view;
	if (TextSource::shm.hMapFile)
		goto close_map;
close_thread:
	TextSource::VNR_close_thread();
close_event:
	CloseHandle(TextSource::shm.hEvent);
	TextSource::shm.hEvent = NULL;
close_mutex:
	CloseHandle(TextSource::shm.hMutex);
	TextSource::shm.hMutex = NULL;
unmap_view:
	UnmapViewOfFile(TextSource::shm.data);
	TextSource::shm.data = nullptr;
close_map:
	CloseHandle(TextSource::shm.hMapFile);
	TextSource::shm.hMapFile = NULL;
}

void TextSource::VNR_close_thread()
{
	TerminateThread(TextSource::shm.hThread, 0);
	CloseHandle(TextSource::shm.hThread);
	TextSource::shm.hThread = NULL;
	TextSource::shm.thread_owner = nullptr;
}

DWORD WINAPI TextSource::VNR_thread(LPVOID lpParam)
{
	for (;;)
		reinterpret_cast<TextSource *>(lpParam)->ReadFromVNR();
	return 1;
}

void TextSource::ReadFromVNR()
{
#if VNR_kyob1010_MultipleStream
	switch (*vnr_mode) {
	case 'o':
		break;
	case 't':
		break;
	case 'b':
	default:
		break;
	}
#endif
	WaitForSingleObject(TextSource::shm.hEvent, INFINITE);
	if (WaitForSingleObject(TextSource::shm.hMutex, INFINITE) ==
	    WAIT_OBJECT_0) {
		ResetEvent(TextSource::shm.hEvent);
		text = TextSource::shm.data;
		// text always not empty? better let vnr add '\n'
		//text.push_back('\n');
		RenderText();
	}
	ReleaseMutex(TextSource::shm.hMutex);
}

/* ------------------------------------------------------------------------- */

static ULONG_PTR gdip_token = 0;

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-text", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "Windows GDI+ text source";
}

#define set_vis(var, val, show)                           \
	do {                                              \
		p = obs_properties_get(props, val);       \
		obs_property_set_visible(p, var == show); \
	} while (false)

static bool use_file_changed(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	bool use_file = obs_data_get_bool(s, S_USE_FILE);
	if (use_file) {
		obs_data_set_bool(s, S_USE_SONG, false);
		obs_data_set_bool(s, S_USE_VNR, false);
#if VNR_kyob1010_MultipleStream
		p = obs_properties_get(props, S_VNR_MODE);
		obs_property_set_visible(p, false);
#endif
	}
	set_vis(use_file, S_TEXT, false);
	set_vis(use_file, S_FILE, true);
	return true;
}

static bool use_song_changed(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	bool use_song = obs_data_get_bool(s, S_USE_SONG);
	if (use_song) {
		obs_data_set_bool(s, S_USE_FILE, false);
		obs_data_set_bool(s, S_USE_VNR, false);
		p = obs_properties_get(props, S_FILE);
		obs_property_set_visible(p, false);
#if VNR_kyob1010_MultipleStream
		p = obs_properties_get(props, S_VNR_MODE);
		obs_property_set_visible(p, false);
#endif
	}

	set_vis(use_song, S_TEXT, false);
	return true;
}

static bool use_vnr_changed(obs_properties_t *props, obs_property_t *p,
			    obs_data_t *s)
{
	bool use_vnr = obs_data_get_bool(s, S_USE_VNR);
	if (use_vnr) {
		obs_data_set_bool(s, S_USE_FILE, false);
		obs_data_set_bool(s, S_USE_SONG, false);
		p = obs_properties_get(props, S_FILE);
		obs_property_set_visible(p, false);
	}

	set_vis(use_vnr, S_TEXT, false);
#if VNR_kyob1010_MultipleStream
	set_vis(use_vnr, S_VNR_MODE, true);
#endif
	return true;
}

static bool outline_changed(obs_properties_t *props, obs_property_t *p,
			    obs_data_t *s)
{
	bool outline = obs_data_get_bool(s, S_OUTLINE);

	set_vis(outline, S_OUTLINE_SIZE, true);
	set_vis(outline, S_OUTLINE_COLOR, true);
	set_vis(outline, S_OUTLINE_OPACITY, true);
	return true;
}

static bool chatlog_mode_changed(obs_properties_t *props, obs_property_t *p,
				 obs_data_t *s)
{
	bool chatlog_mode = obs_data_get_bool(s, S_CHATLOG_MODE);

	set_vis(chatlog_mode, S_CHATLOG_LINES, true);
	return true;
}

static bool gradient_changed(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	bool gradient = obs_data_get_bool(s, S_GRADIENT);

	set_vis(gradient, S_GRADIENT_COLOR, true);
	set_vis(gradient, S_GRADIENT_OPACITY, true);
	set_vis(gradient, S_GRADIENT_DIR, true);
	return true;
}

static bool extents_modified(obs_properties_t *props, obs_property_t *p,
			     obs_data_t *s)
{
	bool use_extents = obs_data_get_bool(s, S_EXTENTS);

	set_vis(use_extents, S_EXTENTS_WRAP, true);
	set_vis(use_extents, S_EXTENTS_CX, true);
	set_vis(use_extents, S_EXTENTS_CY, true);
	return true;
}

#undef set_vis

#pragma optimize("s", on)
static obs_properties_t *get_properties(void *data)
{
	TextSource *s = reinterpret_cast<TextSource *>(data);
	string path;

	obs_properties_t *props = obs_properties_create();
	obs_property_t *p;

	obs_properties_add_font(props, S_FONT, T_FONT);

	p = obs_properties_add_bool(props, S_USE_FILE, T_USE_FILE);
	obs_property_set_modified_callback(p, use_file_changed);
	p = obs_properties_add_bool(props, S_USE_SONG, T_USE_SONG);
	obs_property_set_modified_callback(p, use_song_changed);
	p = obs_properties_add_bool(props, S_USE_VNR, T_USE_VNR);
	obs_property_set_modified_callback(p, use_vnr_changed);
#if VNR_kyob1010_MultipleStream
	p = obs_properties_add_list(props, S_VNR_MODE, T_VNR_MODE,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, "original", "o");
	obs_property_list_add_string(p, "translation", "t");
	obs_property_list_add_string(p, "original + translation", "b");
#endif

	string filter;
	filter += T_FILTER_TEXT_FILES;
	filter += " (*.txt);;";
	filter += T_FILTER_ALL_FILES;
	filter += " (*.*)";

	// s won't be nullptr?
	if (s->file && *(s->file) != '\0') {
		path = s->file;
		replace(path.begin(), path.end(), '\\', '/');
		const size_t pos = path.find_last_of('/');
		if (pos != path.npos)
			path.resize(pos + 1);
	}

	obs_properties_add_text(props, S_TEXT, T_TEXT, OBS_TEXT_MULTILINE);
	obs_properties_add_path(props, S_FILE, T_FILE, OBS_PATH_FILE,
				filter.c_str(), path.c_str());

	p = obs_properties_add_list(props, S_TRANSFORM, T_TRANSFORM,
				    OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(p, T_TRANSFORM_NONE, S_TRANSFORM_NONE);
	obs_property_list_add_int(p, T_TRANSFORM_UPPERCASE,
				  S_TRANSFORM_UPPERCASE);
	obs_property_list_add_int(p, T_TRANSFORM_LOWERCASE,
				  S_TRANSFORM_LOWERCASE);

	obs_properties_add_bool(props, S_VERTICAL, T_VERTICAL);
	obs_properties_add_color(props, S_COLOR, T_COLOR);
	obs_properties_add_int_slider(props, S_OPACITY, T_OPACITY, 0, 100, 1);

	p = obs_properties_add_bool(props, S_GRADIENT, T_GRADIENT);
	obs_property_set_modified_callback(p, gradient_changed);

	obs_properties_add_color(props, S_GRADIENT_COLOR, T_GRADIENT_COLOR);
	obs_properties_add_int_slider(props, S_GRADIENT_OPACITY,
				      T_GRADIENT_OPACITY, 0, 100, 1);
	obs_properties_add_float_slider(props, S_GRADIENT_DIR, T_GRADIENT_DIR,
					0, 360, 0.1);

	obs_properties_add_color(props, S_BKCOLOR, T_BKCOLOR);
	obs_properties_add_int_slider(props, S_BKOPACITY, T_BKOPACITY, 0, 100,
				      1);

	p = obs_properties_add_list(props, S_ALIGN, T_ALIGN,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_ALIGN_LEFT, S_ALIGN_LEFT);
	obs_property_list_add_string(p, T_ALIGN_CENTER, S_ALIGN_CENTER);
	obs_property_list_add_string(p, T_ALIGN_RIGHT, S_ALIGN_RIGHT);

	p = obs_properties_add_list(props, S_VALIGN, T_VALIGN,
				    OBS_COMBO_TYPE_LIST,
				    OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(p, T_VALIGN_TOP, S_VALIGN_TOP);
	obs_property_list_add_string(p, T_VALIGN_CENTER, S_VALIGN_CENTER);
	obs_property_list_add_string(p, T_VALIGN_BOTTOM, S_VALIGN_BOTTOM);

	p = obs_properties_add_bool(props, S_OUTLINE, T_OUTLINE);
	obs_property_set_modified_callback(p, outline_changed);

	obs_properties_add_int(props, S_OUTLINE_SIZE, T_OUTLINE_SIZE, 1, 20, 1);
	obs_properties_add_color(props, S_OUTLINE_COLOR, T_OUTLINE_COLOR);
	obs_properties_add_int_slider(props, S_OUTLINE_OPACITY,
				      T_OUTLINE_OPACITY, 0, 100, 1);

	p = obs_properties_add_bool(props, S_CHATLOG_MODE, T_CHATLOG_MODE);
	obs_property_set_modified_callback(p, chatlog_mode_changed);

	obs_properties_add_int(props, S_CHATLOG_LINES, T_CHATLOG_LINES, 1, 1000,
			       1);

	p = obs_properties_add_bool(props, S_EXTENTS, T_EXTENTS);
	obs_property_set_modified_callback(p, extents_modified);

	obs_properties_add_int(props, S_EXTENTS_CX, T_EXTENTS_CX, 32, 8000, 1);
	obs_properties_add_int(props, S_EXTENTS_CY, T_EXTENTS_CY, 32, 8000, 1);
	obs_properties_add_bool(props, S_EXTENTS_WRAP, T_EXTENTS_WRAP);

	return props;
}

bool obs_module_load(void)
{
	obs_source_info si = {};
	si.id = "text_gdiplus";
	si.type = OBS_SOURCE_TYPE_INPUT;
	si.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;
	si.get_properties = get_properties;

	si.get_name = [](void *) { return obs_module_text("TextGDIPlus"); };
	si.create = [](obs_data_t *settings, obs_source_t *source) {
		return (void *)new TextSource(source, settings);
	};
	si.destroy = [](void *data) {
		delete reinterpret_cast<TextSource *>(data);
	};
	si.get_width = [](void *data) {
		return reinterpret_cast<TextSource *>(data)->cx;
	};
	si.get_height = [](void *data) {
		return reinterpret_cast<TextSource *>(data)->cy;
	};
	si.get_defaults = [](obs_data_t *settings) {
		obs_data_t *font_obj = obs_data_create();
		obs_data_set_default_string(font_obj, "face", "Arial");
		obs_data_set_default_int(font_obj, "size", 36);

		obs_data_set_default_obj(settings, S_FONT, font_obj);
		obs_data_release(font_obj);
#if VNR_kyob1010_MultipleStream
		obs_data_set_default_string(settings, S_VNR_MODE, "t");
#endif
		obs_data_set_default_string(settings, S_ALIGN, S_ALIGN_LEFT);
		obs_data_set_default_string(settings, S_VALIGN, S_VALIGN_TOP);
		obs_data_set_default_int(settings, S_COLOR, 0xFFFFFF);
		obs_data_set_default_int(settings, S_OPACITY, 100);
		obs_data_set_default_int(settings, S_GRADIENT_COLOR, 0xFFFFFF);
		obs_data_set_default_int(settings, S_GRADIENT_OPACITY, 100);
		obs_data_set_default_double(settings, S_GRADIENT_DIR, 90.0);
		obs_data_set_default_int(settings, S_BKCOLOR, 0x000000);
		obs_data_set_default_int(settings, S_BKOPACITY, 0);
		obs_data_set_default_int(settings, S_OUTLINE_SIZE, 2);
		obs_data_set_default_int(settings, S_OUTLINE_COLOR, 0xFFFFFF);
		obs_data_set_default_int(settings, S_OUTLINE_OPACITY, 100);
		obs_data_set_default_int(settings, S_CHATLOG_LINES, 6);
		obs_data_set_default_bool(settings, S_EXTENTS_WRAP, true);
		obs_data_set_default_int(settings, S_EXTENTS_CX, 100);
		obs_data_set_default_int(settings, S_EXTENTS_CY, 100);
		obs_data_set_default_int(settings, S_TRANSFORM,
					 S_TRANSFORM_NONE);
	};
	si.update = [](void *data, obs_data_t *settings) {
		reinterpret_cast<TextSource *>(data)->Update(settings);
	};
	si.video_tick = [](void *data, float seconds) {
		reinterpret_cast<TextSource *>(data)->Tick(seconds);
	};
	si.video_render = [](void *data, gs_effect_t *) {
		reinterpret_cast<TextSource *>(data)->Render();
	};

	obs_register_source(&si);

	const GdiplusStartupInput gdip_input;
	GdiplusStartup(&gdip_token, &gdip_input, nullptr);
	return true;
}

void obs_module_unload(void)
{
	GdiplusShutdown(gdip_token);
}
