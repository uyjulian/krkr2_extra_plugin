#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;
#include <vector>
using namespace std;
#include <stdio.h>

#include "tp_stub.h"

/**
 * ���O�o�͗p
 */
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

//---------------------------------------------------------------------------

// Array �N���X�����o
static iTJSDispatch2 *ArrayCountProp   = NULL;   // Array.count

// -----------------------------------------------------------------

static void
addMember(iTJSDispatch2 *dispatch, const tjs_char *name, iTJSDispatch2 *member)
{
	tTJSVariant var = tTJSVariant(member);
	member->Release();
	dispatch->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		name, // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
		NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
		&var, // �o�^����l
		dispatch // �R���e�L�X�g
		);
}

static iTJSDispatch2*
getMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	tTJSVariant val;
	if (TJS_FAILED(dispatch->PropGet(TJS_IGNOREPROP,
									 name,
									 NULL,
									 &val,
									 dispatch))) {
		ttstr msg = TJS_W("can't get member:");
		msg += name;
		TVPThrowExceptionMessage(msg.c_str());
	}
	return val.AsObject();
}

static iTJSDispatch2*
getMember(iTJSDispatch2 *dispatch, int num)
{
	tTJSVariant val;
	if (TJS_FAILED(dispatch->PropGetByNum(TJS_IGNOREPROP,
										  num,
										  &val,
										  dispatch))) {
		ttstr msg = TJS_W("can't get array index:");
		msg += num;
		TVPThrowExceptionMessage(msg.c_str());
	}
	return val.AsObject();
}

static void
delMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	dispatch->DeleteMember(
		0, // �t���O ( 0 �ł悢 )
		name, // �����o��
		NULL, // �q���g
		dispatch // �R���e�L�X�g
		);
}

static REAL
getRealValue(iTJSDispatch2 *obj, const tjs_char *name)
{
	tTJSVariant val;
	if (TJS_FAILED(obj->PropGet(0,
								name,
								NULL,
								&val,
								obj))) {
		ttstr msg = TJS_W("can't get member:");
		msg += name;
		TVPThrowExceptionMessage(msg.c_str());
	}
	return val.AsReal();
}

//---------------------------------------------------------------------------

// GDI ��{���
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

#include "LayerExBase.h"

//---------------------------------------------------------------------------

#define TJS_NATIVE_CLASSID_NAME ClassID_Brush
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;

/**
 * GDI+ �u���V�I�u�W�F�N�g
 */
class NI_Brush : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
protected:
	Brush *brush;
	
	/**
	 * �u���V�̏���
	 */
	void clearBrush() {
		delete brush;
		brush = NULL;
	}
	
public:
	/**
	 * �Œ�F�u���V�̒ǉ�
	 * @param argb �F�w��
	 */
	void setSolidBrush(ARGB argb) {
		clearBrush();
		brush = new SolidBrush(Color(argb));
	}

	/**
	 * �s�O���f�[�V�����u���V�̒ǉ�
	 */
	void setLinearGradientBrush(double x1, double y1, double x2, double y2, ARGB col1, ARGB col2) {
		clearBrush();
		brush = new LinearGradientBrush(PointF(x1,y1), PointF(x2, y2), Color(col1), Color(col2));
	}

	/**
	 * �p�X�O���f�[�V�����u���V�̒ǉ�
	 *
	 */
	void setPathGradientBrush(int numparams, tTJSVariant **param) {

		clearBrush();
		
		// �_�̐�
		int n = numparams / 2;
		
		// ���[�h
		WrapMode mode = WrapModeClamp;
		if (n * 2 < numparams) {
			mode = (WrapMode)(int)*param[numparams - 1];
		}
		
		Point *points = new Point[n];
		for (int i=0;i<n;i++) {
			points[i].X = (int)*param[i*2];
			points[i].Y = (int)*param[i*2+1];
		}
		
		brush = new PathGradientBrush(points, n, mode);
		delete[] points;
	}
	
