#ifndef _layerExText_hpp_
#define _layerExText_hpp_

#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

#include <vector>
using namespace std;

#include "layerExBase.hpp"

/**
 * GDIPlus �ŗL�����p
 */
struct GdiPlus {
	/**
	 * �v���C�x�[�g�t�H���g�̒ǉ�
	 * @param fontFileName �t�H���g�t�@�C����
	 */
	static void addPrivateFont(const tjs_char *fontFileName);

	/**
	 * �t�H���g�t�@�~���[�����擾
	 * @param privateOnly true �Ȃ�v���C�x�[�g�t�H���g�̂ݎ擾
	 */
	static tTJSVariant getFontList(bool privateOnly);
};

/**
 * �t�H���g���
 */
class FontInfo {
	friend class LayerExDraw;
	friend class Path;

protected:
	FontFamily *fontFamily; //< �t�H���g�t�@�~���[
	ttstr familyName;
	REAL emSize; //< �t�H���g�T�C�Y 
	INT style; //< �t�H���g�X�^�C��
        bool gdiPlusUnsupportedFont; //< GDI+���T�|�[�g�t�H���g
        bool forceSelfPathDraw; // ���O�p�X�`�拭��
        mutable bool propertyModified;
        mutable REAL ascent;
        mutable REAL descent;
        mutable REAL lineSpacing;
        mutable REAL ascentLeading;
        mutable REAL descentLeading;

	/**
	 * �t�H���g���̃N���A
	 */
	void clear();

        OUTLINETEXTMETRIC *createFontMetric(void) const;


public:

	FontInfo();
	/**
	 * �R���X�g���N�^
	 * @param familyName �t�H���g�t�@�~���[
	 * @param emSize �t�H���g�̃T�C�Y
	 * @param style �t�H���g�X�^�C��
	 */
	FontInfo(const tjs_char *familyName, REAL emSize, INT style);
	FontInfo(const FontInfo &orig);

	/**
	 * �f�X�g���N�^
	 */
	virtual ~FontInfo();

	void setFamilyName(const tjs_char *familyName);
	const tjs_char *getFamilyName() { return familyName.c_str(); }
	void setEmSize(REAL emSize) { this->emSize = emSize; propertyModified = true; }
	REAL getEmSize() {  return emSize; }
	void setStyle(INT style) { this->style = style; propertyModified = true; }
	INT getStyle() { return style; }
        void setForceSelfPathDraw(bool state);
        bool getForceSelfPathDraw(void) const;
        bool getSelfPathDraw(void) const;

        void updateSizeParams(void) const;
	REAL getAscent() const;
	REAL getDescent() const;
	REAL getAscentLeading() const;
	REAL getDescentLeading() const;
	REAL getLineSpacing() const;
};

/**
 * �`��O�Ϗ��
 */
class Appearance {
	friend class LayerExDraw;
public:
	// �`����
	struct DrawInfo{
		int type;   // 0:�u���V 1:�y��
		void *info; // ���I�u�W�F�N�g
		REAL ox; //< �\���I�t�Z�b�g
		REAL oy; //< �\���I�t�Z�b�g
		DrawInfo() : ox(0), oy(0), type(0), info(NULL) {}
		DrawInfo(REAL ox, REAL oy, Pen *pen) : ox(ox), oy(oy), type(0), info(pen) {}
		DrawInfo(REAL ox, REAL oy, Brush *brush) : ox(ox), oy(oy), type(1), info(brush) {}
		DrawInfo(const DrawInfo &orig) {
			ox = orig.ox;
			oy = orig.oy;
			type = orig.type;
			if (orig.info) {
				switch (type) {
				case 0:
					info = (void*)((Pen*)orig.info)->Clone();
					break;
				case 1:
					info = (void*)((Brush*)orig.info)->Clone();
					break;
				}
			} else {
				info = NULL;
			}
		}
		virtual ~DrawInfo() {
			if (info) {
				switch (type) {
				case 0:
					delete (Pen*)info;
					break;
				case 1:
					delete (Brush*)info;
					break;
				}
			}
		}	
	};
	vector<DrawInfo>drawInfos;

public:
	Appearance();
	virtual ~Appearance();

	/**
	 * ���̃N���A
	 */
	void clear();
	
