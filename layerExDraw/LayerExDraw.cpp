#pragma comment(lib, "gdiplus.lib")
#include "ncbind/ncbind.hpp"
#include "LayerExDraw.hpp"
#include <vector>
#include <stdio.h>

// GDI+ ��{���
static GdiplusStartupInput gdiplusStartupInput;
static ULONG_PTR gdiplusToken;

/// �v���C�x�[�g�t�H���g���
static PrivateFontCollection *privateFontCollection = NULL;
static vector<void*> fontDatas;

inline static float ToFloat(FIXED& pfx)
{
  LONG l = *(LONG *)&pfx;

  return l / 65536.0f;
}

inline static PointF ToPointF(POINTFX *p)
{
  return PointF(ToFloat(p->x), -ToFloat(p->y));
}

// GDI+ ������
void initGdiPlus()
{
	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

// GDI+ �I��
void deInitGdiPlus()
{
	// �t�H���g�f�[�^�̉��
	delete privateFontCollection;
	vector<void*>::const_iterator i = fontDatas.begin();
	while (i != fontDatas.end()) {
		delete[] *i;
		i++;
	}
	fontDatas.clear();
	GdiplusShutdown(gdiplusToken);
}

/**
 * �摜�ǂݍ��ݏ���
 * @param name �t�@�C����
 * @return �摜���
 */
Image *loadImage(const tjs_char *name)
{
	Image *image = NULL;
	ttstr filename = TVPGetPlacedPath(name);
	if (filename.length()) {
		/* �t�@�C�����������܂܂ɂȂ�̂Ŕp�~
		ttstr localname(TVPGetLocallyAccessibleName(filename));
		if (localname.length()) {
			// ���t�@�C��������
			image = Image::FromFile(localname.c_str(),false);
		}
		else
		 */
		{
			// ���ڋg���g�������������X�g���[�����g���ƂȂ���wmf/emf��OutOfMemory
			// �Ȃ�ꍇ������悤�Ȃ̂ł������񃁃����Ƀ������ɓW�J���Ă���g��
			IStream *in = TVPCreateIStream(filename, TJS_BS_READ);
			if (in) {
				STATSTG stat;
				in->Stat(&stat, STATFLAG_NONAME);
				// �T�C�Y���ӂꖳ������
				ULONG size = (ULONG)stat.cbSize.QuadPart;
				HGLOBAL hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, size);
				if (hBuffer)	{
					void* pBuffer = ::GlobalLock(hBuffer);
					if (pBuffer) {
						if (in->Read(pBuffer, size, &size) == S_OK) {
							IStream* pStream = NULL;
							if(::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream) == S_OK) 	{
								image = Image::FromStream(pStream,false);
								pStream->Release();
							}
						}
						::GlobalUnlock(hBuffer);
					}
					::GlobalFree(hBuffer);
				}
				in->Release();
			}
		}
	}
	if (image && image->GetLastStatus() != Ok) {
		delete image;
		image = NULL;
	}
	return image;
}

RectF *getBounds(Image *image)
{
	RectF srcRect;
	Unit srcUnit;
	image->GetBounds(&srcRect, &srcUnit);
	REAL dpix = image->GetHorizontalResolution();
	REAL dpiy = image->GetVerticalResolution();

	// �s�N�Z���ɕϊ�
	REAL x, y, width, height;
	switch (srcUnit) {
	case UnitPoint:		// 3 -- Each unit is a printer's point, or 1/72 inch.
		x = srcRect.X * dpix / 72;
		y = srcRect.Y * dpiy / 72;
		width  = srcRect.Width * dpix / 72;
		height = srcRect.Height * dpix / 72;
		break;
	case UnitInch:       // 4 -- Each unit is 1 inch.
		x = srcRect.X * dpix;
		y = srcRect.Y * dpiy;
		width  = srcRect.Width * dpix;
		height = srcRect.Height * dpix;
		break;
	case UnitDocument:   // 5 -- Each unit is 1/300 inch.
		x = srcRect.X * dpix / 300;
		y = srcRect.Y * dpiy / 300;
		width  = srcRect.Width * dpix / 300;
		height = srcRect.Height * dpix / 300;
		break;
	case UnitMillimeter: // 6 -- Each unit is 1 millimeter.
		x = srcRect.X * dpix / 25.4F;
		y = srcRect.Y * dpiy / 25.4F;
		width  = srcRect.Width * dpix / 25.4F;
		height = srcRect.Height * dpix / 25.4F;
		break;
	default:
		x = srcRect.X;
		y = srcRect.Y;
		width  = srcRect.Width;
		height = srcRect.Height;
		break;
	}
	return new RectF(x, y, width, height);
}

// --------------------------------------------------------
// �t�H���g���
// --------------------------------------------------------

/**
 * �v���C�x�[�g�t�H���g�̒ǉ�
 * @param fontFileName �t�H���g�t�@�C����
 */
void
GdiPlus::addPrivateFont(const tjs_char *fontFileName)
{
	if (!privateFontCollection) {
		privateFontCollection = new PrivateFontCollection();
	}
	ttstr filename = TVPGetPlacedPath(fontFileName);
	if (filename.length()) {
		ttstr localname(TVPGetLocallyAccessibleName(filename));
		if (localname.length()) {
			// ���t�@�C��������
			privateFontCollection->AddFontFile(localname.c_str());
			return;
		} else {
			// �������Ƀ��[�h���ēW�J
			IStream *in = TVPCreateIStream(filename, TJS_BS_READ);
			if (in) {
				STATSTG stat;
				in->Stat(&stat, STATFLAG_NONAME);
				// �T�C�Y���ӂꖳ������
				ULONG size = (ULONG)stat.cbSize.QuadPart;
				char *data = new char[size];
				if (in->Read(data, size, &size) == S_OK) {
					privateFontCollection->AddMemoryFont(data, size);
					fontDatas.push_back(data);					
				} else {
					delete[] data;
				}
				in->Release();
				return;
			}
		}
	}
	TVPThrowExceptionMessage(L"cannot open:%1", fontFileName);
}

/**
 * �z��Ƀt�H���g�̃t�@�~���[�����i�[
 * @param array �i�[��z��
 * @param fontCollection �t�H���g�����擾���錳�� FontCollection
 */
static void addFontFamilyName(iTJSDispatch2 *array, FontCollection *fontCollection)
{
	int count = fontCollection->GetFamilyCount();
	FontFamily *families = new FontFamily[count];
	if (families) {
		fontCollection->GetFamilies(count, families, &count);
		for (int i=0;i<count;i++) {
			WCHAR familyName[LF_FACESIZE];
			if (families[i].GetFamilyName(familyName) == Ok) {
				tTJSVariant name(familyName), *param = &name;
				array->FuncCall(0, TJS_W("add"), NULL, 0, 1, &param, array);
			}
		}
		delete families;
	}
}

/**
 * �t�H���g�ꗗ�̎擾
 * @param privateOnly true �Ȃ�v���C�x�[�g�t�H���g�̂ݎ擾
 */
tTJSVariant
GdiPlus::getFontList(bool privateOnly)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	if (privateFontCollection)	{
		addFontFamilyName(array, privateFontCollection);
	}
	if (!privateOnly) {
		InstalledFontCollection installedFontCollection;
		addFontFamilyName(array, &installedFontCollection);
	}
	tTJSVariant ret(array,array);
	array->Release();
	return ret;
}

// --------------------------------------------------------
// �t�H���g���
// --------------------------------------------------------

/**
 * �R���X�g���N�^
 */
FontInfo::FontInfo() : fontFamily(NULL), emSize(12), style(0), gdiPlusUnsupportedFont(false), forceSelfPathDraw(false), propertyModified(true) {}

/**
 * �R���X�g���N�^
 * @param familyName �t�H���g�t�@�~���[
 * @param emSize �t�H���g�̃T�C�Y
 * @param style �t�H���g�X�^�C��
 */
FontInfo::FontInfo(const tjs_char *familyName, REAL emSize, INT style) : fontFamily(NULL), gdiPlusUnsupportedFont(false), forceSelfPathDraw(false), propertyModified(true)
{
	setFamilyName(familyName);
	setEmSize(emSize);
	setStyle(style);
}

/**
 * �R�s�[�R���X�g���N�^
 */
FontInfo::FontInfo(const FontInfo &orig)
{
	fontFamily = orig.fontFamily ? orig.fontFamily->Clone() : NULL;
	emSize = orig.emSize;
	style = orig.style;
}

/**
 * �f�X�g���N�^
 */
FontInfo::~FontInfo()
{
	clear();
}

/**
 * �t�H���g���̃N���A
 */
void
FontInfo::clear()
{
	delete fontFamily;
	fontFamily = NULL;
	familyName = "";
        gdiPlusUnsupportedFont = false;
        propertyModified = true;
}

