#include <stdio.h>
#include <windows.h>
#include <list>
#include <map>
using namespace std;

#include "tp_stub.h"

static const char *copyright = 
"----- AntiGrainGeometry Copyright START -----\n"
"Anti-Grain Geometry - Version 2.3\n"
"Copyright (C) 2002-2005 Maxim Shemanarev (McSeem)\n"
"\n"
"Permission to copy, use, modify, sell and distribute this software\n"
"is granted provided this copyright notice appears in all copies. \n"
"This software is provided \"as is\" without express or implied\n"
"warranty, and with no claim as to its suitability for any purpose.\n"
"----- AntiGrainGeometry Copyright END -----\n";

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

#include "LayerExBase.h"
#include "Primitive.hpp"

/**
 * Anti-Grain Geometry �v���~�e�B�u�l�C�e�B�u�C���X�^���X
 */
class NI_AGGPrimitive : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
	friend class NI_AntiGrainGeometry;
protected:
	iTJSDispatch2 * _layer;
public:
	AGGPrimitive * _primitive;
	NI_AGGPrimitive();
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();
	void redraw();
};

void AGGPrimitive::redraw()
{
	owner->redraw();
}

/*
 * ���C���ɒ������� Anti-Grain Geometry ����ێ����邽�߂̃l�C�e�B�u�C���X�^���X
 */
class NI_AntiGrainGeometry : public tTJSNativeInstance
{
	friend class NI_AGGPrimitive;
	
protected:
	iTJSDispatch2 * _layerobj;

protected:
	/// �ĕ`��
	bool _redraw;
public:
	/**
	 * �ĕ`��v��
	 */
	void redraw() {
		if (!_redraw) {
			_redraw = true;
			NI_LayerExBase *base;
			if ((base = NI_LayerExBase::getNative(_layerobj))) {
				base->redraw(_layerobj);
			}
		}
	}

	//-------------------------------------------
	// �\���ʒu
	//-------------------------------------------
protected:
	/// �\���ʒuX
	double _x;
	/// �\���ʒuY
	double _y;
	/// �g��
	double _scale;
	/// ��]
	double _rotate;

public:
	inline double getX() { return _x; };
	inline void setX(double x) { _x = x; redraw(); };

	inline double getY() { return _y; };
	inline void setY(double y) { _y = y; redraw(); };

	inline double getRotate() { return _rotate; };
	inline void setRotate(double rotate) { _rotate = rotate; redraw(); };

	double getScale() { return _scale; };
	inline void setScale(double scale) { _scale = scale; redraw(); };
	
	/*
	 * ���W�w��
	 * @param x X�ړ���
	 * @param y Y�ړ���
	 */
	void setPos(double x, double y) {
		_x = x;
		_y = y;
		redraw();
	}

