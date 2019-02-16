#ifndef __TJSOBJ_H__
#define __TJSOBJ_H__

#include "tjsbase.h"

/**
 * �g���g���p�I�u�W�F�N�g��P���ێ�����JavaScript�p�N���X���
 */
class TJSObject : public TJSBase {

public:
	// �������p
	static void init(Isolate *isolate);
	static void done(Isolate *isolate);

	// �I�u�W�F�N�g����
	static Local<Object> toJSObject(Isolate *isolate, const tTJSVariant &variant);

private:
	// �I�u�W�F�N�g��`
	static Persistent<ObjectTemplate> objectTemplate;

	template<class T>
	static void release(const WeakCallbackData<T,TJSObject>& data) {
		delete data.GetParameter();
	}
	
	// �A�N�Z�X�p���\�b�h
	static void getter(Local<String> property, const PropertyCallbackInfo<Value>& info);
	static void setter(Local<String> property, Local<Value> value, const PropertyCallbackInfo<Value>& info);
	static void caller(const FunctionCallbackInfo<Value>& info);

	// �i�[���R���X�g���N�^
	TJSObject(Isolate *isolate, Local<Object> &obj, const tTJSVariant &variant);
};

#endif