/**
 * �t�H���g�̎w��
 */
void
FontInfo::setFamilyName(const tjs_char *familyName)
{
  propertyModified = true;

  if (forceSelfPathDraw) {
    clear();
    gdiPlusUnsupportedFont = true;
    this->familyName = familyName;
    return;
  }

	if (familyName) {
		clear();
		if (privateFontCollection) {
			fontFamily = new FontFamily(familyName, privateFontCollection);
			if (fontFamily->IsAvailable()) {
				this->familyName = familyName;
				return;
			} else {
				clear();
			}
		}
		fontFamily = new FontFamily(familyName);
		if (fontFamily->IsAvailable()) {
			this->familyName = familyName;
			return;
		} else {
                  clear();
                  gdiPlusUnsupportedFont = true;
                  this->familyName = familyName;
		}
	}
}

void
FontInfo::setForceSelfPathDraw(bool state)
{
  forceSelfPathDraw = state;
  this->setFamilyName(familyName.c_str());
}

bool
FontInfo::getForceSelfPathDraw(void) const
{
  return forceSelfPathDraw;
}

bool
FontInfo::getSelfPathDraw(void) const
{
  return forceSelfPathDraw || gdiPlusUnsupportedFont;
}

OUTLINETEXTMETRIC *
FontInfo::createFontMetric(void) const
{
  HDC dc = ::CreateCompatibleDC(NULL);
  if (dc == NULL)
    return NULL;
  LOGFONT font;
  memset(&font, 0, sizeof(font));
  font.lfHeight = LONG(-emSize);
  font.lfWeight = (style & 1) ? FW_BOLD : FW_REGULAR;
  font.lfItalic = style & 2;
  font.lfUnderline = style & 4;
  font.lfStrikeOut = style & 8;
  font.lfCharSet = DEFAULT_CHARSET;
  wcscpy_s(font.lfFaceName, familyName.c_str());
  HFONT hFont = CreateFontIndirect(&font);
  if (hFont == NULL) {
    DeleteObject(dc);
    return NULL;
  }
  HGDIOBJ hOldFont = SelectObject(dc, hFont);

  int size = ::GetOutlineTextMetrics(dc, 0, NULL);
  if (size > 0) {
    char *buf = new char[size];
    if (::GetOutlineTextMetrics(dc, size, reinterpret_cast<OUTLINETEXTMETRIC*>(buf))) {
      SelectObject(dc, hOldFont);
      DeleteObject(hFont);
      DeleteObject(dc);

      return reinterpret_cast<OUTLINETEXTMETRIC*>(buf);
    }
    delete[] buf;
  }

  SelectObject(dc, hOldFont);
  DeleteObject(hFont);
  DeleteObject(dc);
  return NULL;
}

void
FontInfo::updateSizeParams(void) const
{
  if (! propertyModified)
    return;

  propertyModified = false;
  ascent = 0;
  descent = 0;
  ascentLeading = 0;
  descentLeading = 0;
  lineSpacing = 0;

  OUTLINETEXTMETRIC *otm = createFontMetric();
  if (otm) {
    ascent = REAL(otm->otmTextMetrics.tmAscent);
    descent = REAL(otm->otmTextMetrics.tmDescent);
    ascentLeading = ascent - REAL(otm->otmAscent);
    descentLeading = descent - REAL(- otm->otmDescent);
    lineSpacing = REAL(otm->otmTextMetrics.tmHeight);
    delete otm;
  }
}

REAL 
FontInfo::getAscent() const
{
  this->updateSizeParams();
  return ascent;
}


REAL 
FontInfo::getDescent() const
{
  this->updateSizeParams();
  return descent;
}

REAL 
FontInfo::getAscentLeading() const
{
  this->updateSizeParams();
  return ascentLeading;
}


REAL 
FontInfo::getDescentLeading() const
{
  this->updateSizeParams();
  return descentLeading;
}

REAL 
FontInfo::getLineSpacing() const
{
  this->updateSizeParams();
  return lineSpacing;
}

// --------------------------------------------------------
// �A�s�A�����X���
// --------------------------------------------------------

Appearance::Appearance() {}

Appearance::~Appearance()
{
	clear();
}

/**
 * ���̃N���A
 */
void
Appearance::clear()
{
	drawInfos.clear();

	// customLineCaps���폜
	vector<CustomLineCap*>::const_iterator i = customLineCaps.begin();
	while (i != customLineCaps.end()) {
		delete *i;
		i++;
	}
	customLineCaps.clear();
}

// --------------------------------------------------------
// �e�^�ϊ�����
// --------------------------------------------------------

extern bool IsArray(const tTJSVariant &var);

/**
 * ���W���̐���
 */
extern PointF getPoint(const tTJSVariant &var);

/**
 * �_�̔z����擾
 */
void getPoints(const tTJSVariant &var, vector<PointF> &points)
{
	ncbPropAccessor info(var);
	int c = info.GetArrayCount();
	for (int i=0;i<c;i++) {
		tTJSVariant p;
		if (info.checkVariant(i, p)) {
			points.push_back(getPoint(p));
		}
	}
}

static void getPoints(ncbPropAccessor &info, int n, vector<PointF> &points)
{
	tTJSVariant var;
	if (info.checkVariant(n, var)) {
		getPoints(var, points);
	}
}

static void getPoints(ncbPropAccessor &info, const tjs_char *n, vector<PointF> &points)
{
	tTJSVariant var;
	if (info.checkVariant(n, var)) {
		getPoints(var, points);
	}
}

// -----------------------------

/**
 * ��`���̐���
 */
extern RectF getRect(const tTJSVariant &var);

/**
 * ��`�̔z����擾
 */
void getRects(const tTJSVariant &var, vector<RectF> &rects)
{
	ncbPropAccessor info(var);
	int c = info.GetArrayCount();
	for (int i=0;i<c;i++) {
		tTJSVariant p;
		if (info.checkVariant(i, p)) {
			rects.push_back(getRect(p));
		}
	}
}

// -----------------------------

/**
 * �����̔z����擾
 */
static void getReals(const tTJSVariant &var, vector<REAL> &points)
{
	ncbPropAccessor info(var);
	int c = info.GetArrayCount();
	for (int i=0;i<c;i++) {
		points.push_back((REAL)info.getRealValue(i));
	}
}

static void getReals(ncbPropAccessor &info, int n, vector<REAL> &points)
{
	tTJSVariant var;
	if (info.checkVariant(n, var)) {
		getReals(var, points);
	}
}

static void getReals(ncbPropAccessor &info, const tjs_char *n, vector<REAL> &points)
{
	tTJSVariant var;
	if (info.checkVariant(n, var)) {
		getReals(var, points);
	}
}

// -----------------------------

/**
 * �F�̔z����擾
 */
static void getColors(const tTJSVariant &var, vector<Color> &colors)
{
	ncbPropAccessor info(var);
	int c = info.GetArrayCount();
	for (int i=0;i<c;i++) {
		colors.push_back(Color((ARGB)info.getIntValue(i)));
	}
}

static void getColors(ncbPropAccessor &info, int n, vector<Color> &colors)
{
	tTJSVariant var;
	if (info.checkVariant(n, var)) {
		getColors(var, colors);
	}
}

static void getColors(ncbPropAccessor &info, const tjs_char *n, vector<Color> &colors)
{
	tTJSVariant var;
	if (info.checkVariant(n, var)) {
		getColors(var, colors);
	}
}

template <class T>
void commonBrushParameter(ncbPropAccessor &info, T *brush)
{
	tTJSVariant var;
	// SetBlend
	if (info.checkVariant(L"blend", var)) {
		vector<REAL> factors;
		vector<REAL> positions;
		ncbPropAccessor binfo(var);
		if (IsArray(var)) {
			getReals(binfo, 0, factors);
			getReals(binfo, 1, positions);
		} else {
			getReals(binfo, L"blendFactors", factors);
			getReals(binfo, L"blendPositions", positions);
		}
		int count = (int)factors.size();
		if ((int)positions.size() > count) {
			count = (int)positions.size();
		}
		if (count > 0) {
			brush->SetBlend(&factors[0], &positions[0], count);
		}
	}
	// SetBlendBellShape
	if (info.checkVariant(L"blendBellShape", var)) {
		ncbPropAccessor sinfo(var);
		if (IsArray(var)) {
			brush->SetBlendBellShape((REAL)sinfo.getRealValue(0),
									 (REAL)sinfo.getRealValue(1));
		} else {
			brush->SetBlendBellShape((REAL)info.getRealValue(L"focus"),
									 (REAL)info.getRealValue(L"scale"));
		}
	}
	// SetBlendTriangularShape
	if (info.checkVariant(L"blendTriangularShape", var)) {
		ncbPropAccessor sinfo(var);
		if (IsArray(var)) {
			brush->SetBlendTriangularShape((REAL)sinfo.getRealValue(0),
										   (REAL)sinfo.getRealValue(1));
		} else {
			brush->SetBlendTriangularShape((REAL)info.getRealValue(L"focus"),
										   (REAL)info.getRealValue(L"scale"));
		}
	}
	// SetGammaCorrection
	if (info.checkVariant(L"useGammaCorrection", var)) {
		brush->SetGammaCorrection((BOOL)var);
	}
	// SetInterpolationColors
	if (info.checkVariant(L"interpolationColors", var)) {
		vector<Color> colors;
		vector<REAL> positions;
		ncbPropAccessor binfo(var);
		if (IsArray(var)) {
			getColors(binfo, 0, colors);
			getReals(binfo, 1, positions);
		} else {
			getColors(binfo, L"presetColors", colors);
			getReals(binfo, L"blendPositions", positions);
		}
		int count = (int)colors.size();
		if ((int)positions.size() > count) {
			count = (int)positions.size();
		}
		if (count > 0) {
			brush->SetInterpolationColors(&colors[0], &positions[0], count);
		}
	}
}