	//-------------------------------------------
	// �`��v�f
	//-------------------------------------------
protected:
	list<NI_AGGPrimitive*> _primitives;
public:
	void addPrimitive(NI_AGGPrimitive *primitive) {
		_primitives.push_back(primitive);
		redraw();
	}
	void removePrimitive(NI_AGGPrimitive *primitive) {
		_primitives.remove(primitive);
		redraw();
	}
	
public:
	/**
	 * �h��Ȃ�������
	 */
	void onPaint() {

		if (_redraw) {

			NI_LayerExBase *base;
			if ((base = NI_LayerExBase::getNative(_layerobj))) {
				base->reset(_layerobj);

				// AGG �p�ɐ擪�ʒu�ɕ␳
				unsigned char *buffer = base->_buffer;
				if (base->_pitch < 0) {
					buffer += int(base->_height - 1) * base->_pitch;
				}
				
				/// �����_�����O�p�o�b�t�@
				agg::rendering_buffer rbuf(buffer, base->_width, base->_height, base->_pitch);
				
				// �����_���̏���
				pixfmt pixf(rbuf);
				renderer_base rb(pixf);
				
				// ��x�������� XXX �t���O�����Ă���������Ȃ�����H
				rb.clear(color_type(0,0,0,0));
				
				// �ό`����
				agg::trans_affine mtx;
				mtx *= agg::trans_affine_translation((base->_width) * -0.5, (base->_height) * -0.5);
				mtx *= agg::trans_affine_scaling(_scale);
				mtx *= agg::trans_affine_rotation(agg::deg2rad(_rotate));
				mtx *= agg::trans_affine_translation(base->_width * 0.5 + _x, base->_height * 0.5 + _y);
				
				// �v���~�e�B�u�̍ĕ`��
				{
					list<NI_AGGPrimitive*>::iterator i =  _primitives.begin();
					while (i != _primitives.end()) {
						(*i)->_primitive->paint(rb, mtx);
						i++;
					}
				}
			}
			
			_redraw = false;
		}
	}

public:
	/**
	 * �R���X�g���N�^
	 */
	NI_AntiGrainGeometry(iTJSDispatch2 *layerobj) {
		_layerobj = layerobj; // no addRef
		_redraw = false;
		_x = 0;
		_y = 0;
		_scale = 1.0;
		_rotate = 0;
	}
};

// �N���XID
static tjs_int32 ClassID_AntiGrainGeometry = -1;

/**
 * Layer �� onPaint �����̂̂��Ƃ�p�t�@���N�V����
 */
class tOnPaintFunction : public tTJSDispatch
{
protected:
	/// ���̃��\�b�h
	iTJSDispatch2 *original;
public:
	/// �R���X�g���N�^
	tOnPaintFunction(iTJSDispatch2 *original) : original(original) {}
	
	/// �f�X�g���N�^
	~tOnPaintFunction() {
		if (original) {
			original->Release();
		}
	}

	/// �֐��Ăяo��
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
};

void
addMember(iTJSDispatch2 *dispatch, const tjs_char *memberName, iTJSDispatch2 *member)
{
	tTJSVariant var = tTJSVariant(member);
	member->Release();
	dispatch->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		memberName, // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
		NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
		&var, // �o�^����l
		dispatch // �R���e�L�X�g
		);
}

/**
 * ���C���I�u�W�F�N�g���� Anti-Grain Geometry �p�l�C�e�B�u�C���X�^���X���擾����B
 * �l�C�e�B�u�C���X�^���X�������ĂȂ��ꍇ�͎����I�Ɋ��蓖�Ă�
 * @param objthis ���C���I�u�W�F�N�g
 * @return Anti-Grain Geometry �p�l�C�e�B�u�C���X�^���X�B�擾���s������ NULL
 */
static NI_AntiGrainGeometry *
getAntiGrainGeometryNative(iTJSDispatch2 *layerobj)
{
	if (!layerobj) return NULL;

	NI_AntiGrainGeometry *_this;
	if (TJS_FAILED(layerobj->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
												   ClassID_AntiGrainGeometry, (iTJSNativeInstance**)&_this))) {

		// ���C���g��������
		if (NI_LayerExBase::getNative(layerobj) == NULL) {
			return NULL;
		}
	
		_this = new NI_AntiGrainGeometry(layerobj);
		if (TJS_FAILED(layerobj->NativeInstanceSupport(TJS_NIS_REGISTER,
													   ClassID_AntiGrainGeometry, (iTJSNativeInstance **)&_this))) {
			delete _this;
			return NULL;
		}

		// onPaint ���̂��Ƃ�
		{
			const tjs_char *memberName = TJS_W("onPaint");
			tTJSVariant var;
			if (layerobj->PropGet(0, memberName, NULL, &var, layerobj) == TJS_S_OK) {
				addMember(layerobj, memberName,  new tOnPaintFunction(var.AsObject()));
			} else {
				addMember(layerobj, memberName,  new tOnPaintFunction(NULL));
			}
		}		
	}

	return _this;
}

	/// �֐��Ăяo��
