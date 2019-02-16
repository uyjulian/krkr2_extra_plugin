#include <windows.h>
#include "tp_stub.h"
#include "sqtjsobj.h"

// sqwrapper.cpp ���
extern void sq_pushvariant(HSQUIRRELVM v, tTJSVariant &variant);
extern SQRESULT sq_getvariant(HSQUIRRELVM v, int idx, tTJSVariant *result);
extern SQRESULT ERROR_KRKR(HSQUIRRELVM v, tjs_error error);
extern void SQEXCEPTION(HSQUIRRELVM v);

#include <sqfunc.h>

// �^���
static const SQChar *tjsClassAttrName = _SC("tjsClass");

/**
 * �����o�o�^�����p
 */
class MemberRegister : public tTJSDispatch /** EnumMembers �p */
{

public:
	// �R���X�g���N�^
	MemberRegister(HSQUIRRELVM v, tTJSVariant &tjsClassObj) : v(v), tjsClassObj(tjsClassObj) {
	};

	// EnumMember�p�J��Ԃ����s��
	// param[0] �����o��
	// param[1] �t���O
	// param[2] �����o�̒l
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
			if (param[2]->Type() == tvtObject) {
				const tjs_char *name = param[0]->GetString();
				tTVInteger flag = param[1]->AsInteger();
				bool staticMember = (flag & TJS_STATICMEMBER) != 0;
				iTJSDispatch2 *member = param[2]->AsObjectNoAddRef();
				if (member) {
					if (TJS_SUCCEEDED(member->IsInstanceOf(0,NULL,NULL,L"Function",NULL))) {
						registerFunction(name, *param[2], staticMember);
					} else if (TJS_SUCCEEDED(member->IsInstanceOf(0,NULL,NULL,L"Property",NULL))) {
						registerProperty(name, *param[2], staticMember);
					}
				} else {
					registNull(name);
				}
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}

protected:

	// null �o�^
	void registNull(const tjs_char *functionName) {
		sq_pushstring(v, functionName, -1);
		sq_pushnull(v);
		sq_createslot(v, -3);
	}

	// �t�@���N�V�����o�^
	void registerFunction(const tjs_char *functionName, tTJSVariant &function, bool staticMember) {
		sq_pushstring(v, functionName, -1);
		sq_pushvariant(v, function);
		if (staticMember) {
			sq_pushvariant(v, tjsClassObj);
			sq_newclosure(v, TJSObject::tjsStaticInvoker, 2);
			sq_newslot(v, -3, SQTrue);
		} else {
			sq_newclosure(v, TJSObject::tjsInvoker, 1);
			sq_newslot(v, -3, SQFalse);
		}
	}

	// �v���p�e�B�o�^
	void registerProperty(const tjs_char *propertyName, tTJSVariant &property, bool staticMember) {

		ttstr name = L"set";
		name += toupper(*propertyName);
		name += (propertyName + 1);
		sq_pushstring(v, name.c_str(), -1);
		sq_pushvariant(v, property);
		if (staticMember) {
			sq_pushvariant(v, tjsClassObj);
			sq_newclosure(v, TJSObject::tjsStaticSetter, 2);
			sq_newslot(v, -3, SQTrue);
		} else {
			sq_newclosure(v, TJSObject::tjsSetter, 1);
			sq_newslot(v, -3, SQFalse);
		}

		name = L"get";
		name += toupper(*propertyName);
		name += (propertyName + 1);
		sq_pushstring(v, name.c_str(), -1);
		sq_pushvariant(v, property);
		if (staticMember) {
			sq_pushvariant(v, tjsClassObj);
			sq_newclosure(v, TJSObject::tjsStaticGetter, 2);
			sq_newslot(v, -3, SQTrue);
		} else {
			sq_newclosure(v, TJSObject::tjsGetter, 1);
			sq_newslot(v, -3, SQFalse);
		}
	}
	
private:
	HSQUIRRELVM v;
	tTJSVariant &tjsClassObj;
};

// -----------------------------------------------------------------------

// TJSObject�o�^�p�̃N���XID
int TJSObject::classId;