/**
 * �u���V�̐���
 */
Brush* createBrush(const tTJSVariant colorOrBrush)
{
	Brush *brush;
	if (colorOrBrush.Type() != tvtObject) {
		brush = new SolidBrush(Color((tjs_int)colorOrBrush));
	} else {
		// ��ʂ��Ƃɍ�蕪����
		ncbPropAccessor info(colorOrBrush);
		BrushType type = (BrushType)info.getIntValue(L"type", BrushTypeSolidColor);
		switch (type) {
		case BrushTypeSolidColor:
			brush = new SolidBrush(Color((ARGB)info.getIntValue(L"color", 0xffffffff)));
			break;
		case BrushTypeHatchFill:
			brush = new HatchBrush((HatchStyle)info.getIntValue(L"hatchStyle", HatchStyleHorizontal),
								   Color((ARGB)info.getIntValue(L"foreColor", 0xffffffff)),
								   Color((ARGB)info.getIntValue(L"backColor", 0xff000000)));
			break;
		case BrushTypeTextureFill:
			{
				ttstr imgname = info.GetValue(L"image", ncbTypedefs::Tag<ttstr>());
				Image *image = loadImage(imgname.c_str());
				if (image) {
					WrapMode wrapMode = (WrapMode)info.getIntValue(L"wrapMode", WrapModeTile);
					tTJSVariant dstRect;
					if (info.checkVariant(L"dstRect", dstRect)) {
						brush = new TextureBrush(image, wrapMode, getRect(dstRect));
					} else {
						brush = new TextureBrush(image, wrapMode);
					}
					delete image;
				}
				break;
			}
		case BrushTypePathGradient:
			{
				PathGradientBrush *pbrush;
				vector<PointF> points;
				getPoints(info, L"points", points);
				if ((int)points.size() == 0) TVPThrowExceptionMessage(L"must set poins");
				WrapMode wrapMode = (WrapMode)info.getIntValue(L"wrapMode", WrapModeTile);
				pbrush = new PathGradientBrush(&points[0], (int)points.size(), wrapMode);

				// ���ʃp�����[�^
				commonBrushParameter(info, pbrush);

				tTJSVariant var;
				//SetCenterColor
				if (info.checkVariant(L"centerColor", var)) {
					pbrush->SetCenterColor(Color((ARGB)(tjs_int)var));
				}
				//SetCenterPoint
				if (info.checkVariant(L"centerPoint", var)) {
					pbrush->SetCenterPoint(getPoint(var));
				}
				//SetFocusScales
				if (info.checkVariant(L"focusScales", var)) {
					ncbPropAccessor sinfo(var);
					if (IsArray(var)) {
						pbrush->SetFocusScales((REAL)sinfo.getRealValue(0),
											   (REAL)sinfo.getRealValue(1));
					} else {
						pbrush->SetFocusScales((REAL)info.getRealValue(L"xScale"),
											   (REAL)info.getRealValue(L"yScale"));
					}
				}
				//SetSurroundColors
				if (info.checkVariant(L"surroundColors", var)) {
					vector<Color> colors;
					getColors(var, colors);
					int size = (int)colors.size();
					pbrush->SetSurroundColors(&colors[0], &size);
				}
				brush = pbrush;
			}
			break;
		case BrushTypeLinearGradient:
			{
				LinearGradientBrush *lbrush;
				Color color1((ARGB)info.getIntValue(L"color1", 0));
				Color color2((ARGB)info.getIntValue(L"color2", 0));

				tTJSVariant var;
				if (info.checkVariant(L"point1", var)) {
					PointF point1 = getPoint(var);
					info.checkVariant(L"point2", var);
					PointF point2 = getPoint(var);
					lbrush = new LinearGradientBrush(point1, point2, color1, color2);
				} else if (info.checkVariant(L"rect", var)) {
					RectF rect = getRect(var);
					if (info.HasValue(L"angle")) {
						// �A���O���w�肪����ꍇ
						lbrush = new LinearGradientBrush(rect, color1, color2,
														 (REAL)info.getRealValue(L"angle", 0),
														 (BOOL)info.getIntValue(L"isAngleScalable", 0));
					} else {
						// �����ꍇ�̓��[�h���Q��
						lbrush = new LinearGradientBrush(rect, color1, color2,
														 (LinearGradientMode)info.getIntValue(L"mode", LinearGradientModeHorizontal));
					}
				} else {
					TVPThrowExceptionMessage(L"must set point1,2 or rect");
				}

				// ���ʃp�����[�^
				commonBrushParameter(info, lbrush);

				// SetWrapMode
				if (info.checkVariant(L"wrapMode", var)) {
					lbrush->SetWrapMode((WrapMode)(tjs_int)var);
				}
				brush = lbrush;
			}
			break;
		default:
			TVPThrowExceptionMessage(L"invalid brush type");
			break;
		}
	}
	return brush;
}

/**
 * �u���V�̒ǉ�
 * @param colorOrBrush ARGB�F�w��܂��̓u���V���i�����j
 * @param ox �\���I�t�Z�b�gX
 * @param oy �\���I�t�Z�b�gY
 */
void
Appearance::addBrush(tTJSVariant colorOrBrush, REAL ox, REAL oy)
{
	drawInfos.push_back(DrawInfo(ox, oy, createBrush(colorOrBrush)));
}

/**
 * �y���̒ǉ�
 * @param colorOrBrush ARGB�F�w��܂��̓u���V���i�����j
 * @param widthOrOption �y�����܂��̓y�����i�����j
 * @param ox �\���I�t�Z�b�gX
 * @param oy �\���I�t�Z�b�gY
 */
void
Appearance::addPen(tTJSVariant colorOrBrush, tTJSVariant widthOrOption, REAL ox, REAL oy)
{
	Pen *pen;
	REAL width = 1.0;
	if (colorOrBrush.Type() == tvtObject) {
		Brush *brush = createBrush(colorOrBrush);
		pen = new Pen(brush, width);
		delete brush;
	} else {
		pen = new Pen(Color((ARGB)(tjs_int)colorOrBrush), width);
	}
	if (widthOrOption.Type() != tvtObject) {
		pen->SetWidth((REAL)(tjs_real)widthOrOption);
	} else {
		ncbPropAccessor info(widthOrOption);
		REAL penWidth = 1.0;
		tTJSVariant var;

		// SetWidth
		if (info.checkVariant(L"width", var)) {
			penWidth = (REAL)(tjs_real)var;
		}
		pen->SetWidth(penWidth);

		// SetAlignment
		if (info.checkVariant(L"alignment", var)) {
			pen->SetAlignment((PenAlignment)(tjs_int)var);
		}
		// SetCompoundArray
		if (info.checkVariant(L"compoundArray", var)) {
			vector<REAL> reals;
			getReals(var, reals);
			pen->SetCompoundArray(&reals[0], (int)reals.size());
		}

		// SetDashCap
		if (info.checkVariant(L"dashCap", var)) {
			pen->SetDashCap((DashCap)(tjs_int)var);
		}
		// SetDashOffset
		if (info.checkVariant(L"dashOffset", var)) {
			pen->SetDashOffset((REAL)(tjs_real)var);
		}

		// SetDashStyle
		// SetDashPattern
		if (info.checkVariant(L"dashStyle", var)) {
			if (IsArray(var)) {
				vector<REAL> reals;
				getReals(var, reals);
				pen->SetDashStyle(DashStyleCustom);
				pen->SetDashPattern(&reals[0], (int)reals.size());
			} else {
				pen->SetDashStyle((DashStyle)(tjs_int)var);
			}
		}

		// SetStartCap
		// SetCustomStartCap
		if (info.checkVariant(L"startCap", var)) {
			LineCap cap = LineCapFlat;
			CustomLineCap *custom = NULL;
			if (getLineCap(var, cap, custom, penWidth)) {
				if (custom != NULL) pen->SetCustomStartCap(custom);
				else                pen->SetStartCap(cap);
			}
		}

		// SetEndCap
		// SetCustomEndCap
		if (info.checkVariant(L"endCap", var)) {
			LineCap cap = LineCapFlat;
			CustomLineCap *custom = NULL;
			if (getLineCap(var, cap, custom, penWidth)) {
				if (custom != NULL) pen->SetCustomEndCap(custom);
				else                pen->SetEndCap(cap);
			}
		}

		// SetLineJoin
		if (info.checkVariant(L"lineJoin", var)) {
			pen->SetLineJoin((LineJoin)(tjs_int)var);
		}
		
		// SetMiterLimit
		if (info.checkVariant(L"miterLimit", var)) {
			pen->SetMiterLimit((REAL)(tjs_real)var);
		}
	}
	drawInfos.push_back(DrawInfo(ox, oy, pen));
}

