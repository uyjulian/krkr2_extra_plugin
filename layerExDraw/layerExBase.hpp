#ifndef _layExBase_hpp_
#define _layExBase_hpp_

#include "tp_stub.h"

/**
 * �v���p�e�B�̃L���b�V�������p
 */
struct ObjectCache
{
	typedef iTJSDispatch2*  DispatchT;
	typedef tjs_char const* NameT;
	typedef tTVInteger      IntegerT;
	typedef ttstr           StringT;
	typedef tTJSVariant     VariantT;

	ObjectCache(DispatchT obj, NameT name) : _obj(obj), _cache(0), _name(name) {

		tTJSVariant layer;
		TVPExecuteExpression(TJS_W("Layer"), &layer);
		tTJSVariant var;
		if (TJS_SUCCEEDED(layer.AsObjectNoAddRef()->PropGet(TJS_IGNOREPROP, name, NULL, &var, layer.AsObjectNoAddRef()))) _cache = var;
		else _Exception(TJS_W("FAILED: get property object :"));
	}

	~ObjectCache() {
		if (_cache) _cache->Release();
	}

	inline VariantT GetValue() const {
		VariantT var;
		if (TJS_FAILED(_cache->PropGet(0, 0, 0, &var, _obj)))
			_Exception(TJS_W("FAILED: get property value :"));
		return var;
	}
	inline operator VariantT() const { return GetValue(); }
	inline operator IntegerT() const { return static_cast<IntegerT>(GetValue()); }

	inline VariantT operator ()(int numparams, VariantT **param) {
		VariantT var;
		if (TJS_FAILED(_cache->FuncCall(0, 0, 0, &var, numparams, param, _obj)))
			_Exception(TJS_W("FAILED: function call :"));
		return var;
	}

	inline void SetValue(int n) const {
		VariantT var = n;
		if (TJS_FAILED(_cache->PropSet(0, 0, 0, &var, _obj)))
			_Exception(TJS_W("FAILED: get property value :"));
	}

private:
	DispatchT _obj, _cache;
	NameT _name;

	void _Exception(NameT mes) const {
		TVPThrowExceptionMessage(mes, _name);
	}
};

/**
 * ���C���g�������̃x�[�X
 */
struct layerExBase
{
	typedef iTJSDispatch2* DispatchT;
	typedef ObjectCache    ObjectT;
	typedef unsigned char* BufferT;
	typedef unsigned char* BufferRT;
	typedef tjs_int        PitchT;
	typedef tjs_int        GeometryT;
	DispatchT _obj;
	/**
	 * �R���X�g���N�^
	 */
	layerExBase(DispatchT obj)
		: _obj(obj),
		  _pWidth( obj, TJS_W("imageWidth")),
		  _pHeight(obj, TJS_W("imageHeight")),
		  _pBuffer(obj, TJS_W("mainImageBufferForWrite")),
		  _pPitch( obj, TJS_W("mainImageBufferPitch")),
		  _pUpdate(obj, TJS_W("update")),
		  _pClipLeft(  obj, TJS_W("clipLeft")),
	      _pClipTop(   obj, TJS_W("clipTop")),
	      _pClipWidth( obj, TJS_W("clipWidth")),
	      _pClipHeight(obj, TJS_W("clipHeight")),
		  _width(0), _height(0), _pitch(0), _buffer(0), _clipLeft(0), _clipTop(0), _clipWidth(0), _clipHeight(0)
	{
	}

	/**
	 * �f�X�g���N�^
	 */
	virtual ~layerExBase() {}

	/**
	 * �ĕ`��w��
	 */
	virtual void redraw() {
		tTJSVariant  vars [4] = { _pClipLeft, _pClipTop, _pClipWidth, _pClipHeight };
		tTJSVariant *varsp[4] = { vars, vars+1, vars+2, vars+3 };
		_pUpdate(4, varsp);
	}

	/**
	 * ���X�V
	 */
	virtual void reset() {
		_width  = (GeometryT)_pWidth;
		_height = (GeometryT)_pHeight;
		_buffer = (BufferT)(ObjectCache::IntegerT)_pBuffer;
		_pitch  = (PitchT)_pPitch;
		_clipLeft   = (GeometryT)_pClipLeft;
		_clipTop    = (GeometryT)_pClipTop;
		_clipWidth  = (GeometryT)_pClipWidth;
		_clipHeight = (GeometryT)_pClipHeight;
	}

protected:
	ObjectT   _pWidth, _pHeight, _pBuffer, _pPitch, _pUpdate;

	GeometryT _width, _height;
	BufferT   _buffer;
	PitchT    _pitch;

	// �N���b�v���
	ObjectT   _pClipLeft, _pClipTop, _pClipWidth, _pClipHeight;
	GeometryT _clipLeft, _clipTop, _clipWidth, _clipHeight;
};

#endif