// �������p
void
TJSObject::init(HSQUIRRELVM vm)
{
	// squirrel �C���X�^���X�� TJS���ŕێ����邽�߂�ID���擾
	classId = TJSRegisterNativeClass(L"SquirrelClass");
	
	// TJSObject�N���X���`
	SQTemplate<TJSObject, sqobject::Object> cls(vm);

	// ���\�b�h�o�^
	sq_pushroottable(vm);
	sq_pushstring(vm, _SC("createTJSClass"), -1);
	sq_newclosure(vm, createTJSClass, 0);
	sq_createslot(vm, -3);
	sq_pushstring(vm, _SC("tjsNull"), -1);
	sq_pushuserpointer(vm, NULL);
	sq_createslot(vm, -3);
	sq_pop(vm, 1);
}

// �������p
void
TJSObject::done(HSQUIRRELVM vm)
{
	// �N���X�Q�Ƃ����
	SQClassType<TJSObject>::done(vm);
}

/**
 * �N���X�̓o�^
 * @param HSQUIRRELVM v
 */
SQRESULT
TJSObject::createTJSClass(HSQUIRRELVM v)
{
	SQInteger top = sq_gettop(v);
	if (top < 2) {
		return sq_throwerror(v, _SC("invalid param"));
	}

	// �N���X�𐶐�
	sq_pushobject(v, SQClassType<TJSObject>::ClassObject());
	sq_newclass(v, true); // �p������

	// �����o�o�^
	const tjs_char *tjsClassName = NULL;
	tTJSVariant tjsClassObj;
	for (SQInteger i=top;i>1;i--) {
		if ((tjsClassName = sqobject::getString(v,i))) {
			TVPExecuteExpression(tjsClassName, &tjsClassObj);
			if (tjsClassObj.Type() == tvtObject &&
				TJS_SUCCEEDED(tjsClassObj.AsObjectClosureNoAddRef().IsInstanceOf(0,NULL,NULL,L"Class",NULL))) {
				MemberRegister *r = new MemberRegister(v, tjsClassObj);
				tTJSVariantClosure closure(r);
				tjsClassObj.AsObjectClosureNoAddRef().EnumMembers(TJS_IGNOREPROP, &closure, NULL);
				r->Release();
			}
		}
	}

	if (tjsClassName) {
		// �R���X�g���N�^�o�^
		sq_pushstring(v, _SC("constructor"), -1);
		sq_pushvariant(v, tjsClassObj);
		sq_newclosure(v, tjsConstructor, 1);
		sq_createslot(v, -3);
		// �N���X������ tjs�N���X��o�^
		sq_pushnull(v);
		sq_newtable(v);
		if (SQ_SUCCEEDED(sq_setattributes(v,-3))) {
			sq_pop(v,1);
			sq_pushnull(v);
			if (SQ_SUCCEEDED(sq_getattributes(v, -2))) {
				sq_pushstring(v, tjsClassAttrName, -1);
				sq_pushvariant(v, tjsClassObj);
				if (SQ_SUCCEEDED(sq_createslot(v, -3))) {
					sq_pop(v,1);
				} else {
					sq_pop(v,2);
				}
			} else {
				// XXX
				sq_pop(v,1);
			}
		} else {
			// XXX
			sq_pop(v,2);
		}
		
		// TJS�@�\���\�b�h��o�^
		sq_pushstring(v, _SC("tjsIsValid"), -1);
		sq_newclosure(v, TJSObject::tjsIsValid, 0);
		sq_createslot(v, -3);
		sq_pushstring(v, _SC("tjsOverride"), -1);
		sq_newclosure(v, TJSObject::tjsOverride, 0);
		sq_setparamscheck(v, -2, _SC(".sc"));
		sq_createslot(v, -3);
	}

	return 1;
}

/**
 * squirrel ����g���g���I�u�W�F�N�g���擾
 */