tjs_error TJS_INTF_METHOD
tOnPaintFunction::FuncCall(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
						   tTJSVariant *result,
						   tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	tjs_error ret;
	if (original) {
		ret = original->FuncCall(flag, membername, hint, result, numparams, param, objthis);
	} else {
		ret = TJS_S_OK;
	}
	if (ret == TJS_S_OK) {
		NI_AntiGrainGeometry *_this;
		if ((_this = getAntiGrainGeometryNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		_this->onPaint();
	}
	return ret;
}


#define FUNC(funcname,pnum) \
class funcname : public tTJSDispatch\
{\
protected:\
public:\
	tjs_error TJS_INTF_METHOD FuncCall(\
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,\
		tTJSVariant *result,\
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {\
		NI_AntiGrainGeometry *_this;\
		if ((_this = getAntiGrainGeometryNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;\
		if (numparams < pnum) return TJS_E_BADPARAMCOUNT;

#define FUNCEND \
		return TJS_S_OK;\
	}\
};

#define PROP(funcname) \
class funcname : public tTJSDispatch\
{\
protected:\
public:

#define GETTER \
	tjs_error TJS_INTF_METHOD PropGet(\
		tjs_uint32 flag,\
		const tjs_char * membername,\
		tjs_uint32 *hint,\
		tTJSVariant *result,\
		iTJSDispatch2 *objthis)	{\
		NI_AntiGrainGeometry *_this;\
		if ((_this = getAntiGrainGeometryNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;\
		

#define SETTER \
        return TJS_S_OK;\
    }\
	tjs_error TJS_INTF_METHOD PropSet(\
		tjs_uint32 flag,\
		const tjs_char *membername,\
		tjs_uint32 *hint,\
		const tTJSVariant *param,\
		iTJSDispatch2 *objthis)	{\
		NI_AntiGrainGeometry *_this;\
		if ((_this = getAntiGrainGeometryNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		
#define PROPEND \
		return TJS_S_OK;\
	}\
};

FUNC(tSetPosFunction,2)
	_this->setPos(*param[0], *param[1]);
FUNCEND

PROP(tXProp)
GETTER
	*result = _this->getX();
SETTER
	_this->setX(*param);
FUNCEND

PROP(tYProp)
GETTER
	*result = _this->getY();
SETTER
	_this->setY(*param);
FUNCEND
	
PROP(tRotateProp)
GETTER
	*result = _this->getRotate();
SETTER
	_this->setRotate(*param);
FUNCEND

PROP(tScaleProp)
GETTER
	*result = _this->getScale();
SETTER
	_this->setScale(*param);
PROPEND

//---------------------------------------------------------------------------

// �^�o�^�p
list<RegistType*> *typeList = NULL;
map<ttstr,RegistType*> typeMap;

void registType(RegistType *type)
{
	if (typeList == NULL) {
		typeList = new list<RegistType*>;
	}
	typeList->push_back(type);
}

/**
 * �v���~�e�B�u�l�C�e�B�u�C���X�^���X���\�b�h
 */
NI_AGGPrimitive::NI_AGGPrimitive()
{
	_layer = NULL;
	_primitive = NULL;
}
	
tjs_error TJS_INTF_METHOD
NI_AGGPrimitive::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	if (numparams < 2) return TJS_E_BADPARAMCOUNT;

	if (param[0]->Type() != tvtObject || param[0]->AsObjectNoAddRef()->IsInstanceOf(0,NULL,NULL,L"Layer",param[0]->AsObjectNoAddRef()) == false) {
		TVPThrowExceptionMessage(TJS_W("first parameter must Layer."));
	}
	
	// �e���C��
	_layer = param[0]->AsObject();
	try {
		NI_AntiGrainGeometry *agg;

		// �e���C������ AGG ���C���X�^���X���擾�B�����ꍇ�͐�������B
		if ((agg = getAntiGrainGeometryNative(_layer)) == NULL) {
			TVPThrowExceptionMessage(TJS_W("failed to get AGG Instance from Layer."));
		}

		// �I�u�W�F�N�g����
		map<ttstr,RegistType*>::const_iterator n = typeMap.find(*param[1]);
		if (n != typeMap.end()) {
			_primitive = n->second->create(this, numparams-2, param+2, tjs_obj);
		} else {
			TVPThrowExceptionMessage((ttstr(L"failed to create ") + (ttstr)*param[1]).c_str());
		}

		// �e�Ɏ�����o�^
		agg->addPrimitive(this);

	} catch (...) {
		_layer->Release();
		_layer = NULL;
		throw;
	}

	return TJS_S_OK;
}

void TJS_INTF_METHOD
NI_AGGPrimitive::Invalidate()
{
	if (_layer) {
		NI_AntiGrainGeometry *agg;
		if (TJS_SUCCEEDED(_layer->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
														  ClassID_AntiGrainGeometry, (iTJSNativeInstance**)&agg))) {
			// �e���玩��������
			agg->removePrimitive(this);
		} else {
			log(TJS_W("failed to get AGG Instance from Layer"));
		}
		_layer->Release();
	}
	delete _primitive;
};

void
NI_AGGPrimitive::redraw()
{
	if (_layer) {
		NI_AntiGrainGeometry *agg;
		if (TJS_SUCCEEDED(_layer->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
														  ClassID_AntiGrainGeometry, (iTJSNativeInstance**)&agg))) {
			// �e���ĕ`��
			agg->redraw();
		} else {
			log(TJS_W("failed to get AGG Instance from Layer"));
		}
	}
}

//---------------------------------------------------------------------------
/*
	����� NI_AGGPrimitive �̃I�u�W�F�N�g���쐬���ĕԂ������̊֐��ł��B
	��q�� TJSCreateNativeClassForPlugin �̈����Ƃ��ēn���܂��B
*/
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_AGGPrimitive()
{
	return new NI_AGGPrimitive();
}
//---------------------------------------------------------------------------
/*
	TJS2 �̃l�C�e�B�u�N���X�͈�ӂ� ID �ŋ�ʂ���Ă���K�v������܂��B
	����͌�q�� TJS_BEGIN_NATIVE_MEMBERS �}�N���Ŏ����I�Ɏ擾����܂����A
	���� ID ���i�[����ϐ����ƁA���̕ϐ��������Ő錾���܂��B
	�����l�ɂ͖����� ID ��\�� -1 ���w�肵�Ă��������B
*/
#define TJS_NATIVE_CLASSID_NAME ClassID_AGGPrimitive
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
/*
	TJS2 �p�́u�N���X�v���쐬���ĕԂ��֐��ł��B
*/
static iTJSDispatch2 * Create_NC_AGGPrimitive()
{
	/// �N���X�I�u�W�F�N�g�̍쐬
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("AGGPrimitive"), Create_NI_AGGPrimitive);

	/// �����o��`
	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/AGGPrimitive)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_AGGPrimitive,
			/*TJS class name*/AGGPrimitive)
		{
			return TJS_S_OK;
		}
	    TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/AGGPrimitive)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setPos)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_AGGPrimitive);
			if (numparams < 2) return TJS_E_BADPARAMCOUNT;
			_this->_primitive->setPos(*param[0], *param[1]);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/setPos)

		TJS_BEGIN_NATIVE_PROP_DECL(x)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				if (result) {
					*result = _this->_primitive->getX();
				}
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_BEGIN_NATIVE_PROP_SETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				_this->_primitive->setX(*param);
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(x)

		TJS_BEGIN_NATIVE_PROP_DECL(y)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				if (result) {
					*result = _this->_primitive->getY();
				}
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_BEGIN_NATIVE_PROP_SETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				_this->_primitive->setY(*param);
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(y)

		TJS_BEGIN_NATIVE_PROP_DECL(rotate)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				if (result) {
					*result = _this->_primitive->getRotate();
				}
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_BEGIN_NATIVE_PROP_SETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				_this->_primitive->setRotate(*param);
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(rotate)

		TJS_BEGIN_NATIVE_PROP_DECL(scale)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				if (result) {
					*result = _this->_primitive->getScale();
				}
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER

			TJS_BEGIN_NATIVE_PROP_SETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_AGGPrimitive);
				_this->_primitive->setScale(*param);
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(scale)

	TJS_END_NATIVE_MEMBERS

	return classobj;
}