public:
	/**
	 * �R���X�g���N�^
	 */
	NI_Brush() {
		brush = NULL;
	}

	/**
	 * TJS �R���X�g���N�^
	 * @param numparams �p�����[�^��
	 * @param param
	 * @param tjs_obj this �I�u�W�F�N�g
	 */
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
		if (numparams > 0) {
			setSolidBrush(param[0]->AsInteger());
		}
		return S_OK;
	}

	/**
	 * TJS invalidate
	 */
	void TJS_INTF_METHOD Invalidate() {
		clearBrush();
	}
	
	/**
	 * @param objthis �I�u�W�F�N�g
	 * @return GDI+ �p Brush �C���X�^���X�B�擾���s������ NULL
	 */
	static Brush *getBrush(iTJSDispatch2 *objthis) {
		if (!objthis) return NULL;
		NI_Brush *_this;
		if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
														 TJS_NATIVE_CLASSID_NAME, (iTJSNativeInstance**)&_this))) {
			return _this->brush;
		}
		return NULL;
	}

};

static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_Brush()
{
	return new NI_Brush();
}

static iTJSDispatch2 * Create_NC_Brush()
{
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("Brush"), Create_NI_Brush);

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/Brush)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_Brush,
			/*TJS class name*/Brush)
		{
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Brush)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setSolidBrush)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
				/*var. type*/NI_Brush);
			
			if (numparams > 0) {
				_this->setSolidBrush(param[0]->AsInteger());
			}

			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/setSolidBrush)

	TJS_END_NATIVE_MEMBERS

	/*
		���̊֐��� classobj ��Ԃ��܂��B
	*/
	return classobj;
}

#undef TJS_NATIVE_CLASSID_NAME

//---------------------------------------------------------------------------

#define TJS_NATIVE_CLASSID_NAME ClassID_Pen
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;

/**
 * GDI+ �y���I�u�W�F�N�g
 */
class NI_Pen : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
protected:
	Pen *pen;

	/**
	 * �y���̏���
	 */
	void clearPen() {
		delete pen;
		pen = NULL;
	}

	/**
	 * �u���V�y���̒ǉ�
	 * @param brush �u���V
	 * @param width �y����
	 */
	void setBrushPen(const Brush *brush, REAL width=1.0) {
		clearPen();
		pen = new Pen(brush, width);
	}

	/**
	 * �Œ�F�y���̒ǉ�
	 * @param argb �F�w��
	 * @param width �y����
	 */
	void setColorPen(ARGB argb, REAL width=1.0) {
		clearPen();
		pen = new Pen(Color(argb), width);
	}

	
public:
	/**
	 * �R���X�g���N�^
	 */
	NI_Pen() {
		pen = NULL;
	}

	/**
	 * TJS �R���X�g���N�^
	 * @param numparams �p�����[�^��
	 * @param param
	 * @param tjs_obj this �I�u�W�F�N�g
	 */
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {

		// ���̑����̎w�肪����ꍇ
		if (numparams > 1) {
			switch (param[0]->Type()) {
			case tvtObject:
				setBrushPen(NI_Brush::getBrush(param[0]->AsObjectNoAddRef()), param[1]->AsReal());
				break;
			default:
				setColorPen(param[0]->AsInteger(), param[1]->AsReal());
				break;
			}
		} else if (numparams > 0) {
			switch (param[0]->Type()) {
			case tvtObject:
				setBrushPen(NI_Brush::getBrush(param[0]->AsObjectNoAddRef()));
				break;
			default:
				setColorPen(param[0]->AsInteger());
				break;
			}
		} else {
			pen = new Pen(Color(0));
		}
		return S_OK;
	}

	/**
	 * TJS invalidate
	 */
	void TJS_INTF_METHOD Invalidate() {
		clearPen();
	}
	
	/**
	 * @param objthis �I�u�W�F�N�g
	 * @return GDI+ �p Pen�C���X�^���X�B�擾���s������ NULL
	 */
	static Pen *getPen(iTJSDispatch2 *objthis) {
		if (!objthis) return NULL;
		NI_Pen *_this;
		if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
														 TJS_NATIVE_CLASSID_NAME, (iTJSNativeInstance**)&_this))) {
			return _this->pen;
		}
		return NULL;
	}
	
};

