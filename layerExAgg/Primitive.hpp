#ifndef __primitive__
#define __primitive__

#include <windows.h>
#include "tp_stub.h"

// ----------------------------------------
// Anti-Grain Geometry �p��`�Q
// ----------------------------------------

#include "agg_basics.h"
#include "agg_trans_affine.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_renderer_base.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_p.h"

typedef agg::pixfmt_bgra32 pixfmt;
typedef agg::rgba8 color_type;
typedef agg::renderer_base<pixfmt> renderer_base;
typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_scanline;
typedef agg::rasterizer_scanline_aa<> rasterizer_scanline;
typedef agg::scanline_p8 scanline;

class NI_AGGPrimitive;

/**
 * ���C���ɔz�u�\�� AGG �v���~�e�B�u�̊��N���X
 */
class AGGPrimitive
{
protected:
	NI_AGGPrimitive *owner;

	// �\���ʒu
	double _x;
    double _y;

	/// �␳�l
	double _expand;
	double _scale;
	double _rotate;
	
public:

	/// �R���X�g���N�^
	AGGPrimitive(NI_AGGPrimitive *owner) : owner(owner) {
		_x = 0.0;
		_y = 0.0;
		_expand = 0.0;
		_scale = 1.0;
		_rotate = 1.0;
	}

	/// �f�X�g���N�^
	virtual ~AGGPrimitive() {
	}

	void redraw();
	
public:

	inline double getX() { return _x; };
	inline void setX(double x) { _x = x; redraw(); };

	inline double getY() { return _y; };
	inline void setY(double y) { _y = y; redraw();};

	inline double getRotate() { return _rotate; };
	inline void setRotate(double rotate) { _rotate = rotate; redraw();};

	double getScale() { return _scale; };
	inline void setScale(double scale) { _scale = scale; redraw();};

	double getExpand() { return _expand; };
	inline void setExpand(double expand) { _expand = expand; redraw();};
	
	/*
	 * ���W�w��
	 */
	inline void setPos(double x, double y) {
		_x = x;
		_y = y;
		redraw();
	}
	
	/**
	 * �`�揈��
	 * @param rb �x�[�X�����_��
	 * @param mtx �S�̃A�t�B���ό`�w��
	 */
	virtual void paint(renderer_base &rb, agg::trans_affine &mtx) = 0;
};

extern AGGPrimitive *getAGGPrimitive(iTJSDispatch2 *obj);
extern void addMember(iTJSDispatch2 *dispatch, const tjs_char *memberName, iTJSDispatch2 *member);

#define PRIMFUNC(funcname,pnum,type) \
class funcname : public tTJSDispatch\
{\
protected:\
public:\
	tjs_error TJS_INTF_METHOD FuncCall(\
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,\
		tTJSVariant *result,\
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {\
		type *_this;\
		if ((_this = (type)getAGGPrimitiveNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;\
		if (numparams < pnum) return TJS_E_BADPARAMCOUNT;

#define PRIMFUNCEND \
		return TJS_S_OK;\
	}\
};

// --------------------------------------------------------------------

class RegistType {
public:
	virtual const tjs_char *getTypeName() = 0;
	virtual AGGPrimitive* create(NI_AGGPrimitive *owner, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) = 0;
};

extern void registType(RegistType *type);

/**
 * �^�t�@�N�g���o�^�p�e���v���[�g
 * �C�ӂ� AGGPrimiitveVN �^�� create ���\�b�h��o�^����
 */
template <class U>
class RegistTypeFactory : public RegistType {
public:
	virtual AGGPrimitive* create(NI_AGGPrimitive *owner, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
		return new U(owner, numparams, param, tjs_obj);
	}
	virtual const tjs_char *getTypeName() {
		return U::getTypeName();
	}
	inline RegistTypeFactory() {
		registType(this);
	};
};

#endif
