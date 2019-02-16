#include "tjsinstance.h"
#include "tjsobj.h"

extern Local<Value> toJSValue(Isolate *isolate, const tTJSVariant &variant);
extern tTJSVariant toVariant(Isolate *isolate, Local<Value> &value);
extern tTJSVariant toVariant(Isolate *isolate, Local<Object> &value);
extern tTJSVariant toVariant(Isolate *isolate, Local<Object> &object, Local<Object> &context);

extern Local<Value> ERROR_KRKR(Isolate *isolate, tjs_error error);
extern Local<Value> ERROR_BADINSTANCE(Isolate *isolate);

/**
 * �g���g���ɑ΂��ė�O�ʒm
 */
void
JSEXCEPTION(Isolate* isolate, TryCatch *try_catch)
{
	HandleScope handle_scope(isolate);

	//HandleScope handle_scope;
	String::Value exception(try_catch->Exception());

	Local<Message> message = try_catch->Message();
	if (!message.IsEmpty()) {
		// ��O�\��
		String::Value filename(message->GetScriptResourceName());
		ttstr msg;
		msg += *filename;
		msg += ":";
		msg += tTJSVariant(message->GetLineNumber());
		msg += ":";
		msg += *exception;

		TVPAddLog(msg);
		
		// Print (filename):(line number): (message).
		String::Value sourceline(message->GetSourceLine());
		TVPAddLog(ttstr(*sourceline));

		// �G���[�s�\��
		ttstr wavy;
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++) {
			wavy += " ";
		}
		int end = message->GetEndColumn();
		for (int i = start; i < end; i++) {
			wavy +="^";
		}
		TVPAddLog(wavy);

		// �X�^�b�N�g���[�X�\��
		String::Value stack_trace(try_catch->StackTrace());
		if (stack_trace.length() > 0) {
			TVPAddLog(ttstr(*stack_trace));
		}
	}
	TVPThrowExceptionMessage(*exception);
}

/**
 * �����o�o�^�����p
 */
class MemberRegister : public tTJSDispatch /** EnumMembers �p */
{
public:
	// �R���X�g���N�^
	MemberRegister(Isolate *isolate, Local<FunctionTemplate> &classTemplate) : isolate(isolate), classTemplate(classTemplate) {};
	
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
				}
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}

private:
	// �t�@���N�V�����o�^
	void registerFunction(const tjs_char *functionName, tTJSVariant &function, bool staticMember) {
		classTemplate->PrototypeTemplate()->Set(String::NewFromTwoByte(isolate, functionName), FunctionTemplate::New(isolate, TJSInstance::tjsInvoker, TJSObject::toJSObject(isolate, function)));
	}
	
	// �v���p�e�B�o�^
	void registerProperty(const tjs_char *propertyName, tTJSVariant &property, bool staticMember) {
		classTemplate->PrototypeTemplate()->SetAccessor(String::NewFromTwoByte(isolate, propertyName), TJSInstance::tjsGetter, TJSInstance::tjsSetter, TJSObject::toJSObject(isolate, property));
	}

	Isolate *isolate;
	Local<FunctionTemplate> &classTemplate;
};

// -----------------------------------------------------------------------

int TJSInstance::classId;

// �������p
void
TJSInstance::init(Isolate *isolate, Local<ObjectTemplate> &globalTemplate)
{
	HandleScope handle_scope(isolate);
	// �l�C�e�B�u�C���X�^���X�o�^�p�N���XID�L�^
	classId = TJSRegisterNativeClass(L"JavascriptClass");
	// ���\�b�h��o�^
	globalTemplate->Set(String::NewFromUtf8(isolate, "createTJSClass"), FunctionTemplate::New(isolate, createTJSClass));
}

/**
 * �g���g���N���X���� Javascript �N���X�𐶐�
 * @param args ����
 * @return ����
 */