static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_Pen()
{
	return new NI_Pen();
}

static iTJSDispatch2 * Create_NC_Pen()
{
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("Pen"), Create_NI_Pen);

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/Pen)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_Pen,
			/*TJS class name*/Pen)
		{
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Pen)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setAlignment) 
		{
			Pen *pen;
			if (!objthis || !(pen = NI_Pen::getPen(objthis))) {
				return TJS_E_NATIVECLASSCRASH;
			}
			if (numparams < 1) {
				return TJS_E_BADPARAMCOUNT;
			}
			pen->SetAlignment((PenAlignment)param[0]->AsInteger());
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/setAlignment)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setBrush) 
		{
			Pen *pen;
			if (!objthis || !(pen = NI_Pen::getPen(objthis))) {
				return TJS_E_NATIVECLASSCRASH;
			}
			if (numparams < 1) {
				return TJS_E_BADPARAMCOUNT;
			}
			pen->SetBrush(NI_Brush::getBrush(param[0]->AsObjectNoAddRef()));
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/setBrush)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setColor) 
		{
			Pen *pen;
			if (!objthis || !(pen = NI_Pen::getPen(objthis))) {
				return TJS_E_NATIVECLASSCRASH;
			}
			if (numparams < 1) {
				return TJS_E_BADPARAMCOUNT;
			}
			pen->SetColor(Color(param[0]->AsInteger()));
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/setColor)
		
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setWidth) 
		{
			Pen *pen;
			if (!objthis || !(pen = NI_Pen::getPen(objthis))) {
				return TJS_E_NATIVECLASSCRASH;
			}
			if (numparams < 1) {
				return TJS_E_BADPARAMCOUNT;
			}
			pen->SetWidth(param[0]->AsReal());
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/setWidth)

	TJS_END_NATIVE_MEMBERS

	/*
		���̊֐��� classobj ��Ԃ��܂��B
	*/
	return classobj;
}

#undef TJS_NATIVE_CLASSID_NAME

//---------------------------------------------------------------------------

#define TJS_NATIVE_CLASSID_NAME ClassID_Font
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;

/**
 * GDI+ �t�H���g�I�u�W�F�N�g
 */
class NI_Font : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
	/// �t�H���g�ێ��p
	Font *font;

	void clearFont() {
		delete font;
		font = NULL;
	}

public:
	/**
	 * �t�H���g�ݒ�
	 * @param familyName �t�H���g�t�@�~���[��
	 * @param emSize �T�C�Y�w��(pixcel�P��)
	 * @param style �t�H���g�X�^�C��
	 */
	void setFont(const tjs_char *familyName, REAL emSize=12, INT style=FontStyleRegular) {
		if (familyName) {
			FontFamily  fontFamily(familyName);
			font = new Font(&fontFamily, emSize, FontStyleRegular, UnitPixel);
		}
	}
	
public:
	/**
	 * �R���X�g���N�^
	 */
	NI_Font() {
		font = NULL;
	}

	/**
	 * TJS �R���X�g���N�^
	 * @param numparams �p�����[�^��
	 * @param param
	 * @param tjs_obj this �I�u�W�F�N�g
	 */
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
		if (numparams > 2) {
			setFont(param[0]->GetString(), param[1]->AsReal(), param[2]->AsInteger());
		} else if (numparams > 1) {
			setFont(param[0]->GetString(), param[1]->AsReal());
		} else if (numparams > 0) {
			setFont(param[0]->GetString());
		}
		return S_OK;
	}

	/**
	 * TJS invalidate
	 */
	void TJS_INTF_METHOD Invalidate() {
		clearFont();
	}
	
	/**
	 * @param objthis �I�u�W�F�N�g
	 * @return GDI+ �p Font �C���X�^���X�B�擾���s������ NULL
	 */
	static Font *getFont(iTJSDispatch2 *objthis) {
		if (!objthis) return NULL;
		NI_Font *_this;
		if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
														 TJS_NATIVE_CLASSID_NAME, (iTJSNativeInstance**)&_this))) {
			return _this->font;
		}
		return NULL;
	}
	
};