bool
Appearance::getLineCap(tTJSVariant &in, LineCap &cap, CustomLineCap* &custom, REAL pw)
{
	switch (in.Type()) {
	case tvtVoid:
	case tvtInteger:
		cap = (LineCap)(tjs_int)in;
		break;
	case tvtObject:
		{
			ncbPropAccessor info(in);
			REAL width = pw, height = pw;
			tTJSVariant var;
			if (info.checkVariant(L"width",  var)) width  = (REAL)(tjs_real)var;
			if (info.checkVariant(L"height", var)) height = (REAL)(tjs_real)var;
			BOOL filled = (BOOL)info.getIntValue(L"filled", 1);
			AdjustableArrowCap *arrow = new AdjustableArrowCap(height, width, filled);
			if (info.checkVariant(L"middleInset", var))
				arrow->SetMiddleInset((REAL)(tjs_real)var);
			customLineCaps.push_back((custom = static_cast<CustomLineCap*>(arrow)));
		}
		break;
	default: return false;
	}
	return true;
}


// --------------------------------------------------------
// �t�H���g�`��n
// --------------------------------------------------------

void
LayerExDraw::updateRect(RectF &rect)
{
	if (updateWhenDraw) {
		// �X�V����
		tTJSVariant  vars [4] = { rect.X, rect.Y, rect.Width, rect.Height };
		tTJSVariant *varsp[4] = { vars, vars+1, vars+2, vars+3 };
		_pUpdate(4, varsp);
	}
}

/**
 * �R���X�g���N�^
 */
LayerExDraw::LayerExDraw(DispatchT obj)
	: layerExBase(obj), width(-1), height(-1), pitch(0), buffer(NULL), bitmap(NULL), graphics(NULL),
	  clipLeft(-1), clipTop(-1), clipWidth(-1), clipHeight(-1),
	  smoothingMode(SmoothingModeAntiAlias), textRenderingHint(TextRenderingHintAntiAlias),
	  metaHDC(NULL), metaBuffer(NULL), metaStream(NULL), metafile(NULL), metaGraphics(NULL),
	  updateWhenDraw(true)
{
	metaHDC = ::CreateCompatibleDC(NULL);
}

/**
 * �f�X�g���N�^
 */
LayerExDraw::~LayerExDraw()
{
	destroyRecord();
	delete graphics;
	delete bitmap;
	if (metaHDC) {
		DeleteObject(metaHDC);
		metaHDC = NULL;
	}
}

void
LayerExDraw::reset()
{
	layerExBase::reset();
	// �ύX����Ă���ꍇ�͂���Ȃ���
	if (!(graphics &&
		  width  == _width &&
		  height == _height &&
		  pitch  == _pitch &&
		  buffer == _buffer)) {
		delete graphics;
		delete bitmap;
		width  = _width;
		height = _height;
		pitch  = _pitch;
		buffer = _buffer;
		bitmap = new Bitmap(width, height, pitch, PixelFormat32bppARGB, (unsigned char*)buffer);
		graphics = new Graphics(bitmap);
		graphics->SetCompositingMode(CompositingModeSourceOver);
		graphics->SetTransform(&calcTransform);
		clipWidth = clipHeight = -1;
	}
	// �N���b�s���O�̈�ύX�̏ꍇ�͐ݒ肵�Ȃ���
	if (_clipLeft != clipLeft ||
		_clipTop  != clipTop  ||
		_clipWidth != clipWidth ||
		_clipHeight != clipHeight) {
		clipLeft = _clipLeft;
		clipTop  = _clipTop;
		clipWidth = _clipWidth;
		clipHeight = _clipHeight;
		Region clip(Rect(clipLeft, clipTop, clipWidth, clipHeight));
		graphics->SetClip(&clip);
	}
}

void
LayerExDraw::updateViewTransform()
{
	calcTransform.Reset();
	calcTransform.Multiply(&transform, MatrixOrderAppend);
	calcTransform.Multiply(&viewTransform, MatrixOrderAppend);
	graphics->SetTransform(&calcTransform);
	redrawRecord();
}

/**
 * �\���g�����X�t�H�[���̎w��
 * @param matrix �g�����X�t�H�[���}�g���b�N�X
 */
void
LayerExDraw::setViewTransform(const Matrix *trans)
{
	if (!viewTransform.Equals(trans)) {
		viewTransform.Reset();
		viewTransform.Multiply(trans);
		updateViewTransform();
	}
}

void
LayerExDraw::resetViewTransform()
{
	viewTransform.Reset();
	updateViewTransform();
}

void
LayerExDraw::rotateViewTransform(REAL angle)
{
	viewTransform.Rotate(angle, MatrixOrderAppend);
	updateViewTransform();
}

void
LayerExDraw::scaleViewTransform(REAL sx, REAL sy)
{
	viewTransform.Scale(sx, sy, MatrixOrderAppend);
	updateViewTransform();
}

void
LayerExDraw::translateViewTransform(REAL dx, REAL dy)
{
	viewTransform.Translate(dx, dy, MatrixOrderAppend);
	updateViewTransform();
}

void
LayerExDraw::updateTransform()
{
	calcTransform.Reset();
	calcTransform.Multiply(&transform, MatrixOrderAppend);
	calcTransform.Multiply(&viewTransform, MatrixOrderAppend);
	graphics->SetTransform(&calcTransform);
	if (metaGraphics) {
		metaGraphics->SetTransform(&transform);
	}
}

/**
 * �g�����X�t�H�[���̎w��
 * @param matrix �g�����X�t�H�[���}�g���b�N�X
 */
void
LayerExDraw::setTransform(const Matrix *trans)
{
	if (!transform.Equals(trans)) {
		transform.Reset();
		transform.Multiply(trans);
		updateTransform();
	}
}

void
LayerExDraw::resetTransform()
{
	transform.Reset();
	updateTransform();
}

void
LayerExDraw::rotateTransform(REAL angle)
{
	transform.Rotate(angle, MatrixOrderAppend);
	updateTransform();
}

void
LayerExDraw::scaleTransform(REAL sx, REAL sy)
{
	transform.Scale(sx, sy, MatrixOrderAppend);
	updateTransform();
}

void
LayerExDraw::translateTransform(REAL dx, REAL dy)
{
	transform.Translate(dx, dy, MatrixOrderAppend);
	updateTransform();
}

/**
 * ��ʂ̏���
 * @param argb �����F
 */
void
LayerExDraw::clear(ARGB argb)
{
	graphics->Clear(Color(argb));
	if (metaGraphics) {
		createRecord();
		metaGraphics->Clear(Color(argb));
	}
	_pUpdate(0, NULL);
}

/**
 * �p�X�̗̈�����擾
 * @param app �\���\��
 * @param path �`�悷��p�X
 */
RectF
LayerExDraw::getPathExtents(const Appearance *app, const GraphicsPath *path)
{
	// �̈�L�^�p
	RectF rect;

	// �`������g���Ď��X�`��
	bool first = true;
	vector<Appearance::DrawInfo>::const_iterator i = app->drawInfos.begin();
	while (i != app->drawInfos.end()) {
		if (i->info) {
			Matrix matrix(1,0,0,1,i->ox,i->oy);
			matrix.Multiply(&calcTransform, MatrixOrderAppend);
			switch (i->type) {
			case 0:
				{
					Pen *pen = (Pen*)i->info;
					if (first) {
						path->GetBounds(&rect, &matrix, pen);
						first = false;
					} else {
						RectF r;
						path->GetBounds(&r, &matrix, pen);
						rect.Union(rect, rect, r);
					}
				}
				break;
			case 1:
				if (first) {
					path->GetBounds(&rect, &matrix, NULL);
					first = false;
				} else {
					RectF r;
					path->GetBounds(&r, &matrix, NULL);
					rect.Union(rect, rect, r);
				}
				break;
			}
		}
		i++;
	}
	return rect;
}

