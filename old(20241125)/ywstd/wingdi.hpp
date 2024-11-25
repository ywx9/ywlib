#pragma once

#include "win.hpp"

export namespace win {

inline int add_font_resource(const wchar_t* File) { return ::AddFontResourceW(File); }
using hpalette = ::HPALETTE;
using palette_entry = ::PALETTEENTRY;
inline bool animate_palette(hpalette pal, unsigned StartIndex, unsigned Count, const palette_entry* Entries) { return ::AnimatePalette(pal, StartIndex, Count, Entries); }
using hdc = ::HDC;
inline bool arc(hdc dc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) { return ::Arc(dc, x1, y1, x2, y2, x3, y3, x4, y4); }
inline bool bit_blt(hdc dc, int x, int y, int cx, int cy, hdc src_dc, int x1, int y1, unsigned rop) { return ::BitBlt(dc, x, y, cx, cy, src_dc, x1, y1, rop); }
inline bool cancel_dc(hdc dc) { return ::CancelDC(dc); }
inline bool chord(hdc dc, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) { return ::Chord(dc, x1, y1, x2, y2, x3, y3, x4, y4); }
using pixel_format_descriptor = ::PIXELFORMATDESCRIPTOR;
inline int choose_pixel_format(hdc dc, const pixel_format_descriptor* pfd) { return ::ChoosePixelFormat(dc, pfd); }
using hmetafile = ::HMETAFILE;
inline hmetafile close_meta_file(hdc dc) { return ::CloseMetaFile(dc); }
using hrgn = ::HRGN;
inline int combine_rgn(hrgn Dest, hrgn Src1, hrgn Src2, int Mode) { return ::CombineRgn(Dest, Src1, Src2, Mode); }
using hmetafile = ::HMETAFILE;
inline hmetafile copy_meta_file(hmetafile mf, const wchar_t* File) { return ::CopyMetaFileW(mf, File); }
using hbitmap = ::HBITMAP;
inline hbitmap create_bitmap(int Width, int Height, unsigned Planes, unsigned BitCount, const void* Bits) { return ::CreateBitmap(Width, Height, Planes, BitCount, Bits); }
using bitmap = ::BITMAP;
inline hbitmap create_bitmap_indirect(const bitmap* bm) { return ::CreateBitmapIndirect(bm); }
using hbrush = ::HBRUSH;
using logbrush = ::LOGBRUSH;
inline hbrush create_brush_indirect(const logbrush* brush) { return ::CreateBrushIndirect(brush); }
inline hbitmap create_compatible_bitmap(hdc dc, int cx, int cy) { return ::CreateCompatibleBitmap(dc, cx, cy); }
inline hbitmap create_discardable_bitmap(hdc dc, int cx, int cy) { return ::CreateDiscardableBitmap(dc, cx, cy); }
inline hdc create_compatible_dc(hdc dc) { return ::CreateCompatibleDC(dc); }
using devmode = ::DEVMODEW;
inline hdc create_dc(const wchar_t* Driver, const wchar_t* Device, const wchar_t* Port, const devmode* dm) { return ::CreateDCW(Driver, Device, Port, dm); }
using bitmap_info_header = ::BITMAPINFOHEADER;
using bitmap_info = ::BITMAPINFO;
inline hbitmap create_dibitmap(hdc dc, const bitmap_info_header* bmih, unsigned Init, const void* Bits, const bitmap_info* bmi, unsigned Usage) { return ::CreateDIBitmap(dc, bmih, Init, Bits, bmi, Usage); }
using hglobal = ::HGLOBAL;
inline hbrush create_dib_pattern_brush(hglobal global, unsigned Usage) { return ::CreateDIBPatternBrush(global, Usage); }
inline hbrush create_dib_pattern_brush_pt(const void* PackedDIB, unsigned Usage) { return ::CreateDIBPatternBrushPt(PackedDIB, Usage); }
inline hrgn create_elliptic_rgn(int x1, int y1, int x2, int y2) { return ::CreateEllipticRgn(x1, y1, x2, y2); }
using rect = ::RECT;
inline hrgn create_elliptic_rgn_indirect(const rect* rc) { return ::CreateEllipticRgnIndirect(rc); }
using hfont = ::HFONT;
using logfont = ::LOGFONTW;
inline hfont create_font_indirect(const logfont* lf) { return ::CreateFontIndirectW(lf); }
inline hfont create_font(int Height, int Width, int Escapement, int Orientation, int Weight, unsigned Italic, unsigned Underline, unsigned StrikeOut, unsigned CharSet, unsigned OutPrecision, unsigned ClipPrecision, unsigned Quality, unsigned PitchAndFamily, const wchar_t* FaceName) { return ::CreateFontW(Height, Width, Escapement, Orientation, Weight, Italic, Underline, StrikeOut, CharSet, OutPrecision, ClipPrecision, Quality, PitchAndFamily, FaceName); }
inline hbrush create_hatch_brush(int Hatch, unsigned Color) { return ::CreateHatchBrush(Hatch, Color); }
inline hdc create_ic(const wchar_t* Driver, const wchar_t* Device, const wchar_t* Port, const devmode* dm) { return ::CreateICW(Driver, Device, Port, dm); }
inline hdc create_meta_file(const wchar_t* File) { return ::CreateMetaFileW(File); }
using logpalette = ::LOGPALETTE;
inline hpalette create_palette(const logpalette* lp) { return ::CreatePalette(lp); }
using hpen = ::HPEN;
inline hpen create_pen(int Style, int Width, unsigned Color) { return ::CreatePen(Style, Width, Color); }
using logpen = ::LOGPEN;
inline hpen create_pen_indirect(const logpen* lp) { return ::CreatePenIndirect(lp); }
using point = ::POINT;
inline hrgn create_poly_polygon_rgn(const point* pt, const int* PolyCounts, int Poly, int Mode) { return ::CreatePolyPolygonRgn(pt, PolyCounts, Poly, Mode); }
inline hbrush create_pattern_brush(hbitmap bm) { return ::CreatePatternBrush(bm); }
inline hrgn create_rect_rgn(int x1, int y1, int x2, int y2) { return ::CreateRectRgn(x1, y1, x2, y2); }
inline hrgn create_rect_rgn_indirect(const rect* rc) { return ::CreateRectRgnIndirect(rc); }
inline hrgn create_round_rect_rgn(int x1, int y1, int x2, int y2, int w, int h) { return ::CreateRoundRectRgn(x1, y1, x2, y2, w, h); }
inline bool create_scalable_font_resource(unsigned Hidden, const wchar_t* Font, const wchar_t* File, const wchar_t* Path) { return ::CreateScalableFontResourceW(Hidden, Font, File, Path); }
inline hbrush create_solid_brush(unsigned Color) { return ::CreateSolidBrush(Color); }
inline bool delete_dc(hdc dc) { return ::DeleteDC(dc); }
inline bool delete_meta_file(hmetafile mf) { return ::DeleteMetaFile(mf); }
using hgdiobj = ::HGDIOBJ;
inline bool delete_object(hgdiobj obj) { return ::DeleteObject(obj); }
inline bool describe_pixel_format(hdc dc, int PixelFormat, unsigned Bytes, pixel_format_descriptor* pfd) { return ::DescribePixelFormat(dc, PixelFormat, Bytes, pfd); }
inline int device_capabilities(const wchar_t* Device, const wchar_t* Port, unsigned Capability, wchar_t* Output, const devmode* dm) { return ::DeviceCapabilitiesW(Device, Port, Capability, Output, dm); }
inline int draw_escape(hdc dc, int Escape, int InputSize, const char* Input) { return ::DrawEscape(dc, Escape, InputSize, Input); }
inline bool ellipse(hdc dc, int x1, int y1, int x2, int y2) { return ::Ellipse(dc, x1, y1, x2, y2); }
using font_enum_proc = ::FONTENUMPROCW;
inline int enum_font_families(hdc dc, logfont* lf, font_enum_proc fep, long long lp, unsigned flag) { return ::EnumFontFamiliesExW(dc, lf, fep, lp, flag); }
using gobj_enum_proc = ::GOBJENUMPROC;
inline int enum_objects(hdc dc, int Type, gobj_enum_proc gep, long long lp) { return ::EnumObjects(dc, Type, gep, lp); }
inline bool equal_rgn(hrgn rgn1, hrgn rgn2) { return ::EqualRgn(rgn1, rgn2); }
inline int escape(hdc dc, int Escape, int InputSize, const char* Input, void* Output) { return ::Escape(dc, Escape, InputSize, Input, Output); }
inline int ext_escape(hdc dc, int Escape, int InputSize, const char* Input, int OutputSize, char* Output) { return ::ExtEscape(dc, Escape, InputSize, Input, OutputSize, Output); }
inline int exclude_clip_rect(hdc dc, int left, int top, int right, int bottom) { return ::ExcludeClipRect(dc, left, top, right, bottom); }
using xform = ::XFORM;
using rgndata = ::RGNDATA;
inline hrgn ext_create_region(const xform* x, unsigned Count, const rgndata* Data) { return ::ExtCreateRegion(x, Count, Data); }
inline bool ext_flood_fill(hdc dc, int x, int y, unsigned Color, unsigned Type) { return ::ExtFloodFill(dc, x, y, Color, Type); }
inline bool fill_rgn(hdc dc, hrgn rgn, hbrush br) { return ::FillRgn(dc, rgn, br); }
inline bool flood_fill(hdc dc, int x, int y, unsigned Color) { return ::FloodFill(dc, x, y, Color); }
inline bool frame_rgn(hdc dc, hrgn rgn, hbrush br, int w, int h) { return ::FrameRgn(dc, rgn, br, w, h); }
inline int get_rop2(hdc dc) { return ::GetROP2(dc); }
using size = ::SIZE;
inline bool get_aspect_ratio_filter_ex(hdc dc, size* size) { return ::GetAspectRatioFilterEx(dc, size); }
inline unsigned get_bk_color(hdc dc) { return ::GetBkColor(dc); }
inline unsigned get_dc_brush_color(hdc dc) { return ::GetDCBrushColor(dc); }
inline int get_bk_mode(hdc dc) { return ::GetBkMode(dc); }
inline int get_bitmap_bits(hbitmap bm, int cb, void* bits) { return ::GetBitmapBits(bm, cb, bits); }

WINGDIAPI BOOL WINAPI GetBitmapDimensionEx(_In_ HBITMAP hbit, _Out_ LPSIZE lpsize);
WINGDIAPI UINT WINAPI GetBoundsRect(_In_ HDC hdc, _Out_ LPRECT lprect, _In_ UINT flags);

WINGDIAPI BOOL WINAPI GetBrushOrgEx(_In_ HDC hdc, _Out_ LPPOINT lppt);

WINGDIAPI BOOL WINAPI GetCharWidthA(_In_ HDC hdc, _In_ UINT iFirst, _In_ UINT iLast, _Out_writes_(iLast + 1 - iFirst) LPINT lpBuffer);
WINGDIAPI BOOL WINAPI GetCharWidthW(_In_ HDC hdc, _In_ UINT iFirst, _In_ UINT iLast, _Out_writes_(iLast + 1 - iFirst) LPINT lpBuffer);
#ifdef UNICODE
#define GetCharWidth GetCharWidthW
#else
#define GetCharWidth GetCharWidthA
#endif // !UNICODE
WINGDIAPI BOOL WINAPI GetCharWidth32A(_In_ HDC hdc, _In_ UINT iFirst, _In_ UINT iLast, _Out_writes_(iLast + 1 - iFirst) LPINT lpBuffer);
WINGDIAPI BOOL WINAPI GetCharWidth32W(_In_ HDC hdc, _In_ UINT iFirst, _In_ UINT iLast, _Out_writes_(iLast + 1 - iFirst) LPINT lpBuffer);
#ifdef UNICODE
#define GetCharWidth32 GetCharWidth32W
#else
#define GetCharWidth32 GetCharWidth32A
#endif // !UNICODE
WINGDIAPI BOOL APIENTRY GetCharWidthFloatA(_In_ HDC hdc, _In_ UINT iFirst, _In_ UINT iLast, _Out_writes_(iLast + 1 - iFirst) PFLOAT lpBuffer);
WINGDIAPI BOOL APIENTRY GetCharWidthFloatW(_In_ HDC hdc, _In_ UINT iFirst, _In_ UINT iLast, _Out_writes_(iLast + 1 - iFirst) PFLOAT lpBuffer);
#ifdef UNICODE
#define GetCharWidthFloat GetCharWidthFloatW
#else
#define GetCharWidthFloat GetCharWidthFloatA
#endif // !UNICODE

WINGDIAPI BOOL APIENTRY GetCharABCWidthsA(_In_ HDC hdc, _In_ UINT wFirst, _In_ UINT wLast, _Out_writes_(wLast - wFirst + 1) LPABC lpABC);
WINGDIAPI BOOL APIENTRY GetCharABCWidthsW(_In_ HDC hdc, _In_ UINT wFirst, _In_ UINT wLast, _Out_writes_(wLast - wFirst + 1) LPABC lpABC);
#ifdef UNICODE
#define GetCharABCWidths GetCharABCWidthsW
#else
#define GetCharABCWidths GetCharABCWidthsA
#endif // !UNICODE

WINGDIAPI BOOL APIENTRY GetCharABCWidthsFloatA(_In_ HDC hdc, _In_ UINT iFirst, _In_ UINT iLast, _Out_writes_(iLast + 1 - iFirst) LPABCFLOAT lpABC);
WINGDIAPI BOOL APIENTRY GetCharABCWidthsFloatW(_In_ HDC hdc, _In_ UINT iFirst, _In_ UINT iLast, _Out_writes_(iLast + 1 - iFirst) LPABCFLOAT lpABC);
#ifdef UNICODE
#define GetCharABCWidthsFloat GetCharABCWidthsFloatW
#else
#define GetCharABCWidthsFloat GetCharABCWidthsFloatA
#endif // !UNICODE
WINGDIAPI int WINAPI GetClipBox(_In_ HDC hdc, _Out_ LPRECT lprect);
WINGDIAPI int WINAPI GetClipRgn(_In_ HDC hdc, _In_ HRGN hrgn);
WINGDIAPI int WINAPI GetMetaRgn(_In_ HDC hdc, _In_ HRGN hrgn);
WINGDIAPI HGDIOBJ WINAPI GetCurrentObject(_In_ HDC hdc, _In_ UINT type);
WINGDIAPI BOOL WINAPI GetCurrentPositionEx(_In_ HDC hdc, _Out_ LPPOINT lppt);
WINGDIAPI int WINAPI GetDeviceCaps(_In_opt_ HDC hdc, _In_ int index);
WINGDIAPI int WINAPI GetDIBits(_In_ HDC hdc, _In_ HBITMAP hbm, _In_ UINT start, _In_ UINT cLines, _Out_opt_ LPVOID lpvBits, _At_((LPBITMAPINFOHEADER)lpbmi, _Inout_) LPBITMAPINFO lpbmi, _In_ UINT usage); // SAL actual size of lpbmi is computed from structure elements

_Success_(return != GDI_ERROR) WINGDIAPI DWORD WINAPI GetFontData(_In_ HDC hdc, _In_ DWORD dwTable, _In_ DWORD dwOffset, _Out_writes_bytes_to_opt_(cjBuffer, return) PVOID pvBuffer, _In_ DWORD cjBuffer);

WINGDIAPI DWORD WINAPI GetGlyphOutlineA(_In_ HDC hdc, _In_ UINT uChar, _In_ UINT fuFormat, _Out_ LPGLYPHMETRICS lpgm, _In_ DWORD cjBuffer, _Out_writes_bytes_opt_(cjBuffer) LPVOID pvBuffer, _In_ CONST MAT2* lpmat2);
WINGDIAPI DWORD WINAPI GetGlyphOutlineW(_In_ HDC hdc, _In_ UINT uChar, _In_ UINT fuFormat, _Out_ LPGLYPHMETRICS lpgm, _In_ DWORD cjBuffer, _Out_writes_bytes_opt_(cjBuffer) LPVOID pvBuffer, _In_ CONST MAT2* lpmat2);
#ifdef UNICODE
#define GetGlyphOutline GetGlyphOutlineW
#else
#define GetGlyphOutline GetGlyphOutlineA
#endif // !UNICODE

WINGDIAPI int WINAPI GetGraphicsMode(_In_ HDC hdc);
WINGDIAPI int WINAPI GetMapMode(_In_ HDC hdc);
WINGDIAPI UINT WINAPI GetMetaFileBitsEx(_In_ HMETAFILE hMF, _In_ UINT cbBuffer, _Out_writes_bytes_opt_(cbBuffer) LPVOID lpData);
WINGDIAPI HMETAFILE WINAPI GetMetaFileA(_In_ LPCSTR lpName);
WINGDIAPI HMETAFILE WINAPI GetMetaFileW(_In_ LPCWSTR lpName);
#ifdef UNICODE
#define GetMetaFile GetMetaFileW
#else
#define GetMetaFile GetMetaFileA
#endif // !UNICODE
WINGDIAPI COLORREF WINAPI GetNearestColor(_In_ HDC hdc, _In_ COLORREF color);
WINGDIAPI UINT WINAPI GetNearestPaletteIndex(_In_ HPALETTE h, _In_ COLORREF color);
_Post_satisfies_((return == 0) || (return >= GDI_MIN_OBJ_TYPE && return <= GDI_MAX_OBJ_TYPE)) WINGDIAPI DWORD WINAPI GetObjectType(_In_ HGDIOBJ h);

#ifndef NOTEXTMETRIC

WINGDIAPI UINT APIENTRY GetOutlineTextMetricsA(_In_ HDC hdc, _In_ UINT cjCopy, _Out_writes_bytes_opt_(cjCopy) LPOUTLINETEXTMETRICA potm);
WINGDIAPI UINT APIENTRY GetOutlineTextMetricsW(_In_ HDC hdc, _In_ UINT cjCopy, _Out_writes_bytes_opt_(cjCopy) LPOUTLINETEXTMETRICW potm);
#ifdef UNICODE
#define GetOutlineTextMetrics GetOutlineTextMetricsW
#else
#define GetOutlineTextMetrics GetOutlineTextMetricsA
#endif // !UNICODE

#endif /* NOTEXTMETRIC */

_Ret_range_(0, cEntries) WINGDIAPI UINT WINAPI GetPaletteEntries(_In_ HPALETTE hpal, _In_ UINT iStart, _In_ UINT cEntries, _Out_writes_to_opt_(cEntries, return) LPPALETTEENTRY pPalEntries);
WINGDIAPI COLORREF WINAPI GetPixel(_In_ HDC hdc, _In_ int x, _In_ int y);
WINGDIAPI int WINAPI GetPixelFormat(_In_ HDC hdc);
WINGDIAPI int WINAPI GetPolyFillMode(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI GetRasterizerCaps(_Out_writes_bytes_(cjBytes) LPRASTERIZER_STATUS lpraststat, _In_ UINT cjBytes);

WINGDIAPI int WINAPI GetRandomRgn(_In_ HDC hdc, _In_ HRGN hrgn, _In_ INT i);
WINGDIAPI DWORD WINAPI GetRegionData(_In_ HRGN hrgn, _In_ DWORD nCount, _Out_writes_bytes_to_opt_(nCount, return) LPRGNDATA lpRgnData);
WINGDIAPI int WINAPI GetRgnBox(_In_ HRGN hrgn, _Out_ LPRECT lprc);
WINGDIAPI HGDIOBJ WINAPI GetStockObject(_In_ int i);
WINGDIAPI int WINAPI GetStretchBltMode(_In_ HDC hdc);
WINGDIAPI
UINT WINAPI GetSystemPaletteEntries(_In_ HDC hdc, _In_ UINT iStart, _In_ UINT cEntries, _Out_writes_opt_(cEntries) LPPALETTEENTRY pPalEntries);

WINGDIAPI UINT WINAPI GetSystemPaletteUse(_In_ HDC hdc);
WINGDIAPI int WINAPI GetTextCharacterExtra(_In_ HDC hdc);
WINGDIAPI UINT WINAPI GetTextAlign(_In_ HDC hdc);
WINGDIAPI COLORREF WINAPI GetTextColor(_In_ HDC hdc);

WINGDIAPI
BOOL APIENTRY GetTextExtentPointA(_In_ HDC hdc, _In_reads_(c) LPCSTR lpString, _In_ int c, _Out_ LPSIZE lpsz);
WINGDIAPI
BOOL APIENTRY GetTextExtentPointW(_In_ HDC hdc, _In_reads_(c) LPCWSTR lpString, _In_ int c, _Out_ LPSIZE lpsz);
#ifdef UNICODE
#define GetTextExtentPoint GetTextExtentPointW
#else
#define GetTextExtentPoint GetTextExtentPointA
#endif // !UNICODE

WINGDIAPI
BOOL APIENTRY GetTextExtentPoint32A(_In_ HDC hdc, _In_reads_(c) LPCSTR lpString, _In_ int c, _Out_ LPSIZE psizl);
WINGDIAPI
BOOL APIENTRY GetTextExtentPoint32W(_In_ HDC hdc, _In_reads_(c) LPCWSTR lpString, _In_ int c, _Out_ LPSIZE psizl);
#ifdef UNICODE
#define GetTextExtentPoint32 GetTextExtentPoint32W
#else
#define GetTextExtentPoint32 GetTextExtentPoint32A
#endif // !UNICODE

WINGDIAPI
BOOL APIENTRY GetTextExtentExPointA(_In_ HDC hdc, _In_reads_(cchString) LPCSTR lpszString, _In_ int cchString, _In_ int nMaxExtent, _Out_opt_ LPINT lpnFit, _Out_writes_to_opt_(cchString, *lpnFit) LPINT lpnDx, _Out_ LPSIZE lpSize);
WINGDIAPI
BOOL APIENTRY GetTextExtentExPointW(_In_ HDC hdc, _In_reads_(cchString) LPCWSTR lpszString, _In_ int cchString, _In_ int nMaxExtent, _Out_opt_ LPINT lpnFit, _Out_writes_to_opt_(cchString, *lpnFit) LPINT lpnDx, _Out_ LPSIZE lpSize);
#ifdef UNICODE
#define GetTextExtentExPoint GetTextExtentExPointW
#else
#define GetTextExtentExPoint GetTextExtentExPointA
#endif // !UNICODE

#if (WINVER >= 0x0400)
WINGDIAPI int WINAPI GetTextCharset(_In_ HDC hdc);
WINGDIAPI int WINAPI GetTextCharsetInfo(_In_ HDC hdc, _Out_opt_ LPFONTSIGNATURE lpSig, _In_ DWORD dwFlags);
WINGDIAPI BOOL WINAPI TranslateCharsetInfo(_Inout_ DWORD FAR* lpSrc, _Out_ LPCHARSETINFO lpCs, _In_ DWORD dwFlags);
WINGDIAPI DWORD WINAPI GetFontLanguageInfo(_In_ HDC hdc);
WINGDIAPI DWORD WINAPI GetCharacterPlacementA(_In_ HDC hdc, _In_reads_(nCount) LPCSTR lpString, _In_ int nCount, _In_ int nMexExtent, _Inout_ LPGCP_RESULTSA lpResults, _In_ DWORD dwFlags);
WINGDIAPI DWORD WINAPI GetCharacterPlacementW(_In_ HDC hdc, _In_reads_(nCount) LPCWSTR lpString, _In_ int nCount, _In_ int nMexExtent, _Inout_ LPGCP_RESULTSW lpResults, _In_ DWORD dwFlags);
#ifdef UNICODE
#define GetCharacterPlacement GetCharacterPlacementW
#else
#define GetCharacterPlacement GetCharacterPlacementA
#endif // !UNICODE
#endif /* WINVER >= 0x0400 */

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */
#pragma endregion

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

typedef struct tagWCRANGE {
  WCHAR wcLow;
  USHORT cGlyphs;
} WCRANGE, *PWCRANGE, FAR* LPWCRANGE;

typedef struct tagGLYPHSET {
  DWORD cbThis;
  DWORD flAccel;
  DWORD cGlyphsSupported;
  DWORD cRanges;
  WCRANGE ranges[1];
} GLYPHSET, *PGLYPHSET, FAR* LPGLYPHSET;

/* flAccel flags for the GLYPHSET structure above */

#define GS_8BIT_INDICES 0x00000001

/* flags for GetGlyphIndices */

#define GGI_MARK_NONEXISTING_GLYPHS 0X0001

WINGDIAPI DWORD WINAPI GetFontUnicodeRanges(_In_ HDC hdc, _Out_opt_ LPGLYPHSET lpgs);
WINGDIAPI DWORD WINAPI GetGlyphIndicesA(_In_ HDC hdc, _In_reads_(c) LPCSTR lpstr, _In_ int c, _Out_writes_(c) LPWORD pgi, _In_ DWORD fl);
WINGDIAPI DWORD WINAPI GetGlyphIndicesW(_In_ HDC hdc, _In_reads_(c) LPCWSTR lpstr, _In_ int c, _Out_writes_(c) LPWORD pgi, _In_ DWORD fl);
#ifdef UNICODE
#define GetGlyphIndices GetGlyphIndicesW
#else
#define GetGlyphIndices GetGlyphIndicesA
#endif // !UNICODE
WINGDIAPI BOOL WINAPI GetTextExtentPointI(_In_ HDC hdc, _In_reads_(cgi) LPWORD pgiIn, _In_ int cgi, _Out_ LPSIZE psize);
WINGDIAPI BOOL WINAPI GetTextExtentExPointI(_In_ HDC hdc, _In_reads_(cwchString) LPWORD lpwszString, _In_ int cwchString, _In_ int nMaxExtent, _Out_opt_ LPINT lpnFit, _Out_writes_to_opt_(cwchString, *lpnFit) LPINT lpnDx, _Out_ LPSIZE lpSize);

WINGDIAPI BOOL WINAPI GetCharWidthI(_In_ HDC hdc, _In_ UINT giFirst, _In_ UINT cgi, _In_reads_opt_(cgi) LPWORD pgi, _Out_writes_(cgi) LPINT piWidths);

WINGDIAPI BOOL WINAPI GetCharABCWidthsI(_In_ HDC hdc, _In_ UINT giFirst, _In_ UINT cgi, _In_reads_opt_(cgi) LPWORD pgi, _Out_writes_(cgi) LPABC pabc);

#define STAMP_DESIGNVECTOR (0x8000000 + 'd' + ('v' << 8))
#define STAMP_AXESLIST (0x8000000 + 'a' + ('l' << 8))
#define STAMP_TRUETYPE_VARIATION (0x8000000 + 't' + ('v' << 8))
#define STAMP_CFF2 (0x8000000 + 'c' + ('v' << 8))
#define MM_MAX_NUMAXES 16

typedef struct tagDESIGNVECTOR {
  DWORD dvReserved;
  DWORD dvNumAxes;
  LONG dvValues[MM_MAX_NUMAXES];
} DESIGNVECTOR, *PDESIGNVECTOR, FAR* LPDESIGNVECTOR;

WINGDIAPI int WINAPI AddFontResourceExA(_In_ LPCSTR name, _In_ DWORD fl, _Reserved_ PVOID res);
WINGDIAPI int WINAPI AddFontResourceExW(_In_ LPCWSTR name, _In_ DWORD fl, _Reserved_ PVOID res);
#ifdef UNICODE
#define AddFontResourceEx AddFontResourceExW
#else
#define AddFontResourceEx AddFontResourceExA
#endif // !UNICODE
WINGDIAPI BOOL WINAPI RemoveFontResourceExA(_In_ LPCSTR name, _In_ DWORD fl, _Reserved_ PVOID pdv);
WINGDIAPI BOOL WINAPI RemoveFontResourceExW(_In_ LPCWSTR name, _In_ DWORD fl, _Reserved_ PVOID pdv);
#ifdef UNICODE
#define RemoveFontResourceEx RemoveFontResourceExW
#else
#define RemoveFontResourceEx RemoveFontResourceExA
#endif // !UNICODE
WINGDIAPI HANDLE WINAPI AddFontMemResourceEx(_In_reads_bytes_(cjSize) PVOID pFileView, _In_ DWORD cjSize, _Reserved_ PVOID pvResrved, _In_ DWORD* pNumFonts);

WINGDIAPI BOOL WINAPI RemoveFontMemResourceEx(_In_ HANDLE h);
#define FR_PRIVATE 0x10
#define FR_NOT_ENUM 0x20

// The actual size of the DESIGNVECTOR and ENUMLOGFONTEXDV structures
// is determined by dvNumAxes,
// MM_MAX_NUMAXES only detemines the maximal size allowed

#define MM_MAX_AXES_NAMELEN 16

typedef struct tagAXISINFOA {
  LONG axMinValue;
  LONG axMaxValue;
  BYTE axAxisName[MM_MAX_AXES_NAMELEN];
} AXISINFOA, *PAXISINFOA, FAR* LPAXISINFOA;
typedef struct tagAXISINFOW {
  LONG axMinValue;
  LONG axMaxValue;
  WCHAR axAxisName[MM_MAX_AXES_NAMELEN];
} AXISINFOW, *PAXISINFOW, FAR* LPAXISINFOW;
#ifdef UNICODE
typedef AXISINFOW AXISINFO;
typedef PAXISINFOW PAXISINFO;
typedef LPAXISINFOW LPAXISINFO;
#else
typedef AXISINFOA AXISINFO;
typedef PAXISINFOA PAXISINFO;
typedef LPAXISINFOA LPAXISINFO;
#endif // UNICODE

typedef struct tagAXESLISTA {
  DWORD axlReserved;
  DWORD axlNumAxes;
  AXISINFOA axlAxisInfo[MM_MAX_NUMAXES];
} AXESLISTA, *PAXESLISTA, FAR* LPAXESLISTA;
typedef struct tagAXESLISTW {
  DWORD axlReserved;
  DWORD axlNumAxes;
  AXISINFOW axlAxisInfo[MM_MAX_NUMAXES];
} AXESLISTW, *PAXESLISTW, FAR* LPAXESLISTW;
#ifdef UNICODE
typedef AXESLISTW AXESLIST;
typedef PAXESLISTW PAXESLIST;
typedef LPAXESLISTW LPAXESLIST;
#else
typedef AXESLISTA AXESLIST;
typedef PAXESLISTA PAXESLIST;
typedef LPAXESLISTA LPAXESLIST;
#endif // UNICODE

// The actual size of the AXESLIST and ENUMTEXTMETRIC structure is
// determined by axlNumAxes,
// MM_MAX_NUMAXES only detemines the maximal size allowed

typedef struct tagENUMLOGFONTEXDVA {
  ENUMLOGFONTEXA elfEnumLogfontEx;
  DESIGNVECTOR elfDesignVector;
} ENUMLOGFONTEXDVA, *PENUMLOGFONTEXDVA, FAR* LPENUMLOGFONTEXDVA;
typedef struct tagENUMLOGFONTEXDVW {
  ENUMLOGFONTEXW elfEnumLogfontEx;
  DESIGNVECTOR elfDesignVector;
} ENUMLOGFONTEXDVW, *PENUMLOGFONTEXDVW, FAR* LPENUMLOGFONTEXDVW;
#ifdef UNICODE
typedef ENUMLOGFONTEXDVW ENUMLOGFONTEXDV;
typedef PENUMLOGFONTEXDVW PENUMLOGFONTEXDV;
typedef LPENUMLOGFONTEXDVW LPENUMLOGFONTEXDV;
#else
typedef ENUMLOGFONTEXDVA ENUMLOGFONTEXDV;
typedef PENUMLOGFONTEXDVA PENUMLOGFONTEXDV;
typedef LPENUMLOGFONTEXDVA LPENUMLOGFONTEXDV;
#endif // UNICODE

WINGDIAPI HFONT WINAPI CreateFontIndirectExA(_In_ CONST ENUMLOGFONTEXDVA*);
WINGDIAPI HFONT WINAPI CreateFontIndirectExW(_In_ CONST ENUMLOGFONTEXDVW*);
#ifdef UNICODE
#define CreateFontIndirectEx CreateFontIndirectExW
#else
#define CreateFontIndirectEx CreateFontIndirectExA
#endif // !UNICODE

#ifndef NOTEXTMETRIC
typedef struct tagENUMTEXTMETRICA {
  NEWTEXTMETRICEXA etmNewTextMetricEx;
  AXESLISTA etmAxesList;
} ENUMTEXTMETRICA, *PENUMTEXTMETRICA, FAR* LPENUMTEXTMETRICA;
typedef struct tagENUMTEXTMETRICW {
  NEWTEXTMETRICEXW etmNewTextMetricEx;
  AXESLISTW etmAxesList;
} ENUMTEXTMETRICW, *PENUMTEXTMETRICW, FAR* LPENUMTEXTMETRICW;
#ifdef UNICODE
typedef ENUMTEXTMETRICW ENUMTEXTMETRIC;
typedef PENUMTEXTMETRICW PENUMTEXTMETRIC;
typedef LPENUMTEXTMETRICW LPENUMTEXTMETRIC;
#else
typedef ENUMTEXTMETRICA ENUMTEXTMETRIC;
typedef PENUMTEXTMETRICA PENUMTEXTMETRIC;
typedef LPENUMTEXTMETRICA LPENUMTEXTMETRIC;
#endif // UNICODE
#endif /* NOTEXTMETRIC */

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */
#pragma endregion

#endif // (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

WINGDIAPI BOOL WINAPI GetViewportExtEx(_In_ HDC hdc, _Out_ LPSIZE lpsize);
WINGDIAPI BOOL WINAPI GetViewportOrgEx(_In_ HDC hdc, _Out_ LPPOINT lppoint);
WINGDIAPI BOOL WINAPI GetWindowExtEx(_In_ HDC hdc, _Out_ LPSIZE lpsize);
WINGDIAPI BOOL WINAPI GetWindowOrgEx(_In_ HDC hdc, _Out_ LPPOINT lppoint);

WINGDIAPI int WINAPI IntersectClipRect(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom);
WINGDIAPI BOOL WINAPI InvertRgn(_In_ HDC hdc, _In_ HRGN hrgn);
WINGDIAPI BOOL WINAPI LineDDA(_In_ int xStart, _In_ int yStart, _In_ int xEnd, _In_ int yEnd, _In_ LINEDDAPROC lpProc, _In_opt_ LPARAM data);
WINGDIAPI BOOL WINAPI LineTo(_In_ HDC hdc, _In_ int x, _In_ int y);
WINGDIAPI BOOL WINAPI MaskBlt(_In_ HDC hdcDest, _In_ int xDest, _In_ int yDest, _In_ int width, _In_ int height, _In_ HDC hdcSrc, _In_ int xSrc, _In_ int ySrc, _In_ HBITMAP hbmMask, _In_ int xMask, _In_ int yMask, _In_ DWORD rop);
WINGDIAPI BOOL WINAPI PlgBlt(_In_ HDC hdcDest, _In_reads_(3) CONST POINT* lpPoint, _In_ HDC hdcSrc, _In_ int xSrc, _In_ int ySrc, _In_ int width, _In_ int height, _In_opt_ HBITMAP hbmMask, _In_ int xMask, _In_ int yMask);

WINGDIAPI int WINAPI OffsetClipRgn(_In_ HDC hdc, _In_ int x, _In_ int y);
WINGDIAPI int WINAPI OffsetRgn(_In_ HRGN hrgn, _In_ int x, _In_ int y);
WINGDIAPI BOOL WINAPI PatBlt(_In_ HDC hdc, _In_ int x, _In_ int y, _In_ int w, _In_ int h, _In_ DWORD rop);
WINGDIAPI BOOL WINAPI Pie(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom, _In_ int xr1, _In_ int yr1, _In_ int xr2, _In_ int yr2);
WINGDIAPI BOOL WINAPI PlayMetaFile(_In_ HDC hdc, _In_ HMETAFILE hmf);
WINGDIAPI BOOL WINAPI PaintRgn(_In_ HDC hdc, _In_ HRGN hrgn);
WINGDIAPI BOOL WINAPI PolyPolygon(_In_ HDC hdc, _In_ CONST POINT* apt, _In_reads_(csz) CONST INT* asz, _In_ int csz);
WINGDIAPI BOOL WINAPI PtInRegion(_In_ HRGN hrgn, _In_ int x, _In_ int y);
WINGDIAPI BOOL WINAPI PtVisible(_In_ HDC hdc, _In_ int x, _In_ int y);
WINGDIAPI BOOL WINAPI RectInRegion(_In_ HRGN hrgn, _In_ CONST RECT* lprect);
WINGDIAPI BOOL WINAPI RectVisible(_In_ HDC hdc, _In_ CONST RECT* lprect);
WINGDIAPI BOOL WINAPI Rectangle(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom);
WINGDIAPI BOOL WINAPI RestoreDC(_In_ HDC hdc, _In_ int nSavedDC);
WINGDIAPI HDC WINAPI ResetDCA(_In_ HDC hdc, _In_ CONST DEVMODEA* lpdm);
WINGDIAPI HDC WINAPI ResetDCW(_In_ HDC hdc, _In_ CONST DEVMODEW* lpdm);
#ifdef UNICODE
#define ResetDC ResetDCW
#else
#define ResetDC ResetDCA
#endif // !UNICODE
WINGDIAPI UINT WINAPI RealizePalette(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI RemoveFontResourceA(_In_ LPCSTR lpFileName);
WINGDIAPI BOOL WINAPI RemoveFontResourceW(_In_ LPCWSTR lpFileName);
#ifdef UNICODE
#define RemoveFontResource RemoveFontResourceW
#else
#define RemoveFontResource RemoveFontResourceA
#endif // !UNICODE
WINGDIAPI BOOL WINAPI RoundRect(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom, _In_ int width, _In_ int height);
WINGDIAPI BOOL WINAPI ResizePalette(_In_ HPALETTE hpal, _In_ UINT n);

WINGDIAPI int WINAPI SaveDC(_In_ HDC hdc);
WINGDIAPI int WINAPI SelectClipRgn(_In_ HDC hdc, _In_opt_ HRGN hrgn);
WINGDIAPI int WINAPI ExtSelectClipRgn(_In_ HDC hdc, _In_opt_ HRGN hrgn, _In_ int mode);
WINGDIAPI int WINAPI SetMetaRgn(_In_ HDC hdc);
WINGDIAPI HGDIOBJ WINAPI SelectObject(_In_ HDC hdc, _In_ HGDIOBJ h);
WINGDIAPI HPALETTE WINAPI SelectPalette(_In_ HDC hdc, _In_ HPALETTE hPal, _In_ BOOL bForceBkgd);
WINGDIAPI COLORREF WINAPI SetBkColor(_In_ HDC hdc, _In_ COLORREF color);

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)
WINGDIAPI COLORREF WINAPI SetDCBrushColor(_In_ HDC hdc, _In_ COLORREF color);
WINGDIAPI COLORREF WINAPI SetDCPenColor(_In_ HDC hdc, _In_ COLORREF color);
#endif

WINGDIAPI int WINAPI SetBkMode(_In_ HDC hdc, _In_ int mode);

WINGDIAPI
LONG WINAPI SetBitmapBits(_In_ HBITMAP hbm, _In_ DWORD cb, _In_reads_bytes_(cb) CONST VOID* pvBits);

WINGDIAPI UINT WINAPI SetBoundsRect(_In_ HDC hdc, _In_opt_ CONST RECT* lprect, _In_ UINT flags);
WINGDIAPI int WINAPI SetDIBits(_In_opt_ HDC hdc, _In_ HBITMAP hbm, _In_ UINT start, _In_ UINT cLines, _In_ CONST VOID* lpBits, _In_ CONST BITMAPINFO* lpbmi, _In_ UINT ColorUse);
WINGDIAPI int WINAPI SetDIBitsToDevice(_In_ HDC hdc, _In_ int xDest, _In_ int yDest, _In_ DWORD w, _In_ DWORD h, _In_ int xSrc, _In_ int ySrc, _In_ UINT StartScan, _In_ UINT cLines, _In_ CONST VOID* lpvBits, _In_ CONST BITMAPINFO* lpbmi, _In_ UINT ColorUse);
WINGDIAPI DWORD WINAPI SetMapperFlags(_In_ HDC hdc, _In_ DWORD flags);
WINGDIAPI int WINAPI SetGraphicsMode(_In_ HDC hdc, _In_ int iMode);
WINGDIAPI int WINAPI SetMapMode(_In_ HDC hdc, _In_ int iMode);

#if (WINVER >= 0x0500)
WINGDIAPI DWORD WINAPI SetLayout(_In_ HDC hdc, _In_ DWORD l);
WINGDIAPI DWORD WINAPI GetLayout(_In_ HDC hdc);
#endif /* WINVER >= 0x0500 */

WINGDIAPI HMETAFILE WINAPI SetMetaFileBitsEx(_In_ UINT cbBuffer, _In_reads_bytes_(cbBuffer) CONST BYTE* lpData);
WINGDIAPI UINT WINAPI SetPaletteEntries(_In_ HPALETTE hpal, _In_ UINT iStart, _In_ UINT cEntries, _In_reads_(cEntries) CONST PALETTEENTRY* pPalEntries);
WINGDIAPI COLORREF WINAPI SetPixel(_In_ HDC hdc, _In_ int x, _In_ int y, _In_ COLORREF color);
WINGDIAPI BOOL WINAPI SetPixelV(_In_ HDC hdc, _In_ int x, _In_ int y, _In_ COLORREF color);
WINGDIAPI BOOL WINAPI SetPixelFormat(_In_ HDC hdc, _In_ int format, _In_ CONST PIXELFORMATDESCRIPTOR* ppfd);
WINGDIAPI int WINAPI SetPolyFillMode(_In_ HDC hdc, _In_ int mode);
WINGDIAPI BOOL WINAPI StretchBlt(_In_ HDC hdcDest, _In_ int xDest, _In_ int yDest, _In_ int wDest, _In_ int hDest, _In_opt_ HDC hdcSrc, _In_ int xSrc, _In_ int ySrc, _In_ int wSrc, _In_ int hSrc, _In_ DWORD rop);
WINGDIAPI BOOL WINAPI SetRectRgn(_In_ HRGN hrgn, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom);
WINGDIAPI int WINAPI StretchDIBits(_In_ HDC hdc, _In_ int xDest, _In_ int yDest, _In_ int DestWidth, _In_ int DestHeight, _In_ int xSrc, _In_ int ySrc, _In_ int SrcWidth, _In_ int SrcHeight, _In_opt_ CONST VOID* lpBits, _In_ CONST BITMAPINFO* lpbmi, _In_ UINT iUsage, _In_ DWORD rop);
WINGDIAPI int WINAPI SetROP2(_In_ HDC hdc, _In_ int rop2);
WINGDIAPI int WINAPI SetStretchBltMode(_In_ HDC hdc, _In_ int mode);
WINGDIAPI UINT WINAPI SetSystemPaletteUse(_In_ HDC hdc, _In_ UINT use);
WINGDIAPI int WINAPI SetTextCharacterExtra(_In_ HDC hdc, _In_ int extra);
WINGDIAPI COLORREF WINAPI SetTextColor(_In_ HDC hdc, _In_ COLORREF color);
WINGDIAPI UINT WINAPI SetTextAlign(_In_ HDC hdc, _In_ UINT align);
WINGDIAPI BOOL WINAPI SetTextJustification(_In_ HDC hdc, _In_ int extra, _In_ int count);
WINGDIAPI BOOL WINAPI UpdateColors(_In_ HDC hdc);

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */
#pragma endregion

#ifdef COMBOX_SANDBOX

#if (_WIN32_WINNT >= 0x0600)

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

typedef PVOID(WINAPI* GDIMARSHALLOC)(DWORD dwSize, _In_ LPVOID pGdiRef);

typedef HRESULT(WINAPI* DDRAWMARSHCALLBACKMARSHAL)(_In_ HGDIOBJ hGdiObj, _In_ LPVOID pGdiRef, _Out_ LPVOID* ppDDrawRef);
typedef HRESULT(WINAPI* DDRAWMARSHCALLBACKUNMARSHAL)(_In_ LPVOID pData, _Out_ HDC* phdc, _Out_ LPVOID* ppDDrawRef);
typedef HRESULT(WINAPI* DDRAWMARSHCALLBACKRELEASE)(_In_ LPVOID pDDrawRef);

#define GDIREGISTERDDRAWPACKETVERSION 0x1

typedef struct {
  DWORD dwSize;
  DWORD dwVersion;
  DDRAWMARSHCALLBACKMARSHAL pfnDdMarshal;
  DDRAWMARSHCALLBACKUNMARSHAL pfnDdUnmarshal;
  DDRAWMARSHCALLBACKRELEASE pfnDdRelease;
} GDIREGISTERDDRAWPACKET, *PGDIREGISTERDDRAWPACKET;

WINGDIAPI BOOL WINAPI GdiRegisterDdraw(_In_ PGDIREGISTERDDRAWPACKET pPacket, _Out_ GDIMARSHALLOC* ppfnGdiAlloc);

WINGDIAPI ULONG WINAPI GdiMarshalSize(VOID);
WINGDIAPI VOID WINAPI GdiMarshal(DWORD dwProcessIdTo, _In_ HGDIOBJ hGdiObj, _Inout_ PVOID pData, ULONG ulFlags);
WINGDIAPI HGDIOBJ WINAPI GdiUnmarshal(_In_ PVOID pData, ULONG ulFlags);

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */
#pragma endregion

#endif // (_WIN32_WINNT >= 0x0600)

#endif // COMBOX_SANDBOX

#if (WINVER >= 0x0400)

//
// image blt
//

#pragma region Application Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)

typedef USHORT COLOR16;

typedef struct _TRIVERTEX {
  LONG x;
  LONG y;
  COLOR16 Red;
  COLOR16 Green;
  COLOR16 Blue;
  COLOR16 Alpha;
} TRIVERTEX, *PTRIVERTEX, *LPTRIVERTEX;

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM) */
#pragma endregion

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

typedef struct _GRADIENT_TRIANGLE {
  ULONG Vertex1;
  ULONG Vertex2;
  ULONG Vertex3;
} GRADIENT_TRIANGLE, *PGRADIENT_TRIANGLE, *LPGRADIENT_TRIANGLE;

typedef struct _GRADIENT_RECT {
  ULONG UpperLeft;
  ULONG LowerRight;
} GRADIENT_RECT, *PGRADIENT_RECT, *LPGRADIENT_RECT;

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */
#pragma endregion

#pragma region Application Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM)

typedef struct _BLENDFUNCTION {
  BYTE BlendOp;
  BYTE BlendFlags;
  BYTE SourceConstantAlpha;
  BYTE AlphaFormat;
} BLENDFUNCTION, *PBLENDFUNCTION;

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP | WINAPI_PARTITION_SYSTEM) */
#pragma endregion

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