static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_Font()
{
	return new NI_Font();
}

static iTJSDispatch2 * Create_NC_Font()
{
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("Font"), Create_NI_Font);

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/Font)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_Font,
			/*TJS class name*/Font)
		{
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Font)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/print) // print ���\�b�h
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
				/*var. type*/NI_Font);

//			_this->Print();

			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/print)

	TJS_END_NATIVE_MEMBERS

	/*
		���̊֐��� classobj ��Ԃ��܂��B
	*/
	return classobj;
}

#undef TJS_NATIVE_CLASSID_NAME

//---------------------------------------------------------------------------

/*
 * ���C���ɒ������� GDI+ ����ێ����邽�߂̃l�C�e�B�u�C���X�^���X
 */
class NI_GdiPlusInfo : public tTJSNativeInstance
{
protected:

	/// ���C�����Q�Ƃ���r�b�g�}�b�v
	Bitmap * _bitmap;
	/// ���C���ɑ΂��ĕ`�悷��R���e�L�X�g
	Graphics * _graphics;

	// ���C������r�ێ��p
	tjs_int _width;
	tjs_int _height;
	tjs_int _pitch;
	unsigned char * _buffer;

public:

	/**
	 * GDI+ �p�`��O���t�B�b�N������������
	 * ���C���̃r�b�g�}�b�v��񂪕ύX����Ă���\��������̂Ŗ���`�F�b�N����B
	 * �ύX����Ă���ꍇ�͕`��p�̃R���e�L�X�g��g�݂Ȃ���
	 * @param layerobj ���C���I�u�W�F�N�g
	 */
	void reset(iTJSDispatch2 *layerobj) {
		
		NI_LayerExBase *base = NI_LayerExBase::getNative(layerobj, false);
		base->reset(layerobj);

		// �ύX����ĂȂ��ꍇ�͂���Ȃ���
		if (!(_graphics &&
			  _width  == base->_width &&
			  _height == base->_height &&
			  _pitch  == base->_pitch &&
			  _buffer == base->_buffer)) {
			delete _graphics;
			delete _bitmap;
			_width  = base->_width;
			_height = base->_height;
			_pitch  = base->_pitch;
			_buffer = base->_buffer;
			_bitmap = new Bitmap(_width, _height, _pitch, PixelFormat32bppARGB, (unsigned char*)_buffer);
			_graphics = new Graphics(_bitmap);

			// Graphics ������

			
			// ���W�n������
			
			// �}�g���b�N�X�K�p
				
			// �X���[�W���O�w��
			_graphics->SetSmoothingMode(SmoothingModeHighQuality);
		}
	}

	void redraw(iTJSDispatch2 *layerobj) {
		NI_LayerExBase *base = NI_LayerExBase::getNative(layerobj, false);
		base->redraw(layerobj);
	}
	

public:
	/**
	 * �R���X�g���N�^
	 */
	NI_GdiPlusInfo(iTJSDispatch2 *layerobj) {
		_bitmap = NULL;
		_graphics = NULL;
		_width = 0;
		_height = 0;
		_pitch = 0;
		_buffer = NULL;
	}

	/**
	 * �f�X�g���N�^
	 */
	~NI_GdiPlusInfo() {
		delete _graphics;
		delete _bitmap;
	}

	// ------------------------------------------------------------------
	// �`�惁�\�b�h�Q
	// ------------------------------------------------------------------
public:

	/**
	 * ��ʂ̏���
	 * @param argb �����F
	 */
	void clear(ARGB argb = 0x00000000) {
		_graphics->Clear(Color(argb));
	}

