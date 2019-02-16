#include "LayerExBase.h"

int NI_LayerExBase::classId;

iTJSDispatch2 * NI_LayerExBase::_leftProp   = NULL;
iTJSDispatch2 * NI_LayerExBase::_topProp    = NULL;
iTJSDispatch2 * NI_LayerExBase::_widthProp  = NULL;
iTJSDispatch2 * NI_LayerExBase::_heightProp = NULL;
iTJSDispatch2 * NI_LayerExBase::_pitchProp  = NULL;
iTJSDispatch2 * NI_LayerExBase::_bufferProp = NULL;
iTJSDispatch2 * NI_LayerExBase::_updateProp = NULL;

void
NI_LayerExBase::init(iTJSDispatch2 *layerobj)
{
	// �v���p�e�B�擾
	tTJSVariant var;
	
	if (TJS_FAILED(layerobj->PropGet(TJS_IGNOREPROP, TJS_W("imageLeft"), NULL, &var, layerobj))) {
		TVPThrowExceptionMessage(TJS_W("invoking of Layer.imageLeft failed."));
	} else {
		_leftProp = var;
	}
	if (TJS_FAILED(layerobj->PropGet(TJS_IGNOREPROP, TJS_W("imageTop"), NULL, &var, layerobj))) {
		TVPThrowExceptionMessage(TJS_W("invoking of Layer.imageTop failed."));
	} else {
		_topProp = var;
	}
	if (TJS_FAILED(layerobj->PropGet(TJS_IGNOREPROP, TJS_W("imageWidth"), NULL, &var, layerobj))) {
		TVPThrowExceptionMessage(TJS_W("invoking of Layer.imageWidth failed."));
	} else {
		_widthProp = var;
	}
	if (TJS_FAILED(layerobj->PropGet(TJS_IGNOREPROP, TJS_W("imageHeight"), NULL, &var, layerobj))) {
		TVPThrowExceptionMessage(TJS_W("invoking of Layer.imageHeight failed."));
	} else {
		_heightProp = var;
	}
	if (TJS_FAILED(layerobj->PropGet(TJS_IGNOREPROP, TJS_W("mainImageBufferForWrite"), NULL, &var, layerobj))) {
		TVPThrowExceptionMessage(TJS_W("invoking of Layer.mainImageBufferForWrite failed."));
	} else {
		_bufferProp = var;
	}
	if (TJS_FAILED(layerobj->PropGet(TJS_IGNOREPROP, TJS_W("mainImageBufferPitch"), NULL, &var, layerobj))) {
		TVPThrowExceptionMessage(TJS_W("invoking of Layer.mainImageBufferPitch failed."));
	} else {
		_pitchProp = var;
	}
	if (TJS_FAILED(layerobj->PropGet(TJS_IGNOREPROP, TJS_W("update"), NULL, &var, layerobj))) {
		TVPThrowExceptionMessage(TJS_W("invoking of Layer.update failed."));
	} else {
		_updateProp = var;
	}
}

void
NI_LayerExBase::unInit()
{
	if (_leftProp)   _leftProp->Release();
	if (_topProp)    _topProp->Release();
	if (_widthProp)  _widthProp->Release();
	if (_heightProp) _heightProp->Release();
	if (_bufferProp) _bufferProp->Release();
	if (_pitchProp)  _pitchProp->Release();
	if (_updateProp) _updateProp->Release();
}

/// �v���p�e�B���� int �l���擾����
static tjs_int64 getPropValue(iTJSDispatch2 *dispatch, iTJSDispatch2 *layerobj)
{
	tTJSVariant var;
	if(TJS_FAILED(dispatch->PropGet(0, NULL, NULL, &var, layerobj))) {
		TVPThrowExceptionMessage(TJS_W("can't get int value from property."));
	}
	return var;
}
	
void
NI_LayerExBase::reset(iTJSDispatch2 *layerobj)
{
	_width  = (int)getPropValue(_widthProp, layerobj);
	_height = (int)getPropValue(_heightProp, layerobj);
	_buffer = (unsigned char *)getPropValue(_bufferProp, layerobj);
	_pitch  = (int)getPropValue(_pitchProp, layerobj);
}

/**
 * �X�V�����Ăяo��
 * layer.update(x,y,w,h) ���Ăяo��
 * �摜�̈�S�̂��X�V�Ƃ���
 */
void
NI_LayerExBase::redraw(iTJSDispatch2 *layerobj)
{
	tTJSVariant vars[4];
	_leftProp->PropGet(0,NULL,NULL,&vars[0], layerobj);
	_topProp->PropGet(0,NULL,NULL,&vars[1], layerobj);
	_widthProp->PropGet(0,NULL,NULL,&vars[2], layerobj);
	_heightProp->PropGet(0,NULL,NULL,&vars[3], layerobj);
	tTJSVariant *varsp[4] = {vars, vars+1, vars+2, vars+3};
	_updateProp->FuncCall(0, NULL, NULL, NULL, 4, varsp, layerobj);
}

NI_LayerExBase *
NI_LayerExBase::getNative(iTJSDispatch2 *objthis, bool create)
{
	NI_LayerExBase *_this = NULL;
	if (TJS_FAILED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
												  classId, (iTJSNativeInstance**)&_this)) && create) {
		_this = new NI_LayerExBase();
		if (TJS_FAILED(objthis->NativeInstanceSupport(TJS_NIS_REGISTER,
													  classId, (iTJSNativeInstance **)&_this))) {
			delete _this;
			_this = NULL;
		}
	}
	return _this;
}
	
/**
 * �R���X�g���N�^
 */
NI_LayerExBase::NI_LayerExBase()
{
	_width = 0;
	_height = 0;
	_pitch = 0;
	_buffer = NULL;
}