void
LayerExDraw::draw(Graphics *graphics, const Pen *pen, const Matrix *matrix, const GraphicsPath *path)
{
	GraphicsContainer container = graphics->BeginContainer();
	graphics->MultiplyTransform(matrix);
	graphics->SetSmoothingMode(smoothingMode);
	graphics->DrawPath(pen, path);
	graphics->EndContainer(container);
}

void
LayerExDraw::fill(Graphics *graphics, const Brush *brush, const Matrix *matrix, const GraphicsPath *path)
{
	GraphicsContainer container = graphics->BeginContainer();
	graphics->MultiplyTransform(matrix);
	graphics->SetSmoothingMode(smoothingMode);
	graphics->FillPath(brush, path);
	graphics->EndContainer(container);
}

/**
 * �p�X��`�悷��
 * @param app �\���\��
 * @param path �`�悷��p�X
 * @return �X�V�̈���
 */
RectF
LayerExDraw::_drawPath(const Appearance *app, const GraphicsPath *path)
{
	// �̈�L�^�p
	RectF rect;

	// �`������g���Ď��X�`��
	bool first = true;
	vector<Appearance::DrawInfo>::const_iterator i = app->drawInfos.begin();
	while (i != app->drawInfos.end()) {
		if (i->info) {
			Matrix matrix(1,0,0,1,i->ox,i->oy);
			switch (i->type) {
			case 0:
				{
					Pen *pen = (Pen*)i->info;
					draw(graphics, pen, &matrix, path);
					if (metaGraphics) {
						draw(metaGraphics, pen, &matrix, path);
					}
					matrix.Multiply(&calcTransform, MatrixOrderAppend);
					if (first) {
						path->GetBounds(&rect, &matrix, pen);
						first = false;
					} else {
						RectF r;
						path->GetBounds(&r, &matrix, pen);
						rect.Union(rect, rect, r);
					}
				}
				break;
			case 1:
				fill(graphics, (Brush*)i->info, &matrix, path);
				if (metaGraphics) {
					fill(metaGraphics, (Brush*)i->info, &matrix, path);
				}
				matrix.Multiply(&calcTransform, MatrixOrderAppend);
				if (first) {
					path->GetBounds(&rect, &matrix, NULL);
					first = false;
				} else {
					RectF r;
					path->GetBounds(&r, &matrix, NULL);
					rect.Union(rect, rect, r);
				}
				break;
			}
		}
		i++;
	}
	updateRect(rect);
	return rect;
}

/**
 * �p�X�̕`��
 * @param app �A�s�A�����X
 * @param path �p�X
 */
RectF
LayerExDraw::drawPath(const Appearance *app, const Path *path)
{
	return _drawPath(app, &path->path);
}

/**
 * �~�ʂ̕`��
 * @param x ������W
 * @param y ������W
 * @param width ����
 * @param height �c��
 * @param startAngle ���v�����~�ʊJ�n�ʒu
 * @param sweepAngle �`��p�x
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawArc(const Appearance *app, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle)
{
	GraphicsPath path;
	path.AddArc(x, y, width, height, startAngle, sweepAngle);
	return _drawPath(app, &path);
}

/**
 * �x�W�F�Ȑ��̕`��
 * @param app �A�s�A�����X
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param x3
 * @param y3
 * @param x4
 * @param y4
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawBezier(const Appearance *app, REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL x4, REAL y4)
{
	GraphicsPath path;
	path.AddBezier(x1, y1, x2, y2, x3, y3, x4, y4);
	return _drawPath(app, &path);
}

/**
 * �A���x�W�F�Ȑ��̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawBeziers(const Appearance *app, tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	GraphicsPath path;
	path.AddBeziers(&ps[0], (int)ps.size());
	return _drawPath(app, &path);
}

/**
 * Closed cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawClosedCurve(const Appearance *app, tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	GraphicsPath path;
	path.AddClosedCurve(&ps[0], (int)ps.size());
	return _drawPath(app, &path);
}

/**
 * Closed cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @pram tension tension
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawClosedCurve2(const Appearance *app, tTJSVariant points, REAL tension)
{
	vector<PointF> ps;
	getPoints(points, ps);
	GraphicsPath path;
	path.AddClosedCurve(&ps[0], (int)ps.size(), tension);
	return _drawPath(app, &path);
}

/**
 * cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawCurve(const Appearance *app, tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	GraphicsPath path;
	path.AddCurve(&ps[0], (int)ps.size());
	return _drawPath(app, &path);
}

/**
 * cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @parma tension tension
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawCurve2(const Appearance *app, tTJSVariant points, REAL tension)
{
	vector<PointF> ps;
	getPoints(points, ps);
	GraphicsPath path;
	path.AddCurve(&ps[0], (int)ps.size(), tension);
	return _drawPath(app, &path);
}

/**
 * cardinal spline �̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @param offset
 * @param numberOfSegments
 * @param tension tension
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawCurve3(const Appearance *app, tTJSVariant points, int offset, int numberOfSegments, REAL tension)
{
	vector<PointF> ps;
	getPoints(points, ps);
	GraphicsPath path;
	path.AddCurve(&ps[0], (int)ps.size(), offset, numberOfSegments, tension);
	return _drawPath(app, &path);
}

/**
 * �~���̕`��
 * @param x ������W
 * @param y ������W
 * @param width ����
 * @param height �c��
 * @param startAngle ���v�����~�ʊJ�n�ʒu
 * @param sweepAngle �`��p�x
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawPie(const Appearance *app, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle)
{
	GraphicsPath path;
	path.AddPie(x, y, width, height, startAngle, sweepAngle);
	return _drawPath(app, &path);
}

/**
 * �ȉ~�̕`��
 * @param app �A�s�A�����X
 * @param x
 * @param y
 * @param width
 * @param height
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawEllipse(const Appearance *app, REAL x, REAL y, REAL width, REAL height)
{
	GraphicsPath path;
	path.AddEllipse(x, y, width, height);
	return _drawPath(app, &path);
}

/**
 * �����̕`��
 * @param app �A�s�A�����X
 * @param x1 �n�_X���W
 * @param y1 �n�_Y���W
 * @param x2 �I�_X���W
 * @param y2 �I�_Y���W
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawLine(const Appearance *app, REAL x1, REAL y1, REAL x2, REAL y2)
{
	GraphicsPath path;
	path.AddLine(x1, y1, x2, y2);
	return _drawPath(app, &path);
}

/**
 * �A�������̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawLines(const Appearance *app, tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	GraphicsPath path;
	path.AddLines(&ps[0], (int)ps.size());
	return _drawPath(app, &path);
}

/**
 * ���p�`�̕`��
 * @param app �A�s�A�����X
 * @param points �_�̔z��
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawPolygon(const Appearance *app, tTJSVariant points)
{
	vector<PointF> ps;
	getPoints(points, ps);
	GraphicsPath path;
	path.AddPolygon(&ps[0], (int)ps.size());
	return _drawPath(app, &path);
}


/**
 * ��`�̕`��
 * @param app �A�s�A�����X
 * @param x
 * @param y
 * @param width
 * @param height
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawRectangle(const Appearance *app, REAL x, REAL y, REAL width, REAL height)
{
	GraphicsPath path;
	RectF rect(x, y, width, height);
	path.AddRectangle(rect);
	return _drawPath(app, &path);
}

/**
 * ������`�̕`��
 * @param app �A�s�A�����X
 * @param rects ��`���̔z��
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawRectangles(const Appearance *app, tTJSVariant rects)
{
	vector<RectF> rs;
	getRects(rects, rs);
	GraphicsPath path;
	path.AddRectangles(&rs[0], (int)rs.size());
	return _drawPath(app, &path);
}

/**
 * ������̃p�X�x�[�X�ł̕`��
 * @param font �t�H���g
 * @param app �A�s�A�����X
 * @param x �`��ʒuX
 * @param y �`��ʒuY
 * @param text �`��e�L�X�g
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawPathString(const FontInfo *font, const Appearance *app, REAL x, REAL y, const tjs_char *text)
{
  if (font->getSelfPathDraw())
    return drawPathString2(font, app, x, y, text);

	// ������̃p�X������
	GraphicsPath path;
	path.AddString(text, -1, font->fontFamily, font->style, font->emSize, PointF(x, y), StringFormat::GenericDefault());
	return _drawPath(app, &path);
}

static void transformRect(Matrix &calcTransform, RectF &rect)
{
	PointF points[4]; // �����W�l
	points[0].X = rect.X;
	points[0].Y = rect.Y;
	points[1].X = rect.X + rect.Width;
	points[1].Y = rect.Y;
	points[2].X = rect.X;
	points[2].Y = rect.Y + rect.Height;
	points[3].X = rect.X + rect.Width;
	points[3].Y = rect.Y + rect.Height;
	// �`��̈���Čv�Z
	calcTransform.TransformPoints(points, 4);
	REAL minx = points[0].X;
	REAL maxx = points[0].X;
	REAL miny = points[0].Y;
	REAL maxy = points[0].Y;
	for (int i=1;i<4;i++) {
		if (points[i].X < minx) { minx = points[i].X; }
		if (points[i].X > maxx) { maxx = points[i].X; }
		if (points[i].Y < miny) { miny = points[i].Y; }
		if (points[i].Y > maxy) { maxy = points[i].Y; }
	}
	rect.X = minx;
	rect.Y = miny;
	rect.Width = maxx - minx;
	rect.Height = maxy - miny;
}

/**
 * ������̕`��
 * @param font �t�H���g
 * @param app �A�s�A�����X�i�u���V�̂ݎQ�Ƃ���܂��j
 * @param x �`��ʒuX
 * @param y �`��ʒuY
 * @param text �`��e�L�X�g
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawString(const FontInfo *font, const Appearance *app, REAL x, REAL y, const tjs_char *text)
{
  if (font->getSelfPathDraw())
    return drawPathString2(font, app, x, y, text);

	graphics->SetTextRenderingHint(textRenderingHint);
	if (metaGraphics) {
		metaGraphics->SetTextRenderingHint(textRenderingHint);
	}

	// �̈�L�^�p
	RectF rect;
	// �`��t�H���g
	Font f(font->fontFamily, font->emSize, font->style, UnitPixel);

	// �`������g���Ď��X�`��
	bool first = true;
	vector<Appearance::DrawInfo>::const_iterator i = app->drawInfos.begin();
	while (i != app->drawInfos.end()) {
		if (i->info) {
			if (i->type == 1) { // �u���V�̂�
				Brush *brush = (Brush*)i->info;
				PointF p(x + i->ox, y + i->oy);
				graphics->DrawString(text, -1, &f, p, StringFormat::GenericDefault(), brush);
				if (metaGraphics) {
					metaGraphics->DrawString(text, -1, &f, p, StringFormat::GenericDefault(), brush);
				}
				// �X�V�̈�v�Z
				if (first) {
					graphics->MeasureString(text, -1, &f, p, StringFormat::GenericDefault(), &rect);
					transformRect(calcTransform, rect);
					first = false;
				} else {
					RectF r;
					graphics->MeasureString(text, -1, &f, p, StringFormat::GenericDefault(), &r);
					transformRect(calcTransform, r);
					rect.Union(rect, rect, r);
				}
				break;
			}
		}
		i++;
	}
	updateRect(rect);
	return rect;
}

/**
 * ������̕`��̈���̎擾
 * @param font �t�H���g
 * @param text �`��e�L�X�g
 * @return �`��̈���
 */
