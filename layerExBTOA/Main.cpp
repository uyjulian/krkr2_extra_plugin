#include "ncbind/ncbind.hpp"
#include <vector>
using namespace std;

// ���C���N���X���Q��
iTJSDispatch2 *getLayerClass(void)
{
	tTJSVariant var;
	TVPExecuteExpression(TJS_W("Layer"), &var);
	return  var.AsObjectNoAddRef();
}

//----------------------------------------------
// ���C���C���[�W���샆�[�e�B���e�B

// �o�b�t�@�Q�Ɨp�̌^
typedef unsigned char       *WrtRefT;
typedef unsigned char const *ReadRefT;

static tjs_uint32 hasImageHint, imageWidthHint, imageHeightHint;
static tjs_uint32 mainImageBufferHint, mainImageBufferPitchHint, mainImageBufferForWriteHint;
static tjs_uint32 provinceImageBufferHint, provinceImageBufferPitchHint, provinceImageBufferForWriteHint;
static tjs_uint32 clipLeftHint, clipTopHint, clipWidthHint, clipHeightHint;
static tjs_uint32 updateHint;

/**
 * ���C���̃T�C�Y�ƃo�b�t�@���擾����
 */
static bool
GetLayerSize(iTJSDispatch2 *lay, long &w, long &h, long &pitch)
{
	iTJSDispatch2 *layerClass = getLayerClass();

	// ���C���C���X�^���X�ȊO�ł̓G���[
	if (!lay || TJS_FAILED(lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay))) return false;

	// ���C���C���[�W�͍݂邩�H
	tTJSVariant val;
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("hasImage"), &hasImageHint, &val, lay)) || (val.AsInteger() == 0)) return false;

	// ���C���T�C�Y���擾
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("imageWidth"), &imageWidthHint, &val, lay))) return false;
	w = (long)val.AsInteger();

	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("imageHeight"), &imageHeightHint, &val, lay))) return false;
	h = (long)val.AsInteger();

	// �s�b�`�擾
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("mainImageBufferPitch"), &mainImageBufferPitchHint, &val, lay))) return false;
	pitch = (long)val.AsInteger();

	// ����Ȓl���ǂ���
	return (w > 0 && h > 0 && pitch != 0);
}

// �������ݗp
static bool
GetLayerBufferAndSize(iTJSDispatch2 *lay, long &w, long &h, WrtRefT &ptr, long &pitch)
{
	iTJSDispatch2 *layerClass = getLayerClass();
	
	if (!GetLayerSize(lay, w, h, pitch)) return false;

	// �o�b�t�@�擾
	tTJSVariant val;
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("mainImageBufferForWrite"), &mainImageBufferForWriteHint, &val, lay))) return false;
	ptr = reinterpret_cast<WrtRefT>(val.AsInteger());
	return  (ptr != 0);
}

/**
 * �N���b�v�̈�̃T�C�Y�ƃo�b�t�@���擾����
 */
static bool
GetClipSize(iTJSDispatch2 *lay, long &l, long &t, long &w, long &h, long &pitch)
{
	iTJSDispatch2 *layerClass = getLayerClass();

	// ���C���C���X�^���X�ȊO�ł̓G���[
	if (!lay || TJS_FAILED(lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay))) return false;

	// ���C���C���[�W�͍݂邩�H
	tTJSVariant val;
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("hasImage"), &hasImageHint, &val, lay)) || (val.AsInteger() == 0)) return false;

	// �N���b�v�T�C�Y���擾
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("clipLeft"), &clipLeftHint, &val, lay))) return false;
	l = (long)val.AsInteger();
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("clipTop"),  &clipTopHint, &val, lay))) return false;
	t = (long)val.AsInteger();
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("clipWidth"), &clipWidthHint, &val, lay))) return false;
	w = (long)val.AsInteger();
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("clipHeight"), &clipHeightHint, &val, lay))) return false;
	h = (long)val.AsInteger();

	// �s�b�`�擾
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("mainImageBufferPitch"), &mainImageBufferPitchHint, &val, lay))) return false;
	pitch = (long)val.AsInteger();

	// ����Ȓl���ǂ���
	return (w > 0 && h > 0 && pitch != 0);
}