//
// currentlly defined blend function
//

#define AC_SRC_OVER 0x00

//
// alpha format flags
//

#define AC_SRC_ALPHA 0x01

WINGDIAPI BOOL WINAPI AlphaBlend(_In_ HDC hdcDest, _In_ int xoriginDest, _In_ int yoriginDest, _In_ int wDest, _In_ int hDest, _In_ HDC hdcSrc, _In_ int xoriginSrc, _In_ int yoriginSrc, _In_ int wSrc, _In_ int hSrc, _In_ BLENDFUNCTION ftn);

WINGDIAPI BOOL WINAPI TransparentBlt(_In_ HDC hdcDest, _In_ int xoriginDest, _In_ int yoriginDest, _In_ int wDest, _In_ int hDest, _In_ HDC hdcSrc, _In_ int xoriginSrc, _In_ int yoriginSrc, _In_ int wSrc, _In_ int hSrc, _In_ UINT crTransparent);

//
// gradient drawing modes
//

#define GRADIENT_FILL_RECT_H 0x00000000
#define GRADIENT_FILL_RECT_V 0x00000001
#define GRADIENT_FILL_TRIANGLE 0x00000002
#define GRADIENT_FILL_OP_FLAG 0x000000ff

WINGDIAPI
BOOL WINAPI GradientFill(_In_ HDC hdc, _In_reads_(nVertex) PTRIVERTEX pVertex, _In_ ULONG nVertex, _In_ PVOID pMesh, _In_ ULONG nMesh, _In_ ULONG ulMode);

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */
#pragma endregion

