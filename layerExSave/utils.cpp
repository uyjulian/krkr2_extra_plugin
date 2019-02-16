#include "ncbind/ncbind.hpp"
#include "utils.hpp"

//----------------------------------------------
// ���C���C���[�W���샆�[�e�B���e�B

/**
 * ���C���̃T�C�Y�ƃo�b�t�@���擾����
 */
bool
GetLayerSize(iTJSDispatch2 *lay, long &w, long &h, long &pitch)
{
	// ���C���C���X�^���X�ȊO�ł̓G���[
	if (!lay || TJS_FAILED(lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay))) return false;

	// ���C���C���[�W�͍݂邩�H
	tTJSVariant val;
	if (TJS_FAILED(lay->PropGet(0, TJS_W("hasImage"), 0, &val, lay)) || (val.AsInteger() == 0)) return false;

	// ���C���T�C�Y���擾
	val.Clear();
	if (TJS_FAILED(lay->PropGet(0, TJS_W("imageWidth"), 0, &val, lay))) return false;
	w = (long)val.AsInteger();

	val.Clear();
	if (TJS_FAILED(lay->PropGet(0, TJS_W("imageHeight"), 0, &val, lay))) return false;
	h = (long)val.AsInteger();

	// �s�b�`�擾
	val.Clear();
	if (TJS_FAILED(lay->PropGet(0, TJS_W("mainImageBufferPitch"), 0, &val, lay))) return false;
	pitch = (long)val.AsInteger();

	// ����Ȓl���ǂ���
	return (w > 0 && h > 0 && pitch != 0);
}

// �ǂݍ��ݗp
bool
GetLayerBufferAndSize(iTJSDispatch2 *lay, long &w, long &h, BufRefT &ptr, long &pitch)
{
	if (!GetLayerSize(lay, w, h, pitch)) return false;

	// �o�b�t�@�擾
	tTJSVariant val;
	if (TJS_FAILED(lay->PropGet(0, TJS_W("mainImageBuffer"), 0, &val, lay))) return false;
	ptr = reinterpret_cast<BufRefT>(val.AsInteger());
	return  (ptr != 0);
}

// �������ݗp
bool
GetLayerBufferAndSize(iTJSDispatch2 *lay, long &w, long &h, WrtRefT &ptr, long &pitch)
{
	if (!GetLayerSize(lay, w, h, pitch)) return false;

	// �o�b�t�@�擾
	tTJSVariant val;
	if (TJS_FAILED(lay->PropGet(0, TJS_W("mainImageBufferForWrite"), 0, &val, lay))) return false;
	ptr = reinterpret_cast<WrtRefT>(val.AsInteger());
	return  (ptr != 0);
}

/**
 * ��`�̈�̎����𐶐�
 */
static void
MakeResult(tTJSVariant *result, long x, long y, long w, long h)
{
	ncbDictionaryAccessor dict;
	dict.SetValue(TJS_W("x"), x);
	dict.SetValue(TJS_W("y"), y);
	dict.SetValue(TJS_W("w"), w);
	dict.SetValue(TJS_W("h"), h);
	*result = dict;
}

/**
 * �s�����`�F�b�N�֐�
 */
static bool
CheckTransp(BufRefT p, long next, long count)
{
	for (; count > 0; count--, p+=next) if (p[3] != 0) return true;
	return false;
}

/**
 * ���C���C���[�W���N���b�v�i�㉺���E�̗]������������؂���j�����Ƃ��̃T�C�Y���擾����
 *
 * Layer.getCropRect = function();
 * @return %[ x, y, w, h] �`���̎����C�܂���void�i�S�������̂Ƃ��j
 */