// �������ݗp
static bool
GetClipBufferAndSize(iTJSDispatch2 *lay, long &l, long &t, long &w, long &h, WrtRefT &ptr, long &pitch)
{
	iTJSDispatch2 *layerClass = getLayerClass();

	if (!GetClipSize(lay, l, t, w, h, pitch)) return false;
	
	// �o�b�t�@�擾
	tTJSVariant val;
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("mainImageBufferForWrite"), &mainImageBufferForWriteHint, &val, lay))) return false;
	ptr = reinterpret_cast<WrtRefT>(val.AsInteger());
	if (ptr != 0) {
		ptr += pitch * t + l * 4;
		return true;
	}
	return false;
}


/**
 * Layer.copyRightBlueToLeftAlpha
 * ���C���E������ Blue CHANNEL ���������� Alpha CHANNEL �ɕ�������
 */
static tjs_error TJS_INTF_METHOD
copyRightBlueToLeftAlpha(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	// �������ݐ�
	WrtRefT dbuf = 0;
	long dw, dh, dpitch;
	if (!GetLayerBufferAndSize(lay, dw, dh, dbuf, dpitch)) {
		TVPThrowExceptionMessage(TJS_W("dest must be Layer."));
	}

	// ����
	dw /= 2;
	// �R�s�[

	WrtRefT sbuf = dbuf + dw*4;
	dbuf += 3;
	for (int i=0;i<dh;i++) {
		WrtRefT p = sbuf;   // B�̈�
		WrtRefT q = dbuf;   // A�̈�
		for (int j=0;j<dw;j++) {
			*q = *p;
			p += 4;
			q += 4;
		}
		sbuf += dpitch;
		dbuf += dpitch;
	}
	ncbPropAccessor layObj(lay);
	layObj.FuncCall(0, L"update", &updateHint, NULL, 0, 0, dw, dh);
	return TJS_S_OK;
}

/**
 * Layer.copyBottomBlueToTopAlpha
 * ���C���E������ Blue CHANNEL ���������� Alpha CHANNEL�ɕ�������
 */
static tjs_error TJS_INTF_METHOD
copyBottomBlueToTopAlpha(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	// �������ݐ�
	WrtRefT dbuf = 0;
	long dw, dh, dpitch;
	if (!GetLayerBufferAndSize(lay, dw, dh, dbuf, dpitch)) {
		TVPThrowExceptionMessage(TJS_W("dest must be Layer."));
	}

	// ����
	dh /= 2;

	// �R�s�[
	WrtRefT sbuf = dbuf + dh * dpitch;
	dbuf += 3;
	for (int i=0;i<dh;i++) {
		WrtRefT p = sbuf;   // B�̈�
		WrtRefT q = dbuf;   // A�̈�
		for (int j=0;j<dw;j++) {
			*q = *p;
			p += 4;
			q += 4;
		}
		sbuf += dpitch;
		dbuf += dpitch;
	}
	ncbPropAccessor layObj(lay);
	layObj.FuncCall(0, L"update", &updateHint, NULL, 0, 0, dw, dh);
	return TJS_S_OK;
}

static tjs_error TJS_INTF_METHOD
fillAlpha(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	// �������ݐ�
	WrtRefT dbuf = 0;
	long l, t, dw, dh, dpitch;
	if (!GetClipBufferAndSize(lay, l, t, dw, dh, dbuf, dpitch)) {
		TVPThrowExceptionMessage(TJS_W("dest must be Layer."));
	}
	dbuf += 3;
	// �S�� 0xff�ł��߂�
	for (int i=0;i<dh;i++) {
		WrtRefT q = dbuf;   // A�̈�
		for (int j=0;j<dw;j++) {
			*q = 0xff;
			q += 4;
		}
		dbuf += dpitch;
	}
	ncbPropAccessor layObj(lay);
	layObj.FuncCall(0, L"update", &updateHint, NULL, l, t, dw, dh);
	return TJS_S_OK;
}

