#include <stdio.h>
#include "ncbind/ncbind.hpp"
#include <vector>
#include <include/v8.h>
#include <include/v8-debug.h>

using namespace v8;

// �g���g���I�u�W�F�N�g�N���X����
#include "tjsobj.h"
#include "tjsinstance.h"

// javascript ��ł� TJS2�̃O���[�o����Ԃ̎Q�Ɩ�
#define KIRIKIRI_GLOBAL L"krkr"
// TJS2 ��ł� javascript �̃O���[�o����Ԃ̎Q�Ɩ�
#define JAVASCRIPT_GLOBAL L"jsglobal"

// �l�̊i�[�E�擾�p
extern Local<Value> toJSValue(Isolate *isolate, const tTJSVariant &variant);
extern tTJSVariant toVariant(Isolate *isolate, Local<Value> &value);
extern void JSEXCEPTION(Isolate *isolate, TryCatch *try_catch);

// �R�s�[���C�g�\�L
static const char *copyright =
#include "LICENSE.h"
;

//---------------------------------------------------------------------------

/**
 * Scripts �N���X�ւ� Javascript ���s���\�b�h�̒ǉ�
 */
class ScriptsJavascript {

public:
	/**
	 * �o�^�����O
	 */
	static void PreRegisterCallback() {
		// Copyright �\��
		TVPAddImportantLog(ttstr(copyright));
		V8::SetFatalErrorHandler(OnFatalError);
		getInstance();
	}
	
	/**
	 * �J��������
	 */
	static void PostUnregisterCallback() {
		destroy();
	}

	/**
	 * javascript �X�N���v�g�̎��s
	 * @param script �X�N���v�g
	 * @return ���s����
	 */
	static tjs_error TJS_INTF_METHOD exec(tTJSVariant *result,
										  tjs_int numparams,
										  tTJSVariant **param,
										  iTJSDispatch2 *objthis) {
		if (numparams <= 0) return TJS_E_BADPARAMCOUNT;
		return getInstance()->_exec(NULL, param[0]->GetString(), result);
	}

	/**
	 * javascript �X�N���v�g�̃t�@�C������̎��s
	 * @param filename �t�@�C����
	 * @param ... ����
	 * @return ���s����
	 */
	static tjs_error TJS_INTF_METHOD execStorage(tTJSVariant *result,
												 tjs_int numparams,
												 tTJSVariant **param,
												 iTJSDispatch2 *objthis) {
		if (numparams <= 0) return TJS_E_BADPARAMCOUNT;
		const tjs_char *filename = param[0]->GetString();
		iTJSTextReadStream * stream = TVPCreateTextStreamForRead(filename, TJS_W(""));
		ttstr data;
		try {
			stream->Read(data, 0);
			stream->Destruct();
		}
		catch(...)
		{
			stream->Destruct();
			throw;
		}
		return getInstance()->_exec(filename, data.c_str(), result);
	}

	/**
	 * �f�o�b�K�̗L����
	 * @param port �|�[�g�ԍ�(�f�t�H���g��5858)
	 * @return ���s����
	 */
	static tjs_error TJS_INTF_METHOD enableDebug(tTJSVariant *result,
												 tjs_int numparams,
												 tTJSVariant **param,
												 iTJSDispatch2 *objthis) {
		int  port  = numparams > 0 ? (int)*param[0] : 5858;
		bool wait  = numparams > 1 ? (int)*param[1] != 0 : false;
		Debug::EnableAgent("kirikiriV8", port, wait);
		return TJS_S_OK;
	}
	
	// �f�o�b�K�쓮�p
	static tjs_error TJS_INTF_METHOD processDebug(tTJSVariant *result,
												  tjs_int numparams,
												  tTJSVariant **param,
												  iTJSDispatch2 *objthis) {
		getInstance()->processDebug();
		return TJS_S_OK;
	}

	static ScriptsJavascript *getInstance() {
		if (!instance) {
			instance = new ScriptsJavascript();
		}
		return instance;
	}
	
	Local<Context> getContext() {
		return Local<Context>::New(isolate, mainContext);
	}
	
protected:

	static void OnFatalError(const char* location, const char* message) {
		char msg[1024];
		if (location) {
			_snprintf_s(msg, 1024, _TRUNCATE, "FATAL ERROR: %s %s\n", location, message);
		} else {
			_snprintf_s(msg, 1024, _TRUNCATE, "FATAL ERROR: %s\n", message);
		}
		TVPAddLog(msg);
		::DebugBreak();
	}
	