#endif // (WINVER >= 0x0400)

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)

WINGDIAPI BOOL WINAPI GdiAlphaBlend(_In_ HDC hdcDest, _In_ int xoriginDest, _In_ int yoriginDest, _In_ int wDest, _In_ int hDest, _In_ HDC hdcSrc, _In_ int xoriginSrc, _In_ int yoriginSrc, _In_ int wSrc, _In_ int hSrc, _In_ BLENDFUNCTION ftn);

WINGDIAPI BOOL WINAPI GdiTransparentBlt(_In_ HDC hdcDest, _In_ int xoriginDest, _In_ int yoriginDest, _In_ int wDest, _In_ int hDest, _In_ HDC hdcSrc, _In_ int xoriginSrc, _In_ int yoriginSrc, _In_ int wSrc, _In_ int hSrc, _In_ UINT crTransparent);

WINGDIAPI BOOL WINAPI GdiGradientFill(_In_ HDC hdc, _In_reads_(nVertex) PTRIVERTEX pVertex, _In_ ULONG nVertex, _In_ PVOID pMesh, _In_ ULONG nCount, _In_ ULONG ulMode);

#endif

#ifndef NOMETAFILE

WINGDIAPI BOOL WINAPI PlayMetaFileRecord(_In_ HDC hdc, _In_reads_(noObjs) LPHANDLETABLE lpHandleTable, _In_ LPMETARECORD lpMR, _In_ UINT noObjs);