void
TJSInstance::createTJSClass(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	if (args.Length() < 1) {
		args.GetReturnValue().Set(isolate->ThrowException(String::NewFromUtf8(isolate, "invalid param")));
		return;
	}

	// TJS�N���X���擾
	String::Value tjsClassName(args[0]);
	tTJSVariant tjsClassObj;
	TVPExecuteExpression(*tjsClassName, &tjsClassObj);
	if (tjsClassObj.Type() != tvtObject || TJS_FAILED(tjsClassObj.AsObjectClosureNoAddRef().IsInstanceOf(0,NULL,NULL,L"Class",NULL))) {
		args.GetReturnValue().Set(isolate->ThrowException(String::NewFromUtf8(isolate, "invalid param")));
		return;
	}
	
	// �N���X�e���v���[�g�𐶐�
	Local<FunctionTemplate> classTemplate = FunctionTemplate::New(isolate, tjsConstructor, TJSObject::toJSObject(isolate, tjsClassObj));
	classTemplate->SetClassName(args[0]->ToString()); // �\����
	
	// �����o�o�^����
	for (int i=args.Length()-1;i>=0;i--) {
		String::Value className(args[i]);
		tTJSVariant classObj;
		TVPExecuteExpression(*className, &classObj);
		if (classObj.Type() == tvtObject &&
			TJS_SUCCEEDED(classObj.AsObjectClosureNoAddRef().IsInstanceOf(0,NULL,NULL,L"Class",NULL))) {
			MemberRegister *caller = new MemberRegister(isolate, classTemplate);
			tTJSVariantClosure closure(caller);
			classObj.AsObjectClosureNoAddRef().EnumMembers(TJS_IGNOREPROP, &closure, NULL);
			caller->Release();
		}
	}

	// TJS�@�\���\�b�h��o�^
	Local<ObjectTemplate> protoTemplate = classTemplate->PrototypeTemplate();
	protoTemplate->Set(String::NewFromUtf8(isolate, "tjsIsValid"), FunctionTemplate::New(isolate, tjsIsValid));
	protoTemplate->Set(String::NewFromUtf8(isolate, "tjsOverride"), FunctionTemplate::New(isolate, tjsOverride));
	
	args.GetReturnValue().Set(classTemplate->GetFunction());
}

/**
 * �g���g���I�u�W�F�N�g�� javascript�I�u�W�F�N�g�ɕϊ�
 * @return �o�^����
 */
bool
TJSInstance::getJSObject(Local<Object> &result, const tTJSVariant &variant)
{
	iTJSDispatch2 *dispatch = variant.AsObjectNoAddRef();
	iTJSNativeInstance *ninstance;
	if (TJS_SUCCEEDED(dispatch->NativeInstanceSupport(TJS_NIS_GETINSTANCE, classId, &ninstance))) {
		// Javascript������o�^���ꂽ�I�u�W�F�N�g�̏ꍇ�͌��� Javascript�I�u�W�F�N�g�������̂܂ܕԂ�
		TJSInstance *self = (TJSInstance*)ninstance;
		result = self->getObject();
		return true;
	}
	return false;
}

extern Local<Context> getContext();

// �v���p�e�B�擾���ʏ���
tjs_error
TJSInstance::getProp(Isolate *isolate, Local<Object> &obj, const tjs_char *membername, tTJSVariant *result)
{
	if (!membername) {
		return TJS_E_NOTIMPL;
	}
	
	HandleScope handle_scope(isolate);
	Context::Scope context_scope(getContext());
	TryCatch try_catch;
	
	Local<Value> ret = obj->Get(String::NewFromTwoByte(isolate, membername));
	if (ret.IsEmpty()) {
		return TJS_E_MEMBERNOTFOUND;
	} else {
		if (result) {
			if (ret->IsFunction()) {
				*result = toVariant(isolate, ret->ToObject(), obj);
			} else {
				*result = toVariant(isolate, ret);
			}
		}
	}
	return TJS_S_OK;
}