bool
TJSObject::getVariant(HSQUIRRELVM v, SQInteger idx, tTJSVariant *variant)
{
	if (sq_gettype(v, idx) == OT_CLASS) {
		if (idx < 0) {
			idx = sq_gettop(v) + 1 + idx;
		}
		bool ret = false;
		// �N���X��������I�u�W�F�N�g���������o��
		sq_pushnull(v);
		if (SQ_SUCCEEDED(sq_getattributes(v, idx))) {
			sq_pushstring(v, tjsClassAttrName, -1);
			if (SQ_SUCCEEDED(sq_get(v,-2))) {
				if (SQ_SUCCEEDED(sq_getvariant(v,-1, variant))) {
					ret = true;
				}
				sq_pop(v,1);
			}
			sq_pop(v,1);
		} else {
			// XXX
			sq_pop(v,1);
		}
		return ret;
	} else if (sq_gettype(v, idx) == OT_INSTANCE) {
		TJSObject *obj = SQClassType<TJSObject>::getInstance(v, idx);
		if (obj && obj->instance.AsObjectClosureNoAddRef().IsValid(0, NULL, NULL, NULL) == TJS_S_TRUE) {
			*variant = obj->instance;
			return true;
		}
	}
	return false;
}

/**
 * �g���g���I�u�W�F�N�g�� squirrel �ɓo�^�Bsquirrel ���Ő������ꂽ�I�u�W�F�N�g�̏ꍇ�͌��̃I�u�W�F�N�g�������̂܂ܕԂ�
 * @return �o�^����
 */
bool
TJSObject::pushVariant(HSQUIRRELVM v, tTJSVariant &variant)
{
	// �o�^�ς݃C���X�^���X���ǂ����̔���
	iTJSNativeInstance *ninstance;
	if (TJS_SUCCEEDED(variant.AsObjectNoAddRef()->NativeInstanceSupport(TJS_NIS_GETINSTANCE, classId, &ninstance))) {
		// ���X squirrel ������o�^���ꂽ�I�u�W�F�N�g�̏ꍇ�͌��� squirrel �I�u�W�F�N�g�������̂܂ܕԂ�
		TJSObject *self = (TJSObject*)ninstance;
		self->self.push(v);
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------


//---------------------------------------------------------------------------
// missing�֐�
//---------------------------------------------------------------------------
class tMissingFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
									   tTJSVariant *result,
									   tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		return TJSObject::missing(flag, membername, hint, result, numparams, param, objthis);
	};
};

/**
 * missing �����p�̌�
 * TJS�C���X�^���X�Ƀ����o�����݂��Ȃ������ꍇ�� squirrel�C���X�^���X���Q�Ƃ���
 */
tjs_error TJSObject::missing(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
							 tTJSVariant *result,
							 tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
	
	if (numparams < 3) {return TJS_E_BADPARAMCOUNT;};

	// �o�C���h����Ă��� squirrel �I�u�W�F�N�g���擾
	iTJSNativeInstance *ninstance;
	if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE, classId, &ninstance))) {
		TJSObject *self = (TJSObject*)ninstance;

		bool ret = false;
		HSQUIRRELVM gv = sqobject::getGlobalVM();
		if (!(int)*params[0]) { // get
			self->self.push(gv); // self
			sq_pushstring(gv, params[1]->GetString(), -1); // key
			if (SQ_SUCCEEDED(sq_get(gv, -2))) {
				tTJSVariant value;
				if (SQ_SUCCEEDED(sq_getvariant(gv, -1, &value))) {
					params[2]->AsObjectClosureNoAddRef().PropSet(0, NULL, NULL, &value, NULL);
					ret = true;
				}
				sq_pop(gv, 1); // value
			}
			sq_pop(gv, 1); // self
		} else { // set
			self->self.push(gv); // self
			sq_pushstring(gv, params[1]->GetString(), -1); // key
			sq_pushvariant(gv, *params[2]);    // value
			if (SQ_SUCCEEDED(sq_set(gv, -3))) {
				ret = true;
			}
			sq_pop(gv,1); // self
		}
		if (result) {
			*result = ret;
		}
	}
	return TJS_E_NATIVECLASSCRASH;
}

//---------------------------------------------------------------------------
// callSQ�֐�
//---------------------------------------------------------------------------
class tCallSQFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
									   tTJSVariant *result,
									   tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		return TJSObject::call(flag, membername, hint, result, numparams, param, objthis);
	};
};

