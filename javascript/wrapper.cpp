/**
 * Javascript ���� �g���g���u���b�W����
 * �g���g���̃I�u�W�F�N�g�� XXXX �Ƃ��ĊǗ�����
 */

#include "tjsobj.h"
#include "tjsinstance.h"

// �l�̊i�[�E�擾�p
Local<Value> toJSValue(Isolate *isolate, const tTJSVariant &variant);
tTJSVariant toVariant(Isolate *isolate, Local<Value> &value);

#define JSOBJECTCLASS L"JavascriptObject"

/**
 * Javascript object �p iTJSDispatch2 ���b�p�[
 */
class iTJSDispatch2Wrapper : public tTJSDispatch
{
public:
	/**
	 * �R���X�g���N�^
	 * @param obj IDispatch
	 */
	iTJSDispatch2Wrapper(Isolate *isolate, Local<Object> &obj) : isolate(isolate) {
		this->obj.Reset(isolate,obj);
	}
	
	/**
	 * �f�X�g���N�^
	 */
	~iTJSDispatch2Wrapper() {
		obj.Reset();
	}

	/**
	 * �ێ����Ă�l��Ԃ�
	 */
	Local<Object> getObject() {
		return Local<Object>::New(isolate, obj);
	}

public:
	// �I�u�W�F�N�g����
	tjs_error TJS_INTF_METHOD CreateNew(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		iTJSDispatch2 **result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis) {
		HandleScope handle_scope(isolate);
		return TJSInstance::createMethod(isolate, getObject(), membername, result, numparams, param);
	}

	// ���\�b�h�Ăяo��
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams,
		tTJSVariant **param,
		iTJSDispatch2 *objthis
		) {
		HandleScope handle_scope(isolate);
		return TJSInstance::callMethod(isolate, getObject(), membername, result, numparams, param, objthis);
	}

	// �v���p�e�B�擾
	tjs_error TJS_INTF_METHOD PropGet(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		tTJSVariant *result,
		iTJSDispatch2 *objthis) {
		if (!membername) {
			return TJS_E_NOTIMPL;
		}
		HandleScope handle_scope(isolate);
		return TJSInstance::getProp(isolate, getObject(), membername, result);
	}

	// �v���p�e�B�ݒ�
	tjs_error TJS_INTF_METHOD PropSet(
		tjs_uint32 flag,
		const tjs_char *membername,
		tjs_uint32 *hint,
		const tTJSVariant *param,
		iTJSDispatch2 *objthis) {
		HandleScope handle_scope(isolate);
		return TJSInstance::setProp(isolate, getObject(), membername, param);
	}

	// �����o�폜
	tjs_error TJS_INTF_METHOD DeleteMember(
		tjs_uint32 flag, const tjs_char *membername, tjs_uint32 *hint,
		iTJSDispatch2 *objthis) {
		HandleScope handle_scope(isolate);
		return TJSInstance::remove(isolate, getObject(), membername);
	}

	tjs_error TJS_INTF_METHOD IsInstanceOf(
		tjs_uint32 flag,
		const tjs_char * membername,
		tjs_uint32 *hint,
		const tjs_char * classname,
		iTJSDispatch2 *objthis
		) {
		if (membername == NULL && wcscmp(classname, JSOBJECTCLASS) == 0) {
			return TJS_S_TRUE;
		}
		return TJS_S_FALSE;
	}

protected:
	/// �����ێ��p
	Persistent<Object> obj;
	Isolate *isolate;
};

//----------------------------------------------------------------------------
// �ϊ��p
//----------------------------------------------------------------------------

/**
 * tTJSVariant �� squirrel �̋�Ԃɓ�������
 * @param result javascrpt value
 * @param variant tTJSVariant
 */
Local<Value>
toJSValue(Isolate *isolate, const tTJSVariant &variant)
{
	switch (variant.Type()) {
	case tvtVoid:
		return Undefined(isolate);
	case tvtObject:
		{
			iTJSDispatch2 *obj = variant.AsObjectNoAddRef();
			if (obj == NULL) {
				// NULL�̏���
				return Null(isolate);
			} else if (obj->IsInstanceOf(0, NULL, NULL, JSOBJECTCLASS, obj) == TJS_S_TRUE) {
				// Javascript ���b�s���O�I�u�W�F�N�g�̏ꍇ
				iTJSDispatch2Wrapper *wobj = (iTJSDispatch2Wrapper*)obj;
				return wobj->getObject();
			} else {
				Local<Object> result;
				if (TJSInstance::getJSObject(result, variant)) {
					// �o�^�ς݃C���X�^���X�̏ꍇ
					return result;
				}
				// �P�����b�s���O
				return TJSObject::toJSObject(isolate, variant);
			}
		}
		break;
	case tvtString:
		{
			const tjs_char *str = variant.GetString();
			return String::NewFromTwoByte(isolate, str ? str : L"");
		}
	case tvtOctet:
		return Null(isolate);
	case tvtInteger:
	case tvtReal:
		return Number::New(isolate, (tTVReal)variant);
	}
	return Undefined(isolate);
}

tTJSVariant
toVariant(Isolate *isolate, Local<Object> &object, Local<Object> &context)
{
	tTJSVariant result;
	iTJSDispatch2 *tjsobj = new iTJSDispatch2Wrapper(isolate, object);
	iTJSDispatch2 *tjsctx = new iTJSDispatch2Wrapper(isolate, context);
	if (tjsobj && tjsctx) {
		result = tTJSVariant(tjsobj, tjsctx);
		tjsobj->Release();
		tjsctx->Release();
	} else {
		if (tjsobj) { tjsobj->Release(); };
		if (tjsctx) { tjsctx->Release(); };
	}
	return result;
}

tTJSVariant
toVariant(Isolate *isolate, Local<Object> &object)
{
	tTJSVariant result;
	iTJSDispatch2 *tjsobj = new iTJSDispatch2Wrapper(isolate, object);
	if (tjsobj) {
		result = tTJSVariant(tjsobj, tjsobj);
		tjsobj->Release();
	}
	return result;
}

/**
 * javascript�l�� tTJSVariant �ɕϊ�����
 * @param value Javascript�l
 * @return tTJSVariant
 */
tTJSVariant
toVariant(Isolate *isolate, Local<Value> &value)
{
	tTJSVariant result;
	if (value->IsNull()) {
		result = (iTJSDispatch2*)0;
	} else if (value->IsTrue()) {
		result = true;
	} else if (value->IsFalse()) {
		result = false;
	} else if (value->IsString()) {
		String::Value str(value);
		result = *str;
	} else if (value->IsFunction() || value->IsArray() || value->IsDate()) {
		// �P�����b�s���O
		result = toVariant(isolate, value->ToObject());
	} else if (value->IsObject()) {
		HandleScope handle_scope(isolate);
		Local<Object> obj = value->ToObject();
		if (!TJSBase::getVariant(isolate, result, obj)) {
			// �P�����b�s���O
			result = toVariant(isolate, obj);
		}
	} else if (value->IsBoolean()) {
		result = value->BooleanValue();
	} else if (value->IsNumber()) {
		result = value->NumberValue();
	} else if (value->IsInt32()) {
		result = value->Int32Value();
	} else if (value->IsUint32()) {
		result = (tTVInteger)value->Uint32Value();
	}
	// value->IsUndefined()
	// value->IsExternal()
	return result;
}
