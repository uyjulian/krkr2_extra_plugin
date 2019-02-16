//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
#include <string>
//---------------------------------------------------------------------------

// ���s�t�@�C��������ꏊ�� WHND ��ۑ�����
void
storeHWND(HWND hwnd)
{
	tTJSVariant varScripts;
	TVPExecuteExpression(TJS_W("System.exeName"), &varScripts);
	ttstr path = varScripts;
	path += ".HWND";
	IStream *stream = TVPCreateIStream(path, TJS_BS_WRITE);
	if (stream != NULL) {
		char buf[100];
		DWORD len;
		_snprintf(buf, sizeof buf, "%d", (int)hwnd);
		stream->Write(buf, strlen(buf), &len);
		stream->Release();
	}
}

//---------------------------------------------------------------------------
// ���b�Z�[�W��M�֐�
//---------------------------------------------------------------------------
static bool __stdcall MyReceiver(void *userdata, tTVPWindowMessage *Message)
{
	iTJSDispatch2 *obj = (iTJSDispatch2 *)userdata;

	switch(Message->Msg) {
	case TVP_WM_DETACH:
		break;
	case TVP_WM_ATTACH:
		storeHWND((HWND)Message->LParam);
		break;
	case WM_COPYDATA:
		// �R�s�y�w��
		{
			HWND             hwndFrom = (HWND)Message->WParam;
			COPYDATASTRUCT*  pcds     = (COPYDATASTRUCT*)Message->LParam;
			std::string str((const char*)pcds->lpData, pcds->cbData);
			tTJSVariant msg = str.c_str();
			tTJSVariant *p[] = {&msg};
			obj->FuncCall(0, TJS_W("onCopyData"), NULL, NULL, 1, p, obj);
		}
		break;
	default:
		// ���̑��͖���
		break;
	}
	return false;
	/* true ��Ԃ��� �g���g���̃E�B���h�E�͂��̃��b�Z�[�W�Ɋ֒m���Ȃ��Ȃ�B
		   TVP_WM_DETACH �� TVP_WM_ATTACH �ւ̉����Ɋւ��Ă͖߂�l�͖�������� */
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// �J�n�֐�
//---------------------------------------------------------------------------
class tWMRStartFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if(numparams < 1) return TJS_E_BADPARAMCOUNT;
		// �E�C���h�E���w��
		iTJSDispatch2 *obj = param[0]->AsObjectNoAddRef();
		// registerMessageReceiver ���Ă�
		tTJSVariant mode, proc, userdata;
		tTJSVariant *p[3] = {&mode, &proc, &userdata};
		mode = (tTVInteger)(tjs_int)wrmRegister;
		proc = (tTVInteger)reinterpret_cast<tjs_int>(MyReceiver);
		userdata = (tTVInteger)(tjs_int)obj;
		obj->FuncCall(0, TJS_W("registerMessageReceiver"), NULL,
					  NULL, 3, p, obj);

		// �E�B���h�E�n���h�����擾���ċL�^
		tTJSVariant val;
		obj->PropGet(0, TJS_W("HWND"), NULL, &val, obj);
		storeHWND(reinterpret_cast<HWND>((tjs_int)(val)));
		
		return TJS_S_OK;
	}
} * WMRStartFunction;
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// �I���֐�
//---------------------------------------------------------------------------
class tWMRStopFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if(numparams < 1) return TJS_E_BADPARAMCOUNT;

		// *param[0] �� Window �N���X�̃I�u�W�F�N�g�ł���K�v������
		iTJSDispatch2 *obj = param[0]->AsObjectNoAddRef();

		// registerMessageReceiver ���Ă�
		tTJSVariant mode, proc, userdata;
		tTJSVariant *p[3] = {&mode, &proc, &userdata};
		mode = (tTVInteger)(tjs_int)wrmUnregister;
		proc = (tTVInteger)reinterpret_cast<tjs_int>(MyReceiver);
		userdata = (tTVInteger)(tjs_int)0;
		obj->FuncCall(0, TJS_W("registerMessageReceiver"), NULL,
			NULL, 3, p, obj);

		return TJS_S_OK;
	}
} * WMRStopFunction;
//---------------------------------------------------------------------------








//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
	// �X�^�u�̏�����(�K���L�q����)
	TVPInitImportStub(exporter);

	// WMRStartFunction, WMRStopFunction �̍쐬�Ɠo�^
	tTJSVariant val;

	// TJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// 1 �܂��I�u�W�F�N�g���쐬
	WMRStartFunction = new tWMRStartFunction();

	// 2 TestFunction �� tTJSVariant �^�ɕϊ�
	val = tTJSVariant(WMRStartFunction);

	// 3 ���ł� val �� TestFunction ��ێ����Ă���̂ŁAWMRStartFunction ��
	//   Release ����
	WMRStartFunction->Release();


	// 4 global �� PropSet ���\�b�h��p���A�I�u�W�F�N�g��o�^����
	global->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		TJS_W("wmrStart"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
		NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
		&val, // �o�^����l
		global // �R���e�L�X�g ( global �ł悢 )
		);

	// 1 �܂��I�u�W�F�N�g���쐬
	WMRStopFunction = new tWMRStopFunction();

	// 2 TestFunction �� tTJSVariant �^�ɕϊ�
	val = tTJSVariant(WMRStopFunction);

	// 3 ���ł� val �� TestFunction ��ێ����Ă���̂ŁAWMRStopFunction ��
	//   Release ����
	WMRStopFunction->Release();


	// 4 global �� PropSet ���\�b�h��p���A�I�u�W�F�N�g��o�^����
	global->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		TJS_W("wmrStop"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
		NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
		&val, // �o�^����l
		global // �R���e�L�X�g ( global �ł悢 )
		);


	// - global �� Release ����
	global->Release();

	// val ���N���A����B
	// ����͕K���s���B�������Ȃ��� val ���ێ����Ă���I�u�W�F�N�g
	// �� Release ���ꂸ�A���Ɏg�� TVPPluginGlobalRefCount �����m�ɂȂ�Ȃ��B
	val.Clear();


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

	// TJS �̃O���[�o���I�u�W�F�N�g�ɓo�^�����֐����폜����

	// - �܂��ATJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global �� DeleteMember ���\�b�h��p���A�I�u�W�F�N�g���폜����
	if(global)
	{
		// TJS ���̂����ɉ������Ă����Ƃ��Ȃǂ�
		// global �� NULL �ɂȂ蓾��̂� global �� NULL �łȂ�
		// ���Ƃ��`�F�b�N����

		global->DeleteMember(
			0, // �t���O ( 0 �ł悢 )
			TJS_W("wmrStart"), // �����o��
			NULL, // �q���g
			global // �R���e�L�X�g
			);
		global->DeleteMember(
			0, // �t���O ( 0 �ł悢 )
			TJS_W("wmrStop"), // �����o��
			NULL, // �q���g
			global // �R���e�L�X�g
			);
			// �o�^�����֐�����������ꍇ�� ������J��Ԃ�
	}

	// - global �� Release ����
	if(global) global->Release();

	// �X�^�u�̎g�p�I��(�K���L�q����)
	TVPUninitImportStub();

	return S_OK;
}
//---------------------------------------------------------------------------