	/**
	 * �ȉ~�̕`��
	 * @param pen �y��
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 */
	void drawEllipse(Pen *pen, REAL x, REAL y, REAL width, REAL height) {
		if (pen) {
			_graphics->DrawEllipse(pen, x, y, width, height);
		}
	}

	/**
	 * �ȉ~�̓h��Ԃ�
	 * @param brush �u���V
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 */
	void fillEllipse(Brush *brush, REAL x, REAL y, REAL width, REAL height) {
		if (brush) {
			_graphics->FillEllipse(brush, x, y, width, height);
		}
	}
	
	/**
	 * �����̕`��
	 * @param x1 �n�_X���W
	 * @param y1 �n�_Y���W
	 * @param x2 �I�_X���W
	 * @param y2 �I�_Y���W
	 */
	void drawLine(Pen *pen, REAL x1, REAL y1, REAL x2, REAL y2) {
		if (pen) {
			_graphics->DrawLine(pen, x1, y1, x2, y2);
		}
	}

	/**
	 * ��`�̕`��
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 */
	void drawRectangle(Pen *pen, REAL x, REAL y, REAL width, REAL height) {
		if (pen) {
			_graphics->DrawRectangle(pen, x, y, width, height);
		}
	}
	
	/**
	 * ��`�̓h��Ԃ�
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 */
	void fillRectangle(Brush *brush, REAL x, REAL y, REAL width, REAL height) {
		if (brush) {
			_graphics->FillRectangle(brush, x, y, width, height);
		}
	}

	/**
	 * �x�W�F�Ȑ��̕`��
	 * @param width
	 * @param height
	 */
	void drawBezier(Pen *pen, REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL x4, REAL y4) {
		if (pen) {
			_graphics->DrawBezier(pen, x1, y1, x2, y2, x3, y3, x4, y4);
		}
	}

	/**
	 * �x�W�F�Ȑ��̕`��
	 * @param width
	 * @param height
	 */
	void drawBeziers(Pen *pen, iTJSDispatch2 *array) {
		if (pen) {
			// �_�̌��擾
			tjs_int count = 0;
			{
				tTJSVariant result;
				if (TJS_SUCCEEDED(ArrayCountProp->PropGet(0, NULL, NULL, &result, array))) {
					count = result.AsInteger();
				}
			}
			if (count) {
				PointF *points = new PointF[count];
				for (tjs_int i=0;i<count;i++) {
					iTJSDispatch2 *obj = getMember(array, i);
					points[i].X = getRealValue(obj, L"x");
					points[i].Y = getRealValue(obj, L"y");
				}			
				_graphics->DrawBeziers(pen, points, count);
				delete[] points;
			}
		}
	}
	
	
	/**
	 * ������̕`��
	 * @param text �`��e�L�X�g
	 * @param font �t�H���g
	 * @param x ���_X
	 * @param y ���_Y
	 * @param brush �u���V
	 */
	void drawString(const tjs_char *text, Font *font, REAL x, REAL y, Brush *brush) {
		if (text && font && brush) {
			PointF origin(x, y);
			_graphics->DrawString(text, -1, font, origin, brush);
		}
	}
	
	/**
	 * �摜�̕`��
	 * @param name �摜��
	 * @param x �\���ʒuX
	 * @param y �\���ʒuY
	 * @param rect �\�����̈�w��BNULL �̏ꍇ�͑S��
	 */
	void drawImage(const ttstr &name, REAL x=0, REAL y=0, RectF *rect = NULL) {

		// �摜�ǂݍ���
		IStream *in = TVPCreateIStream(name, TJS_BS_READ);
		if(!in) {
			TVPThrowExceptionMessage((ttstr(TJS_W("cannot open : ")) + ttstr(name)).c_str());
		}

		try {
			// �摜����
			Image image(in);
			int ret;
			if ((ret = image.GetLastStatus()) != Ok) {
				TVPThrowExceptionMessage((ttstr(TJS_W("cannot load : ")) + ttstr(name) + ttstr(L" : ") + ttstr(ret)).c_str());
			}
			// �`�揈��
			if (rect) {
				_graphics->DrawImage(&image, x, y, rect->X, rect->Y, rect->Width, rect->Height, UnitPixel);
			} else {
				_graphics->DrawImage(&image, x, y);
			}
		} catch (...) {
			in->Release();
			throw;
		}
		in->Release();
	}
	
public:
	/**
	 * �p�X��`�悷��
	 * @param path �`�悷��p�X
	 */
	void drawPath(Pen *pen, GraphicsPath *path) {
		_graphics->DrawPath(pen, path);
	}