static tjs_error TJS_INTF_METHOD
copyAlphaToProvince(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	iTJSDispatch2 *layerClass = getLayerClass();

	ReadRefT sbuf = 0;
	WrtRefT  dbuf = 0;
	long l, t, w, h, spitch, dpitch, threshold = -1, matched = 1, otherwise = 0;
	if (TJS_PARAM_EXIST(0)) {
		threshold = (long)(param[0]->AsInteger());
	}
	if (TJS_PARAM_EXIST(1)) {
		matched = (long)(param[1]->AsInteger());
	}
	if (TJS_PARAM_EXIST(2)) {
		otherwise = (long)(param[2]->AsInteger());
	}

	if (!GetClipSize(lay, l, t, w, h, spitch)) {
		TVPThrowExceptionMessage(TJS_W("src must be Layer."));
	}

	tTJSVariant val;
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("mainImageBuffer"), &mainImageBufferHint, &val, lay)) ||
		(sbuf = reinterpret_cast<ReadRefT>(val.AsInteger())) == NULL) {
		TVPThrowExceptionMessage(TJS_W("src has no image."));
	}
	sbuf += spitch * t + l * 4;

	val.Clear();
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("provinceImageBufferForWrite"), &provinceImageBufferForWriteHint, &val, lay)) ||
		(dbuf = reinterpret_cast<WrtRefT>(val.AsInteger())) == NULL) {
		TVPThrowExceptionMessage(TJS_W("dst has no province image."));
	}
	val.Clear();
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("provinceImageBufferPitch"), &provinceImageBufferPitchHint, &val, lay)) ||
		(dpitch = (long)val.AsInteger()) == 0) {
		TVPThrowExceptionMessage(TJS_W("dst has no province pitch."));
	}
	dbuf += dpitch * t + l;

	sbuf += 3;
	unsigned char th = (unsigned char)threshold;
	unsigned char on  = (unsigned char)matched;
	unsigned char off = (unsigned char)otherwise;
	int mode = 0;
	if (threshold >= 0 && threshold < 256) {
		bool enmatch = (matched   >= 0 && matched   < 256);
		bool enother = (otherwise >= 0 && otherwise < 256);
		if (!enmatch && !enother) return TJS_S_OK; // �ύX�Ȃ�
		mode = (enmatch && enother) ? 1 : enmatch ? 3 : 4;
	} else if (threshold >= 256) {
		if (otherwise >= 0 && otherwise < 256) mode = 2;
		else return TJS_S_OK; // �ύX�Ȃ�
	}

	for (int y = 0; y < h; y++) {
		WrtRefT  p = dbuf;
		ReadRefT q = sbuf;
		switch (mode) {
		case 0:
			for (int x = 0; x < w; x++, q+=4) *p++ = *q;
			break;
		case 1:
			for (int x = 0; x < w; x++, q+=4) *p++ = (*q >= th) ? on : off;
			break;
		case 2:
			for (int x = 0; x < w; x++, q+=4) *p++ = off;
			break;
		case 3:
			for (int x = 0; x < w; x++, q+=4, p++) if (*q >= th) *p = on;
			break;
		case 4:
			for (int x = 0; x < w; x++, q+=4, p++) if (*q < th) *p = off;
			break;
		}
		sbuf += spitch;
		dbuf += dpitch;
	}
	ncbPropAccessor layObj(lay);
	layObj.FuncCall(0, L"update", &updateHint, NULL, l, t, w, h);
	return TJS_S_OK;
}