RectF
LayerExDraw::measureString(const FontInfo *font, const tjs_char *text)
{
  if (font->getSelfPathDraw())
    return measureString2(font, text);

	RectF rect;
	graphics->SetTextRenderingHint(textRenderingHint);
	Font f(font->fontFamily, font->emSize, font->style, UnitPixel);
	graphics->MeasureString(text, -1, &f, PointF(0,0), StringFormat::GenericDefault(), &rect);
	return rect;
}

/**
 * ������ɊO�ڂ���̈���̎擾
 * @param font �t�H���g
 * @param text �`��e�L�X�g
 * @return �̈���̎��� left, top, width, height
 */
RectF
LayerExDraw::measureStringInternal(const FontInfo *font, const tjs_char *text)
{
  if (font->getSelfPathDraw())
    return measureStringInternal2(font, text);
  
  RectF rect;
  graphics->SetTextRenderingHint(textRenderingHint);
  Font f(font->fontFamily, font->emSize, font->style, UnitPixel);
  graphics->MeasureString(text, -1, &f, PointF(0,0), StringFormat::GenericDefault(), &rect);
  CharacterRange charRange(0, INT(wcslen(text)));
  StringFormat stringFormat = StringFormat::GenericDefault();
  stringFormat.SetMeasurableCharacterRanges(1, &charRange);
  Region region;
  graphics->MeasureCharacterRanges(text, -1, &f, rect, &stringFormat, 1, &region);
  RectF regionBounds;
  region.GetBounds(&regionBounds, graphics);
  return regionBounds;
}

/**
 * �摜�̕`��B�R�s�[��͌��摜�� Bounds ��z�������ʒu�A�T�C�Y�� Pixel �w��ɂȂ�܂��B
 * @param x �R�s�[�挴�_
 * @param y  �R�s�[�挴�_
 * @param src �R�s�[���摜
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawImage(REAL x, REAL y, Image *src) 
{
	RectF rect;
	if (src) {
		RectF *bounds = getBounds(src);
		rect = drawImageRect(x + bounds->X, y + bounds->Y, src, 0, 0, bounds->Width, bounds->Height);
		delete bounds;
		updateRect(rect);
	}
	return rect;
}

/**
 * �摜�̋�`�R�s�[
 * @param dleft �R�s�[�捶�[
 * @param dtop  �R�s�[���[
 * @param src �R�s�[���摜
 * @param sleft ����`�̍��[
 * @param stop  ����`�̏�[
 * @param swidth ����`�̉���
 * @param sheight  ����`�̏c��
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawImageRect(REAL dleft, REAL dtop, Image *src, REAL sleft, REAL stop, REAL swidth, REAL sheight)
{
	return drawImageAffine(src, sleft, stop, swidth, sheight, true, 1, 0, 0, 1, dleft, dtop);
}

/**
 * �摜�̊g��k���R�s�[
 * @param dleft �R�s�[�捶�[
 * @param dtop  �R�s�[���[
 * @param dwidth �R�s�[��̉���
 * @param dheight  �R�s�[��̏c��
 * @param src �R�s�[���摜
 * @param sleft ����`�̍��[
 * @param stop  ����`�̏�[
 * @param swidth ����`�̉���
 * @param sheight  ����`�̏c��
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawImageStretch(REAL dleft, REAL dtop, REAL dwidth, REAL dheight, Image *src, REAL sleft, REAL stop, REAL swidth, REAL sheight)
{
	return drawImageAffine(src, sleft, stop, swidth, sheight, true, dwidth/swidth, 0, 0, dheight/sheight, dleft, dtop);
}

/**
 * �摜�̃A�t�B���ϊ��R�s�[
 * @param sleft ����`�̍��[
 * @param stop  ����`�̏�[
 * @param swidth ����`�̉���
 * @param sheight  ����`�̏c��
 * @param affine �A�t�B���p�����[�^�̎��(true:�ϊ��s��, false:���W�w��), 
 * @return �X�V�̈���
 */
RectF
LayerExDraw::drawImageAffine(Image *src, REAL sleft, REAL stop, REAL swidth, REAL sheight, bool affine, REAL A, REAL B, REAL C, REAL D, REAL E, REAL F)
{
	RectF rect;
	if (src) {
		PointF points[4]; // �����W�l
		if (affine) {
#define AFFINEX(x,y) A*x+C*y+E
#define AFFINEY(x,y) B*x+D*y+F
			points[0].X = AFFINEX(0,0);
			points[0].Y = AFFINEY(0,0);
			points[1].X = AFFINEX(swidth,0);
			points[1].Y = AFFINEY(swidth,0);
			points[2].X = AFFINEX(0,sheight);
			points[2].Y = AFFINEY(0,sheight);
			points[3].X = AFFINEX(swidth,sheight);
			points[3].Y = AFFINEY(swidth,sheight);
		} else {
			points[0].X = A;
			points[0].Y = B;
			points[1].X = C;
			points[1].Y = D;
			points[2].X = E;
			points[2].Y = F;
			points[3].X = C-A+E;
			points[3].Y = D-B+F;
		}
		graphics->DrawImage(src, points, 3, sleft, stop, swidth, sheight, UnitPixel, NULL, NULL, NULL);
		if (metaGraphics) {
			metaGraphics->DrawImage(src, points, 3, sleft, stop, swidth, sheight, UnitPixel, NULL, NULL, NULL);
		}

		// �`��̈���擾
		calcTransform.TransformPoints(points, 4);
		REAL minx = points[0].X;
		REAL maxx = points[0].X;
		REAL miny = points[0].Y;
		REAL maxy = points[0].Y;
		for (int i=1;i<4;i++) {
			if (points[i].X < minx) { minx = points[i].X; }
			if (points[i].X > maxx) { maxx = points[i].X; }
			if (points[i].Y < miny) { miny = points[i].Y; }
			if (points[i].Y > maxy) { maxy = points[i].Y; }
		}
		rect.X = minx;
		rect.Y = miny;
		rect.Width = maxx - minx;
		rect.Height = maxy - miny;

		updateRect(rect);
	}
	return rect;
}