typedef int(CALLBACK* MFENUMPROC)(_In_ HDC hdc, _In_reads_(nObj) HANDLETABLE FAR* lpht, _In_ METARECORD FAR* lpMR, _In_ int nObj, _In_opt_ LPARAM param);
WINGDIAPI BOOL WINAPI EnumMetaFile(_In_ HDC hdc, _In_ HMETAFILE hmf, _In_ MFENUMPROC proc, _In_opt_ LPARAM param);

typedef int(CALLBACK* ENHMFENUMPROC)(_In_ HDC hdc, _In_reads_(nHandles) HANDLETABLE FAR* lpht, _In_ CONST ENHMETARECORD* lpmr, _In_ int nHandles, _In_opt_ LPARAM data);

// Enhanced Metafile Function Declarations

WINGDIAPI HENHMETAFILE WINAPI CloseEnhMetaFile(_In_ HDC hdc);
WINGDIAPI HENHMETAFILE WINAPI CopyEnhMetaFileA(_In_ HENHMETAFILE hEnh, _In_opt_ LPCSTR lpFileName);
WINGDIAPI HENHMETAFILE WINAPI CopyEnhMetaFileW(_In_ HENHMETAFILE hEnh, _In_opt_ LPCWSTR lpFileName);
#ifdef UNICODE
#define CopyEnhMetaFile CopyEnhMetaFileW
#else
#define CopyEnhMetaFile CopyEnhMetaFileA
#endif // !UNICODE
WINGDIAPI HDC WINAPI CreateEnhMetaFileA(_In_opt_ HDC hdc, _In_opt_ LPCSTR lpFilename, _In_opt_ CONST RECT* lprc, _In_opt_ LPCSTR lpDesc);
WINGDIAPI HDC WINAPI CreateEnhMetaFileW(_In_opt_ HDC hdc, _In_opt_ LPCWSTR lpFilename, _In_opt_ CONST RECT* lprc, _In_opt_ LPCWSTR lpDesc);
#ifdef UNICODE
#define CreateEnhMetaFile CreateEnhMetaFileW
#else
#define CreateEnhMetaFile CreateEnhMetaFileA
#endif // !UNICODE
WINGDIAPI BOOL WINAPI DeleteEnhMetaFile(_In_opt_ HENHMETAFILE hmf);
WINGDIAPI BOOL WINAPI EnumEnhMetaFile(_In_opt_ HDC hdc, _In_ HENHMETAFILE hmf, _In_ ENHMFENUMPROC proc, _In_opt_ LPVOID param, _In_opt_ CONST RECT* lpRect);
WINGDIAPI HENHMETAFILE WINAPI GetEnhMetaFileA(_In_ LPCSTR lpName);
WINGDIAPI HENHMETAFILE WINAPI GetEnhMetaFileW(_In_ LPCWSTR lpName);
#ifdef UNICODE
#define GetEnhMetaFile GetEnhMetaFileW
#else
#define GetEnhMetaFile GetEnhMetaFileA
#endif // !UNICODE
WINGDIAPI UINT WINAPI GetEnhMetaFileBits(_In_ HENHMETAFILE hEMF, _In_ UINT nSize, _Out_writes_bytes_opt_(nSize) LPBYTE lpData);
WINGDIAPI UINT WINAPI GetEnhMetaFileDescriptionA(_In_ HENHMETAFILE hemf, _In_ UINT cchBuffer, _Out_writes_opt_(cchBuffer) LPSTR lpDescription);
WINGDIAPI UINT WINAPI GetEnhMetaFileDescriptionW(_In_ HENHMETAFILE hemf, _In_ UINT cchBuffer, _Out_writes_opt_(cchBuffer) LPWSTR lpDescription);
#ifdef UNICODE
#define GetEnhMetaFileDescription GetEnhMetaFileDescriptionW
#else
#define GetEnhMetaFileDescription GetEnhMetaFileDescriptionA
#endif // !UNICODE
WINGDIAPI UINT WINAPI GetEnhMetaFileHeader(_In_ HENHMETAFILE hemf, _In_ UINT nSize, _Out_writes_bytes_opt_(nSize) LPENHMETAHEADER lpEnhMetaHeader);
WINGDIAPI UINT WINAPI GetEnhMetaFilePaletteEntries(_In_ HENHMETAFILE hemf, _In_ UINT nNumEntries, _Out_writes_opt_(nNumEntries) LPPALETTEENTRY lpPaletteEntries);

WINGDIAPI UINT WINAPI GetEnhMetaFilePixelFormat(_In_ HENHMETAFILE hemf, _In_ UINT cbBuffer, _Out_writes_bytes_opt_(cbBuffer) PIXELFORMATDESCRIPTOR* ppfd);
WINGDIAPI UINT WINAPI GetWinMetaFileBits(_In_ HENHMETAFILE hemf, _In_ UINT cbData16, _Out_writes_bytes_opt_(cbData16) LPBYTE pData16, _In_ INT iMapMode, _In_ HDC hdcRef);
WINGDIAPI BOOL WINAPI PlayEnhMetaFile(_In_ HDC hdc, _In_ HENHMETAFILE hmf, _In_ CONST RECT* lprect);
WINGDIAPI BOOL WINAPI PlayEnhMetaFileRecord(_In_ HDC hdc, _In_reads_(cht) LPHANDLETABLE pht, _In_ CONST ENHMETARECORD* pmr, _In_ UINT cht);

WINGDIAPI HENHMETAFILE WINAPI SetEnhMetaFileBits(_In_ UINT nSize, _In_reads_bytes_(nSize) CONST BYTE* pb);

WINGDIAPI HENHMETAFILE WINAPI SetWinMetaFileBits(_In_ UINT nSize, _In_reads_bytes_(nSize) CONST BYTE* lpMeta16Data, _In_opt_ HDC hdcRef, _In_opt_ CONST METAFILEPICT* lpMFP);
WINGDIAPI BOOL WINAPI GdiComment(_In_ HDC hdc, _In_ UINT nSize, _In_reads_bytes_(nSize) CONST BYTE* lpData);

#endif /* NOMETAFILE */

#ifndef NOTEXTMETRIC

WINGDIAPI BOOL WINAPI GetTextMetricsA(_In_ HDC hdc, _Out_ LPTEXTMETRICA lptm);
WINGDIAPI BOOL WINAPI GetTextMetricsW(_In_ HDC hdc, _Out_ LPTEXTMETRICW lptm);
#ifdef UNICODE
#define GetTextMetrics GetTextMetricsW
#else
#define GetTextMetrics GetTextMetricsA
#endif // !UNICODE