/**
 * call �����p�̌�
 * TJS�C���X�^���X����squirrel�C���X�^���X�̃��\�b�h�𒼐ڌĂяo��
 */
tjs_error
TJSObject::call(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
				tTJSVariant *result,
				tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 1) {return TJS_E_BADPARAMCOUNT;};
	// �o�C���h����Ă��� squirrel �I�u�W�F�N�g���擾
	iTJSNativeInstance *ninstance;
	if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE, classId, &ninstance))) {
		TJSObject *self = (TJSObject*)ninstance;
		HSQUIRRELVM gv = sqobject::getGlobalVM();
		self->self.push(gv); // this
		sq_pushstring(gv, param[0]->GetString(), -1); // methodName
		if (SQ_SUCCEEDED(sq_get(gv, -2))) {
			self->self.push(gv); // this
			for (int i=1;i<numparams;i++) {	// ����
				sq_pushvariant(gv, *param[i]);
			}
			if (SQ_SUCCEEDED(sq_call(gv, numparams, result ? SQTrue:SQFalse, SQTrue))) {
				if (result) {
					sq_getvariant(gv, -1, result);
					sq_pop(gv, 1); // result
				}
				sq_pop(gv, 2); // func, this
				return TJS_S_OK;
			} else {
				sq_pop(gv, 2); // func, this
				SQEXCEPTION(gv);
			}
		} else {
			sq_pop(gv, 1); // this
			SQEXCEPTION(gv);
		}
	}
	return TJS_E_NATIVECLASSCRASH;
}


/**
 * �R���X�g���N�^
 */
TJSObject::TJSObject(HSQUIRRELVM v, int idx, tTJSVariant &instance) : instance(instance)
{
	initSelf(v, idx);
	iTJSDispatch2 *objthis = instance.AsObjectNoAddRef();

	// TJS�C���X�^���X�Ƀl�C�e�B�u�C���X�^���X�Ƃ��ēo�^���Ă���
	iTJSNativeInstance *ninstance = this;
	objthis->NativeInstanceSupport(TJS_NIS_REGISTER, classId, &ninstance);

	// callSQ ���\�b�h�o�^
	tCallSQFunction *callSQ = new tCallSQFunction();
	if (callSQ) {
		tTJSVariant val(callSQ, objthis);
		objthis->PropSet(TJS_MEMBERENSURE, TJS_W("callSQ"), NULL, &val, objthis);
		callSQ->Release();
	}
	
	// missing ���\�b�h�o�^
	tMissingFunction *missing = new tMissingFunction();
	if (missing) {
		tTJSVariant val(missing, objthis);
		const tjs_char *missingName = TJS_W("missing");
		objthis->PropSet(TJS_MEMBERENSURE, missingName, NULL, &val, objthis);
		missing->Release();
		// missing �L����
		tTJSVariant name(missingName);
		objthis->ClassInstanceInfo(TJS_CII_SET_MISSING, 0, &name);
	}
}

static bool TJS_USERENTRY Catch(void * data, const tTVPExceptionDesc & desc) {
	return false;
}

// �f�X�g���N�^
TJSObject::~TJSObject()
{
}

static void TJS_USERENTRY TryInvalidate(void * data) {
	tTJSVariant *v = (tTJSVariant*)data;
	v->AsObjectClosureNoAddRef().Invalidate(0, NULL, NULL, NULL);
}

void
TJSObject::invalidate()
{
	// TJS�I�u�W�F�N�g��j�󂵂ĎQ�Ƃ��N���A����
	// ����ɂ��ATJS���ŉ�����������肱�̃I�u�W�F�N�g���̂�
	// �l�C�e�B�u�C���X�^���X�̃N���A�����Ŕj�������
	if (instance.Type() == tvtObject && instance.AsObjectClosureNoAddRef().IsValid(0, NULL, NULL, NULL) == TJS_S_TRUE) {
		TVPDoTryBlock(TryInvalidate, Catch, NULL, (void *)&instance);
	}
	instance.Clear();
}

/**
 * �I�u�W�F�N�g�̃����[�T
 */