	/**
	 * �u���V�̒ǉ�
	 * @param colorOrBrush ARGB�F�w��܂��̓u���V���i�����j
	 * @param ox �\���I�t�Z�b�gX
	 * @param oy �\���I�t�Z�b�gY
	 */
	void addBrush(tTJSVariant colorOrBrush, REAL ox=0, REAL oy=0);
	
	/**
	 * �y���̒ǉ�
	 * @param colorOrBrush ARGB�F�w��܂��̓u���V���i�����j
	 * @param widthOrOption �y�����܂��̓y�����i�����j
	 * @param ox �\���I�t�Z�b�gX
	 * @param oy �\���I�t�Z�b�gY
	 */
	void addPen(tTJSVariant colorOrBrush, tTJSVariant widthOrOption, REAL ox=0, REAL oy=0);

protected:
	/**
	 * LineCap�̎擾
	 */
	bool getLineCap(tTJSVariant &in, LineCap &cap, CustomLineCap* &custom, REAL pw);
	vector<CustomLineCap*>customLineCaps;
};


/**
 * �`��O�Ϗ��
 */
class Path {
	friend class LayerExDraw;
public:
	Path();
	virtual ~Path();
	void startFigure();
	void closeFigure();
	void drawArc(REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
	void drawBezier(REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL x4, REAL y4);
	void drawBeziers(tTJSVariant points);
	void drawClosedCurve(tTJSVariant points);
	void drawClosedCurve2(tTJSVariant points, REAL tension);
	void drawCurve(tTJSVariant points);
	void drawCurve2(tTJSVariant points, REAL tension);
	void drawCurve3(tTJSVariant points, int offset, int numberOfSegments, REAL tension);
	void drawPie(REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
	void drawEllipse(REAL x, REAL y, REAL width, REAL height);
	void drawLine(REAL x1, REAL y1, REAL x2, REAL y2);
	void drawLines(tTJSVariant points);
	void drawPolygon(tTJSVariant points);
	void drawRectangle(REAL x, REAL y, REAL width, REAL height);
	void drawRectangles(tTJSVariant rects);
	void drawPath(const Path *path, bool connect);
protected:
	GraphicsPath path;
};

/*
 * �A�E�g���C���x�[�X�̃e�L�X�g�`�惁�\�b�h�̒ǉ�
 */
class LayerExDraw : public layerExBase
{
protected:
	// ���ێ��p
	GeometryT width, height;
	BufferT   buffer;
	PitchT    pitch;
	GeometryT clipLeft, clipTop, clipWidth, clipHeight;
	
	/// ���C�����Q�Ƃ���r�b�g�}�b�v
	Bitmap *bitmap;
	/// ���C���ɑ΂��ĕ`�悷��R���e�L�X�g
	Graphics *graphics;

	// Transform �w��
	Matrix transform;
	Matrix viewTransform;
	Matrix calcTransform;

protected:
	// �`��X���[�W���O�w��
	SmoothingMode smoothingMode;
	// drawString �̃A���`�G�C���A�X�w��
	TextRenderingHint textRenderingHint;

public:
	int getSmoothingMode() {
		return (int)smoothingMode;
	}
	void setSmoothingMode(int mode) {
		smoothingMode = (SmoothingMode)mode;
	}

	int getTextRenderingHint() {
		return (int)textRenderingHint;
	}
	void setTextRenderingHint(int hint) {
		textRenderingHint = (TextRenderingHint)hint;
	}

protected:
	/// �`����e�L�^�p���^�t�@�C��
	HDC metaHDC;
	HGLOBAL metaBuffer;
	IStream *metaStream;
	Metafile *metafile;
	Graphics *metaGraphics;

	bool updateWhenDraw;
	void updateRect(RectF &rect);
	
public:
	void setUpdateWhenDraw(int updateWhenDraw) {
		this->updateWhenDraw = updateWhenDraw != 0;
	}
	int getUpdateWhenDraw() { return updateWhenDraw ? 1 : 0; }

	inline operator Image*() const { return (Image*)bitmap; }
	inline operator Bitmap*() const { return bitmap; }
	inline operator Graphics*() const { return graphics; }
	inline operator const Image*() const { return (const Image*)bitmap; }
	inline operator const Bitmap*() const { return bitmap; }
	inline operator const Graphics*() const { return graphics; }
	
	template <class T>
	struct BridgeFunctor {
		T* operator()(LayerExDraw *p) const {
			return (T*)*p;
		}
	};

public:	
	LayerExDraw(DispatchT obj);
	~LayerExDraw();
	virtual void reset();

	// ------------------------------------------------------------------
	// �`��p�����[�^�w��
	// ------------------------------------------------------------------

protected:
	void updateViewTransform();
	void updateTransform();
	
public:
	/**
	 * �\���g�����X�t�H�[���̎w��
	 */
	void setViewTransform(const Matrix *transform);
	void resetViewTransform();
	void rotateViewTransform(REAL angle);
	void scaleViewTransform(REAL sx, REAL sy);
	void translateViewTransform(REAL dx, REAL dy);
	
	/**
	 * �g�����X�t�H�[���̎w��
	 * @param matrix �g�����X�t�H�[���}�g���b�N�X
	 */
	void setTransform(const Matrix *transform);
	void resetTransform();
	void rotateTransform(REAL angle);
	void scaleTransform(REAL sx, REAL sy);
	void translateTransform(REAL dx, REAL dy);

	// ------------------------------------------------------------------
	// �`�惁�\�b�h�Q
	// ------------------------------------------------------------------

protected:

	/**
	 * �p�X�̍X�V�̈�����擾
	 * @param app �\���\��
	 * @param path �`�悷��p�X
	 * @return �X�V�̈���
	 */
	RectF getPathExtents(const Appearance *app, const GraphicsPath *path);

	/**
	 * �p�X�̕`��p����������
	 * @param graphics �`���
	 * @param pen �`��p�y��
	 * @param matrix �`��ʒu�����pmatrix
	 * @param path �`����e
	 */
	void draw(Graphics *graphics, const Pen *pen, const Matrix *matrix, const GraphicsPath *path);

	/**
	 * �h��̕`��p����������
	 * @param graphics �`���
	 * @param brush �`��p�u���V
	 * @param matrix �`��ʒu�����pmatrix
	 * @param path �`����e
	 */
	void fill(Graphics *graphics, const Brush *brush, const Matrix *matrix, const GraphicsPath *path);
	
	/**
	 * �p�X�̕`��
	 * @param app �A�s�A�����X
	 * @param path �`�悷��p�X
	 * @return �X�V�̈���
	 */
	RectF _drawPath(const Appearance *app, const GraphicsPath *path);

        /**
         * �O���t�A�E�g���C���̎擾
         * @param font �t�H���g
         * @param offset �I�t�Z�b�g
         * @param path �O���t�������o���p�X
         * @param glyph �`�悷��O���t
         */
        void getGlyphOutline(const FontInfo *font, PointF &offset, GraphicsPath *path, UINT glyph);

        /*
         * �e�L�X�g�A�E�g���C���̎擾
         * @param font �t�H���g
         * @param offset �I�t�Z�b�g
         * @param path �O���t�������o���p�X
         * @param text �`�悷��e�L�X�g
         */
         void getTextOutline(const FontInfo *font, PointF &offset, GraphicsPath *path, ttstr text);

public:
	/**
	 * ��ʂ̏���
	 * @param argb �����F
	 */
	void clear(ARGB argb);

	/**
	 * �p�X�̕`��
	 * @param app �A�s�A�����X
	 * @param path �p�X
	 */
	RectF drawPath(const Appearance *app, const Path *path);
	
	/**
	 * �~�ʂ̕`��
	 * @param app �A�s�A�����X
	 * @param x ������W
	 * @param y ������W
	 * @param width ����
	 * @param height �c��
	 * @param startAngle ���v�����~�ʊJ�n�ʒu
	 * @param sweepAngle �`��p�x
	 * @return �X�V�̈���
	 */
	RectF drawArc(const Appearance *app, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);

	/**
	 * �~���̕`��
	 * @param app �A�s�A�����X
	 * @param x ������W
	 * @param y ������W
	 * @param width ����
	 * @param height �c��
	 * @param startAngle ���v�����~�ʊJ�n�ʒu
	 * @param sweepAngle �`��p�x
	 * @return �X�V�̈���
	 */
	RectF drawPie(const Appearance *app, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
	
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
	RectF drawBezier(const Appearance *app, REAL x1, REAL y1, REAL x2, REAL y2, REAL x3, REAL y3, REAL x4, REAL y4);

	/**
	 * �A���x�W�F�Ȑ��̕`��
	 * @param app �A�s�A�����X
	 * @param points �_�̔z��
	 * @return �X�V�̈���
	 */
	RectF drawBeziers(const Appearance *app, tTJSVariant points);

	/**
	 * Closed cardinal spline �̕`��
	 * @param app �A�s�A�����X
	 * @param points �_�̔z��
	 * @return �X�V�̈���
	 */
	RectF drawClosedCurve(const Appearance *app, tTJSVariant points);

	/**
	 * Closed cardinal spline �̕`��
	 * @param app �A�s�A�����X
	 * @param points �_�̔z��
	 * @pram tension tension
	 * @return �X�V�̈���
	 */
	RectF drawClosedCurve2(const Appearance *app, tTJSVariant points, REAL tension);

	/**
	 * cardinal spline �̕`��
	 * @param app �A�s�A�����X
	 * @param points �_�̔z��
	 * @return �X�V�̈���
	 */
	RectF drawCurve(const Appearance *app, tTJSVariant points);

	/**
	 * cardinal spline �̕`��
	 * @param app �A�s�A�����X
	 * @param points �_�̔z��
	 * @parma tension tension
	 * @return �X�V�̈���
	 */
	RectF drawCurve2(const Appearance *app, tTJSVariant points, REAL tension);

	/**
	 * cardinal spline �̕`��
	 * @param app �A�s�A�����X
	 * @param points �_�̔z��
	 * @param offset
	 * @param numberOfSegment
	 * @param tension tension
	 * @return �X�V�̈���
	 */
	RectF drawCurve3(const Appearance *app, tTJSVariant points, int offset, int numberOfSegments, REAL tension);
	
	/**
	 * �ȉ~�̕`��
	 * @param app �A�s�A�����X
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 * @return �X�V�̈���
	 */
	RectF drawEllipse(const Appearance *app, REAL x, REAL y, REAL width, REAL height);

	/**
	 * �����̕`��
	 * @param app �A�s�A�����X
	 * @param x1 �n�_X���W
	 * @param y1 �n�_Y���W
	 * @param x2 �I�_X���W
	 * @param y2 �I�_Y���W
	 * @return �X�V�̈���
	 */
	RectF drawLine(const Appearance *app, REAL x1, REAL y1, REAL x2, REAL y2);

	/**
	 * �A�������̕`��
	 * @param app �A�s�A�����X
	 * @param points �_�̔z��
	 * @return �X�V�̈���
	 */
	RectF drawLines(const Appearance *app, tTJSVariant points);

	/**
	 * ���p�`�̕`��
	 * @param app �A�s�A�����X
	 * @param points �_�̔z��
	 * @return �X�V�̈���
	 */
	RectF drawPolygon(const Appearance *app, tTJSVariant points);
	
	/**
	 * ��`�̕`��
	 * @param app �A�s�A�����X
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 * @return �X�V�̈���
	 */
	RectF drawRectangle(const Appearance *app, REAL x, REAL y, REAL width, REAL height);

	/**
	 * ������`�̕`��
	 * @param app �A�s�A�����X
	 * @param rects ��`���̔z��
	 * @return �X�V�̈���
	 */
	RectF drawRectangles(const Appearance *app, tTJSVariant rects);

	/**
	 * ������̕`��
	 * @param font �t�H���g
	 * @param app �A�s�A�����X
	 * @param x �`��ʒuX
	 * @param y �`��ʒuY
	 * @param text �`��e�L�X�g
	 * @return �X�V�̈���
	 */
	RectF drawPathString(const FontInfo *font, const Appearance *app, REAL x, REAL y, const tjs_char *text);

	/**
	 * ������̕`��(OpenType��PostScript�t�H���g�Ή�)
	 * @param font �t�H���g
	 * @param app �A�s�A�����X
	 * @param x �`��ʒuX
	 * @param y �`��ʒuY
	 * @param text �`��e�L�X�g
	 * @return �X�V�̈���
	 */
	RectF drawPathString2(const FontInfo *font, const Appearance *app, REAL x, REAL y, const tjs_char *text);

	// -------------------------------------------------------------------------------
	
	/**
	 * ������̕`��
	 * @param font �t�H���g
	 * @param app �A�s�A�����X
	 * @param x �`��ʒuX
	 * @param y �`��ʒuY
	 * @param text �`��e�L�X�g
	 * @return �X�V�̈���
	 */
	RectF drawString(const FontInfo *font, const Appearance *app, REAL x, REAL y, const tjs_char *text);

	/**
	 * ������̕`��X�V�̈���̎擾
	 * @param font �t�H���g
	 * @param text �`��e�L�X�g
	 * @return �X�V�̈���̎��� left, top, width, height
	 */
	RectF measureString(const FontInfo *font, const tjs_char *text);

	/**
	 * ������ɂ҂�����Ɛڂ������`�̎擾
	 * @param font �t�H���g
	 * @param text �`��e�L�X�g
	 * @return �̈���̎��� left, top, width, height
	 */
	RectF measureStringInternal(const FontInfo *font, const tjs_char *text);

	/**
	 * ������̕`��X�V�̈���̎擾(OpenType��PostScript�t�H���g�Ή�)
	 * @param font �t�H���g
	 * @param text �`��e�L�X�g
	 * @return �X�V�̈���̎��� left, top, width, height
	 */
	RectF measureString2(const FontInfo *font, const tjs_char *text);

	/**
	 * ������ɂ҂�����Ɛڂ������`�̎擾(OpenType��PostScript�t�H���g�Ή�)
	 * @param font �t�H���g
	 * @param text �`��e�L�X�g
	 * @return �̈���̎��� left, top, width, height
	 */
	RectF measureStringInternal2(const FontInfo *font, const tjs_char *text);

	// -----------------------------------------------------------------------------
	
	/**
	 * �摜�̕`��B�R�s�[��͌��摜�� Bounds ��z�������ʒu�A�T�C�Y�� Pixel �w��ɂȂ�܂��B
	 * @param x �R�s�[�挴�_X
	 * @param y �R�s�[�挴�_Y
	 * @param image �R�s�[���摜
	 * @return �X�V�̈���
	 */
	RectF drawImage(REAL x, REAL y, Image *src);

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
	RectF drawImageRect(REAL dleft, REAL dtop, Image *src, REAL sleft, REAL stop, REAL swidth, REAL sheight);

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
	RectF drawImageStretch(REAL dleft, REAL dtop, REAL dwidth, REAL dheight, Image *src, REAL sleft, REAL stop, REAL swidth, REAL sheight);

	/**
	 * �摜�̃A�t�B���ϊ��R�s�[
	 * @param src �R�s�[���摜
	 * @param sleft ����`�̍��[
	 * @param stop  ����`�̏�[
	 * @param swidth ����`�̉���
	 * @param sheight  ����`�̏c��
	 * @param affine �A�t�B���p�����[�^�̎��(true:�ϊ��s��, false:���W�w��), 
	 * @return �X�V�̈���
	 */
	RectF drawImageAffine(Image *src, REAL sleft, REAL stop, REAL swidth, REAL sheight, bool affine, REAL A, REAL B, REAL C, REAL D, REAL E, REAL F);

	// ------------------------------------------------
	// ���^�t�@�C������
	// ------------------------------------------------

protected:

	/**
	 * �L�^���̐���
	 */
	void createRecord();

	/**
	 * �L�^���̐���
	 */
	void recreateRecord();
	
	/**
	 * �L�^���̔j��
	 */
	void destroyRecord();

	/**
	 * �ĕ`��p
	 */
	bool redraw(Image *image);
	
public:
	/**
	 * @param record �`����e���L�^���邩�ǂ���
	 */
	void setRecord(bool record);

	/**
	 * @return record �`����e���L�^���邩�ǂ���
	 */
	bool getRecord() {
		return metafile != NULL;
	}

	/**
	 * �L�^���e�� Image �Ƃ��Ď擾
	 * @return ���������� true
	 */
	Image *getRecordImage();
	
	/**
	 * �L�^���e�̍ĕ`��
	 * @return �ĕ`�悵���� true
	 */
	bool redrawRecord();

	/**
	 * �L�^���e�̕ۑ�
	 * @param filename �ۑ��t�@�C����
	 * @return ���������� true
	 */
	bool saveRecord(const tjs_char *filename);

	/**
	 * �L�^���e�̓ǂݍ���
	 * @param filename �ǂݍ��݃t�@�C����
	 * @return ���������� true
	 */
	bool loadRecord(const tjs_char *filename);

	/**
	 * �摜�̕ۑ�
	 */
	static tjs_error TJS_INTF_METHOD saveImage(tTJSVariant *result,
											   tjs_int numparams,
											   tTJSVariant **param,
											   iTJSDispatch2 *objthis);

	// ------------------------------------------------
	// ���[�W�����擾
	// ------------------------------------------------
	
	tTJSVariant getColorRegionRects(ARGB color);
};

#endif
