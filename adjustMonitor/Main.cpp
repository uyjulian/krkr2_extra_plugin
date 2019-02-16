//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <windows.h>
#include <list>
#include "tp_stub.h"

//---------------------------------------------------------------------------

// ���j�^�[�X�̃p�����[�^�ێ�
typedef struct _MONIAREA
{
	int nNo;			// ���j�^�[�ԍ�
	long lAreaSize;		// ���j�^�[����߂�E�B���h�E�̃T�C�Y
	RECT rcMoniRect;	// ���j�^�[�̋�`
}MONIAREA;

// ���j�^�[�񋓃R�[���o�b�N�֐��ւ̈���
typedef struct _DATA
{
	int	nMonitorCount;
	RECT rcWndRect;
	std::list<MONIAREA> listMoniArea;
}DATA;

// ���j�^�[�񋓃R�[���o�b�N�֐�
static BOOL CALLBACK MonitorEnumProc( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	wchar_t	sStr[MAX_PATH];

	DATA* ptData = (DATA*)dwData;

	wsprintf(sStr, L"%d�Ԗڂ̃��j�^�̈ʒu : left:%d top:%d  right:%d bottom:%d", ptData->nMonitorCount, lprcMonitor->left, lprcMonitor->top, lprcMonitor->right, lprcMonitor->bottom );
//	::MessageBox(NULL, sStr, L"Info", MB_OK);
	
	RECT rcWndRect = ptData->rcWndRect;
	RECT rcMoniRect = *lprcMonitor;

	// �ŏ��ɍŒ�������ێ����Ă���
	int nLenWidth = rcWndRect.right - rcWndRect.left; 
	
	{// ��
	
		// �����W����`��荶�O�ɂ���
		if( rcWndRect.left < rcMoniRect.left )
			nLenWidth -= ( rcMoniRect.left - rcWndRect.left );
		// �E���W����`��荶�O�ɂ���
		if( rcWndRect.right > rcMoniRect.right )
			nLenWidth -= ( rcWndRect.right - rcMoniRect.right );
		
		if( nLenWidth < 0 )
			nLenWidth = 0;
	}
	
	// �ŏ��ɍŒ�������ێ����Ă���
	int nLenHeight = rcWndRect.bottom - rcWndRect.top;
	
	{// �c
	
		// ����W����`���E�O�ɂ���
		if( rcWndRect.top < rcMoniRect.top )
			nLenHeight -= ( rcMoniRect.top - rcWndRect.top );
		// �����W����`���E�O�ɂ���
		if( rcWndRect.bottom > rcMoniRect.bottom )
			nLenHeight -= ( rcWndRect.bottom - rcMoniRect.bottom );

		if( nLenHeight < 0 )
			nLenHeight = 0;
	}

	MONIAREA aMoniArea;
	
	aMoniArea.nNo = ptData->nMonitorCount;
	aMoniArea.rcMoniRect = rcMoniRect;
	aMoniArea.lAreaSize = nLenWidth*nLenHeight;

	// �E�B���h�E�̋�`�ێ�
	ptData->listMoniArea.push_back( aMoniArea );
	
	wsprintf( sStr, L"%d�Ԗڂ̃��j�^�ɂ���ʐ� : %d", ptData->nMonitorCount, aMoniArea.lAreaSize );
//	::MessageBox(NULL, sStr, L"Info", MB_OK);

	ptData->nMonitorCount++;

	return TRUE;	// TRUE�͒T���p���CFALSE�ŏI��
}

// list��sort�R�[���o�b�N
bool LessHeight(const MONIAREA& rLeft, const MONIAREA& rRight) { return rLeft.lAreaSize > rRight.lAreaSize; }


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
class tAdjustMoniFunction : public tTJSDispatch
{
	// �g���g���ŗ��p�\�Ȋ֐����ȒP�ɍ쐬����ɂ́A
	// tTJSDispatch ���p�������AFuncCall ����������B

	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
} * AdjustMoniFunction;
	// �e�X�g�֐���ێ�
	// iTJSDispatch2 �̔h���� ( tTJSDispatch �� tAdjustMoniFunction ������) �͂Ȃ�
	// �ׂ��q�[�v�Ɋm�� ( �܂�Anew �Ŋm�ۂ��� ) �悤�ɂ��ׂ��B