SQRESULT
TJSObject::release(SQUserPointer up, SQInteger size)
{
	TJSObject *self = (TJSObject*)up;
	if (self) {
		self->destructor();
		self->invalidate();
	}
	return SQ_OK;
}

// ---------------------------
// NativeInstance �Ή��p�����o
// ---------------------------

// �������ĂѕԂ�
tjs_error TJS_INTF_METHOD
TJSObject::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	return TJS_S_OK;
}

// Invalidate���ĂѕԂ�
void TJS_INTF_METHOD
TJSObject::Invalidate()
{
}

// �j�����ĂѕԂ�
void TJS_INTF_METHOD
TJSObject::Destruct()
{
	delete this;
}

// TJS�̗�O����Ăяo�������p
class FuncInfo {

public:
	// �R���X�g���N�^
	FuncInfo(HSQUIRRELVM v, tTJSVariant &instance, int offset=0) : v(v), instance(instance), argc(0), args(NULL), result(SQ_OK) {
		// ��������
		if (offset > 0) {
			argc = (tjs_int)(sq_gettop(v) - offset);
			if (argc > 0) {
				args = new tTJSVariant*[(size_t)argc];
				for (tjs_int i=0;i<argc;i++) {
					args[i] = new tTJSVariant();
					sq_getvariant(v, 2+i, args[i]);
				}
			}
		}
	}

	// �f�X�g���N�^
	~FuncInfo() {
		// �����j��
		if (args) {
			for (int i=0;i<argc;i++) {
				delete args[i];
			}
			delete[] args;
		}
	}
	
	SQRESULT create(tTJSVariant &target) {
		TVPDoTryBlock(TryCreate, Catch, Finally, (void *)this);
		target = r;
		return result;
	}

	SQRESULT exec() {
		TVPDoTryBlock(TryExec, Catch, Finally, (void *)this);
		return result;
	}

	SQRESULT setter() {
		TVPDoTryBlock(TrySetter, Catch, Finally, (void *)this);
		return result;
	}

	SQRESULT getter() {
		TVPDoTryBlock(TryGetter, Catch, Finally, (void *)this);
		return result;
	}

private:

	void _TryCreate() {
		tjs_error error;
		iTJSDispatch2 *newinstance;
		if (TJS_SUCCEEDED(error = instance.AsObjectClosureNoAddRef().CreateNew(0, NULL, NULL, &newinstance, argc, args, NULL))) {
			r = tTJSVariant(newinstance, newinstance);
			newinstance->Release();
			result = SQ_OK;
		} else {
			result = ERROR_KRKR(v, error);
		}
	}

	static void TJS_USERENTRY TryCreate(void * data) {
		FuncInfo *info = (FuncInfo*)data;
		info->_TryCreate();
	}

	void _TryExec() {
		tjs_error error;
		tTJSVariant method;
		if (SQ_SUCCEEDED(result = sq_getvariant(v, -1, &method))) {
			if (TJS_SUCCEEDED(error = method.AsObjectNoAddRef()->FuncCall(0, NULL, NULL, &r, argc, args, instance.AsObjectNoAddRef()))) {
				if (r.Type() != tvtVoid) {
					sq_pushvariant(v, r);
					result = 1;
				} else {
					result = 0;
				}
			} else {
				result = ERROR_KRKR(v, error);
			}
		}
	}
	
	static void TJS_USERENTRY TryExec(void * data) {
		FuncInfo *info = (FuncInfo*)data;
		info->_TryExec();
	}

	void _TrySetter() {
		sq_getvariant(v, 2, &r);
		tjs_error error;
		tTJSVariant prop;
		if (SQ_SUCCEEDED(result = sq_getvariant(v, -1, &prop))) {
			if (TJS_SUCCEEDED(error = prop.AsObjectNoAddRef()->PropSet(TJS_MEMBERENSURE, NULL, NULL, &r, instance.AsObjectNoAddRef()))) {
				result = SQ_OK;
			} else {
				result = ERROR_KRKR(v, error);
			}
		}
	}
	