static tjs_error TJS_INTF_METHOD
clipAlphaRect(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *dst)
{
	iTJSDispatch2 *layerClass = getLayerClass();
	ncbPropAccessor layObj(dst);

	ReadRefT sbuf = 0;
	WrtRefT  dbuf = 0;
	iTJSDispatch2 *src = 0;
	tTJSVariant val;
	long w, h;
	long dx, dy, dl, dt, diw, dih, dpitch;
	long sx, sy, siw, sih, spitch;
	unsigned char clrval = 0;
	bool clr = false;
	if (numparams < 7) return TJS_E_BADPARAMCOUNT;

	dx  = (long)param[0]->AsInteger();
	dy  = (long)param[1]->AsInteger();
	src =       param[2]->AsObjectNoAddRef();
	sx  = (long)param[3]->AsInteger();
	sy  = (long)param[4]->AsInteger();
	w   = (long)param[5]->AsInteger();
	h   = (long)param[6]->AsInteger();
	if (numparams >= 8 && param[7]->Type() != tvtVoid) {
		long n = (long)param[7]->AsInteger();
		clr = (n >= 0 && n < 256);
		clrval = (unsigned char)(n & 255);
	}
	if (w <= 0|| h <= 0) return TJS_E_INVALIDPARAM;

	// �`���N���b�s���O�̈�
	if (!GetClipSize(dst, dl, dt, diw, dih, dpitch)) {
		TVPThrowExceptionMessage(TJS_W("dest must be Layer."));
	}
	if (!GetLayerSize(src, siw, sih, spitch)) {
		TVPThrowExceptionMessage(TJS_W("src must be Layer."));
	}

	// �o�b�t�@�擾
	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("mainImageBuffer"), &mainImageBufferHint, &val, src))) return false;
	sbuf = reinterpret_cast<ReadRefT>(val.AsInteger());

	if (TJS_FAILED(layerClass->PropGet(0, TJS_W("mainImageBufferForWrite"), &mainImageBufferForWriteHint, &val, dst))) return false;
	dbuf = reinterpret_cast<WrtRefT>(val.AsInteger());
	
	if (!sbuf || !dbuf) TVPThrowExceptionMessage(TJS_W("Layer has no images."));

	dbuf += dpitch * dt + dl * 4;

	// �`��̈�̃N���b�s���O�Ή�
	dx -= dl;
	dy -= dt;

	// �N���b�s���O

	// src���͈͊O
	if (sx+w <= 0   || sy+h <= 0    ||
		sx   >= siw || sy   >= sih) goto none;

	// src�̕������̃J�b�g
	if (sx < 0) { w += sx; dx -= sx; sx = 0; }
	if (sy < 0) { h += sy; dy -= sy; sy = 0; }

	// src�̐������̃J�b�g
	long cut;
	if ((cut = sx + w - siw) > 0) w -= cut;
	if ((cut = sy + h - sih) > 0) h -= cut;

	// dst���͈͊O
	if (dx+w <= 0   || dy+h <= 0    ||
		dx   >= diw || dy   >= dih) goto none;

	// dst�̕������̃J�b�g
	if (dx < 0) { w += dx; sx -= dx; dx = 0; }
	if (dy < 0) { h += dy; sy -= dy; dy = 0; }

	// dst�̐������̃J�b�g
	if ((cut = dx + w - diw) > 0) w -= cut;
	if ((cut = dy + h - dih) > 0) h -= cut;

	if (w <= 0 || h <= 0) goto none;

	long x, y;
	WrtRefT  p;
	ReadRefT q;
	if (clr) {
		for (y = 0;    y < dy;  y++) for ((x=0, p=dbuf+y*dpitch+3); x < diw; x++, p+=4) *p = clrval;
		for (y = dy+h; y < dih; y++) for ((x=0, p=dbuf+y*dpitch+3); x < diw; x++, p+=4) *p = clrval;
	}
	for (y = 0; y < h; y++) {
		if (clr) for ((x=0, p=dbuf+(y+dy)*dpitch+3); x < dx; x++, p+=4) *p = clrval;

		p = dbuf + (y + dy) * dpitch + 3 + (dx*4);
		q = sbuf + (y + sy) * spitch + 3 + (sx*4);
		for (x = 0; x < w; x++, p+=4, q+=4) {
			unsigned long n = (unsigned long)(*p) * (unsigned long)(*q);
			*p = (unsigned char)((n + (n >> 7)) >> 8);
		}
		if (clr) for (x = dx+w; x < diw; x++, p+=4) *p = clrval;
	}
	if (clr) {
		layObj.FuncCall(0, L"update", &updateHint, NULL, dl, dt, diw, dih);
	} else {
		layObj.FuncCall(0, L"update", &updateHint, NULL, dl+dx, dt+dy, w, h);
	}
	return TJS_S_OK;