//---------------------------------------------------------------------------
tjs_error TJS_INTF_METHOD tAdjustMoniFunction::FuncCall(
	tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
	tTJSVariant *result,
	tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	// flag       : �Ăяo���t���O
	//              �������Ă��܂�Ȃ�
	// membername : �����o��
	//              �������Ă��܂�Ȃ����A�ꉞ
	if(membername) return TJS_E_MEMBERNOTFOUND;
	//              �ƋL�q���Ă����Ɨǂ�
	// hint       : �����o���q���g
	//              �{����membername�̃n�b�V���l�����A�������Ă��܂�Ȃ�
	// result     : ���ʊi�[��
	//              NULL �̏ꍇ�͌��ʂ��K�v�Ȃ���
	// numparams  : �p�����[�^�̐�
	// param      : �p�����[�^
	// objthis    : �R���e�L�X�g
	//              �N���X���\�b�h�ł͂Ȃ��̂Ŗ������Ă��܂�Ȃ�

	if(numparams == 0) return TJS_E_BADPARAMCOUNT;
		// ���̏ꍇ�͈����� 0 �ł����Ă͂Ȃ�Ȃ��̂ł��̂悤�ɂ���


	wchar_t			sStr[MAX_PATH];
	iTJSDispatch2*	elm	= param[0]->AsObjectNoAddRef();
	tTJSVariant		val;
	RECT			rcWndRect;
	RECT			rcWndRect2;
	bool			bResRect=true;	// �߂�l��RECT�����z��

	// HWND�Ŏw�肳��Ă���ꍇ�́A�E�B���h�E�n���h�������`�𓾂�
	if( elm->IsValid(0, L"window", NULL, elm) == TJS_S_TRUE )
	{
	
		// ��������l�����o��
		TJS_SUCCEEDED( elm->PropGet(0, L"window", NULL, &val, elm) );
		
		tTJSVariant		valHWND;
		iTJSDispatch2*	tmp = val.AsObjectNoAddRef();
		TJS_SUCCEEDED( tmp->PropGet(0, L"HWND", NULL, &valHWND, tmp) );
		
		HWND hWnd = (HWND)(tjs_int)valHWND;
		::GetWindowRect( hWnd, &rcWndRect );
		
	}
	else
	// RECT�w��̏ꍇ�́A����RECT�����`�𓾂�
	if( elm->IsValid(0, L"left", NULL, elm) == TJS_S_TRUE )
	{

		// ��������l�����o��
		TJS_SUCCEEDED( elm->PropGet(0, L"left", NULL, &val, elm) );
		rcWndRect.left = (long)(tjs_int)val;
		
		// ��������l�����o��
		TJS_SUCCEEDED( elm->PropGet(0, L"top", NULL, &val, elm) );
		rcWndRect.top = (long)(tjs_int)val;

		// right,bottom�\�L�̏ꍇ
		if( elm->IsValid(0, L"right", NULL, elm) == TJS_S_TRUE )
		{
			
			// ��������l�����o��
			TJS_SUCCEEDED( elm->PropGet(0, L"right", NULL, &val, elm) );
			rcWndRect.right = (long)(tjs_int)val;

			// ��������l�����o��
			TJS_SUCCEEDED( elm->PropGet(0, L"bottom", NULL, &val, elm) );
			rcWndRect.bottom = (long)(tjs_int)val;
		}
		else
		// width,height�\�L�̏ꍇ
		if( elm->IsValid(0, L"width", NULL, elm) == TJS_S_TRUE )
		{
		
			// ��������l�����o��
			TJS_SUCCEEDED( elm->PropGet(0, L"width", NULL, &val, elm) );
			rcWndRect.right = rcWndRect.left + (long)(tjs_int)val;

			// ��������l�����o��
			TJS_SUCCEEDED( elm->PropGet(0, L"height", NULL, &val, elm) );
			rcWndRect.bottom = rcWndRect.top + (long)(tjs_int)val;
		
		}
		
	}
	
	{// ������������`������ꍇ�͂����炩��
	
		// ����������HWND������ꍇ�́A���̋�`������B�߂�l�͒����ς݈ʒu�ƂȂ�
		if( elm->IsValid(0, L"window2", NULL, elm) == TJS_S_TRUE )
		{

			// ��������l�����o��
			TJS_SUCCEEDED( elm->PropGet(0, L"window2", NULL, &val, elm) );
			
			tTJSVariant		valHWND;
			iTJSDispatch2*	tmp = val.AsObjectNoAddRef();
			TJS_SUCCEEDED( tmp->PropGet(0, L"HWND", NULL, &valHWND, tmp) );
			
			HWND hWnd = (HWND)(tjs_int)valHWND;
			::GetWindowRect( hWnd, &rcWndRect2 );
		
			bResRect = false;	// �߂�l��POS�����z��
		}
		else
		// ����������RECT������ꍇ�́A���̋�`������B�߂�l�͒����ς݈ʒu�ƂȂ�
		if( elm->IsValid(0, L"left2", NULL, elm) == TJS_S_TRUE )
		{
			// ��������l�����o��
			TJS_SUCCEEDED( elm->PropGet(0, L"left2", NULL, &val, elm) );
			rcWndRect2.left = (long)(tjs_int)val;
			
			// ��������l�����o��
			TJS_SUCCEEDED( elm->PropGet(0, L"top2", NULL, &val, elm) );
			rcWndRect2.top = (long)(tjs_int)val;

			// right,bottom�\�L�̏ꍇ
			if( elm->IsValid(0, L"right2", NULL, elm) == TJS_S_TRUE )
			{
				// ��������l�����o��
				TJS_SUCCEEDED( elm->PropGet(0, L"right2", NULL, &val, elm) );
				rcWndRect2.right = (long)(tjs_int)val;

				// ��������l�����o��
				TJS_SUCCEEDED( elm->PropGet(0, L"bottom2", NULL, &val, elm) );
				rcWndRect2.bottom = (long)(tjs_int)val;
			}
			else
			// width,height�\�L�̏ꍇ
			if( elm->IsValid(0, L"width2", NULL, elm) == TJS_S_TRUE )
			{
				// ��������l�����o��
				TJS_SUCCEEDED( elm->PropGet(0, L"width2", NULL, &val, elm) );
				rcWndRect2.right = rcWndRect2.left + (long)(tjs_int)val;

				// ��������l�����o��
				TJS_SUCCEEDED( elm->PropGet(0, L"height2", NULL, &val, elm) );
				rcWndRect2.bottom = rcWndRect2.top + (long)(tjs_int)val;
			}

			bResRect = false;	// �߂�l��POS�����z��
		}

	}

	wsprintf(sStr, L"��������E�B���h�E�̋�` left:%d top:%d  right:%d bottom:%d", rcWndRect.left, rcWndRect.top, rcWndRect.right, rcWndRect.bottom );
//	::MessageBox(NULL, sStr, L"Info", MB_OK);

	DATA data;

	data.nMonitorCount = 0;
	data.rcWndRect = rcWndRect;
	
	// ���j�^�̗񋓂ƃE�B���h�E����߂�ʐς̌v�Z
	EnumDisplayMonitors( NULL, NULL, MonitorEnumProc, (LPARAM)&data );
	
	// �e���j�^���߂�E�B���h�E�̋�`
	data.listMoniArea.sort( LessHeight );

	
	std::list<MONIAREA>::iterator itBegin = data.listMoniArea.begin();
	wsprintf(sStr, L"%d�Ԗڂ̃��j�^��ɂ���܂��B\n��` left:%d top:%d  right:%d bottom:%d", (*itBegin).nNo, (*itBegin).rcMoniRect.left, (*itBegin).rcMoniRect.top, (*itBegin).rcMoniRect.right, (*itBegin).rcMoniRect.bottom );
//		::MessageBox(NULL, sStr, L"Info", MB_OK);

	RECT rcMoniRect = (*itBegin).rcMoniRect;

	// TJS�̔z��I�u�W�F�N�g�쐬
	iTJSDispatch2* tjsArray = TJSCreateArrayObject();
	
	// �߂�l��RECT�����z��̏ꍇ
	if( bResRect == true )
	{
		tTJSVariant var = (tjs_int)rcMoniRect.left;

		tjsArray->PropSet(
			TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
			TJS_W("left"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
			NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
			&var, // �o�^����l
			tjsArray // �R���e�L�X�g ( global �ł悢 )
			);

		var = (tjs_int)rcMoniRect.top;

		tjsArray->PropSet(
			TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
			TJS_W("top"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
			NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
			&var, // �o�^����l
			tjsArray // �R���e�L�X�g ( global �ł悢 )
			);

		var = (tjs_int)rcMoniRect.right;

		tjsArray->PropSet(
			TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
			TJS_W("right"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
			NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
			&var, // �o�^����l
			tjsArray // �R���e�L�X�g ( global �ł悢 )
			);

		var = (tjs_int)rcMoniRect.bottom;

		tjsArray->PropSet(
			TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
			TJS_W("bottom"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
			NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
			&var, // �o�^����l
			tjsArray // �R���e�L�X�g ( global �ł悢 )
			);
	}
	else
	// �߂�l��POS�����z��̏ꍇ
	{
		POINT ptAdjustPos = { rcWndRect2.left, rcWndRect2.top };
		if(rcWndRect2.left < rcMoniRect.left)		ptAdjustPos.x = rcMoniRect.left;
		if(rcWndRect2.top < rcMoniRect.top)			ptAdjustPos.y = rcMoniRect.top;
		if(rcWndRect2.right > rcMoniRect.right)		ptAdjustPos.x = rcMoniRect.right - (rcWndRect2.right - rcWndRect2.left);
		if(rcWndRect2.bottom > rcMoniRect.bottom)	ptAdjustPos.y = rcMoniRect.bottom - (rcWndRect2.bottom - rcWndRect2.top);

		tTJSVariant var = (tjs_int)ptAdjustPos.x;

		tjsArray->PropSet(
			TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
			TJS_W("x"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
			NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
			&var, // �o�^����l
			tjsArray // �R���e�L�X�g ( global �ł悢 )
			);

		var = (tjs_int)ptAdjustPos.y;

		tjsArray->PropSet(
			TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
			TJS_W("y"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
			NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
			&var, // �o�^����l
			tjsArray // �R���e�L�X�g ( global �ł悢 )
			);
	}

	// �߂�l�ɐݒ�
	*result = tTJSVariant(tjsArray,tjsArray);

	tjsArray->Release();

	return TJS_S_OK;
}
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" __declspec(dllexport) HRESULT __stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// �X�^�u�̏�����(�K���L�q����)
	TVPInitImportStub(exporter);

	// AdjustMoniFunction �̍쐬�Ɠo�^
	tTJSVariant val;

	// TJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	//-----------------------------------------------------------------------
	// 1 �܂��I�u�W�F�N�g���쐬
	AdjustMoniFunction = new tAdjustMoniFunction();

	// 2 AdjustMoniFunction �� tTJSVariant �^�ɕϊ�
	val = tTJSVariant(AdjustMoniFunction);

	// 3 ���ł� val �� AdjustMoniFunction ��ێ����Ă���̂ŁAAdjustMoniFunction ��
	//   Release ����
	AdjustMoniFunction->Release();


	// 4 global �� PropSet ���\�b�h��p���A�I�u�W�F�N�g��o�^����
	global->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		TJS_W("AdjustMoni"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
		NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
		&val, // �o�^����l
		global // �R���e�L�X�g ( global �ł悢 )
		);
	//-----------------------------------------------------------------------

	// - global �� Release ����
	global->Release();

	// �����A�o�^����֐�����������ꍇ�� 1 �` 4 ���J��Ԃ�


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
extern "C" __declspec(dllexport) HRESULT __stdcall V2Unlink()
{
	// �g���g��������A�v���O�C����������悤�Ƃ���Ƃ��ɌĂ΂��֐��B

	// �������炩�̏����Ńv���O�C��������ł��Ȃ��ꍇ��
	// ���̎��_�� E_FAIL ��Ԃ��悤�ɂ���B
	// �����ł́ATVPPluginGlobalRefCount �� GlobalRefCountAtInit ����
	// �傫���Ȃ��Ă���Ύ��s�Ƃ������Ƃɂ���B
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL ���A��ƁAPlugins.unlink ���\�b�h�͋U��Ԃ�

	// TJS �̃O���[�o���I�u�W�F�N�g�ɓo�^���� average �֐��Ȃǂ��폜����

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
			TJS_W("AdjustMoni"), // �����o��
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