	/**
	 * �p�X�œh��Ԃ�
	 * @param path �`�悷��p�X
	 */
	void fillPath(Brush *brush, GraphicsPath *path) {
		_graphics->FillPath(brush, path);
	}
};

// �N���XID
static tjs_int32 ClassID_GdiPlusInfo = -1;

/**
 * ���C���I�u�W�F�N�g���� GDI+ �p�l�C�e�B�u�C���X�^���X���擾����B
 * �l�C�e�B�u�C���X�^���X�������ĂȂ��ꍇ�͎����I�Ɋ��蓖�Ă�
 * @param objthis ���C���I�u�W�F�N�g
 * @return GDI+ �p�l�C�e�B�u�C���X�^���X�B�擾���s������ NULL
 */
NI_GdiPlusInfo *
getGdiPlusNative(iTJSDispatch2 *layerobj)
{
	if (!layerobj) return NULL;

	NI_GdiPlusInfo *_this;
	if (TJS_FAILED(layerobj->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
												   ClassID_GdiPlusInfo, (iTJSNativeInstance**)&_this))) {
		// ���C���g��������
		if (NI_LayerExBase::getNative(layerobj) == NULL) {
			return NULL;
		}
		_this = new NI_GdiPlusInfo(layerobj);
		if (TJS_FAILED(layerobj->NativeInstanceSupport(TJS_NIS_REGISTER,
													   ClassID_GdiPlusInfo, (iTJSNativeInstance **)&_this))) {
			return NULL;
		}
	}

	_this->reset(layerobj);
	return _this;
}

/**
 * ��`�`��
 */
class tDrawEllipseFunction : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		NI_GdiPlusInfo *_this;
		if ((_this = getGdiPlusNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
		if (numparams < 5) return TJS_E_BADPARAMCOUNT;

		_this->drawEllipse(NI_Pen::getPen(param[0]->AsObjectNoAddRef()),
						   param[1]->AsReal(),   // x
						   param[2]->AsReal(),   // y
						   param[3]->AsReal(),   // width
						   param[4]->AsReal());  // height
		return TJS_S_OK;
	}
};

/**
 * ��`�h��Ԃ�
 */
class tFillEllipseFunction : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		NI_GdiPlusInfo *_this;
		if ((_this = getGdiPlusNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
		if (numparams < 5) return TJS_E_BADPARAMCOUNT;

		_this->fillEllipse(NI_Brush::getBrush(param[0]->AsObjectNoAddRef()),
						   param[1]->AsReal(),   // x
						   param[2]->AsReal(),   // y
						   param[3]->AsReal(),   // width
						   param[4]->AsReal());  // height
		return TJS_S_OK;
	}
};

/**
 * �����`��
 */
class tDrawLineFunction : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		NI_GdiPlusInfo *_this;
		if ((_this = getGdiPlusNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
		if (numparams < 5) return TJS_E_BADPARAMCOUNT;

		_this->drawLine(NI_Pen::getPen(param[0]->AsObjectNoAddRef()),
						param[1]->AsReal(),   // x1
						param[2]->AsReal(),   // y1
						param[3]->AsReal(),   // x2
						param[4]->AsReal());  // y2
		return TJS_S_OK;
	}
};

/**
 * ��`�`��
 */