// �v���p�e�B�ݒ苤�ʏ���
tjs_error
TJSInstance::setProp(Isolate *isolate, Local<Object> &obj, const tjs_char *membername, const tTJSVariant *param)
{
	if (!membername) {
		return TJS_E_NOTIMPL;
	}

	HandleScope handle_scope(isolate);
	Context::Scope context_scope(getContext());
	TryCatch try_catch;
	
	if (obj->Set(String::NewFromTwoByte(isolate, membername), toJSValue(isolate, *param))) {
		return TJS_S_OK;
	}
	return TJS_E_MEMBERNOTFOUND;
}

tjs_error
TJSInstance::remove(Isolate *isolate, Local<Object> &obj, const tjs_char *membername)
{
	if (!membername) {
		return TJS_E_NOTIMPL;
	}

	HandleScope handle_scope(isolate);
	Context::Scope context_scope(getContext());
	TryCatch try_catch;
	return obj->Delete(String::NewFromTwoByte(isolate, membername)) ? TJS_S_OK : TJS_S_FALSE;
}

// �R���X�g���N�^�Ăяo�����ʏ���
tjs_error
TJSInstance::createMethod(Isolate *isolate, Local<Object> &obj, const tjs_char *membername, iTJSDispatch2 **result, tjs_int numparams, tTJSVariant **param)
{
	if (membername) {
		return TJS_E_MEMBERNOTFOUND;
	}

	HandleScope handle_scope(isolate);
	Context::Scope context_scope(getContext());
	TryCatch try_catch;

	if (!obj->IsFunction()) {
		return TJS_E_NOTIMPL;
	}
	
	// �֐����o
	Local<Function> func = Local<Function>::Cast(obj->ToObject());
	// ����
	Handle<Value> *argv = new Handle<Value>[numparams];
	for (int i=0;i<numparams;i++) {
		argv[i] = toJSValue(isolate, *param[i]);
	}
	Local<Object> ret = func->NewInstance(numparams, argv);
	delete argv;
	
	if (ret.IsEmpty()) {
		JSEXCEPTION(isolate, &try_catch);
	} else {
		if (result) {
			*result = toVariant(isolate, ret);
		}
	}
	return TJS_S_OK;
}

// ���\�b�h�Ăяo�����ʏ���
tjs_error
TJSInstance::callMethod(Isolate *isolate, Local<Object> &obj, const tjs_char *membername, tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	HandleScope handle_scope(isolate);
	Context::Scope context_scope(getContext());
	TryCatch try_catch;

	Local<Object> context = membername ? obj : objthis ? toJSValue(isolate, tTJSVariant(objthis))->ToObject() : getContext()->Global();
	Local<Object> method  = membername ? obj->Get(String::NewFromTwoByte(isolate, membername))->ToObject() : obj;

	if (!method->IsFunction()) {
		return TJS_E_NOTIMPL;
	}
	
	// �֐����o
	Local<Function> func = Local<Function>::Cast(method);
	// ����
	Handle<Value> *argv = new Handle<Value>[numparams];
	for (int i=0;i<numparams;i++) {
		argv[i] = toJSValue(isolate, *param[i]);
	}
	Local<Value> ret = func->Call(context, numparams, argv);
	delete argv;
	
	if (ret.IsEmpty()) {
		JSEXCEPTION(isolate, &try_catch);
	} else {
		if (result) {
			*result = toVariant(isolate, ret);
		}
	}
	return TJS_S_OK;
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
		return TJSInstance::missing(flag, membername, hint, result, numparams, param, objthis);
	};
};

/**
 * missing �����p�̌�
 * TJS�C���X�^���X�Ƀ����o�����݂��Ȃ������ꍇ�� javascript�C���X�^���X���Q�Ƃ���
 */