/**
 * ���C���I�u�W�F�N�g���� Anti-Grain Geometry �p�l�C�e�B�u�C���X�^���X���擾����B
 * �l�C�e�B�u�C���X�^���X�������ĂȂ��ꍇ�͎����I�Ɋ��蓖�Ă�
 * @param objthis ���C���I�u�W�F�N�g
 * @return �Y���v���~�e�B�u�� AGGPrimitive �B�擾���s������ NULL
 */
AGGPrimitive *
getAGGPrimitive(iTJSDispatch2 *obj)
{
	if (!obj) return NULL;
	NI_AGGPrimitive *_this;
	if (TJS_FAILED(obj->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
											  TJS_NATIVE_CLASSID_NAME, (iTJSNativeInstance**)&_this))) {
		return NULL;
	}
	return _this->_primitive;
}


#undef TJS_NATIVE_CLASSID_NAME


//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}

//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
	// �X�^�u�̏�����(�K���L�q����)
	TVPInitImportStub(exporter);

	TVPAddImportantLog(ttstr(copyright));
	
	// �^���X�g���^�}�b�v�ɕϊ�
	{
		list<RegistType*>::iterator i = typeList->begin();
		while (i != typeList->end()) {
			typeMap[ttstr((*i)->getTypeName())] = *i;
			i++;
		}
	}

	// �N���X�I�u�W�F�N�g�`�F�b�N
	if ((NI_LayerExBase::classId = TJSFindNativeClassID(L"LayerExBase")) <= 0) {
		NI_LayerExBase::classId = TJSRegisterNativeClass(L"LayerExBase");
	}
	
	// �N���X�I�u�W�F�N�g�o�^
	ClassID_AntiGrainGeometry = TJSRegisterNativeClass(TJS_W("AntiGrainGeometry"));

	{
		// TJS �̃O���[�o���I�u�W�F�N�g���擾����
		iTJSDispatch2 * global = TVPGetScriptDispatch();

		// Layer �N���X�I�u�W�F�N�g���擾
		tTJSVariant varScripts;
		TVPExecuteExpression(TJS_W("Layer"), &varScripts);
		iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
		if (dispatch) {
			// �v���p�e�B������
			NI_LayerExBase::init(dispatch);

			// ��p���\�b�h�̒ǉ�
			addMember(dispatch, L"aggSetPos", new tSetPosFunction());
			addMember(dispatch, L"aggRotate", new tRotateProp());
			addMember(dispatch, L"aggScale",  new tScaleProp());
			addMember(dispatch, L"aggX",      new tXProp());
			addMember(dispatch, L"aggY",      new tYProp());
		}

		// AGGPrimitive �^�o�^
		addMember(global, L"AGGPrimitive", Create_NC_AGGPrimitive());
		
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
extern "C" HRESULT _stdcall _export V2Unlink()
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
	if (global) {
		// TJS ���̂����ɉ������Ă����Ƃ��Ȃǂ�
		// global �� NULL �ɂȂ蓾��̂� global �� NULL �łȂ�
		// ���Ƃ��`�F�b�N����
		global->Release();
	}


	// �X�^�u�̎g�p�I��(�K���L�q����)
	TVPUninitImportStub();

	return S_OK;
}