#if defined(_M_CEE)
#undef GetTextMetrics
__inline BOOL GetTextMetrics(HDC hdc, LPTEXTMETRIC lptm) {
#ifdef UNICODE
  return GetTextMetricsW(
#else
  return GetTextMetricsA(
#endif
    hdc, lptm);
}
#endif /* _M_CEE */

#endif

/* new GDI */

typedef struct tagDIBSECTION {
  BITMAP dsBm;
  BITMAPINFOHEADER dsBmih;
  DWORD dsBitfields[3];
  HANDLE dshSection;
  DWORD dsOffset;
} DIBSECTION, FAR *LPDIBSECTION, *PDIBSECTION;

WINGDIAPI BOOL WINAPI AngleArc(_In_ HDC hdc, _In_ int x, _In_ int y, _In_ DWORD r, _In_ FLOAT StartAngle, _In_ FLOAT SweepAngle);
WINGDIAPI BOOL WINAPI PolyPolyline(_In_ HDC hdc, _In_ CONST POINT* apt, _In_reads_(csz) CONST DWORD* asz, _In_ DWORD csz);
WINGDIAPI BOOL WINAPI GetWorldTransform(_In_ HDC hdc, _Out_ LPXFORM lpxf);
WINGDIAPI BOOL WINAPI SetWorldTransform(_In_ HDC hdc, _In_ CONST XFORM* lpxf);
WINGDIAPI BOOL WINAPI ModifyWorldTransform(_In_ HDC hdc, _In_opt_ CONST XFORM* lpxf, _In_ DWORD mode);
WINGDIAPI BOOL WINAPI CombineTransform(_Out_ LPXFORM lpxfOut, _In_ CONST XFORM* lpxf1, _In_ CONST XFORM* lpxf2);

#define GDI_WIDTHBYTES(bits) ((DWORD)(((bits) + 31) & (~31)) / 8)
#define GDI_DIBWIDTHBYTES(bi) (DWORD) GDI_WIDTHBYTES((DWORD)(bi).biWidth*(DWORD)(bi).biBitCount)
#define GDI__DIBSIZE(bi) (GDI_DIBWIDTHBYTES(bi) * (DWORD)(bi).biHeight)
#define GDI_DIBSIZE(bi) ((bi).biHeight < 0 ? (-1) * (GDI__DIBSIZE(bi)) : GDI__DIBSIZE(bi))

WINGDIAPI _Success_(return != NULL) HBITMAP WINAPI CreateDIBSection(_In_opt_ HDC hdc, _In_ CONST BITMAPINFO* pbmi, _In_ UINT usage, _When_((pbmi->bmiHeader.biBitCount != 0), _Outptr_result_bytebuffer_(_Inexpressible_(GDI_DIBSIZE((pbmi->bmiHeader))))) _When_((pbmi->bmiHeader.biBitCount == 0), _Outptr_result_bytebuffer_((pbmi->bmiHeader).biSizeImage)) VOID** ppvBits, _In_opt_ HANDLE hSection, _In_ DWORD offset);

_Ret_range_(0, cEntries) WINGDIAPI UINT WINAPI GetDIBColorTable(_In_ HDC hdc, _In_ UINT iStart, _In_ UINT cEntries, _Out_writes_to_(cEntries, return) RGBQUAD* prgbq);
WINGDIAPI UINT WINAPI SetDIBColorTable(_In_ HDC hdc, _In_ UINT iStart, _In_ UINT cEntries, _In_reads_(cEntries) CONST RGBQUAD* prgbq);

/* Flags value for COLORADJUSTMENT */
#define CA_NEGATIVE 0x0001
#define CA_LOG_FILTER 0x0002

/* IlluminantIndex values */
#define ILLUMINANT_DEVICE_DEFAULT 0
#define ILLUMINANT_A 1
#define ILLUMINANT_B 2
#define ILLUMINANT_C 3
#define ILLUMINANT_D50 4
#define ILLUMINANT_D55 5
#define ILLUMINANT_D65 6
#define ILLUMINANT_D75 7
#define ILLUMINANT_F2 8
#define ILLUMINANT_MAX_INDEX ILLUMINANT_F2

#define ILLUMINANT_TUNGSTEN ILLUMINANT_A
#define ILLUMINANT_DAYLIGHT ILLUMINANT_C
#define ILLUMINANT_FLUORESCENT ILLUMINANT_F2
#define ILLUMINANT_NTSC ILLUMINANT_C

/* Min and max for RedGamma, GreenGamma, BlueGamma */
#define RGB_GAMMA_MIN (WORD)02500
#define RGB_GAMMA_MAX (WORD)65000

/* Min and max for ReferenceBlack and ReferenceWhite */
#define REFERENCE_WHITE_MIN (WORD)6000
#define REFERENCE_WHITE_MAX (WORD)10000
#define REFERENCE_BLACK_MIN (WORD)0
#define REFERENCE_BLACK_MAX (WORD)4000

/* Min and max for Contrast, Brightness, Colorfulness, RedGreenTint */
#define COLOR_ADJ_MIN (SHORT) - 100
#define COLOR_ADJ_MAX (SHORT)100

typedef struct tagCOLORADJUSTMENT {
  WORD caSize;
  WORD caFlags;
  WORD caIlluminantIndex;
  WORD caRedGamma;
  WORD caGreenGamma;
  WORD caBlueGamma;
  WORD caReferenceBlack;
  WORD caReferenceWhite;
  SHORT caContrast;
  SHORT caBrightness;
  SHORT caColorfulness;
  SHORT caRedGreenTint;
} COLORADJUSTMENT, *PCOLORADJUSTMENT, FAR* LPCOLORADJUSTMENT;

WINGDIAPI BOOL WINAPI SetColorAdjustment(_In_ HDC hdc, _In_ CONST COLORADJUSTMENT* lpca);
WINGDIAPI BOOL WINAPI GetColorAdjustment(_In_ HDC hdc, _Out_ LPCOLORADJUSTMENT lpca);
WINGDIAPI HPALETTE WINAPI CreateHalftonePalette(_In_opt_ HDC hdc);

#ifdef STRICT
typedef BOOL(CALLBACK* ABORTPROC)(_In_ HDC, _In_ int);
#else
typedef FARPROC ABORTPROC;
#endif

typedef struct _DOCINFOA {
  int cbSize;
  LPCSTR lpszDocName;
  LPCSTR lpszOutput;
#if (WINVER >= 0x0400)
  LPCSTR lpszDatatype;
  DWORD fwType;
#endif /* WINVER */
} DOCINFOA, *LPDOCINFOA;
typedef struct _DOCINFOW {
  int cbSize;
  LPCWSTR lpszDocName;
  LPCWSTR lpszOutput;
#if (WINVER >= 0x0400)
  LPCWSTR lpszDatatype;
  DWORD fwType;
#endif /* WINVER */
} DOCINFOW, *LPDOCINFOW;
#ifdef UNICODE
typedef DOCINFOW DOCINFO;
typedef LPDOCINFOW LPDOCINFO;
#else
typedef DOCINFOA DOCINFO;
typedef LPDOCINFOA LPDOCINFO;
#endif // UNICODE

#if (WINVER >= 0x0400)
#define DI_APPBANDING 0x00000001
#define DI_ROPS_READ_DESTINATION 0x00000002
#endif /* WINVER >= 0x0400 */

WINGDIAPI int WINAPI StartDocA(_In_ HDC hdc, _In_ CONST DOCINFOA* lpdi);
WINGDIAPI int WINAPI StartDocW(_In_ HDC hdc, _In_ CONST DOCINFOW* lpdi);
#ifdef UNICODE
#define StartDoc StartDocW
#else
#define StartDoc StartDocA
#endif // !UNICODE
WINGDIAPI int WINAPI EndDoc(_In_ HDC hdc);
WINGDIAPI int WINAPI StartPage(_In_ HDC hdc);
WINGDIAPI int WINAPI EndPage(_In_ HDC hdc);
WINGDIAPI int WINAPI AbortDoc(_In_ HDC hdc);
WINGDIAPI int WINAPI SetAbortProc(_In_ HDC hdc, _In_ ABORTPROC proc);

WINGDIAPI BOOL WINAPI AbortPath(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI ArcTo(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom, _In_ int xr1, _In_ int yr1, _In_ int xr2, _In_ int yr2);
WINGDIAPI BOOL WINAPI BeginPath(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI CloseFigure(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI EndPath(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI FillPath(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI FlattenPath(_In_ HDC hdc);
WINGDIAPI int WINAPI GetPath(_In_ HDC hdc, _Out_writes_opt_(cpt) LPPOINT apt, _Out_writes_opt_(cpt) LPBYTE aj, int cpt);
WINGDIAPI HRGN WINAPI PathToRegion(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI PolyDraw(_In_ HDC hdc, _In_reads_(cpt) CONST POINT* apt, _In_reads_(cpt) CONST BYTE* aj, _In_ int cpt);
WINGDIAPI BOOL WINAPI SelectClipPath(_In_ HDC hdc, _In_ int mode);
WINGDIAPI int WINAPI SetArcDirection(_In_ HDC hdc, _In_ int dir);
WINGDIAPI BOOL WINAPI SetMiterLimit(_In_ HDC hdc, _In_ FLOAT limit, _Out_opt_ PFLOAT old);
WINGDIAPI BOOL WINAPI StrokeAndFillPath(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI StrokePath(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI WidenPath(_In_ HDC hdc);
WINGDIAPI HPEN WINAPI ExtCreatePen(_In_ DWORD iPenStyle, _In_ DWORD cWidth, _In_ CONST LOGBRUSH* plbrush, _In_ DWORD cStyle, _In_reads_opt_(cStyle) CONST DWORD* pstyle);
WINGDIAPI BOOL WINAPI GetMiterLimit(_In_ HDC hdc, _Out_ PFLOAT plimit);
WINGDIAPI int WINAPI GetArcDirection(_In_ HDC hdc);

WINGDIAPI int WINAPI GetObjectA(_In_ HANDLE h, _In_ int c, _Out_writes_bytes_opt_(c) LPVOID pv);
WINGDIAPI int WINAPI GetObjectW(_In_ HANDLE h, _In_ int c, _Out_writes_bytes_opt_(c) LPVOID pv);
#ifdef UNICODE
#define GetObject GetObjectW
#else
#define GetObject GetObjectA
#endif // !UNICODE
#if defined(_M_CEE)
#undef GetObject
__inline int GetObject(HANDLE h, int c, LPVOID pv) {
#ifdef UNICODE
  return GetObjectW(
#else
  return GetObjectA(
#endif
    h, c, pv);
}
#endif /* _M_CEE */

WINGDIAPI BOOL WINAPI MoveToEx(_In_ HDC hdc, _In_ int x, _In_ int y, _Out_opt_ LPPOINT lppt);
WINGDIAPI BOOL WINAPI TextOutA(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCSTR lpString, _In_ int c);
WINGDIAPI BOOL WINAPI TextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_reads_(c) LPCWSTR lpString, _In_ int c);
#ifdef UNICODE
#define TextOut TextOutW
#else
#define TextOut TextOutA
#endif // !UNICODE
WINGDIAPI BOOL WINAPI ExtTextOutA(_In_ HDC hdc, _In_ int x, _In_ int y, _In_ UINT options, _In_opt_ CONST RECT* lprect, _In_reads_opt_(c) LPCSTR lpString, _In_ UINT c, _In_reads_opt_(c) CONST INT* lpDx);
WINGDIAPI BOOL WINAPI ExtTextOutW(_In_ HDC hdc, _In_ int x, _In_ int y, _In_ UINT options, _In_opt_ CONST RECT* lprect, _In_reads_opt_(c) LPCWSTR lpString, _In_ UINT c, _In_reads_opt_(c) CONST INT* lpDx);
#ifdef UNICODE
#define ExtTextOut ExtTextOutW
#else
#define ExtTextOut ExtTextOutA
#endif // !UNICODE
WINGDIAPI BOOL WINAPI PolyTextOutA(_In_ HDC hdc, _In_reads_(nstrings) CONST POLYTEXTA* ppt, _In_ int nstrings);
WINGDIAPI BOOL WINAPI PolyTextOutW(_In_ HDC hdc, _In_reads_(nstrings) CONST POLYTEXTW* ppt, _In_ int nstrings);
#ifdef UNICODE
#define PolyTextOut PolyTextOutW
#else
#define PolyTextOut PolyTextOutA
#endif // !UNICODE

WINGDIAPI HRGN WINAPI CreatePolygonRgn(_In_reads_(cPoint) CONST POINT* pptl, _In_ int cPoint, _In_ int iMode);
WINGDIAPI BOOL WINAPI DPtoLP(_In_ HDC hdc, _Inout_updates_(c) LPPOINT lppt, _In_ int c);
WINGDIAPI BOOL WINAPI LPtoDP(_In_ HDC hdc, _Inout_updates_(c) LPPOINT lppt, _In_ int c);
WINGDIAPI BOOL WINAPI Polygon(_In_ HDC hdc, _In_reads_(cpt) CONST POINT* apt, _In_ int cpt);
WINGDIAPI BOOL WINAPI Polyline(_In_ HDC hdc, _In_reads_(cpt) CONST POINT* apt, _In_ int cpt);

WINGDIAPI BOOL WINAPI PolyBezier(_In_ HDC hdc, _In_reads_(cpt) CONST POINT* apt, _In_ DWORD cpt);
WINGDIAPI BOOL WINAPI PolyBezierTo(_In_ HDC hdc, _In_reads_(cpt) CONST POINT* apt, _In_ DWORD cpt);
WINGDIAPI BOOL WINAPI PolylineTo(_In_ HDC hdc, _In_reads_(cpt) CONST POINT* apt, _In_ DWORD cpt);

WINGDIAPI BOOL WINAPI SetViewportExtEx(_In_ HDC hdc, _In_ int x, _In_ int y, _Out_opt_ LPSIZE lpsz);
WINGDIAPI BOOL WINAPI SetViewportOrgEx(_In_ HDC hdc, _In_ int x, _In_ int y, _Out_opt_ LPPOINT lppt);
WINGDIAPI BOOL WINAPI SetWindowExtEx(_In_ HDC hdc, _In_ int x, _In_ int y, _Out_opt_ LPSIZE lpsz);
WINGDIAPI BOOL WINAPI SetWindowOrgEx(_In_ HDC hdc, _In_ int x, _In_ int y, _Out_opt_ LPPOINT lppt);

WINGDIAPI BOOL WINAPI OffsetViewportOrgEx(_In_ HDC hdc, _In_ int x, _In_ int y, _Out_opt_ LPPOINT lppt);
WINGDIAPI BOOL WINAPI OffsetWindowOrgEx(_In_ HDC hdc, _In_ int x, _In_ int y, _Out_opt_ LPPOINT lppt);
WINGDIAPI BOOL WINAPI ScaleViewportExtEx(_In_ HDC hdc, _In_ int xn, _In_ int dx, _In_ int yn, _In_ int yd, _Out_opt_ LPSIZE lpsz);
WINGDIAPI BOOL WINAPI ScaleWindowExtEx(_In_ HDC hdc, _In_ int xn, _In_ int xd, _In_ int yn, _In_ int yd, _Out_opt_ LPSIZE lpsz);
WINGDIAPI BOOL WINAPI SetBitmapDimensionEx(_In_ HBITMAP hbm, _In_ int w, _In_ int h, _Out_opt_ LPSIZE lpsz);
WINGDIAPI BOOL WINAPI SetBrushOrgEx(_In_ HDC hdc, _In_ int x, _In_ int y, _Out_opt_ LPPOINT lppt);

WINGDIAPI int WINAPI GetTextFaceA(_In_ HDC hdc, _In_ int c, _Out_writes_to_opt_(c, return) LPSTR lpName);
WINGDIAPI int WINAPI GetTextFaceW(_In_ HDC hdc, _In_ int c, _Out_writes_to_opt_(c, return) LPWSTR lpName);
#ifdef UNICODE
#define GetTextFace GetTextFaceW
#else
#define GetTextFace GetTextFaceA
#endif // !UNICODE

#define FONTMAPPER_MAX 10

typedef struct tagKERNINGPAIR {
  WORD wFirst;
  WORD wSecond;
  int iKernAmount;
} KERNINGPAIR, *LPKERNINGPAIR;

WINGDIAPI DWORD WINAPI GetKerningPairsA(_In_ HDC hdc, _In_ DWORD nPairs, _Out_writes_to_opt_(nPairs, return) LPKERNINGPAIR lpKernPair);
WINGDIAPI DWORD WINAPI GetKerningPairsW(_In_ HDC hdc, _In_ DWORD nPairs, _Out_writes_to_opt_(nPairs, return) LPKERNINGPAIR lpKernPair);
#ifdef UNICODE
#define GetKerningPairs GetKerningPairsW
#else
#define GetKerningPairs GetKerningPairsA
#endif // !UNICODE

WINGDIAPI BOOL WINAPI GetDCOrgEx(_In_ HDC hdc, _Out_ LPPOINT lppt);
WINGDIAPI BOOL WINAPI FixBrushOrgEx(_In_ HDC hdc, _In_ int x, _In_ int y, _In_opt_ LPPOINT ptl);
WINGDIAPI BOOL WINAPI UnrealizeObject(_In_ HGDIOBJ h);

WINGDIAPI BOOL WINAPI GdiFlush(void);
WINGDIAPI DWORD WINAPI GdiSetBatchLimit(_In_ DWORD dw);
WINGDIAPI DWORD WINAPI GdiGetBatchLimit(void);

#if (WINVER >= 0x0400)

#define ICM_OFF 1
#define ICM_ON 2
#define ICM_QUERY 3
#define ICM_DONE_OUTSIDEDC 4

typedef int(CALLBACK* ICMENUMPROCA)(LPSTR, LPARAM);
typedef int(CALLBACK* ICMENUMPROCW)(LPWSTR, LPARAM);
#ifdef UNICODE
#define ICMENUMPROC ICMENUMPROCW
#else
#define ICMENUMPROC ICMENUMPROCA
#endif // !UNICODE

WINGDIAPI int WINAPI SetICMMode(_In_ HDC hdc, _In_ int mode);
WINGDIAPI BOOL WINAPI CheckColorsInGamut(_In_ HDC hdc, _In_reads_(nCount) LPRGBTRIPLE lpRGBTriple, _Out_writes_bytes_(nCount) LPVOID dlpBuffer, _In_ DWORD nCount);

WINGDIAPI HCOLORSPACE WINAPI GetColorSpace(_In_ HDC hdc);
WINGDIAPI BOOL WINAPI GetLogColorSpaceA(_In_ HCOLORSPACE hColorSpace, _Out_writes_bytes_(nSize) LPLOGCOLORSPACEA lpBuffer, _In_ DWORD nSize);
WINGDIAPI BOOL WINAPI GetLogColorSpaceW(_In_ HCOLORSPACE hColorSpace, _Out_writes_bytes_(nSize) LPLOGCOLORSPACEW lpBuffer, _In_ DWORD nSize);
#ifdef UNICODE
#define GetLogColorSpace GetLogColorSpaceW
#else
#define GetLogColorSpace GetLogColorSpaceA
#endif // !UNICODE

WINGDIAPI HCOLORSPACE WINAPI CreateColorSpaceA(_In_ LPLOGCOLORSPACEA lplcs);
WINGDIAPI HCOLORSPACE WINAPI CreateColorSpaceW(_In_ LPLOGCOLORSPACEW lplcs);
#ifdef UNICODE
#define CreateColorSpace CreateColorSpaceW
#else
#define CreateColorSpace CreateColorSpaceA
#endif // !UNICODE
WINGDIAPI HCOLORSPACE WINAPI SetColorSpace(_In_ HDC hdc, _In_ HCOLORSPACE hcs);
WINGDIAPI BOOL WINAPI DeleteColorSpace(_In_ HCOLORSPACE hcs);
WINGDIAPI BOOL WINAPI GetICMProfileA(_In_ HDC hdc, _Inout_ LPDWORD pBufSize, _Out_writes_opt_(*pBufSize) LPSTR pszFilename);
WINGDIAPI BOOL WINAPI GetICMProfileW(_In_ HDC hdc, _Inout_ LPDWORD pBufSize, _Out_writes_opt_(*pBufSize) LPWSTR pszFilename);
#ifdef UNICODE
#define GetICMProfile GetICMProfileW
#else
#define GetICMProfile GetICMProfileA
#endif // !UNICODE

WINGDIAPI BOOL WINAPI SetICMProfileA(_In_ HDC hdc, _In_ LPSTR lpFileName);
WINGDIAPI BOOL WINAPI SetICMProfileW(_In_ HDC hdc, _In_ LPWSTR lpFileName);
#ifdef UNICODE
#define SetICMProfile SetICMProfileW
#else
#define SetICMProfile SetICMProfileA
#endif // !UNICODE
WINGDIAPI BOOL WINAPI GetDeviceGammaRamp(_In_ HDC hdc, _Out_writes_bytes_(3 * 256 * 2) LPVOID lpRamp);
WINGDIAPI BOOL WINAPI SetDeviceGammaRamp(_In_ HDC hdc, _In_reads_bytes_(3 * 256 * 2) LPVOID lpRamp);
WINGDIAPI BOOL WINAPI ColorMatchToTarget(_In_ HDC hdc, _In_ HDC hdcTarget, _In_ DWORD action);
WINGDIAPI int WINAPI EnumICMProfilesA(_In_ HDC hdc, _In_ ICMENUMPROCA proc, _In_opt_ LPARAM param);
WINGDIAPI int WINAPI EnumICMProfilesW(_In_ HDC hdc, _In_ ICMENUMPROCW proc, _In_opt_ LPARAM param);
#ifdef UNICODE
#define EnumICMProfiles EnumICMProfilesW
#else
#define EnumICMProfiles EnumICMProfilesA
#endif // !UNICODE
// The Win95 update API UpdateICMRegKeyA is deprecated to set last error to ERROR_NOT_SUPPORTED and return FALSE
WINGDIAPI BOOL WINAPI UpdateICMRegKeyA(_Reserved_ DWORD reserved, _In_ LPSTR lpszCMID, _In_ LPSTR lpszFileName, _In_ UINT command);
// The Win95 update API UpdateICMRegKeyW is deprecated to set last error to ERROR_NOT_SUPPORTED and return FALSE
WINGDIAPI BOOL WINAPI UpdateICMRegKeyW(_Reserved_ DWORD reserved, _In_ LPWSTR lpszCMID, _In_ LPWSTR lpszFileName, _In_ UINT command);
#ifdef UNICODE
#define UpdateICMRegKey UpdateICMRegKeyW
#else
#define UpdateICMRegKey UpdateICMRegKeyA
#endif // !UNICODE

#ifndef _CONTRACT_GEN
#pragma deprecated(UpdateICMRegKeyW)
#pragma deprecated(UpdateICMRegKeyA)
#endif // _CONTRACT_GEN

#endif /* WINVER >= 0x0400 */

#if (WINVER >= 0x0500)
WINGDIAPI BOOL WINAPI ColorCorrectPalette(_In_ HDC hdc, _In_ HPALETTE hPal, _In_ DWORD deFirst, _In_ DWORD num);
#endif

#ifndef NOMETAFILE

// Enhanced metafile constants.

#ifndef _MAC
#define ENHMETA_SIGNATURE 0x464D4520
#else
#define ENHMETA_SIGNATURE 0x20454D46
#endif

// Stock object flag used in the object handle index in the enhanced
// metafile records.
// E.g. The object handle index (META_STOCK_OBJECT | BLACK_BRUSH)
// represents the stock object BLACK_BRUSH.

#define ENHMETA_STOCK_OBJECT 0x80000000

// Enhanced metafile record types.

#define EMR_HEADER 1
#define EMR_POLYBEZIER 2
#define EMR_POLYGON 3
#define EMR_POLYLINE 4
#define EMR_POLYBEZIERTO 5
#define EMR_POLYLINETO 6
#define EMR_POLYPOLYLINE 7
#define EMR_POLYPOLYGON 8
#define EMR_SETWINDOWEXTEX 9
#define EMR_SETWINDOWORGEX 10
#define EMR_SETVIEWPORTEXTEX 11
#define EMR_SETVIEWPORTORGEX 12
#define EMR_SETBRUSHORGEX 13
#define EMR_EOF 14
#define EMR_SETPIXELV 15
#define EMR_SETMAPPERFLAGS 16
#define EMR_SETMAPMODE 17
#define EMR_SETBKMODE 18
#define EMR_SETPOLYFILLMODE 19
#define EMR_SETROP2 20
#define EMR_SETSTRETCHBLTMODE 21
#define EMR_SETTEXTALIGN 22
#define EMR_SETCOLORADJUSTMENT 23
#define EMR_SETTEXTCOLOR 24
#define EMR_SETBKCOLOR 25
#define EMR_OFFSETCLIPRGN 26
#define EMR_MOVETOEX 27
#define EMR_SETMETARGN 28
#define EMR_EXCLUDECLIPRECT 29
#define EMR_INTERSECTCLIPRECT 30
#define EMR_SCALEVIEWPORTEXTEX 31
#define EMR_SCALEWINDOWEXTEX 32
#define EMR_SAVEDC 33
#define EMR_RESTOREDC 34
#define EMR_SETWORLDTRANSFORM 35
#define EMR_MODIFYWORLDTRANSFORM 36
#define EMR_SELECTOBJECT 37
#define EMR_CREATEPEN 38
#define EMR_CREATEBRUSHINDIRECT 39
#define EMR_DELETEOBJECT 40
#define EMR_ANGLEARC 41
#define EMR_ELLIPSE 42
#define EMR_RECTANGLE 43
#define EMR_ROUNDRECT 44
#define EMR_ARC 45
#define EMR_CHORD 46
#define EMR_PIE 47
#define EMR_SELECTPALETTE 48
#define EMR_CREATEPALETTE 49
#define EMR_SETPALETTEENTRIES 50
#define EMR_RESIZEPALETTE 51
#define EMR_REALIZEPALETTE 52
#define EMR_EXTFLOODFILL 53
#define EMR_LINETO 54
#define EMR_ARCTO 55
#define EMR_POLYDRAW 56
#define EMR_SETARCDIRECTION 57
#define EMR_SETMITERLIMIT 58
#define EMR_BEGINPATH 59
#define EMR_ENDPATH 60
#define EMR_CLOSEFIGURE 61
#define EMR_FILLPATH 62
#define EMR_STROKEANDFILLPATH 63
#define EMR_STROKEPATH 64
#define EMR_FLATTENPATH 65
#define EMR_WIDENPATH 66
#define EMR_SELECTCLIPPATH 67
#define EMR_ABORTPATH 68

#define EMR_GDICOMMENT 70
#define EMR_FILLRGN 71
#define EMR_FRAMERGN 72
#define EMR_INVERTRGN 73
#define EMR_PAINTRGN 74
#define EMR_EXTSELECTCLIPRGN 75
#define EMR_BITBLT 76
#define EMR_STRETCHBLT 77
#define EMR_MASKBLT 78
#define EMR_PLGBLT 79
#define EMR_SETDIBITSTODEVICE 80
#define EMR_STRETCHDIBITS 81
#define EMR_EXTCREATEFONTINDIRECTW 82
#define EMR_EXTTEXTOUTA 83
#define EMR_EXTTEXTOUTW 84
#define EMR_POLYBEZIER16 85
#define EMR_POLYGON16 86
#define EMR_POLYLINE16 87
#define EMR_POLYBEZIERTO16 88
#define EMR_POLYLINETO16 89
#define EMR_POLYPOLYLINE16 90
#define EMR_POLYPOLYGON16 91
#define EMR_POLYDRAW16 92
#define EMR_CREATEMONOBRUSH 93
#define EMR_CREATEDIBPATTERNBRUSHPT 94
#define EMR_EXTCREATEPEN 95
#define EMR_POLYTEXTOUTA 96
#define EMR_POLYTEXTOUTW 97

#if (WINVER >= 0x0400)
#define EMR_SETICMMODE 98
#define EMR_CREATECOLORSPACE 99
#define EMR_SETCOLORSPACE 100
#define EMR_DELETECOLORSPACE 101
#define EMR_GLSRECORD 102
#define EMR_GLSBOUNDEDRECORD 103
#define EMR_PIXELFORMAT 104
#endif /* WINVER >= 0x0400 */

#if (WINVER >= 0x0500)
#define EMR_RESERVED_105 105
#define EMR_RESERVED_106 106
#define EMR_RESERVED_107 107
#define EMR_RESERVED_108 108
#define EMR_RESERVED_109 109
#define EMR_RESERVED_110 110
#define EMR_COLORCORRECTPALETTE 111
#define EMR_SETICMPROFILEA 112
#define EMR_SETICMPROFILEW 113
#define EMR_ALPHABLEND 114
#define EMR_SETLAYOUT 115
#define EMR_TRANSPARENTBLT 116
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)
#define EMR_RESERVED_117 117
#endif // (_WIN32_WINNT >= _WIN32_WINNT_WIN2K)
#define EMR_GRADIENTFILL 118
#define EMR_RESERVED_119 119
#define EMR_RESERVED_120 120
#define EMR_COLORMATCHTOTARGETW 121
#define EMR_CREATECOLORSPACEW 122
#endif /* WINVER >= 0x0500 */

#define EMR_MIN 1

#if (WINVER >= 0x0500)
#define EMR_MAX 122
#elif (WINVER >= 0x0400)
#define EMR_MAX 104
#else
#define EMR_MAX 97
#endif

// Base record type for the enhanced metafile.

typedef struct tagEMR {
  DWORD iType; // Enhanced metafile record type
  DWORD nSize; // Length of the record in bytes.
               // This must be a multiple of 4.
} EMR, *PEMR;

// Base text record type for the enhanced metafile.

typedef struct tagEMRTEXT {
  POINTL ptlReference;
  DWORD nChars;
  DWORD offString; // Offset to the string
  DWORD fOptions;
  RECTL rcl;
  DWORD offDx; // Offset to the inter-character spacing array.
               // This is always given.
} EMRTEXT, *PEMRTEXT;

// Record structures for the enhanced metafile.

typedef struct tagABORTPATH {
  EMR emr;
} EMRABORTPATH, *PEMRABORTPATH, EMRBEGINPATH, *PEMRBEGINPATH, EMRENDPATH, *PEMRENDPATH, EMRCLOSEFIGURE, *PEMRCLOSEFIGURE, EMRFLATTENPATH, *PEMRFLATTENPATH, EMRWIDENPATH, *PEMRWIDENPATH, EMRSETMETARGN, *PEMRSETMETARGN, EMRSAVEDC, *PEMRSAVEDC, EMRREALIZEPALETTE, *PEMRREALIZEPALETTE;

typedef struct tagEMRSELECTCLIPPATH {
  EMR emr;
  DWORD iMode;
} EMRSELECTCLIPPATH, *PEMRSELECTCLIPPATH, EMRSETBKMODE, *PEMRSETBKMODE, EMRSETMAPMODE, *PEMRSETMAPMODE,
#if (WINVER >= 0x0500)
  EMRSETLAYOUT, *PEMRSETLAYOUT,
#endif /* WINVER >= 0x0500 */
  EMRSETPOLYFILLMODE, *PEMRSETPOLYFILLMODE, EMRSETROP2, *PEMRSETROP2, EMRSETSTRETCHBLTMODE, *PEMRSETSTRETCHBLTMODE, EMRSETICMMODE, *PEMRSETICMMODE, EMRSETTEXTALIGN, *PEMRSETTEXTALIGN;

typedef struct tagEMRSETMITERLIMIT {
  EMR emr;
  FLOAT eMiterLimit;
} EMRSETMITERLIMIT, *PEMRSETMITERLIMIT;

typedef struct tagEMRRESTOREDC {
  EMR emr;
  LONG iRelative; // Specifies a relative instance
} EMRRESTOREDC, *PEMRRESTOREDC;

typedef struct tagEMRSETARCDIRECTION {
  EMR emr;
  DWORD iArcDirection; // Specifies the arc direction in the
                       // advanced graphics mode.
} EMRSETARCDIRECTION, *PEMRSETARCDIRECTION;

typedef struct tagEMRSETMAPPERFLAGS {
  EMR emr;
  DWORD dwFlags;
} EMRSETMAPPERFLAGS, *PEMRSETMAPPERFLAGS;

typedef struct tagEMRSETTEXTCOLOR {
  EMR emr;
  COLORREF crColor;
} EMRSETBKCOLOR, *PEMRSETBKCOLOR, EMRSETTEXTCOLOR, *PEMRSETTEXTCOLOR;

typedef struct tagEMRSELECTOBJECT {
  EMR emr;
  DWORD ihObject; // Object handle index
} EMRSELECTOBJECT, *PEMRSELECTOBJECT, EMRDELETEOBJECT, *PEMRDELETEOBJECT;

typedef struct tagEMRSELECTPALETTE {
  EMR emr;
  DWORD ihPal; // Palette handle index, background mode only
} EMRSELECTPALETTE, *PEMRSELECTPALETTE;

typedef struct tagEMRRESIZEPALETTE {
  EMR emr;
  DWORD ihPal; // Palette handle index
  DWORD cEntries;
} EMRRESIZEPALETTE, *PEMRRESIZEPALETTE;

typedef struct tagEMRSETPALETTEENTRIES {
  EMR emr;
  DWORD ihPal; // Palette handle index
  DWORD iStart;
  DWORD cEntries;
  PALETTEENTRY aPalEntries[1]; // The peFlags fields do not contain any flags
} EMRSETPALETTEENTRIES, *PEMRSETPALETTEENTRIES;

typedef struct tagEMRSETCOLORADJUSTMENT {
  EMR emr;
  COLORADJUSTMENT ColorAdjustment;
} EMRSETCOLORADJUSTMENT, *PEMRSETCOLORADJUSTMENT;

typedef struct tagEMRGDICOMMENT {
  EMR emr;
  DWORD cbData; // Size of data in bytes
  BYTE Data[1];
} EMRGDICOMMENT, *PEMRGDICOMMENT;

typedef struct tagEMREOF {
  EMR emr;
  DWORD nPalEntries;   // Number of palette entries
  DWORD offPalEntries; // Offset to the palette entries
  DWORD nSizeLast;     // Same as nSize and must be the last DWORD
                       // of the record.  The palette entries,
                       // if exist, precede this field.
} EMREOF, *PEMREOF;

typedef struct tagEMRLINETO {
  EMR emr;
  POINTL ptl;
} EMRLINETO, *PEMRLINETO, EMRMOVETOEX, *PEMRMOVETOEX;

typedef struct tagEMROFFSETCLIPRGN {
  EMR emr;
  POINTL ptlOffset;
} EMROFFSETCLIPRGN, *PEMROFFSETCLIPRGN;

typedef struct tagEMRFILLPATH {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
} EMRFILLPATH, *PEMRFILLPATH, EMRSTROKEANDFILLPATH, *PEMRSTROKEANDFILLPATH, EMRSTROKEPATH, *PEMRSTROKEPATH;

typedef struct tagEMREXCLUDECLIPRECT {
  EMR emr;
  RECTL rclClip;
} EMREXCLUDECLIPRECT, *PEMREXCLUDECLIPRECT, EMRINTERSECTCLIPRECT, *PEMRINTERSECTCLIPRECT;

typedef struct tagEMRSETVIEWPORTORGEX {
  EMR emr;
  POINTL ptlOrigin;
} EMRSETVIEWPORTORGEX, *PEMRSETVIEWPORTORGEX, EMRSETWINDOWORGEX, *PEMRSETWINDOWORGEX, EMRSETBRUSHORGEX, *PEMRSETBRUSHORGEX;

typedef struct tagEMRSETVIEWPORTEXTEX {
  EMR emr;
  SIZEL szlExtent;
} EMRSETVIEWPORTEXTEX, *PEMRSETVIEWPORTEXTEX, EMRSETWINDOWEXTEX, *PEMRSETWINDOWEXTEX;

typedef struct tagEMRSCALEVIEWPORTEXTEX {
  EMR emr;
  LONG xNum;
  LONG xDenom;
  LONG yNum;
  LONG yDenom;
} EMRSCALEVIEWPORTEXTEX, *PEMRSCALEVIEWPORTEXTEX, EMRSCALEWINDOWEXTEX, *PEMRSCALEWINDOWEXTEX;

typedef struct tagEMRSETWORLDTRANSFORM {
  EMR emr;
  XFORM xform;
} EMRSETWORLDTRANSFORM, *PEMRSETWORLDTRANSFORM;

typedef struct tagEMRMODIFYWORLDTRANSFORM {
  EMR emr;
  XFORM xform;
  DWORD iMode;
} EMRMODIFYWORLDTRANSFORM, *PEMRMODIFYWORLDTRANSFORM;

typedef struct tagEMRSETPIXELV {
  EMR emr;
  POINTL ptlPixel;
  COLORREF crColor;
} EMRSETPIXELV, *PEMRSETPIXELV;

typedef struct tagEMREXTFLOODFILL {
  EMR emr;
  POINTL ptlStart;
  COLORREF crColor;
  DWORD iMode;
} EMREXTFLOODFILL, *PEMREXTFLOODFILL;

typedef struct tagEMRELLIPSE {
  EMR emr;
  RECTL rclBox; // Inclusive-inclusive bounding rectangle
} EMRELLIPSE, *PEMRELLIPSE, EMRRECTANGLE, *PEMRRECTANGLE;

typedef struct tagEMRROUNDRECT {
  EMR emr;
  RECTL rclBox; // Inclusive-inclusive bounding rectangle
  SIZEL szlCorner;
} EMRROUNDRECT, *PEMRROUNDRECT;

typedef struct tagEMRARC {
  EMR emr;
  RECTL rclBox; // Inclusive-inclusive bounding rectangle
  POINTL ptlStart;
  POINTL ptlEnd;
} EMRARC, *PEMRARC, EMRARCTO, *PEMRARCTO, EMRCHORD, *PEMRCHORD, EMRPIE, *PEMRPIE;

typedef struct tagEMRANGLEARC {
  EMR emr;
  POINTL ptlCenter;
  DWORD nRadius;
  FLOAT eStartAngle;
  FLOAT eSweepAngle;
} EMRANGLEARC, *PEMRANGLEARC;

typedef struct tagEMRPOLYLINE {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  DWORD cptl;
  POINTL aptl[1];
} EMRPOLYLINE, *PEMRPOLYLINE, EMRPOLYBEZIER, *PEMRPOLYBEZIER, EMRPOLYGON, *PEMRPOLYGON, EMRPOLYBEZIERTO, *PEMRPOLYBEZIERTO, EMRPOLYLINETO, *PEMRPOLYLINETO;

typedef struct tagEMRPOLYLINE16 {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  DWORD cpts;
  POINTS apts[1];
} EMRPOLYLINE16, *PEMRPOLYLINE16, EMRPOLYBEZIER16, *PEMRPOLYBEZIER16, EMRPOLYGON16, *PEMRPOLYGON16, EMRPOLYBEZIERTO16, *PEMRPOLYBEZIERTO16, EMRPOLYLINETO16, *PEMRPOLYLINETO16;

typedef struct tagEMRPOLYDRAW {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  DWORD cptl;      // Number of points
  POINTL aptl[1];  // Array of points
  BYTE abTypes[1]; // Array of point types
} EMRPOLYDRAW, *PEMRPOLYDRAW;

typedef struct tagEMRPOLYDRAW16 {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  DWORD cpts;      // Number of points
  POINTS apts[1];  // Array of points
  BYTE abTypes[1]; // Array of point types
} EMRPOLYDRAW16, *PEMRPOLYDRAW16;

typedef struct tagEMRPOLYPOLYLINE {
  EMR emr;
  RECTL rclBounds;      // Inclusive-inclusive bounds in device units
  DWORD nPolys;         // Number of polys
  DWORD cptl;           // Total number of points in all polys
  DWORD aPolyCounts[1]; // Array of point counts for each poly
  POINTL aptl[1];       // Array of points
} EMRPOLYPOLYLINE, *PEMRPOLYPOLYLINE, EMRPOLYPOLYGON, *PEMRPOLYPOLYGON;

typedef struct tagEMRPOLYPOLYLINE16 {
  EMR emr;
  RECTL rclBounds;      // Inclusive-inclusive bounds in device units
  DWORD nPolys;         // Number of polys
  DWORD cpts;           // Total number of points in all polys
  DWORD aPolyCounts[1]; // Array of point counts for each poly
  POINTS apts[1];       // Array of points
} EMRPOLYPOLYLINE16, *PEMRPOLYPOLYLINE16, EMRPOLYPOLYGON16, *PEMRPOLYPOLYGON16;

typedef struct tagEMRINVERTRGN {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  DWORD cbRgnData; // Size of region data in bytes
  BYTE RgnData[1];
} EMRINVERTRGN, *PEMRINVERTRGN, EMRPAINTRGN, *PEMRPAINTRGN;

typedef struct tagEMRFILLRGN {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  DWORD cbRgnData; // Size of region data in bytes
  DWORD ihBrush;   // Brush handle index
  BYTE RgnData[1];
} EMRFILLRGN, *PEMRFILLRGN;

typedef struct tagEMRFRAMERGN {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  DWORD cbRgnData; // Size of region data in bytes
  DWORD ihBrush;   // Brush handle index
  SIZEL szlStroke;
  BYTE RgnData[1];
} EMRFRAMERGN, *PEMRFRAMERGN;

typedef struct tagEMREXTSELECTCLIPRGN {
  EMR emr;
  DWORD cbRgnData; // Size of region data in bytes
  DWORD iMode;
  BYTE RgnData[1];
} EMREXTSELECTCLIPRGN, *PEMREXTSELECTCLIPRGN;

typedef struct tagEMREXTTEXTOUTA {
  EMR emr;
  RECTL rclBounds;     // Inclusive-inclusive bounds in device units
  DWORD iGraphicsMode; // Current graphics mode
  FLOAT exScale;       // X and Y scales from Page units to .01mm units
  FLOAT eyScale;       //   if graphics mode is GM_COMPATIBLE.
  EMRTEXT emrtext;     // This is followed by the string and spacing
                       // array
} EMREXTTEXTOUTA, *PEMREXTTEXTOUTA, EMREXTTEXTOUTW, *PEMREXTTEXTOUTW;

typedef struct tagEMRPOLYTEXTOUTA {
  EMR emr;
  RECTL rclBounds;     // Inclusive-inclusive bounds in device units
  DWORD iGraphicsMode; // Current graphics mode
  FLOAT exScale;       // X and Y scales from Page units to .01mm units
  FLOAT eyScale;       //   if graphics mode is GM_COMPATIBLE.
  LONG cStrings;
  EMRTEXT aemrtext[1]; // Array of EMRTEXT structures.  This is
                       // followed by the strings and spacing arrays.
} EMRPOLYTEXTOUTA, *PEMRPOLYTEXTOUTA, EMRPOLYTEXTOUTW, *PEMRPOLYTEXTOUTW;

typedef struct tagEMRBITBLT {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  LONG xDest;
  LONG yDest;
  LONG cxDest;
  LONG cyDest;
  DWORD dwRop;
  LONG xSrc;
  LONG ySrc;
  XFORM xformSrc;        // Source DC transform
  COLORREF crBkColorSrc; // Source DC BkColor in RGB
  DWORD iUsageSrc;       // Source bitmap info color table usage
                         // (DIB_RGB_COLORS)
  DWORD offBmiSrc;       // Offset to the source BITMAPINFO structure
  DWORD cbBmiSrc;        // Size of the source BITMAPINFO structure
  DWORD offBitsSrc;      // Offset to the source bitmap bits
  DWORD cbBitsSrc;       // Size of the source bitmap bits
} EMRBITBLT, *PEMRBITBLT;

typedef struct tagEMRSTRETCHBLT {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  LONG xDest;
  LONG yDest;
  LONG cxDest;
  LONG cyDest;
  DWORD dwRop;
  LONG xSrc;
  LONG ySrc;
  XFORM xformSrc;        // Source DC transform
  COLORREF crBkColorSrc; // Source DC BkColor in RGB
  DWORD iUsageSrc;       // Source bitmap info color table usage
                         // (DIB_RGB_COLORS)
  DWORD offBmiSrc;       // Offset to the source BITMAPINFO structure
  DWORD cbBmiSrc;        // Size of the source BITMAPINFO structure
  DWORD offBitsSrc;      // Offset to the source bitmap bits
  DWORD cbBitsSrc;       // Size of the source bitmap bits
  LONG cxSrc;
  LONG cySrc;
} EMRSTRETCHBLT, *PEMRSTRETCHBLT;

typedef struct tagEMRMASKBLT {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  LONG xDest;
  LONG yDest;
  LONG cxDest;
  LONG cyDest;
  DWORD dwRop;
  LONG xSrc;
  LONG ySrc;
  XFORM xformSrc;        // Source DC transform
  COLORREF crBkColorSrc; // Source DC BkColor in RGB
  DWORD iUsageSrc;       // Source bitmap info color table usage
                         // (DIB_RGB_COLORS)
  DWORD offBmiSrc;       // Offset to the source BITMAPINFO structure
  DWORD cbBmiSrc;        // Size of the source BITMAPINFO structure
  DWORD offBitsSrc;      // Offset to the source bitmap bits
  DWORD cbBitsSrc;       // Size of the source bitmap bits
  LONG xMask;
  LONG yMask;
  DWORD iUsageMask;  // Mask bitmap info color table usage
  DWORD offBmiMask;  // Offset to the mask BITMAPINFO structure if any
  DWORD cbBmiMask;   // Size of the mask BITMAPINFO structure if any
  DWORD offBitsMask; // Offset to the mask bitmap bits if any
  DWORD cbBitsMask;  // Size of the mask bitmap bits if any
} EMRMASKBLT, *PEMRMASKBLT;

typedef struct tagEMRPLGBLT {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  POINTL aptlDest[3];
  LONG xSrc;
  LONG ySrc;
  LONG cxSrc;
  LONG cySrc;
  XFORM xformSrc;        // Source DC transform
  COLORREF crBkColorSrc; // Source DC BkColor in RGB
  DWORD iUsageSrc;       // Source bitmap info color table usage
                         // (DIB_RGB_COLORS)
  DWORD offBmiSrc;       // Offset to the source BITMAPINFO structure
  DWORD cbBmiSrc;        // Size of the source BITMAPINFO structure
  DWORD offBitsSrc;      // Offset to the source bitmap bits
  DWORD cbBitsSrc;       // Size of the source bitmap bits
  LONG xMask;
  LONG yMask;
  DWORD iUsageMask;  // Mask bitmap info color table usage
  DWORD offBmiMask;  // Offset to the mask BITMAPINFO structure if any
  DWORD cbBmiMask;   // Size of the mask BITMAPINFO structure if any
  DWORD offBitsMask; // Offset to the mask bitmap bits if any
  DWORD cbBitsMask;  // Size of the mask bitmap bits if any
} EMRPLGBLT, *PEMRPLGBLT;

typedef struct tagEMRSETDIBITSTODEVICE {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  LONG xDest;
  LONG yDest;
  LONG xSrc;
  LONG ySrc;
  LONG cxSrc;
  LONG cySrc;
  DWORD offBmiSrc;  // Offset to the source BITMAPINFO structure
  DWORD cbBmiSrc;   // Size of the source BITMAPINFO structure
  DWORD offBitsSrc; // Offset to the source bitmap bits
  DWORD cbBitsSrc;  // Size of the source bitmap bits
  DWORD iUsageSrc;  // Source bitmap info color table usage
  DWORD iStartScan;
  DWORD cScans;
} EMRSETDIBITSTODEVICE, *PEMRSETDIBITSTODEVICE;

typedef struct tagEMRSTRETCHDIBITS {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  LONG xDest;
  LONG yDest;
  LONG xSrc;
  LONG ySrc;
  LONG cxSrc;
  LONG cySrc;
  DWORD offBmiSrc;  // Offset to the source BITMAPINFO structure
  DWORD cbBmiSrc;   // Size of the source BITMAPINFO structure
  DWORD offBitsSrc; // Offset to the source bitmap bits
  DWORD cbBitsSrc;  // Size of the source bitmap bits
  DWORD iUsageSrc;  // Source bitmap info color table usage
  DWORD dwRop;
  LONG cxDest;
  LONG cyDest;
} EMRSTRETCHDIBITS, *PEMRSTRETCHDIBITS;

typedef struct tagEMREXTCREATEFONTINDIRECTW {
  EMR emr;
  DWORD ihFont; // Font handle index
  EXTLOGFONTW elfw;
} EMREXTCREATEFONTINDIRECTW, *PEMREXTCREATEFONTINDIRECTW;

typedef struct tagEMRCREATEPALETTE {
  EMR emr;
  DWORD ihPal;     // Palette handle index
  LOGPALETTE lgpl; // The peFlags fields in the palette entries
                   // do not contain any flags
} EMRCREATEPALETTE, *PEMRCREATEPALETTE;

typedef struct tagEMRCREATEPEN {
  EMR emr;
  DWORD ihPen; // Pen handle index
  LOGPEN lopn;
} EMRCREATEPEN, *PEMRCREATEPEN;

typedef struct tagEMREXTCREATEPEN {
  EMR emr;
  DWORD ihPen;     // Pen handle index
  DWORD offBmi;    // Offset to the BITMAPINFO structure if any
  DWORD cbBmi;     // Size of the BITMAPINFO structure if any
                   // The bitmap info is followed by the bitmap
                   // bits to form a packed DIB.
  DWORD offBits;   // Offset to the brush bitmap bits if any
  DWORD cbBits;    // Size of the brush bitmap bits if any
  EXTLOGPEN32 elp; // The extended pen with the style array.
} EMREXTCREATEPEN, *PEMREXTCREATEPEN;

typedef struct tagEMRCREATEBRUSHINDIRECT {
  EMR emr;
  DWORD ihBrush; // Brush handle index
  LOGBRUSH32 lb; // The style must be BS_SOLID, BS_HOLLOW,
                 // BS_NULL or BS_HATCHED.
} EMRCREATEBRUSHINDIRECT, *PEMRCREATEBRUSHINDIRECT;

typedef struct tagEMRCREATEMONOBRUSH {
  EMR emr;
  DWORD ihBrush; // Brush handle index
  DWORD iUsage;  // Bitmap info color table usage
  DWORD offBmi;  // Offset to the BITMAPINFO structure
  DWORD cbBmi;   // Size of the BITMAPINFO structure
  DWORD offBits; // Offset to the bitmap bits
  DWORD cbBits;  // Size of the bitmap bits
} EMRCREATEMONOBRUSH, *PEMRCREATEMONOBRUSH;

typedef struct tagEMRCREATEDIBPATTERNBRUSHPT {
  EMR emr;
  DWORD ihBrush; // Brush handle index
  DWORD iUsage;  // Bitmap info color table usage
  DWORD offBmi;  // Offset to the BITMAPINFO structure
  DWORD cbBmi;   // Size of the BITMAPINFO structure
                 // The bitmap info is followed by the bitmap
                 // bits to form a packed DIB.
  DWORD offBits; // Offset to the bitmap bits
  DWORD cbBits;  // Size of the bitmap bits
} EMRCREATEDIBPATTERNBRUSHPT, *PEMRCREATEDIBPATTERNBRUSHPT;

typedef struct tagEMRFORMAT {
  DWORD dSignature; // Format signature, e.g. ENHMETA_SIGNATURE.
  DWORD nVersion;   // Format version number.
  DWORD cbData;     // Size of data in bytes.
  DWORD offData;    // Offset to data from GDICOMMENT_IDENTIFIER.
                    // It must begin at a DWORD offset.
} EMRFORMAT, *PEMRFORMAT;

#if (WINVER >= 0x0400)

typedef struct tagEMRGLSRECORD {
  EMR emr;
  DWORD cbData; // Size of data in bytes
  BYTE Data[1];
} EMRGLSRECORD, *PEMRGLSRECORD;

typedef struct tagEMRGLSBOUNDEDRECORD {
  EMR emr;
  RECTL rclBounds; // Bounds in recording coordinates
  DWORD cbData;    // Size of data in bytes
  BYTE Data[1];
} EMRGLSBOUNDEDRECORD, *PEMRGLSBOUNDEDRECORD;

typedef struct tagEMRPIXELFORMAT {
  EMR emr;
  PIXELFORMATDESCRIPTOR pfd;
} EMRPIXELFORMAT, *PEMRPIXELFORMAT;

typedef struct tagEMRCREATECOLORSPACE {
  EMR emr;
  DWORD ihCS;         // ColorSpace handle index
  LOGCOLORSPACEA lcs; // Ansi version of LOGCOLORSPACE
} EMRCREATECOLORSPACE, *PEMRCREATECOLORSPACE;

typedef struct tagEMRSETCOLORSPACE {
  EMR emr;
  DWORD ihCS; // ColorSpace handle index
} EMRSETCOLORSPACE, *PEMRSETCOLORSPACE, EMRSELECTCOLORSPACE, *PEMRSELECTCOLORSPACE, EMRDELETECOLORSPACE, *PEMRDELETECOLORSPACE;

#endif /* WINVER >= 0x0400 */

#if (WINVER >= 0x0500)

typedef struct tagEMREXTESCAPE {
  EMR emr;
  INT iEscape;     // Escape code
  INT cbEscData;   // Size of escape data
  BYTE EscData[1]; // Escape data
} EMREXTESCAPE, *PEMREXTESCAPE, EMRDRAWESCAPE, *PEMRDRAWESCAPE;

typedef struct tagEMRNAMEDESCAPE {
  EMR emr;
  INT iEscape;     // Escape code
  INT cbDriver;    // Size of driver name
  INT cbEscData;   // Size of escape data
  BYTE EscData[1]; // Driver name and Escape data
} EMRNAMEDESCAPE, *PEMRNAMEDESCAPE;

#define SETICMPROFILE_EMBEDED 0x00000001

typedef struct tagEMRSETICMPROFILE {
  EMR emr;
  DWORD dwFlags; // flags
  DWORD cbName;  // Size of desired profile name
  DWORD cbData;  // Size of raw profile data if attached
  BYTE Data[1];  // Array size is cbName + cbData
} EMRSETICMPROFILE, *PEMRSETICMPROFILE, EMRSETICMPROFILEA, *PEMRSETICMPROFILEA, EMRSETICMPROFILEW, *PEMRSETICMPROFILEW;

#define CREATECOLORSPACE_EMBEDED 0x00000001

typedef struct tagEMRCREATECOLORSPACEW {
  EMR emr;
  DWORD ihCS;         // ColorSpace handle index
  LOGCOLORSPACEW lcs; // Unicode version of logical color space structure
  DWORD dwFlags;      // flags
  DWORD cbData;       // size of raw source profile data if attached
  BYTE Data[1];       // Array size is cbData
} EMRCREATECOLORSPACEW, *PEMRCREATECOLORSPACEW;

#define COLORMATCHTOTARGET_EMBEDED 0x00000001

typedef struct tagCOLORMATCHTOTARGET {
  EMR emr;
  DWORD dwAction; // CS_ENABLE, CS_DISABLE or CS_DELETE_TRANSFORM
  DWORD dwFlags;  // flags
  DWORD cbName;   // Size of desired target profile name
  DWORD cbData;   // Size of raw target profile data if attached
  BYTE Data[1];   // Array size is cbName + cbData
} EMRCOLORMATCHTOTARGET, *PEMRCOLORMATCHTOTARGET;

typedef struct tagCOLORCORRECTPALETTE {
  EMR emr;
  DWORD ihPalette;   // Palette handle index
  DWORD nFirstEntry; // Index of first entry to correct
  DWORD nPalEntries; // Number of palette entries to correct
  DWORD nReserved;   // Reserved
} EMRCOLORCORRECTPALETTE, *PEMRCOLORCORRECTPALETTE;

typedef struct tagEMRALPHABLEND {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  LONG xDest;
  LONG yDest;
  LONG cxDest;
  LONG cyDest;
  DWORD dwRop;
  LONG xSrc;
  LONG ySrc;
  XFORM xformSrc;        // Source DC transform
  COLORREF crBkColorSrc; // Source DC BkColor in RGB
  DWORD iUsageSrc;       // Source bitmap info color table usage
                         // (DIB_RGB_COLORS)
  DWORD offBmiSrc;       // Offset to the source BITMAPINFO structure
  DWORD cbBmiSrc;        // Size of the source BITMAPINFO structure
  DWORD offBitsSrc;      // Offset to the source bitmap bits
  DWORD cbBitsSrc;       // Size of the source bitmap bits
  LONG cxSrc;
  LONG cySrc;
} EMRALPHABLEND, *PEMRALPHABLEND;

typedef struct tagEMRGRADIENTFILL {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  DWORD nVer;
  DWORD nTri;
  ULONG ulMode;
  TRIVERTEX Ver[1];
} EMRGRADIENTFILL, *PEMRGRADIENTFILL;

typedef struct tagEMRTRANSPARENTBLT {
  EMR emr;
  RECTL rclBounds; // Inclusive-inclusive bounds in device units
  LONG xDest;
  LONG yDest;
  LONG cxDest;
  LONG cyDest;
  DWORD dwRop;
  LONG xSrc;
  LONG ySrc;
  XFORM xformSrc;        // Source DC transform
  COLORREF crBkColorSrc; // Source DC BkColor in RGB
  DWORD iUsageSrc;       // Source bitmap info color table usage
                         // (DIB_RGB_COLORS)
  DWORD offBmiSrc;       // Offset to the source BITMAPINFO structure
  DWORD cbBmiSrc;        // Size of the source BITMAPINFO structure
  DWORD offBitsSrc;      // Offset to the source bitmap bits
  DWORD cbBitsSrc;       // Size of the source bitmap bits
  LONG cxSrc;
  LONG cySrc;
} EMRTRANSPARENTBLT, *PEMRTRANSPARENTBLT;

#endif /* WINVER >= 0x0500 */

#define GDICOMMENT_IDENTIFIER 0x43494447
#define GDICOMMENT_WINDOWS_METAFILE 0x80000001
#define GDICOMMENT_BEGINGROUP 0x00000002
#define GDICOMMENT_ENDGROUP 0x00000003
#define GDICOMMENT_MULTIFORMATS 0x40000004
#define EPS_SIGNATURE 0x46535045
#define GDICOMMENT_UNICODE_STRING 0x00000040
#define GDICOMMENT_UNICODE_END 0x00000080

#endif /* NOMETAFILE */

// OpenGL wgl prototypes

WINGDIAPI BOOL WINAPI wglCopyContext(HGLRC, HGLRC, UINT);
WINGDIAPI HGLRC WINAPI wglCreateContext(HDC);
WINGDIAPI HGLRC WINAPI wglCreateLayerContext(HDC, int);
WINGDIAPI BOOL WINAPI wglDeleteContext(HGLRC);
WINGDIAPI HGLRC WINAPI wglGetCurrentContext(VOID);
WINGDIAPI HDC WINAPI wglGetCurrentDC(VOID);
WINGDIAPI PROC WINAPI wglGetProcAddress(LPCSTR);
WINGDIAPI BOOL WINAPI wglMakeCurrent(HDC, HGLRC);
WINGDIAPI BOOL WINAPI wglShareLists(HGLRC, HGLRC);
WINGDIAPI BOOL WINAPI wglUseFontBitmapsA(HDC, DWORD, DWORD, DWORD);
WINGDIAPI BOOL WINAPI wglUseFontBitmapsW(HDC, DWORD, DWORD, DWORD);
#ifdef UNICODE
#define wglUseFontBitmaps wglUseFontBitmapsW
#else
#define wglUseFontBitmaps wglUseFontBitmapsA
#endif // !UNICODE
WINGDIAPI BOOL WINAPI SwapBuffers(HDC);

typedef struct _POINTFLOAT {
  FLOAT x;
  FLOAT y;
} POINTFLOAT, *PPOINTFLOAT;

typedef struct _GLYPHMETRICSFLOAT {
  FLOAT gmfBlackBoxX;
  FLOAT gmfBlackBoxY;
  POINTFLOAT gmfptGlyphOrigin;
  FLOAT gmfCellIncX;
  FLOAT gmfCellIncY;
} GLYPHMETRICSFLOAT, *PGLYPHMETRICSFLOAT, FAR* LPGLYPHMETRICSFLOAT;

#define WGL_FONT_LINES 0
#define WGL_FONT_POLYGONS 1
WINGDIAPI BOOL WINAPI wglUseFontOutlinesA(HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT);
WINGDIAPI BOOL WINAPI wglUseFontOutlinesW(HDC, DWORD, DWORD, DWORD, FLOAT, FLOAT, int, LPGLYPHMETRICSFLOAT);
#ifdef UNICODE
#define wglUseFontOutlines wglUseFontOutlinesW
#else
#define wglUseFontOutlines wglUseFontOutlinesA
#endif // !UNICODE

/* Layer plane descriptor */
typedef struct tagLAYERPLANEDESCRIPTOR { // lpd
  WORD nSize;
  WORD nVersion;
  DWORD dwFlags;
  BYTE iPixelType;
  BYTE cColorBits;
  BYTE cRedBits;
  BYTE cRedShift;
  BYTE cGreenBits;
  BYTE cGreenShift;
  BYTE cBlueBits;
  BYTE cBlueShift;
  BYTE cAlphaBits;
  BYTE cAlphaShift;
  BYTE cAccumBits;
  BYTE cAccumRedBits;
  BYTE cAccumGreenBits;
  BYTE cAccumBlueBits;
  BYTE cAccumAlphaBits;
  BYTE cDepthBits;
  BYTE cStencilBits;
  BYTE cAuxBuffers;
  BYTE iLayerPlane;
  BYTE bReserved;
  COLORREF crTransparent;
} LAYERPLANEDESCRIPTOR, *PLAYERPLANEDESCRIPTOR, FAR* LPLAYERPLANEDESCRIPTOR;

/* LAYERPLANEDESCRIPTOR flags */
#define LPD_DOUBLEBUFFER 0x00000001
#define LPD_STEREO 0x00000002
#define LPD_SUPPORT_GDI 0x00000010
#define LPD_SUPPORT_OPENGL 0x00000020
#define LPD_SHARE_DEPTH 0x00000040
#define LPD_SHARE_STENCIL 0x00000080
#define LPD_SHARE_ACCUM 0x00000100
#define LPD_SWAP_EXCHANGE 0x00000200
#define LPD_SWAP_COPY 0x00000400
#define LPD_TRANSPARENT 0x00001000

#define LPD_TYPE_RGBA 0
#define LPD_TYPE_COLORINDEX 1

/* wglSwapLayerBuffers flags */
#define WGL_SWAP_MAIN_PLANE 0x00000001
#define WGL_SWAP_OVERLAY1 0x00000002
#define WGL_SWAP_OVERLAY2 0x00000004
#define WGL_SWAP_OVERLAY3 0x00000008
#define WGL_SWAP_OVERLAY4 0x00000010
#define WGL_SWAP_OVERLAY5 0x00000020
#define WGL_SWAP_OVERLAY6 0x00000040
#define WGL_SWAP_OVERLAY7 0x00000080
#define WGL_SWAP_OVERLAY8 0x00000100
#define WGL_SWAP_OVERLAY9 0x00000200
#define WGL_SWAP_OVERLAY10 0x00000400
#define WGL_SWAP_OVERLAY11 0x00000800
#define WGL_SWAP_OVERLAY12 0x00001000
#define WGL_SWAP_OVERLAY13 0x00002000
#define WGL_SWAP_OVERLAY14 0x00004000
#define WGL_SWAP_OVERLAY15 0x00008000
#define WGL_SWAP_UNDERLAY1 0x00010000
#define WGL_SWAP_UNDERLAY2 0x00020000
#define WGL_SWAP_UNDERLAY3 0x00040000
#define WGL_SWAP_UNDERLAY4 0x00080000
#define WGL_SWAP_UNDERLAY5 0x00100000
#define WGL_SWAP_UNDERLAY6 0x00200000
#define WGL_SWAP_UNDERLAY7 0x00400000
#define WGL_SWAP_UNDERLAY8 0x00800000
#define WGL_SWAP_UNDERLAY9 0x01000000
#define WGL_SWAP_UNDERLAY10 0x02000000
#define WGL_SWAP_UNDERLAY11 0x04000000
#define WGL_SWAP_UNDERLAY12 0x08000000
#define WGL_SWAP_UNDERLAY13 0x10000000
#define WGL_SWAP_UNDERLAY14 0x20000000
#define WGL_SWAP_UNDERLAY15 0x40000000

WINGDIAPI BOOL WINAPI wglDescribeLayerPlane(HDC, int, int, UINT, LPLAYERPLANEDESCRIPTOR);
WINGDIAPI int WINAPI wglSetLayerPaletteEntries(HDC, int, int, int, CONST COLORREF*);
WINGDIAPI int WINAPI wglGetLayerPaletteEntries(HDC, int, int, int, COLORREF*);
WINGDIAPI BOOL WINAPI wglRealizeLayerPalette(HDC, int, BOOL);
WINGDIAPI BOOL WINAPI wglSwapLayerBuffers(HDC, UINT);

#if (WINVER >= 0x0500)

typedef struct _WGLSWAP {
  HDC hdc;
  UINT uiFlags;
} WGLSWAP, *PWGLSWAP, FAR* LPWGLSWAP;

#define WGL_SWAPMULTIPLE_MAX 16

WINGDIAPI DWORD WINAPI wglSwapMultipleBuffers(UINT, CONST WGLSWAP*);

#endif // (WINVER >= 0x0500)

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */
#pragma endregion

#endif /* NOGDI */

#pragma region Desktop Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */
#pragma endregion

#ifdef __cplusplus
}
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

#endif /* _WINGDI_ */
}