void
LayerExDraw::createRecord()
{
	destroyRecord();
	if ((metaBuffer = ::GlobalAlloc(GMEM_MOVEABLE, 0))){
		if (::CreateStreamOnHGlobal(metaBuffer, FALSE, &metaStream) == S_OK) 	{
			metafile = new Metafile(metaStream, metaHDC, EmfTypeEmfPlusOnly);
			metaGraphics = new Graphics(metafile);
			metaGraphics->SetCompositingMode(CompositingModeSourceOver);
			metaGraphics->SetTransform(&transform);
		}
	}
}

/**
 * �L�^���̔j��
 */
void
LayerExDraw::destroyRecord()
{
	if (metaGraphics) {
		delete metaGraphics;
		metaGraphics = NULL;
	}
	if (metafile) {
		delete metafile;
		metafile = NULL;
	}
	if (metaStream) {
		metaStream->Release();
		metaStream = NULL;
	}
	if (metaBuffer) {
		::GlobalFree(metaBuffer);
		metaBuffer = NULL;
	}
}


/**
 * @param record �`����e���L�^���邩�ǂ���
 */
void
LayerExDraw::setRecord(bool record)
{
	if (record) {
		if (!metafile) {
			createRecord();
		}
	} else {
		if (metafile) {
			destroyRecord();
		}
	}
}

bool
LayerExDraw::redraw(Image *image)
{
	if (image) {
		RectF *bounds = getBounds(image);
		if (metaGraphics) {
			metaGraphics->Clear(Color(0));
			metaGraphics->ResetTransform();
			metaGraphics->DrawImage(image, bounds->X, bounds->Y, bounds->Width, bounds->Height);
			metaGraphics->SetTransform(&transform);
		}
		graphics->Clear(Color(0));
		graphics->SetTransform(&viewTransform);
		graphics->DrawImage(image, bounds->X, bounds->Y, bounds->Width, bounds->Height);
		graphics->SetTransform(&calcTransform);
		delete bounds;
		_pUpdate(0, NULL);
		return true;
	}
	return false;
}

/**
 * �L�^���e�� Image �Ƃ��Ď擾
 * @return ���������� true
 */
Image *
LayerExDraw::getRecordImage()
{
	Image *image = NULL;
	if (metafile) {
		// ���^�����擾����ɂ͈�x����K�v������
		if (metaGraphics) {
			delete metaGraphics;
			metaGraphics = NULL;
		}

		//�������ƌp�����邽�߂̍ĕ`����ʓr�\�z
		HGLOBAL oldBuffer = metaBuffer;
		metaBuffer = NULL;
		createRecord();
		
		// �ĕ`��
		if (oldBuffer) {
			IStream* pStream = NULL;
			if(::CreateStreamOnHGlobal(oldBuffer, FALSE, &pStream) == S_OK) 	{
				image = Image::FromStream(pStream,false);
				if (image) {
					redraw(image);
				}
				pStream->Release();
			}
			::GlobalFree(oldBuffer);
		}
	}
	return image;
}

/**
 * �L�^���e�̌��݂̉𑜓x�ł̍ĕ`��
 */
bool
LayerExDraw::redrawRecord()
{
	// �ĕ`�揈��
	Image *image = getRecordImage();
	if (image) {
		delete image;
		return true;
	}
	return false;
}

/**
 * �L�^���e�̕ۑ�
 * @param filename �ۑ��t�@�C����
 * @return ���������� true
 */
bool
LayerExDraw::saveRecord(const tjs_char *filename)
{
	bool ret = false;
	if (metafile) {		
		// ���^�����擾����ɂ͈�x����K�v������
		delete metaGraphics;
		metaGraphics = NULL;
		ULONG size;
		// �t�@�C���ɏ����o��
		if (metaBuffer && (size = (ULONG)::GlobalSize(metaBuffer)) > 0) {
			IStream *out = TVPCreateIStream(filename, TJS_BS_WRITE);
			if (out) {
				void* pBuffer = ::GlobalLock(metaBuffer);
				if (pBuffer) {
					ret = (out->Write(pBuffer, size, &size) == S_OK);
					::GlobalUnlock(metaBuffer);
				}
				out->Release();
			}
		}
		// �ĕ`�揈��
		Image *image = getRecordImage();
		if (image) {
			delete image;
		}
	}
	return ret;
}


/**
 * �L�^���e�̓ǂݍ���
 * @param filename �ǂݍ��݃t�@�C����
 * @return ���������� true
 */
bool
LayerExDraw::loadRecord(const tjs_char *filename)
{
	bool ret = false;
	Image *image;
	if (filename && (image = loadImage(filename))) {
		createRecord();
		ret =  redraw(image);
		delete image;
	}
	return false;
}

/**
 * �O���t�A�E�g���C���̎擾
 * @param font �t�H���g
 * @param offset �I�t�Z�b�g
 * @param path �O���t�������o���p�X
 * @param glyph �`�悷��O���t
 */
void
LayerExDraw::getGlyphOutline(const FontInfo *fontInfo, PointF &offset, GraphicsPath *path, UINT glyph)
{
  static const MAT2 mat = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };

  GLYPHMETRICS gm;

  DWORD flags = GGO_BEZIER;
  // �t�B�b�e�B���O�w�肪������� UNHINTED�ɂ���Bxs
  if (! (textRenderingHint & 1))
    flags |= GGO_UNHINTED;

  int size = GetGlyphOutlineW(metaHDC,
                              glyph,
                              flags, //  | GGO_GLYPH_INDEX,
                              &gm, 
                              0,
                              NULL, 
                              &mat 
                              );
  char *buffer = NULL;
  if (size > 0) {
	  buffer = new char[size];
	  int result = GetGlyphOutlineW(metaHDC,
									glyph,
									flags, //  | GGO_GLYPH_INDEX,
									&gm, 
									size, 
									buffer, 
									&mat 
									);
	  if (result <= 0) {
		  delete[] buffer;
		  return;
	  }
  } else {
	  GetGlyphOutlineW(metaHDC,
					   glyph,
					   GGO_METRICS,
					   &gm, 
					   0, 
					   NULL, 
					   &mat 
					   );
  }

  int index = 0;
  PointF aOffset = offset;
  aOffset.Y += fontInfo->getAscent();

  while(index < size) {
    TTPOLYGONHEADER * header = (TTPOLYGONHEADER *)(buffer + index);
    int endCurve = index + header->cb;
    index += sizeof(TTPOLYGONHEADER);
    PointF p0 = ToPointF(&header->pfxStart) + aOffset;
    while(index < endCurve) {
      TTPOLYCURVE * hcurve = (TTPOLYCURVE *)(buffer + index);
      index += 2 * sizeof(WORD);
      POINTFX * points = (POINTFX *)(buffer + index);
      index += hcurve->cpfx * sizeof(POINTFX);
      std::vector<PointF> pts(1 + hcurve->cpfx);
      pts[0] = p0;
      for(int i = 0; i < hcurve->cpfx; i++)
        pts[i + 1] = ToPointF(points + i) + aOffset;
      p0 = pts[pts.size() - 1];
      switch(hcurve->wType) {
      case TT_PRIM_LINE:
        path->AddLines(&pts[0], int(pts.size()));
        break;

      case TT_PRIM_QSPLINE:
        TVPAddLog(ttstr(L"qspline"));
        break;

      case TT_PRIM_CSPLINE:
        path->AddBeziers(&pts[0], int(pts.size()));
        break;
      }
    }

    path->CloseFigure();
  }

  offset.X += gm.gmCellIncX;

  delete[] buffer;
}

/*
 * �e�L�X�g�A�E�g���C���̎擾
 * @param font �t�H���g
 * @param offset �I�t�Z�b�g
 * @param path �O���t�������o���p�X
 * @param text �`�悷��e�L�X�g
 */
void
LayerExDraw::getTextOutline(const FontInfo *fontInfo, PointF &offset, GraphicsPath *path, ttstr text)
{
  if (metaHDC == NULL) 
    return;

  if (text.IsEmpty())
    return;

  LOGFONT font;
  memset(&font, 0, sizeof(font));
  font.lfHeight = -(LONG(fontInfo->emSize));
  font.lfWeight = (fontInfo->style & 1) ? FW_BOLD : FW_REGULAR;
  font.lfItalic = fontInfo->style & 2;
  font.lfUnderline = fontInfo->style & 4;
  font.lfStrikeOut = fontInfo->style & 8;
  font.lfCharSet = DEFAULT_CHARSET;
  wcscpy_s(font.lfFaceName, fontInfo->familyName.c_str());

  HFONT hFont = CreateFontIndirect(&font);
  HGDIOBJ hOldFont = SelectObject(metaHDC, hFont);

  for (tjs_int i = 0; i < text.GetLen(); i++) {
    this->getGlyphOutline(fontInfo, offset, path, text[i]);
  }

  SelectObject(metaHDC, hOldFont);
  DeleteObject(hFont);
}