	static void TJS_USERENTRY TrySetter(void * data) {
		FuncInfo *info = (FuncInfo*)data;
		info->_TrySetter();
	}
	
	void _TryGetter() {
		tjs_error error;
		tTJSVariant prop;
		if (SQ_SUCCEEDED(result = sq_getvariant(v, -1, &prop))) {
			if (TJS_SUCCEEDED(error = prop.AsObjectNoAddRef()->PropGet(0, NULL, NULL, &r, instance.AsObjectNoAddRef()))) {
				sq_pushvariant(v, r);
				result = 1;
			} else {
				result = ERROR_KRKR(v, error);
			}
		}
	}
	
	static void TJS_USERENTRY TryGetter(void * data) {
		FuncInfo *info = (FuncInfo*)data;
		info->_TryGetter();
	}

	static bool TJS_USERENTRY Catch(void * data, const tTVPExceptionDesc & desc) {
		FuncInfo *info = (FuncInfo*)data;
		info->result = sq_throwerror(info->v, desc.message.c_str());
		// ��O�͏�ɖ���
		return false;
	}

	static void TJS_USERENTRY Finally(void * data) {
	}

private:
	HSQUIRRELVM v;
	tTJSVariant &instance;
	tjs_int argc;
	tTJSVariant **args;
	tTJSVariant r;
	SQRESULT result;
};

/**
 * TJS�I�u�W�F�N�g�̃R���X�g���N�^
 * ����1 �I�u�W�F�N�g
 * ����2�` ����
 * ���R�ϐ�1 TJS�N���X�I�u�W�F�N�g
 */
SQRESULT
TJSObject::tjsConstructor(HSQUIRRELVM v)
{
	// �N���X�𐶐�����
	tTJSVariant tjsClassObj;
	if (SQ_SUCCEEDED(sq_getvariant(v, -1, &tjsClassObj)) && tjsClassObj.Type() == tvtObject) {
		FuncInfo info(v, tjsClassObj, 2);
		tTJSVariant variant;
		SQRESULT ret = info.create(variant);
		if (SQ_SUCCEEDED(ret)) {
			TJSObject *self = new TJSObject(v, 1, variant);
			if (SQ_SUCCEEDED(sq_setinstanceup(v, 1, self))) {
				sq_setreleasehook(v, 1, release);
			} else {
				delete self;
			}
		}
		return ret;
	}
	return ERROR_CREATE(v);
}

/**
 * TJS�I�u�W�F�N�g�p�̃��\�b�h
 * ����1 �I�u�W�F�N�g
 * ����2�` ����
 * ���R�ϐ�1 ���\�b�h
 */