class tDrawRectangleFunction : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		NI_GdiPlusInfo *_this;
		if ((_this = getGdiPlusNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
		if (numparams < 5) return TJS_E_BADPARAMCOUNT;

		_this->drawRectangle(NI_Pen::getPen(param[0]->AsObjectNoAddRef()),
							 param[1]->AsReal(),   // x
							 param[2]->AsReal(),   // y
							 param[3]->AsReal(),   // width
							 param[4]->AsReal());  // height
		return TJS_S_OK;
	}
};

/**
 * ��`�h��Ԃ�
 */
class tFillRectangleFunction : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		NI_GdiPlusInfo *_this;
		if ((_this = getGdiPlusNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
		if (numparams < 5) return TJS_E_BADPARAMCOUNT;

		_this->fillRectangle(NI_Brush::getBrush(param[0]->AsObjectNoAddRef()),
							 param[1]->AsReal(),   // x
							 param[2]->AsReal(),   // y
							 param[3]->AsReal(),   // width
							 param[4]->AsReal());  // height
		return TJS_S_OK;
	}
};

/**
 * �x�W�F�`��
 */
class tDrawBezierFunction : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		NI_GdiPlusInfo *_this;
		if ((_this = getGdiPlusNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
		if (numparams < 9) return TJS_E_BADPARAMCOUNT;

		_this->drawBezier(NI_Pen::getPen(param[0]->AsObjectNoAddRef()),
						  param[1]->AsReal(),
						  param[2]->AsReal(),
						  param[3]->AsReal(),
						  param[4]->AsReal(),
						  param[5]->AsReal(),
						  param[6]->AsReal(),
						  param[7]->AsReal(),
						  param[8]->AsReal()
						  );  // height
		return TJS_S_OK;
	}
};

/**
 * �x�W�F�`��
 */
class tDrawBeziersFunction : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		NI_GdiPlusInfo *_this;
		if ((_this = getGdiPlusNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		_this->drawBeziers(NI_Pen::getPen(param[0]->AsObjectNoAddRef()),
						   param[1]->AsObjectNoAddRef()
						   );
		return TJS_S_OK;
	}
};


/**
 * �e�L�X�g�`��
 */
class tDrawStringFunction : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		NI_GdiPlusInfo *_this;
		if ((_this = getGdiPlusNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
		if (numparams < 5) return TJS_E_BADPARAMCOUNT;
		
		_this->drawString(param[0]->GetString(),
						  NI_Font::getFont(param[1]->AsObjectNoAddRef()),
						  param[2]->AsReal(),
						  param[3]->AsReal(),
						  NI_Brush::getBrush(param[4]->AsObjectNoAddRef()));
		return TJS_S_OK;
	}
};

/**
 * ��`�`��
 */
class tDrawImageFunction : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		NI_GdiPlusInfo *_this;
		if ((_this = getGdiPlusNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;

		if (numparams < 3) {
			_this->drawImage(*param[0]);
		} else if (numparams < 7) {
			_this->drawImage(*param[0], param[1]->AsReal(), param[2]->AsReal());
		} else {
			RectF rect(param[3]->AsReal(), param[4]->AsReal(), param[5]->AsReal(), param[6]->AsReal());
			_this->drawImage(*param[0], param[1]->AsReal(), param[2]->AsReal(), &rect);
		}
		
		return TJS_S_OK;
	}
};

//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}