	static void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
		if (args.Length() < 1) return;
		HandleScope scope(args.GetIsolate());
		String::Value value(args[0]);
		ttstr msg = *value;
		TVPAddLog(msg);
	}

	
	/**
	 * �R���X�g���N�^
	 */
	ScriptsJavascript(){
		v8::V8::InitializeICU();
		int argc = 0;
		char *argv[] = { "process" };
		v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

		isolate = v8::Isolate::GetCurrent();
		isolate->Enter();

		{
			HandleScope handle_scope(isolate);
		
			// �O���[�o���e���v���[�g�̏���
			Local<ObjectTemplate> globalTemplate = ObjectTemplate::New(isolate);
			// �O���[�o���֐��ɓo�^
			globalTemplate->Set(String::NewFromUtf8(isolate, "log"), FunctionTemplate::New(isolate, LogCallback));
			
			TJSInstance::init(isolate, globalTemplate);
			TJSObject::init(isolate);
			
			// �R���e�L�X�g����
			Local<Context> context = Context::New(isolate, 0, globalTemplate);
			// �L�^���Ă���
			mainContext.Reset(isolate, context);

			{
				Context::Scope context_scope(context);
				// �O���[�o���I�u�W�F�N�g�̏���
				iTJSDispatch2 * global = TVPGetScriptDispatch();
				if (global) {
					// �g���g���̃O���[�o���� Javascript �̃O���[�o����o�^����
					{
						Local<Value> g = context->Global();
						tTJSVariant result = toVariant(isolate, g);
						global->PropSet(TJS_MEMBERENSURE, JAVASCRIPT_GLOBAL, NULL, &result, global);
					}
					// Javascript �� �O���[�o���ɋg���g���� �O���[�o����o�^����
					context->Global()->Set(String::NewFromTwoByte(isolate, KIRIKIRI_GLOBAL), toJSValue(isolate, tTJSVariant(global, global)));
					global->Release();
				}
			}
		}
	};

	/**
	 * �f�X�g���N�^
	 */
	~ScriptsJavascript(){
		mainContext.Reset();
		TJSObject::done(isolate);
		isolate->Exit();
		isolate->Dispose();
		isolate = 0;
		V8::Dispose();
	}
	
	/**
	 * javascript �X�N���v�g�̎��s
	 * @param script �X�N���v�g
	 * @param ... ����
	 * @return ���s����
	 */
	tjs_error _exec(const tjs_char *filename,
					const tjs_char *scriptText,
					tTJSVariant *result) {

		HandleScope handle_scope(isolate);
		Context::Scope context_scope(getContext());
		TryCatch try_catch;

		Local<Script> script = Script::Compile(String::NewFromTwoByte(isolate, scriptText), filename ? String::NewFromTwoByte(isolate, filename) : String::NewFromTwoByte(isolate, L""));
		if (script.IsEmpty()) {
			// Print errors that happened during compilation.
			JSEXCEPTION(isolate, &try_catch);
		} else {
			Local<Value> ret = script->Run();
			if (ret.IsEmpty()) {
				JSEXCEPTION(isolate, &try_catch);
			} else {
				// ���ʂ��i�[
				if (result) {
					*result = toVariant(isolate, ret);
				}
			}
		}
		return TJS_S_OK;
	}

	void processDebug() {
		HandleScope handle_scope(isolate);
		Context::Scope context_scope(getContext());
		Debug::ProcessDebugMessages();
	}

	static void destroy() {
		if (instance) {
			delete instance;
			instance = 0;
		}
	}

private:
	static ScriptsJavascript *instance;
	Isolate* isolate;
	Persistent<Context> mainContext;
};

ScriptsJavascript *ScriptsJavascript::instance = 0;

NCB_ATTACH_FUNCTION(execJS, Scripts, ScriptsJavascript::exec);
NCB_ATTACH_FUNCTION(execStorageJS, Scripts, ScriptsJavascript::execStorage);
NCB_ATTACH_FUNCTION(enableDebugJS, Scripts, ScriptsJavascript::enableDebug);
NCB_ATTACH_FUNCTION(processDebugJS, Scripts, ScriptsJavascript::processDebug);

Local<Context> getContext() {
	return ScriptsJavascript::getInstance()->getContext();
}

//---------------------------------------------------------------------------

static void PreRegisterCallback()
{
	ScriptsJavascript::PreRegisterCallback();
}

/**
 * �J�������O
 */
static void PostUnregisterCallback()
{
	ScriptsJavascript::PostUnregisterCallback();
}


NCB_PRE_REGIST_CALLBACK(PreRegisterCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregisterCallback);