SQRESULT
TJSObject::tjsInvoker(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		FuncInfo info(v, instance, 2);
		return info.exec();
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * TJS�I�u�W�F�N�g�p�̃v���p�e�B�Q�b�^�[
 * ����1 �I�u�W�F�N�g
 * ���R�ϐ�1 �v���p�e�B
 */
SQRESULT
TJSObject::tjsGetter(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		FuncInfo info(v, instance);
		return info.getter();
	} 
	return ERROR_BADINSTANCE(v);
}

/**
 * TJS�I�u�W�F�N�g�p�̃v���p�e�B�Z�b�^�[
 * ����1 �I�u�W�F�N�g
 * ����2 �ݒ�l
 * ���R�ϐ�1 �v���p�e�B
 */
SQRESULT
TJSObject::tjsSetter(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		FuncInfo info(v, instance);
		return info.setter();
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * TJS�I�u�W�F�N�g�p�̐ÓI���\�b�h
 * ����1 �I�u�W�F�N�g
 * ����2�` ����
 * ���R�ϐ�1 �N���X�I�u�W�F�N�g
 * ���R�ϐ�2 ���\�b�h
 */
SQRESULT
TJSObject::tjsStaticInvoker(HSQUIRRELVM v)
{
	tTJSVariant tjsClassObj;
	if (SQ_SUCCEEDED(sq_getvariant(v, -2, &tjsClassObj)) && tjsClassObj.Type() == tvtObject) {
		FuncInfo info(v, tjsClassObj, 3);
		return info.exec();
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * TJS�I�u�W�F�N�g�p�̐ÓI�v���p�e�B�Q�b�^�[
 * ����1 �I�u�W�F�N�g
 * ���R�ϐ�1 �N���X�I�u�W�F�N�g
 * ���R�ϐ�2 �v���p�e�B
 */
SQRESULT
TJSObject::tjsStaticGetter(HSQUIRRELVM v)
{
	tTJSVariant tjsClassObj;
	if (SQ_SUCCEEDED(sq_getvariant(v, -2, &tjsClassObj)) && tjsClassObj.Type() == tvtObject) {
		FuncInfo info(v, tjsClassObj);
		return info.getter();
	} 
	return ERROR_BADINSTANCE(v);
}
	
/**
 * TJS�I�u�W�F�N�g�p�̐ÓI�v���p�e�B�Z�b�^�[
 * ����1 �I�u�W�F�N�g
 * ����2 �ݒ�l
 * ���R�ϐ�1 �N���X�I�u�W�F�N�g
 * ���R�ϐ�2 �v���p�e�B
 */
SQRESULT
TJSObject::tjsStaticSetter(HSQUIRRELVM v)
{
	tTJSVariant tjsClassObj;
	if (SQ_SUCCEEDED(sq_getvariant(v, -2, &tjsClassObj)) && tjsClassObj.Type() == tvtObject) {
		FuncInfo info(v, tjsClassObj);
		return info.setter();
	}
	return ERROR_BADINSTANCE(v);
}

/**
 * TJS�I�u�W�F�N�g�̗L���m�F
 * ����1 �I�u�W�F�N�g
 */
SQRESULT 
TJSObject::tjsIsValid(HSQUIRRELVM v)
{
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		SQBool ret = instance.AsObjectClosureNoAddRef().IsValid(0, NULL, NULL, NULL) == TJS_S_TRUE ? SQTrue : SQFalse;
		sq_pushbool(v, ret);
		return 1;
	}
	sq_pushbool(v, SQFalse);
	return 1;
}

/**
 * TJS�I�u�W�F�N�g�̃I�[�o���C�h����
 * ����1 �I�u�W�F�N�g
 * ����2 ���O
 * ����3 �l�B�ȗ����� squirrel �C���X�^���X���疼�O�ŎQ��
 */
SQRESULT 
TJSObject::tjsOverride(HSQUIRRELVM v)
{
	SQRESULT result;
	tTJSVariant instance;
	if (getVariant(v,1,&instance) && instance.Type() == tvtObject) {
		tTJSVariant value;
		const SQChar *methodName = sqobject::getString(v,2);
		int n = sq_gettop(v);
		if (n >= 3) {
			// �����Ŗ����w�肳��Ă���
			if (SQ_FAILED(result = sq_getvariant(v, 3, &value))) {
				return result;
			}
		} else {
			// ���ȃI�u�W�F�N�g���疼�O�Ō������ēo�^
			sq_push(v, 1);
			sq_pushstring(v, methodName, -1);
			if (SQ_FAILED(result = sq_get(v, -2))) {
				sq_pop(v, 1); //self
				return result;
			} else {
				SQObjectType type = sq_gettype(v, -1);
				if (type == OT_CLOSURE || type == OT_NATIVECLOSURE) {
					// �N���[�W���̏ꍇ�� bindenv ���Ă���
					sq_push(v, 1);
					if (SQ_FAILED(result = sq_bindenv(v, -2))) {
						sq_pop(v, 2); // func, self
						return result;
					} else {
						sq_remove(v, -2); // original func
					}
				}
				if (SQ_FAILED(result = sq_getvariant(v, -1, &value))) { // value
					sq_pop(v, 1); //self
					return result;
				}
				sq_pop(v, 1); // self
			}
		}
		tjs_error error;
		if (TJS_SUCCEEDED(error = instance.AsObjectClosureNoAddRef().PropSet(TJS_MEMBERENSURE, methodName, NULL, &value, NULL))) {
			return SQ_OK;
		} else {
			return  ERROR_KRKR(v, error);
		}
	}
	return ERROR_BADINSTANCE(v);
}