static tjs_error TJS_INTF_METHOD
GetCropRect(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	// ���C���o�b�t�@�̎擾
	BufRefT p, r = 0;
	long w, h, nl, nc = 4;
	if (!GetLayerBufferAndSize(lay, w, h, r, nl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	// ���ʗ̈�
	long x1=0, y1=0, x2=w-1, y2=h-1;
	result->Clear();

	for (p=r;             x1 <  w; x1++,p+=nc) if (CheckTransp(p, nl,  h)) break; // �����瓧���̈�𒲂ׂ�
	/*                                      */ if (x1 >= w) return TJS_S_OK;      // �S�������Ȃ� void ��Ԃ�
	for (p=r+x2*nc;       x2 >= 0; x2--,p-=nc) if (CheckTransp(p, nl,  h)) break; // �E���瓧���̈�𒲂ׂ�
	/*                                      */ long rw = x2 - x1 + 1;             // ���E�ɋ��܂ꂽ�c��̕�
	for (p=r+x1*nc;       y1 <  h; y1++,p+=nl) if (CheckTransp(p, nc, rw)) break; // �ォ�瓧���̈�𒲂ׂ�
	for (p=r+x1*nc+y2*nl; y2 >= 0; y2--,p-=nl) if (CheckTransp(p, nc, rw)) break; // �����瓧���̈�𒲂ׂ�

	// ���ʂ������ɕԂ�
	MakeResult(result, x1, y1, rw, y2 - y1 + 1);

	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(getCropRect, Layer, GetCropRect);

/**
 * �s�����`�F�b�N�֐�
 */
static bool
CheckZerop(BufRefT p, long next, long count)
{
	for (; count > 0; count--, p+=next) if (p[3] != 0 || p[2] != 0 || p[1] != 0 || p[0] != 0) return true;
	return false;
}

/**
 * ���C���C���[�W���N���b�v�i�㉺���E�̊��S����������؂���j�����Ƃ��̃T�C�Y���擾����
 *
 * Layer.getCropRectZero = function();
 * @return %[ x, y, w, h] �`���̎����C�܂���void�i�S�������̂Ƃ��j
 */
static tjs_error TJS_INTF_METHOD
GetCropRectZero(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	// ���C���o�b�t�@�̎擾
	BufRefT p, r = 0;
	long w, h, nl, nc = 4;
	if (!GetLayerBufferAndSize(lay, w, h, r, nl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	// ���ʗ̈�
	long x1=0, y1=0, x2=w-1, y2=h-1;
	result->Clear();

	for (p=r;             x1 <  w; x1++,p+=nc) if (CheckZerop(p, nl,  h)) break; // �����瓧���̈�𒲂ׂ�
	/*                                      */ if (x1 >= w) return TJS_S_OK;      // �S�������Ȃ� void ��Ԃ�
	for (p=r+x2*nc;       x2 >= 0; x2--,p-=nc) if (CheckZerop(p, nl,  h)) break; // �E���瓧���̈�𒲂ׂ�
	/*                                      */ long rw = x2 - x1 + 1;             // ���E�ɋ��܂ꂽ�c��̕�
	for (p=r+x1*nc;       y1 <  h; y1++,p+=nl) if (CheckZerop(p, nc, rw)) break; // �ォ�瓧���̈�𒲂ׂ�
	for (p=r+x1*nc+y2*nl; y2 >= 0; y2--,p-=nl) if (CheckZerop(p, nc, rw)) break; // �����瓧���̈�𒲂ׂ�

	// ���ʂ������ɕԂ�
	MakeResult(result, x1, y1, rw, y2 - y1 + 1);

	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(getCropRectZero, Layer, GetCropRectZero);

/**
 * �F��r�֐�
 */
#define IS_SAME_COLOR(A1,R1,G1,B1, A2,R2,G2,B2) \
	(((A1)==(A2)) && ((A1)==0 || ((R1)==(R2) && (G1)==(G2) && (B1)==(B2))))
//	!((p1[3] != p2[3]) || (p1[3] != 0 && (p1[0] != p2[0] || p1[1] != p2[1] || p1[2] != p2[2])))
//  ((A1 == A2) && (A1 == 0 || (R1==R2 && G1==G2 && B1==B2)))


static bool
CheckDiff(BufRefT p1, long p1n, BufRefT p2, long p2n, long count)
{
	for (; count > 0; count--, p1+=p1n, p2+=p2n)
		if (!IS_SAME_COLOR( p1[3],p1[2],p1[1],p1[0],  p2[3],p2[2],p2[1],p2[0] )) return true;
	return false;
}

/**
 * ���C���̍����̈���擾����
 * 
 * Layer.getDiffRegion = function(base);
 * @param base �������ƂȂ�x�[�X�p�̉摜�i�C���X�^���X���g�Ɠ����摜�T�C�Y�ł��邱�Ɓj
 * @return %[ x, y, w, h ] �`���̎����C�܂���void�i���S�ɓ����摜�̂Ƃ��j
 */

static tjs_error TJS_INTF_METHOD
GetDiffRect(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	// �����̐��`�F�b�N
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;

	iTJSDispatch2 *base = param[0]->AsObjectNoAddRef();

	// ���C���o�b�t�@�̎擾
	BufRefT fp, tp, fr = 0, tr = 0;
	long w, h, tnl, fw, fh, fnl, nc = 4;
	if (!GetLayerBufferAndSize(lay,   w,  h, tr, tnl) || 
		!GetLayerBufferAndSize(base, fw, fh, fr, fnl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	// ���C���̃T�C�Y�͓�����
	if (w != fw || h != fh)
		TVPThrowExceptionMessage(TJS_W("Different layer size."));

	// ���ʗ̈�
	long x1=0, y1=0, x2=w-1, y2=h-1;
	result->Clear();

	for (fp=fr,             tp=tr;              x1 <  w; x1++,fp+=nc, tp+=nc ) if (CheckDiff(fp, fnl, tp, tnl,  h)) break; // �����瓧���̈�𒲂ׂ�
	/*                                                                      */ if (x1 >= w) return TJS_S_OK;               // �S�������Ȃ� void ��Ԃ�
	for (fp=fr+x2*nc,       tp=tr+x2*nc;        x2 >= 0; x2--,fp-=nc, tp-=nc ) if (CheckDiff(fp, fnl, tp, tnl,  h)) break; // �E���瓧���̈�𒲂ׂ�
	/*                                                                      */ long rw = x2 - x1 + 1;                      // ���E�ɋ��܂ꂽ�c��̕�
	for (fp=fr+x1*nc,       tp=tr+x1*nc;        y1 <  h; y1++,fp+=fnl,tp+=tnl) if (CheckDiff(fp, nc,  tp, nc,  rw)) break; // �ォ�瓧���̈�𒲂ׂ�
	for (fp=fr+x1*nc+y2*fnl,tp=tr+x1*nc+y2*tnl; y2 >= 0; y2--,fp-=fnl,tp-=tnl) if (CheckDiff(fp, nc,  tp, nc,  rw)) break; // �����瓧���̈�𒲂ׂ�

	// ���ʂ������ɕԂ�
	MakeResult(result, x1, y1, rw, y2 - y1 + 1);

	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(getDiffRect, Layer, GetDiffRect);

/**
 * ���C���̃s�N�Z����r���s��
 * 
 * Layer.getDiffPixel = function(base, samecol, diffcol);
 * @param base �������ƂȂ�x�[�X�p�̉摜�i�C���X�^���X���g�Ɠ����摜�T�C�Y�ł��邱�Ɓj
 * @param samecol �����ꍇ�ɓh��Ԃ��F(0xAARRGGBB)�ivoid�E�ȗ��Ȃ�h��Ԃ��Ȃ��j
 * @param diffcol �Ⴄ�ꍇ�ɓh��Ԃ��F(0xAARRGGBB)�ivoid�E�ȗ��Ȃ�h��Ԃ��Ȃ��j
 * @return �Ⴄ�s�N�Z���̃J�E���g
 */

static tjs_error TJS_INTF_METHOD
GetDiffPixel(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	DWORD scol = 0, dcol = 0;
	bool sfill = false, dfill = false;
	tTVInteger count = 0;

	// �����̐��`�F�b�N
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;

	if (numparams >= 2 && param[1]->Type() != tvtVoid) {
		scol = (DWORD)(param[1]->AsInteger());
		sfill = true;
	}

	if (numparams >= 3 && param[2]->Type() != tvtVoid) {
		dcol = (DWORD)(param[2]->AsInteger());
		dfill = true;
	}

	iTJSDispatch2 *base = param[0]->AsObjectNoAddRef();

	// ���C���o�b�t�@�̎擾
	BufRefT fp, fr = 0;
	WrtRefT tp, tr = 0;
	long w, h, tnl, fw, fh, fnl, nc = 4;
	if (!GetLayerBufferAndSize(lay,   w,  h, tr, tnl) || 
		!GetLayerBufferAndSize(base, fw, fh, fr, fnl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	// ���C���̃T�C�Y�͓�����
	if (w != fw || h != fh)
		TVPThrowExceptionMessage(TJS_W("Different layer size."));

	// �h��Ԃ�
	for (long y = 0; (fp=fr, tp=tr, y < h); y++, fr+=fnl, tr+=tnl) {
		for (long x = 0; x < w; x++, fp+=nc, tp+=nc) {
			bool same = IS_SAME_COLOR(fp[3],fp[2],fp[1],fp[0], tp[3],tp[2],tp[1],tp[0]);
			if (      same &&     sfill) *(DWORD*)tp = scol;
			else if (!same) { if (dfill) *(DWORD*)tp = dcol; count++; }
		}
	}
	if (result) *result = count;

	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(getDiffPixel, Layer, GetDiffPixel);


// �F�����Z
static inline void AddColor(DWORD &r, DWORD &g, DWORD &b, BufRefT p) {
	r += p[2], g += p[1], b += p[0];
}

/**
 * ���C���̕��̐F�𓧖������܂ň����L�΂��i�k�����ɋU�F���o��̂�h���j
 * 
 * Layer.oozeColor = function(level, threshold=1);
 * @param level �������s���񐔁B�傫���قǈ����L�΂��̈悪������
 * @param threshold �A���t�@��臒l(1�`255)������Ⴂ�s�N�Z���ֈ����L�΂�
 * @param fillColor �����̈�ȊO�̓h��Ԃ��F
 */
static tjs_error TJS_INTF_METHOD
OozeColor(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	// �����̐��`�F�b�N
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;
	int level = (int)(param[0]->AsInteger());
	if (level <= 0) 
		TVPThrowExceptionMessage(TJS_W("Invalid level count."));
	unsigned char threshold = (unsigned char)(numparams > 1 ? param[1]->AsInteger() : 1);
	unsigned long fillColor = (unsigned long)(numparams > 2 ? param[2]->AsInteger() : 0);
	unsigned char fillR = (unsigned char)((fillColor >> 16) & 0xff);
	unsigned char fillG = (unsigned char)((fillColor >> 8) & 0xff);
	unsigned char fillB = (unsigned char)((fillColor) & 0xff);
	if (threshold < 1)
		threshold = 1;
	else if (threshold > 255)
		threshold = 255;

	// ���C���o�b�t�@�̎擾
	WrtRefT p, r = 0;
	long x, y, w, h, nl, nc = 4, ow, oh;
	if (!GetLayerBufferAndSize(lay, w, h, r, nl))
		TVPThrowExceptionMessage(TJS_W("Invalid layer image."));

	ow = w+2, oh = h+2; // oozed map �̃T�C�Y
	char *o, *otop, *oozed = new char[ow*oh];
	otop = oozed + ow + 1; // oozed map ����
	ZeroMemory(oozed, ow*oh); // �N���A
	try {
		// �A���t�@�}�b�v�𒲂ׂ�
		for (y = 0; y < h; y++) {
			o = otop + y*ow;
			p = r    + y*nl;
			for (x = 0; x < w; x++, o++, p+=nc) {
				if (p[3] >= threshold) *o = -1;
				else {
					p[2] = fillR;
					p[1] = fillG;
					p[0] = fillB; // 臒l�ȉ��̕s���������̐F���w��F�ŃN���A
				}
			}
		}

		// �����L�΂�����
		for (int i = 0; i < level; i++) {
			bool L, R, U, D;
			for (y = 0; y < h; y++) {
				o = otop + y*ow;
				p = r    + y*nl;
				for (x = 0; x < w; x++, p+=nc, o++) {
					// �������̈���`�F�b�N
					if (!*o) {
						DWORD cr = 0, cg = 0, cb = 0;
						// �㉺���E�̗̈���`�F�b�N
						U=o[-ow]<0, D=o[ow]<0, L=o[-1]<0, R=o[1]<0;
						if (U || D || L || R) {
							int cnt = 0;
							if (U) AddColor(cr, cg, cb, p-nl), cnt++;
							if (D) AddColor(cr, cg, cb, p+nl), cnt++;
							if (L) AddColor(cr, cg, cb, p-nc), cnt++;
							if (R) AddColor(cr, cg, cb, p+nc), cnt++;
							p[2] = (unsigned char)(cr / cnt);
							p[1] = (unsigned char)(cg / cnt);
							p[0] = (unsigned char)(cb / cnt);
							*o = 1;
						}
					}
				}
			}
			// �����σ}�b�v�̒l���Đݒ�
			for (y = 0; y < h; y++) {
				o = otop + y*ow;
				for (x = 0; x < w; x++, o++) if (*o>0) *o=-1;
			}
		}
	} catch (...) {
		delete[] oozed;
		throw;
	}
	delete[] oozed;

	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(oozeColor, Layer, OozeColor);

/**
 * Layer.copyAlpha = function(src);
 * src �� B�l�� ���̈�ɃR�s�[����
 */
static tjs_error TJS_INTF_METHOD
CopyBlueToAlpha(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	if (numparams < 0) {
		return TJS_E_BADPARAMCOUNT;
	}

	// �ǂݍ��݂���
	BufRefT sbuf = 0;
	long sw, sh, spitch;
	if (!GetLayerBufferAndSize(param[0]->AsObjectNoAddRef(), sw, sh, sbuf, spitch)) {
		TVPThrowExceptionMessage(TJS_W("src must be Layer."));
	}
	// �������ݐ�
	WrtRefT dbuf = 0;
	long dw, dh, dpitch;
	if (!GetLayerBufferAndSize(lay, dw, dh, dbuf, dpitch)) {
		TVPThrowExceptionMessage(TJS_W("dest must be Layer."));
	}

	// �������̈敪
	int w = (sw < dw ? sw : dw);
	int h = sh < dh ? sh : dh;
	// �R�s�[
	for (int i=0;i<h;i++) {
		BufRefT p = sbuf;     // B�̈�
		WrtRefT q = dbuf+3;   // A�̈�
		for (int j=0;j<w;j++) {
			*q = *p;
			p += 4;
			q += 4;
		}
		sbuf += spitch;
		dbuf += dpitch;
	}
	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(copyBlueToAlpha, Layer, CopyBlueToAlpha);

/**
 * Layer.isBlank = function(x,y,w,h);
 * �w��̈悪�u�����N�f�[�^���ǂ����m�F����
 */
static tjs_error TJS_INTF_METHOD
isBlank(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 4) {
		return TJS_E_BADPARAMCOUNT;
	}

	// �ǂݍ��݂���
	BufRefT sbuf = 0;
	long sw, sh, spitch;
	if (!GetLayerBufferAndSize(objthis, sw, sh, sbuf, spitch)) {
		TVPThrowExceptionMessage(TJS_W("src must be Layer."));
	}

	tjs_int left   = *param[0];
	tjs_int top    = *param[1];
	tjs_int width  = *param[2];
	tjs_int height = *param[3];

	// �\�[�X�͈͂ŃN���b�s���O
	if (left < 0) { width += left; left = 0; }
	if (top  < 0) { height += top; top  = 0; }
	tjs_int cut;
	if ((cut = left + width  - sw) > 0) width  -= cut;
	if ((cut = top  + height - sh) > 0) height -= cut;

	// �͈̓`�F�b�N
	if (width >= 0 && height >= 0) {
		// ���菈��
		for (tjs_int y = top; y < top + height; y++) {
			BufRefT buffer = sbuf + left * 4 + spitch * y;
			for (tjs_int x = left; x < left + width; x++, buffer += 4) {
				if (*buffer) {
					if (result) {
						*result = 0;
					}
					return TJS_S_OK;
				}
			}
		}
	}
	
	if (result) {
		*result = 1;
	}
	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(isBlank, Layer, isBlank);

/**
 * Layer.clearAlpha = function (threthold, fillColor=0)
 * �����w���菬�������������S����������
 */
static tjs_error TJS_INTF_METHOD
clearAlpha(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *lay)
{
	int threthold = numparams <= 0 ? 0 : *param[0];
	unsigned long fillColor = (unsigned long) ((numparams > 1 ? param[1]->AsInteger() : 0) & 0xffffff);
	
	// �������ݐ�
	WrtRefT dbuf = 0;
	long w, h, pitch;
	if (!GetLayerBufferAndSize(lay, w, h, dbuf, pitch)) {
		TVPThrowExceptionMessage(TJS_W("dest must be Layer."));
	}

	// �R�s�[
	for (int i=0;i<h;i++) {
		WrtRefT q = dbuf;   // A�̈�
		for (int j=0;j<w;j++) {
			if (q[3] <= threthold) {
				*((unsigned long*)q) = fillColor;
			}
			q += 4;
		}
		dbuf += pitch;
	}
	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(clearAlpha, Layer, clearAlpha);

/**
 * Layer.getAverageColor = function(x,y,w,h);
 * �w��̈�̕��ϐF��Ԃ�
 */
static tjs_error TJS_INTF_METHOD
getAverageColor(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 4) {
		return TJS_E_BADPARAMCOUNT;
	}

	// �ǂݍ��݂���
	BufRefT sbuf = 0;
	long sw, sh, spitch;
	if (!GetLayerBufferAndSize(objthis, sw, sh, sbuf, spitch)) {
		TVPThrowExceptionMessage(TJS_W("src must be Layer."));
	}

	tjs_int left   = *param[0];
	tjs_int top    = *param[1];
	tjs_int width  = *param[2];
	tjs_int height = *param[3];

	// �\�[�X�͈͂ŃN���b�s���O
	if (left < 0) { width += left; left = 0; }
	if (top  < 0) { height += top; top  = 0; }
	tjs_int cut;
	if ((cut = left + width  - sw) > 0) width  -= cut;
	if ((cut = top  + height - sh) > 0) height -= cut;

	// �͈̓`�F�b�N
	if (width <= 0 || height <= 0) 
		TVPThrowExceptionMessage(L"invalid layer range");

	double a = 0;
	double r = 0;
	double g = 0;
	double b = 0;
	double size = width * height;
	
	// �l�擾
	for (tjs_int y = top; y < top + height; y++) {
		BufRefT buffer = sbuf + left * 4 + spitch * y;
		for (tjs_int x = left; x < left + width; x++, buffer += 4) {
			a += buffer[0];
			r += buffer[1];
			g += buffer[2];
			b += buffer[3];
		}
	}
	a /= size;
	r /= size;
	g /= size;
	b /= size;
	DWORD color = (((DWORD)a & 0xff) << 24 |
								 ((DWORD)r & 0xff) << 16 |
								 ((DWORD)g & 0xff) << 8 |
								 ((DWORD)b & 0xff) << 0);
  if (result) {
	  *result = (tjs_int)color;
	}
	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(getAverageColor, Layer, getAverageColor);