tjs_error TJSInstance::missing(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
							 tTJSVariant *result,
							 tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
	
	if (numparams < 3) {return TJS_E_BADPARAMCOUNT;};
	iTJSNativeInstance *ninstance;
	if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE, classId, &ninstance))) {
		TJSInstance *self = (TJSInstance*)ninstance;
		HandleScope handle_scope(self->isolate);
		bool ret = false;
		if (!(int)*params[0]) { // get
			tTJSVariant result;
			if (TJS_SUCCEEDED(getProp(self->isolate, self->getObject(), params[1]->GetString(), &result))) {
				params[2]->AsObjectClosureNoAddRef().PropSet(0, NULL, NULL, &result, NULL);
				ret = true;
			}
		} else { // set
			if (TJS_SUCCEEDED(setProp(self->isolate, self->getObject(), params[1]->GetString(), params[2]))) {
				ret = true;
			}
		}
		if (result) {
			*result = ret;
		}
	}
	return TJS_E_NATIVECLASSCRASH;
}

//---------------------------------------------------------------------------
// callJS�֐�
//---------------------------------------------------------------------------
class tCallJSFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
									   tTJSVariant *result,
									   tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		return TJSInstance::call(flag, membername, hint, result, numparams, param, objthis);
	};
};

/**
 * call �����p�̌�
 * TJS�C���X�^���X����javascript�C���X�^���X�̃��\�b�h�𒼐ڌĂяo��
 */
tjs_error
TJSInstance::call(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
				tTJSVariant *result,
				tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 1) {return TJS_E_BADPARAMCOUNT;};
	iTJSNativeInstance *ninstance;
	if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE, classId, &ninstance))) {
		TJSInstance *self = (TJSInstance*)ninstance;
		HandleScope handle_scope(self->isolate);
		return callMethod(self->isolate, self->getObject(), param[0]->GetString(), result, numparams-1, param+1, objthis);
	}
	return TJS_E_NATIVECLASSCRASH;
}


/**
 * �R���X�g���N�^
 */