/**
 * ������̕`��X�V�̈���̎擾(OpenType�t�H���g�Ή�)
 * @param font �t�H���g
 * @param text �`��e�L�X�g
 * @return �X�V�̈���̎��� left, top, width, height
 */
RectF 
LayerExDraw::measureString2(const FontInfo *font, const tjs_char *text)
{
  // ������̃p�X������
  GraphicsPath path;
  PointF offset(0, 0);
  this->getTextOutline(font, offset, &path, text);
  RectF result;
  path.GetBounds(&result, NULL, NULL);
  result.X = 0;
  result.Y = 0;
  result.Width += REAL(0.167 * font->emSize * 2);
  result.Height = REAL(font->getLineSpacing() * 1.124);
  return result;
}

/**
 * ������ɊO�ڂ���̈���̎擾(OpenType��PostScript�t�H���g�Ή�)
 * @param font �t�H���g
 * @param text �`��e�L�X�g
 * @return �X�V�̈���̎��� left, top, width, height
 */
RectF 
LayerExDraw::measureStringInternal2(const FontInfo *font, const tjs_char *text)
{
  // ������̃p�X������
  GraphicsPath path;
  PointF offset(0, 0);
  this->getTextOutline(font, offset, &path, text);
  RectF result;
  path.GetBounds(&result, NULL, NULL);
  result.X = REAL(LONG(0.167 * font->emSize));
  result.Y = 0;
  result.Height = font->getLineSpacing();
  return result;
}

/**
 * ������̕`��(OpenType�t�H���g�Ή�)
 * @param font �t�H���g
 * @param app �A�s�A�����X
 * @param x �`��ʒuX
 * @param y �`��ʒuY
 * @param text �`��e�L�X�g
 * @return �X�V�̈���
 */
RectF 
LayerExDraw::drawPathString2(const FontInfo *font, const Appearance *app, REAL x, REAL y, const tjs_char *text)
{
  // ������̃p�X������
  GraphicsPath path;
  PointF offset(x + LONG(0.167 * font->emSize) - 0.5f, y - 0.5f);
  this->getTextOutline(font, offset, &path, text);
  RectF result = _drawPath(app, &path);
  result.X = x;
  result.Y = y;
  result.Width += REAL(0.167 * font->emSize * 2);
  result.Height = REAL(font->getLineSpacing() * 1.124);
  return result;
}

static bool getEncoder(const tjs_char* mimeType, CLSID* pClsid)
{
	UINT num = 0, size = 0;
	::GetImageEncodersSize(&num, &size);
	if (size > 0) {
		ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)malloc(size);
		if (pImageCodecInfo) {
			GetImageEncoders(num, size, pImageCodecInfo);
			for (UINT j = 0; j < num; ++j) {
				if (wcscmp(pImageCodecInfo[j].MimeType, mimeType) == 0) {
					*pClsid = pImageCodecInfo[j].Clsid;
					free(pImageCodecInfo);
					return true;
				}
			}
			free(pImageCodecInfo);
		}
	}
	return false;
}

/**
 * �G���R�[�h�p�����[�^���̎Q�Ɨp
 */
class EncoderParameterGetter : public tTJSDispatch /** EnumMembers �p */
{
public:
	struct EncoderInfo {
		const char *name;
		GUID guid;
		long value;
		EncoderInfo(const char *name, GUID guid, long value) : name(name), guid(guid), value(value) {};
		EncoderInfo(){};
	} infos[7];
	EncoderParameters *params;

	EncoderParameterGetter() {
		infos[0] = EncoderInfo("compression", EncoderCompression, -1);
		infos[1] = EncoderInfo("scanmethod", EncoderScanMethod, -1);
		infos[2] = EncoderInfo("version", EncoderVersion, -1);
		infos[3] = EncoderInfo("render", EncoderRenderMethod, -1);
		infos[4] = EncoderInfo("tansform", EncoderTransformation, -1);
		infos[5] = EncoderInfo("quality", EncoderQuality, -1);
		infos[6] = EncoderInfo("depth", EncoderColorDepth, 24);
		params = (EncoderParameters*)malloc(sizeof(EncoderParameters) + 6 * sizeof(EncoderParameter));
	};

	~EncoderParameterGetter() {
		delete params;
	}

	void checkResult() {
		int n = 0;
		for (int i=0;i<7;i++) {
			if (infos[i].value >= 0) {
				params->Parameter[n].Guid = infos[i].guid;
				params->Parameter[n].Type = EncoderParameterValueTypeLong;
				params->Parameter[n].NumberOfValues = 1;
				params->Parameter[n].Value = &infos[i].value;
				n++;
			}
		}
		params->Count = n;
	}
	
	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (numparams > 1) {
			tTVInteger flag = param[1]->AsInteger();
			if (!(flag & TJS_HIDDENMEMBER)) {
				ttstr name = *param[0];
				for (int i=0;i<7;i++) {
					if (name == infos[i].name) {
						infos[i].value = (tjs_int)*param[1];
						break;
					}
				}
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}
};

/**
 * �摜�̕ۑ�
 */
tjs_error
LayerExDraw::saveImage(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	// rawcallback ���� hook �������ĂȂ��͗l
	LayerExDraw *self = ncbInstanceAdaptor<LayerExDraw>::GetNativeInstance(objthis);
	if (!self) {
		self = new LayerExDraw(objthis);
		ncbInstanceAdaptor<LayerExDraw>::SetNativeInstance(objthis, self);
	}
	self->reset();
	
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;
	ttstr filename = TVPNormalizeStorageName(param[0]->AsStringNoAddRef());
	TVPGetLocalName(filename);
	ttstr type;
	if (numparams > 1) {
		type = *param[1];
	} else {
		type = L"image/bmp";
	}
	CLSID clsid;
	if (!getEncoder(type.c_str(), &clsid)) {
		TVPThrowExceptionMessage(L"unknown format:%1", type);
	}

	EncoderParameterGetter *caller = new EncoderParameterGetter();
	// �p�����[�^����������
	if (numparams > 2 && param[2]->Type() == tvtObject) {
		tTJSVariantClosure closure(caller);
		param[2]->AsObjectClosureNoAddRef().EnumMembers(TJS_IGNOREPROP, &closure, NULL);
	}
	caller->checkResult();
	Status ret = self->bitmap->Save(filename.c_str(), &clsid, caller->params);
	caller->Release();

	if (result) {
		*result = ret == 0;
	}
	return TJS_S_OK;
}

static ARGB getColor(Bitmap *bitmap, int x, int y)
{
	Color c;
	bitmap->GetPixel(x, y, &c);
	return c.GetValue();
}

tTJSVariant
LayerExDraw::getColorRegionRects(ARGB color)
{
	iTJSDispatch2 *array = TJSCreateArrayObject();
	if (bitmap) {
		int width  = bitmap->GetWidth();
		int height = bitmap->GetHeight();
		
		Region region(Rect(0,0,0,0));
		for (int j=0;j<height;j++) {
			for (int i=0;i<width;i++) {
				if (getColor(bitmap, i, j) == color) {
					int x0 = i++;
					while (i < width && getColor(bitmap, i, j) == color) i++;
					region.Union(Rect(x0, j, i - x0, 1));
				}
			}
		}

		// ��`�ꗗ�擾
		Matrix matrix;
		int count = region.GetRegionScansCount(&matrix);
		if (count > 0) {
			RectF *rects = (RectF*)malloc(count*sizeof(RectF));
			region.GetRegionScans(&matrix, rects, &count);
			for (int i=0;i<count;i++) {
				RectF *rect = &rects[i];
				tTJSVariant x(rect->X);
				tTJSVariant y(rect->Y);
				tTJSVariant w(rect->Width);
				tTJSVariant h(rect->Height);
				tTJSVariant *points[4] = {&x, &y, &w, &h};
				static tjs_uint32 pushHint;
				iTJSDispatch2 *rarray = TJSCreateArrayObject();
				rarray->FuncCall(0, TJS_W("push"), &pushHint, 0, 4, points, rarray);
				tTJSVariant var(rarray,rarray), *param = &var;
				rarray->Release();
				array->FuncCall(0, TJS_W("push"), &pushHint, 0, 1, &param, array);
			}
			delete[] rects;
		}
	}
	tTJSVariant ret(array,array);
	array->Release();
	return ret;
}