none:
	// �̈�͈͊O�ŉ��Z���s���Ȃ��ꍇ
	if (clr) {
		for (long y = 0; y < dih; y++) {
			WrtRefT  p = dbuf + y * dpitch + 3;
			for (long x = 0; x < diw; x++, p+=4) *p = clrval;
		}
		layObj.FuncCall(0, L"update", &updateHint, NULL, dl, dt, diw, dih);
	}
	return TJS_S_OK;
}


static tjs_error TJS_INTF_METHOD
fillByProvince(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	iTJSDispatch2 *layerClass = getLayerClass();
	
	if (numparams < 2) return TJS_E_BADPARAMCOUNT;
	unsigned char index = (int)*param[0];
	DWORD color = (int)*param[1];

	// �������ݐ�
	WrtRefT dbuf = 0;
	long l, t, dw, dh, dpitch;
	if (!GetClipBufferAndSize(lay, l, t, dw, dh, dbuf, dpitch)) {
		TVPThrowExceptionMessage(TJS_W("must be Layer."));
	}

	ReadRefT sbuf = 0;
	long spitch;
	{
		tTJSVariant val;
		if (TJS_FAILED(layerClass->PropGet(0, TJS_W("provinceImageBuffer"), &provinceImageBufferHint, &val, lay)) ||
			(sbuf = reinterpret_cast<ReadRefT>(val.AsInteger())) == NULL) {
			TVPThrowExceptionMessage(TJS_W("no province image."));
		}
		if (TJS_FAILED(layerClass->PropGet(0, TJS_W("provinceImageBufferPitch"), &provinceImageBufferPitchHint, &val, lay)) ||
			(spitch = (long)val.AsInteger()) == 0) {
			TVPThrowExceptionMessage(TJS_W("no province pitch."));
		}
	}
	sbuf += t * spitch + l;
	
	for (int y = 0; y < dh; y++) {
		ReadRefT q = sbuf;
		DWORD *p = (DWORD*)dbuf;
		ttstr s;
		for (int x = 0; x < dw; x++) {
			if (*q == index) {
				*(DWORD*)p = color;
			}
			q++;
			p++;
		}
		sbuf += spitch;
		dbuf += dpitch;
	}
	ncbPropAccessor layObj(lay);
	layObj.FuncCall(0, L"update", &updateHint, NULL, l, t, dw, dh);
	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(copyRightBlueToLeftAlpha, Layer, copyRightBlueToLeftAlpha);
NCB_ATTACH_FUNCTION(copyBottomBlueToTopAlpha, Layer, copyBottomBlueToTopAlpha);
NCB_ATTACH_FUNCTION(fillAlpha, Layer, fillAlpha);

NCB_ATTACH_FUNCTION(copyAlphaToProvince, Layer, copyAlphaToProvince);
NCB_ATTACH_FUNCTION(clipAlphaRect, Layer, clipAlphaRect);
NCB_ATTACH_FUNCTION(fillByProvince, Layer, fillByProvince);