TJSInstance::TJSInstance(Isolate *isolate, Local<Object> &obj, const tTJSVariant &variant) : isolate(isolate), TJSBase(variant)
{
	HandleScope handle_scope(isolate);

	// Javascript �I�u�W�F�N�g�Ɋi�[
	wrap(isolate, obj);
	self.Reset(isolate, obj);
	self.SetWeak(this, release);
	
	iTJSDispatch2 *objthis = variant.AsObjectNoAddRef();

	// TJS�C���X�^���X�Ƀl�C�e�B�u�C���X�^���X�Ƃ��ēo�^���Ă���
	iTJSNativeInstance *ninstance = this;
	objthis->NativeInstanceSupport(TJS_NIS_REGISTER, classId, &ninstance);

	// callJS ���\�b�h�o�^
	tCallJSFunction *callJS = new tCallJSFunction();
	if (callJS) {
		tTJSVariant val(callJS, objthis);
		objthis->PropSet(TJS_MEMBERENSURE, TJS_W("callJS"), NULL, &val, objthis);
		callJS->Release();
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

static void TJS_USERENTRY TryInvalidate(void * data) {
	tTJSVariant *v = (tTJSVariant*)data;
	v->AsObjectClosureNoAddRef().Invalidate(0, NULL, NULL, NULL);
}

void
TJSInstance::invalidate()
{
	// TJS�I�u�W�F�N�g��j�󂵂ĎQ�Ƃ��N���A����
	// ����ɂ��ATJS���ŉ�����������肱�̃I�u�W�F�N�g���̂�
	// �l�C�e�B�u�C���X�^���X�̃N���A�����Ŕj�������
	if (variant.Type() == tvtObject && variant.AsObjectClosureNoAddRef().IsValid(0, NULL, NULL, NULL) == TJS_S_TRUE) {
		TVPDoTryBlock(TryInvalidate, Catch, NULL, (void *)&variant);
	}
	variant.Clear();
}

// ---------------------------
// NativeInstance �Ή��p�����o
// ---------------------------

// �������ĂѕԂ�
tjs_error TJS_INTF_METHOD
TJSInstance::Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
{
	return TJS_S_OK;
}

// Invalidate���ĂѕԂ�
void TJS_INTF_METHOD
TJSInstance::Invalidate()
{
}

// �j�����ĂѕԂ�
void TJS_INTF_METHOD
TJSInstance::Destruct()
{
	delete this;
}

// -----------------------------------------------------------------------------------------------------------

// TJS�̗�O����Ăяo�������p
class CreateInfo {

public:
	// �R���X�g���N�^
	CreateInfo(const tTJSVariant &classObj, const FunctionCallbackInfo<Value>& args) : classObj(classObj), args(args), argc(0), argv(NULL) {
		// ��������
		argc = args.Length();
		if (argc > 0) {
			argv = new tTJSVariant*[(size_t)argc];
			for (tjs_int i=0;i<argc;i++) {
				argv[i] = new tTJSVariant();
				*argv[i] = toVariant(args.GetIsolate(), args[i]);
			}
		}
	}

	// �f�X�g���N�^
	~CreateInfo() {
		// �����j��
		if (argv) {
			for (int i=0;i<argc;i++) {
				delete argv[i];
			}
			delete[] argv;
		}
	}
	
	Local<Value> create() {
		TVPDoTryBlock(TryCreate, Catch, Finally, (void *)this);
		return ret;
	}

private:

	void _TryCreate() {
		tjs_error error;
		iTJSDispatch2 *newinstance;
		if (TJS_SUCCEEDED(error = classObj.AsObjectClosureNoAddRef().CreateNew(0, NULL, NULL, &newinstance, argc, argv, NULL))) {
			new TJSInstance(args.GetIsolate(), args.This(), tTJSVariant(newinstance, newinstance));
			newinstance->Release();
			ret = args.This();
		} else {
			ret = ERROR_KRKR(args.GetIsolate(), error);
		}
	}

	static void TJS_USERENTRY TryCreate(void * data) {
		CreateInfo *info = (CreateInfo*)data;
		info->_TryCreate();
	}

	static bool TJS_USERENTRY Catch(void * data, const tTVPExceptionDesc & desc) {
		CreateInfo *info = (CreateInfo*)data;
		info->ret = info->args.GetIsolate()->ThrowException(String::NewFromTwoByte(info->args.GetIsolate(), desc.message.c_str()));
		// ��O�͏�ɖ���
		return false;
	}

	static void TJS_USERENTRY Finally(void * data) {
	}

private:
	const tTJSVariant &classObj;
	const FunctionCallbackInfo<Value>& args;
	tjs_int argc;
	tTJSVariant **argv;
	Local<Value> ret;
};

// TJS�̗�O����Ăяo�������p
class FuncInfo {

public:
	// �R���X�g���N�^
	FuncInfo(const tTJSVariant &instance, const tTJSVariant &method, const FunctionCallbackInfo<Value>& args) : instance(instance), method(method), args(args), argc(0), argv(NULL) {
		// ��������
		argc = args.Length();
		if (argc > 0) {
			argv = new tTJSVariant*[(size_t)argc];
			for (tjs_int i=0;i<argc;i++) {
				argv[i] = new tTJSVariant();
				*argv[i] = toVariant(args.GetIsolate(), args[i]);
			}
		}
	}

	// �f�X�g���N�^
	~FuncInfo() {
		// �����j��
		if (argv) {
			for (int i=0;i<argc;i++) {
				delete argv[i];
			}
			delete[] argv;
		}
	}
	
	Local<Value> exec() {
		TVPDoTryBlock(TryExec, Catch, Finally, (void *)this);
		return ret;
	}

private:

	void _TryExec() {
		tjs_error error;
		tTJSVariant r;
		if (TJS_SUCCEEDED(error = method.AsObjectNoAddRef()->FuncCall(0, NULL, NULL, &r, argc, argv, instance.AsObjectNoAddRef()))) {
			ret = toJSValue(args.GetIsolate(), r);
		} else {
			ret = ERROR_KRKR(args.GetIsolate(), error);
		}
	}
	
	static void TJS_USERENTRY TryExec(void * data) {
		FuncInfo *info = (FuncInfo*)data;
		info->_TryExec();
	}

	static bool TJS_USERENTRY Catch(void * data, const tTVPExceptionDesc & desc) {
		FuncInfo *info = (FuncInfo*)data;
		info->ret = info->args.GetIsolate()->ThrowException(String::NewFromTwoByte(info->args.GetIsolate(), desc.message.c_str()));
		// ��O�͏�ɖ���
		return false;
	}

	static void TJS_USERENTRY Finally(void * data) {
	}

private:
	const tTJSVariant &instance;
	const tTJSVariant &method;
	const FunctionCallbackInfo<Value>& args;
	tjs_int argc;
	tTJSVariant **argv;
	Local<Value> ret;
};

// TJS�̗�O����Ăяo�������p
class PropSetter {

public:
	// �R���X�g���N�^
	PropSetter(const tTJSVariant &instance, const tTJSVariant &method, Local<Value> value, const PropertyCallbackInfo<void>& info) : instance(instance), method(method), info(info) {
		param    = toVariant(info.GetIsolate(), value);
	}
	
	void exec() {
		TVPDoTryBlock(TrySetter, Catch, Finally, (void *)this);
	}

private:
	void _TrySetter() {
		method.AsObjectNoAddRef()->PropSet(TJS_MEMBERENSURE, NULL, NULL, &param, instance.AsObjectNoAddRef());
	}
	
	static void TJS_USERENTRY TrySetter(void * data) {
		PropSetter *info = (PropSetter*)data;
		info->_TrySetter();
	}
	
	static bool TJS_USERENTRY Catch(void * data, const tTVPExceptionDesc & desc) {
		// ��O�͏�ɖ���
		return false;
	}
	
	static void TJS_USERENTRY Finally(void * data) {
	}

private:
	const tTJSVariant &instance;
	const tTJSVariant &method;
	const PropertyCallbackInfo<void>& info;
	tTJSVariant param;
};

// TJS�̗�O����Ăяo�������p
class PropGetter {

public:
	// �R���X�g���N�^
	PropGetter(const tTJSVariant &instance, const tTJSVariant &method, const PropertyCallbackInfo<Value>& info) : instance(instance), method(method), info(info) {
	}
	
	Local<Value> exec() {
		TVPDoTryBlock(TryGetter, Catch, Finally, (void *)this);
		return ret;
	}

private:
	void _TryGetter() {
		tjs_error error;
		tTJSVariant r;
		if (TJS_SUCCEEDED(error = method.AsObjectNoAddRef()->PropGet(0, NULL, NULL, &r, instance.AsObjectNoAddRef()))) {
			ret = toJSValue(info.GetIsolate(), r);
		} else {
			ret = ERROR_KRKR(info.GetIsolate(), error);
		}
	}
	
	static void TJS_USERENTRY TryGetter(void * data) {
		PropGetter *info = (PropGetter*)data;
		info->_TryGetter();
	}

	static bool TJS_USERENTRY Catch(void * data, const tTVPExceptionDesc & desc) {
		PropGetter *info = (PropGetter*)data;
		info->ret = info->info.GetIsolate()->ThrowException(String::NewFromTwoByte(info->info.GetIsolate(), desc.message.c_str()));
		// ��O�͏�ɖ���
		return false;
	}
	
	static void TJS_USERENTRY Finally(void * data) {
	}

private:
	const tTJSVariant &instance;
	const tTJSVariant &method;
	const PropertyCallbackInfo<Value>& info;
	Local<Value> ret;
};

/**
 * TJS�I�u�W�F�N�g�̃R���X�g���N�^
 */
void
TJSInstance::tjsConstructor(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	tTJSVariant classObj;
	if (getVariant(isolate, classObj, args.Data()->ToObject())) {
		CreateInfo info(classObj, args);
		args.GetReturnValue().Set(info.create());
		return;
	}
	args.GetReturnValue().Set(ERROR_BADINSTANCE(isolate));
}

/**
 * TJS�I�u�W�F�N�g�p�̃��\�b�h
 * @param args ����
 * @return ����
 */
void
TJSInstance::tjsInvoker(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	tTJSVariant instance;
	tTJSVariant method;
	if (getVariant(isolate, instance, args.This()) && getVariant(isolate, method, args.Data()->ToObject())) {
		FuncInfo info(instance, method, args);
		args.GetReturnValue().Set(info.exec());
		return;
	}
	args.GetReturnValue().Set(ERROR_BADINSTANCE(isolate));
}

/**
 * TJS�I�u�W�F�N�g�p�̃v���p�e�B�Q�b�^�[
 * @param args ����
 */
void
TJSInstance::tjsGetter(Local<String> property, const PropertyCallbackInfo<Value>& info)
{
	Isolate *isolate = info.GetIsolate();
	HandleScope handle_scope(isolate);
	tTJSVariant instance;
	tTJSVariant method;
	if (getVariant(isolate, instance, info.This()) && getVariant(isolate, method, info.Data()->ToObject())) {
		PropGetter get(instance, method, info);
		info.GetReturnValue().Set(get.exec());
		return;
	}
	info.GetReturnValue().Set(ERROR_BADINSTANCE(isolate));
}

/**
 * TJS�I�u�W�F�N�g�p�̃v���p�e�B�Z�b�^�[
 * @param args ����
 * @return ����
 */
void
TJSInstance::tjsSetter(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info)
{
	Isolate *isolate = info.GetIsolate();
	HandleScope handle_scope(isolate);
	tTJSVariant instance;
	tTJSVariant method;
	if (getVariant(isolate, instance, info.This()) && getVariant(isolate, method, info.Data()->ToObject())) {
		PropSetter set(instance, method, value, info);
		set.exec();
	}
}

/**
 * TJS�I�u�W�F�N�g�̗L���m�F
 * @param args ����
 * @return ����
 */
void
TJSInstance::tjsIsValid(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	tTJSVariant instance;
	if (getVariant(isolate, instance, args.This())) {
		args.GetReturnValue().Set(Boolean::New(isolate, instance.AsObjectClosureNoAddRef().IsValid(0, NULL, NULL, NULL) == TJS_S_TRUE));
		return;
	}
	args.GetReturnValue().Set(ERROR_BADINSTANCE(isolate));
}

/**
 * TJS�I�u�W�F�N�g�̃I�[�o���C�h����
 * @param args ����
 * @return ����
 */
void
TJSInstance::tjsOverride(const FunctionCallbackInfo<Value>& args)
{
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	tTJSVariant instance;
	if (getVariant(isolate, instance, args.This())) {
		if (args.Length() > 0) {
			Local<Value> func = args.Length() > 1 ? args[1] : args.This()->Get(args[0]);
			if (func->IsFunction()) {
				tTJSVariant value = toVariant(isolate, func->ToObject(), args.This());
				String::Value methodName(args[0]);
				tjs_error error;
				if (TJS_FAILED(error = instance.AsObjectClosureNoAddRef().PropSet(TJS_MEMBERENSURE, *methodName, NULL, &value, NULL))) {
					args.GetReturnValue().Set(ERROR_KRKR(isolate, error));
					return;
				}
				args.GetReturnValue().Set(Undefined(isolate));
				return;
			}
		}
		args.GetReturnValue().Set(isolate->ThrowException(String::NewFromUtf8(isolate, "not function")));
		return;
	}
	args.GetReturnValue().Set(ERROR_BADINSTANCE(isolate));
}
