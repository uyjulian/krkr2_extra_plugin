//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// �w�肳�ꂽ�f�B���N�g�����̃t�@�C���̈ꗗ�𓾂�֐�
//---------------------------------------------------------------------------
class tGetDirListFunction : public tTJSDispatch
{
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
	{
		if(membername) return TJS_E_MEMBERNOTFOUND;

		// ���� : �f�B���N�g��
		if(numparams < 1) return TJS_E_BADPARAMCOUNT;

		ttstr dir(*param[0]);

		if(dir.GetLastChar() != TJS_W('/'))
			TVPThrowExceptionMessage(TJS_W("'/' must be specified at the end of given directory name."));

		// OS�l�C�e�B�u�ȕ\���ɕϊ�
		dir = TVPNormalizeStorageName(dir);
		TVPGetLocalName(dir);

		// Array �N���X�̃I�u�W�F�N�g���쐬
		iTJSDispatch2 * array;

		{
			tTJSVariant result;
			TVPExecuteExpression(TJS_W("[]"), &result);
			// �Ȃɂ� TJS �X�N���v�g�ŏo�������Ȃ��Ƃ�C++�ł��̂��ʓ|�Ȃ��
			// ���̂悤�� TJS �������s���Ă��܂��̂������葁��
			array = result.AsObject();
		}

		try
		{
			//
			char nfile[MAX_PATH + 1];
			char ndir[MAX_PATH + 1];
			char nwildcard[MAX_PATH + 1];

			// dir �� ndir �ɕϊ�
			int dir_narrow_len = dir.GetNarrowStrLen();
			if(dir_narrow_len >= MAX_PATH - 3)
				TVPThrowExceptionMessage(TJS_W("Too long directory name."));

			dir.ToNarrowStr(ndir, MAX_PATH);

			// FindFirstFile ���g���ăt�@�C�����
			strcpy(nwildcard, ndir);
			strcat(nwildcard, "*.*");

			WIN32_FIND_DATA data;
			HANDLE handle = FindFirstFile(nwildcard, &data);
			if(handle != INVALID_HANDLE_VALUE)
			{
				tjs_int count = 0;
				do
				{
					strcpy(nfile, ndir);
					strcat(nfile, data.cFileName);

					if(GetFileAttributes(nfile) & FILE_ATTRIBUTE_DIRECTORY)
					{
						// �f�B���N�g���̏ꍇ�͍Ō�� / ������
						strcpy(nfile, data.cFileName);
						strcat(nfile, "/");
					}
					else
					{
						// ���ʂ̃t�@�C���̏ꍇ�͂��̂܂�
						strcpy(nfile, data.cFileName);
					}

					// �z��ɒǉ�����
					tTJSVariant val((ttstr)(nfile));
					array->PropSetByNum(0, count++, &val, array);

				} while(FindNextFile(handle, &data));
				FindClose(handle);
			}
			else
			{
				TVPThrowExceptionMessage(TJS_W("Directory not found."));
			}

			if(result)
				*result = tTJSVariant(array, array);
		}
		catch(...)
		{
			array->Release();
			throw;
		}

		array->Release();

		// �߂�
		return TJS_S_OK;
	}
} * GetDirListFunction;
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

	// GetDirListFunction �̍쐬�Ɠo�^
	tTJSVariant val;

	// TJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// 1 �܂��I�u�W�F�N�g���쐬
	GetDirListFunction = new tGetDirListFunction();

	// 2 GetDirListFunction �� tTJSVariant �^�ɕϊ�
	val = tTJSVariant(GetDirListFunction);

	// 3 ���ł� val �� GetDirListFunction ��ێ����Ă���̂ŁAGetDirListFunction ��
	//   Release ����
	GetDirListFunction->Release();


	// 4 global �� PropSet ���\�b�h��p���A�I�u�W�F�N�g��o�^����
	global->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		TJS_W("getDirList"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
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
	TVPAddLog(TVPPluginGlobalRefCount);
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
	TVPAddLog(TVPPluginGlobalRefCount);
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL ���A��ƁAPlugins.unlink ���\�b�h�͋U��Ԃ�

	// TJS �̃O���[�o���I�u�W�F�N�g�ɓo�^���� getDirList �֐����폜����

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
			TJS_W("getDirList"), // �����o��
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