//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// �X�^�u�̏�����(�K���L�q����)
	TVPInitImportStub(exporter);

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// �N���X�I�u�W�F�N�g�`�F�b�N
	if ((NI_LayerExBase::classId = TJSFindNativeClassID(L"LayerExBase")) <= 0) {
		NI_LayerExBase::classId = TJSRegisterNativeClass(L"LayerExBase");
	}
	
	// �N���X�I�u�W�F�N�g�o�^
	ClassID_GdiPlusInfo = TJSRegisterNativeClass(TJS_W("GdiPlusInfo"));

	// TJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	
	if (global) {

		// Arary �N���X�����o�[�擾
		{
			tTJSVariant varScripts;
			TVPExecuteExpression(TJS_W("Array"), &varScripts);
			iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
			// �����o�擾
			ArrayCountProp = getMember(dispatch, TJS_W("count"));
		}

		// Layer �N���X�I�u�W�F�N�g���擾
		{
			tTJSVariant varScripts;
			TVPExecuteExpression(TJS_W("Layer"), &varScripts);
			iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
			if (dispatch) {
				// �v���p�e�B������
				NI_LayerExBase::init(dispatch);
				
				// �ŗL�g���N���X�ǉ�
				addMember(dispatch, L"Pen",    Create_NC_Pen());
				addMember(dispatch, L"Brush",  Create_NC_Brush());
				addMember(dispatch, L"Font",   Create_NC_Font());
				
				// �����o�ǉ�
				addMember(dispatch, L"drawEllipse",   new tDrawEllipseFunction());
				addMember(dispatch, L"fillEllipse",   new tFillEllipseFunction());
				addMember(dispatch, L"drawLine",      new tDrawLineFunction());
				addMember(dispatch, L"drawRectangle", new tDrawRectangleFunction());
				addMember(dispatch, L"fillRectangle", new tFillRectangleFunction());
				
				addMember(dispatch, L"drawBezier", new tDrawBezierFunction());
				addMember(dispatch, L"drawBeziers", new tDrawBeziersFunction());

				addMember(dispatch, L"drawString",    new tDrawStringFunction());
				addMember(dispatch, L"drawImage",     new tDrawImageFunction());
			}
			
		}
		
		global->Release();
	}
			
	// ���̎��_�ł� TVPPluginGlobalRefCount �̒l��
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	// �Ƃ��čT���Ă����BTVPPluginGlobalRefCount �͂��̃v���O�C������
	// �Ǘ�����Ă��� tTJSDispatch �h���I�u�W�F�N�g�̎Q�ƃJ�E���^�̑��v�ŁA
	// ������ɂ͂���Ɠ������A����������Ȃ��Ȃ��ĂȂ��ƂȂ�Ȃ��B
	// �����Ȃ��ĂȂ���΁A�ǂ����ʂ̂Ƃ���Ŋ֐��Ȃǂ��Q�Ƃ���Ă��āA
	// �v���O�C���͉���ł��Ȃ��ƌ������ƂɂȂ�B

	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall  V2Unlink()
{
	// �g���g��������A�v���O�C����������悤�Ƃ���Ƃ��ɌĂ΂��֐��B

	// �������炩�̏����Ńv���O�C��������ł��Ȃ��ꍇ��
	// ���̎��_�� E_FAIL ��Ԃ��悤�ɂ���B
	// �����ł́ATVPPluginGlobalRefCount �� GlobalRefCountAtInit ����
	// �傫���Ȃ��Ă���Ύ��s�Ƃ������Ƃɂ���B
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL ���A��ƁAPlugins.unlink ���\�b�h�͋U��Ԃ�

	// �v���p�e�B�J��
	NI_LayerExBase::unInit();

	// - �܂��ATJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global �� DeleteMember ���\�b�h��p���A�I�u�W�F�N�g���폜����
	if (global)	{

		{
			// Layer �N���X�I�u�W�F�N�g���擾
			tTJSVariant varScripts;
			TVPExecuteExpression(TJS_W("Layer"), &varScripts);
			iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
			if (dispatch) {
				delMember(dispatch, L"Pen");
				delMember(dispatch, L"Brush");
				delMember(dispatch, L"Font");
				
				delMember(dispatch, L"drawEllipse");
				delMember(dispatch, L"fillEllipse");
				delMember(dispatch, L"drawString");
				delMember(dispatch, L"drawLine");
				delMember(dispatch, L"drawRectangle");
				delMember(dispatch, L"fillRectangle");
				delMember(dispatch, L"drawImage");
			}
		}

		if (ArrayCountProp) {
			ArrayCountProp->Release();
			ArrayCountProp = NULL;
		}
		
		global->Release();
	}

	GdiplusShutdown(gdiplusToken);
	
	// �X�^�u�̎g�p�I��(�K���L�q����)
	TVPUninitImportStub();

	return S_OK;
}
