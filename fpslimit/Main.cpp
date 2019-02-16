#include <windows.h>
#include "tp_stub.h"

// fps �̐����l
static int fpsLimitValue = 1000;

// �v���p�e�B��
#define FPSLIMITNAME L"fpslimit"

/*
 * fps �̐����l�̎Q�Ɨp�̃v���p�e�B
 */
class tFpsLimitProp : public tTJSDispatch
{
public:
	tjs_error TJS_INTF_METHOD PropGet(tjs_uint32 flag,
									  const tjs_char * membername,
									  tjs_uint32 *hint,
									  tTJSVariant *result,
									  iTJSDispatch2 *objthis)	{
		if (result) {
			*result = fpsLimitValue;
		}
		return TJS_S_OK;
	}
	
	tjs_error TJS_INTF_METHOD PropSet(tjs_uint32 flag,
									  const tjs_char *membername,
									  tjs_uint32 *hint,
									  const tTJSVariant *param,
									  iTJSDispatch2 *objthis) {
		fpsLimitValue = *param;
		if (fpsLimitValue == 0) {
			fpsLimitValue = 1000;
		}
		return TJS_S_OK;
	}
};


// �O��̌Ăяo������
static tjs_uint64 prevTime;

// ���~�b�g����
class CFPSLimit : public tTVPContinuousEventCallbackIntf {
public:
	CFPSLimit() {};
	virtual void TJS_INTF_METHOD OnContinuousCallback(tjs_uint64 tick) {
		int diff = (int)((prevTime + 1000 / fpsLimitValue) - TVPGetTickCount());
		if (diff > 0) {
			Sleep(diff);
		}
		prevTime = TVPGetTickCount();
	}
};

static CFPSLimit limit;

//---------------------------------------------------------------------------
int WINAPI
DllEntryPoint(HINSTANCE /*hinst*/, unsigned long /*reason*/, void* /*lpReserved*/)
{
	return 1;
}

static tjs_int GlobalRefCountAtInit = 0;
extern "C" __declspec(dllexport) HRESULT __stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// �X�^�u�̏�����(�K���L�q����)
	TVPInitImportStub(exporter);

	// ������
	prevTime = TVPGetTickCount();
	TVPAddContinuousEventHook(&limit);

	{
		// TJS �̃O���[�o���I�u�W�F�N�g���擾����
		iTJSDispatch2 * global = TVPGetScriptDispatch();
		
		// Layer �N���X�I�u�W�F�N�g���擾
		tTJSVariant varScripts;
		TVPExecuteExpression(TJS_W("System"), &varScripts);
		iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
		if (dispatch) {
			tTJSDispatch *method = new tFpsLimitProp();
			tTJSVariant var(method);
			dispatch->PropSet(TJS_MEMBERENSURE, FPSLIMITNAME, NULL, &var, dispatch);
			method->Release();
		}
		global->Release();
	}
			
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	return S_OK;
}


//---------------------------------------------------------------------------
extern "C" __declspec(dllexport) HRESULT __stdcall V2Unlink()
{
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;

	// - �܂��ATJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	if (global) {
		tTJSVariant varScripts;
		TVPExecuteExpression(TJS_W("System"), &varScripts);
		iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
		if (dispatch) {
			dispatch->DeleteMember(0, FPSLIMITNAME, NULL, dispatch);
		}
		global->Release();
	}

	TVPRemoveContinuousEventHook(&limit);
	
	TVPUninitImportStub();

	return S_OK;
}
