//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
#include <windows.h>
#include "tp_stub.h"

//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
// �e�X�g�N���X
//---------------------------------------------------------------------------
/*
	�e�I�u�W�F�N�g (iTJSDispatch2 �C���^�[�t�F�[�X) �ɂ̓l�C�e�B�u�C���X�^���X��
	�Ă΂��AiTJSNativeInstance �^�̃I�u�W�F�N�g��o�^���邱�Ƃ��ł��A�����
	�I�u�W�F�N�g������o�����Ƃ��ł��܂��B
	�܂��A�l�C�e�B�u�C���X�^���X�̎����ł��B�l�C�e�B�u�C���X�^���X����������ɂ�
	tTJSNativeInstance ����N���X�𓱏o���܂��BtTJSNativeInstance ��
	iTJSNativeInstance �̊�{�I�ȓ�����������Ă��܂��B
*/
class NI_Parser : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
public:
	NI_Parser()
	{
		// �R���X�g���N�^
		/*
			NI_Parser �̃R���X�g���N�^�ł��BC++ �N���X�Ƃ��Ă̏������� ��q��
			Construct ���������ōς܂��Ă����AConstruct �ł̏������͍ŏ����̕�
			�ɂ��邱�Ƃ��������߂��܂��B
		*/
		DicObj = NULL;
		Dictionary_clear = NULL;

		/*
			Name_ch �Ȃǂɂ��炩���� ������ �����Ă���
			ttstr �́A���ꂪ�ێ����Ă��镶����� �n�b�V�� ���Ƃ��ɕۑ����邱��
			���ł��邽�߁A������̌����̌������悭���邽�߂�
			���炩���ߕ������ݒ肵�A���̕������ iTJSDispatch2::PropSet �Ȃǂ�
			�g����
		*/
		Name_ch = TJS_W("ch"); // Name_ch

		try
		{
			// �����z��I�u�W�F�N�g���쐬����
			// �����z��I�u�W�F�N�g�́A����� getNext �Ăяo���ōĐ���
			// ����Ȃ��悤�ɁA���Đ����������g���񂷂��ƂƂ���
			DicObj = TJSCreateDictionaryObject();

			// Dictionary.clear ���擾����
			// ������A���� Dictionary.clear ���擾���Ă���Ǝ��Ԃ������邽��
			// �����ł��炩���ߎ擾���Ă���
			tTJSVariant val;
			TVPExecuteExpression(TJS_W("global.Dictionary.clear"), &val);
			Dictionary_clear = val.AsObject();
				// AsObject �͎Q�ƃJ�E���g�𑝂₷�̂�
				// ���Ƃ� Dictionary_clear �� Release ����K�v����
		}
		catch(...)
		{
			// ��L try �u���b�N���ŗ�O�����������ꍇ�̕ی�
			Invalidate();
			throw;
		}
	}

	tjs_error TJS_INTF_METHOD
		Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
	{
		// TJS2 �I�u�W�F�N�g���쐬�����Ƃ��ɌĂ΂��
		/*
			TJS2 �� new ���Z�q�� TJS2 �I�u�W�F�N�g���쐬�����Ƃ��ɌĂ΂�܂��B
			numparams �� param ������ new ���Z�q�ɓn���ꂽ������\���Ă��܂��B
			tjs_obj �����́A�쐬����� TJS �I�u�W�F�N�g�ł��B
		*/

		// ����������΂���������l�Ƃ��� Value �ɓ����
		return S_OK;
	}

	void TJS_INTF_METHOD Invalidate()
	{
		// �I�u�W�F�N�g�������������Ƃ��ɌĂ΂��
		/*
			�I�u�W�F�N�g�������������Ƃ��ɌĂ΂�郁�\�b�h�ł��B�����ɏI������
			�������Ɨǂ��ł��傤�B
		*/
		if(DicObj) DicObj->Release();
		if(Dictionary_clear) Dictionary_clear->Release();
	}

	/*
		�f�[�^�����o�𑀍삷�邽�߂̌��J���\�b�h�Q�ł��B��q����l�C�e�B�u�N���X
		���ŁA�����𗘗p����R�[�h�������܂��B
	*/

	void Load(const ttstr & name)
	{
		// Script �ɃX�N���v�g��ǂݍ��݂܂�
		// TextReadStream ���쐬���A����𗘗p���ăX�N���v�g��ǂݍ��݂܂�
		iTJSTextReadStream * stream =
			TVPCreateTextStreamForRead(name, TJS_W(""));
		try
		{
			stream->Read(Script, 0); // �S�Ă���C�ɓǂݍ���
			Pointer = 0; // �|�C���^�̏�����
		}
		catch(...)
		{
			stream->Destruct();
			throw;
		}
		stream->Destruct();
	}

	iTJSDispatch2 * GetNext()
	{
		// ���̃g�[�N���̏��� DicObj �ɓ����
		// �ǂݏo���ɐ�������� DicObj �I�u�W�F�N�g�A
		// ���s����� (�X�N���v�g�̏I�[�ɒB�����) NULL
		// ��Ԃ��B
		// �Ԃ���� DicObj �I�u�W�F�N�g�̎Q�ƃJ�E���^��
		// �C���N�������g *����Ȃ�* �̂Œ���

		// DicObj ���N���A
		Dictionary_clear->FuncCall(
			0, // flag
			NULL, // membername
			NULL, // hint
			NULL, // result
			0, // numparams
			NULL, // param
			DicObj // objthis
			);

		// �X�N���v�g�̏I�[�ɒB������?
		// note: ttstr �͓����ɕ�����̒����������Ă���̂�
		// strlen �̂悤�ɕ����񒷂��X�L�������Ė��񓾂鎖�͂Ȃ��̂�
		// ��r�I����
		if(Pointer >= Script.GetLen()) return NULL;

		// �ꕶ���𓾂āADicObj �� ch �����o�ɐݒ肷��
		// �܂��APointer ���C���N�������g����
		// note: �����ł͊ȒP�̂��߂ɂP�R�[�h�|�C���g���P�����ł���ƌ��Ȃ�
		tjs_char ch = Script.c_str()[Pointer];
		Pointer ++;

		ttstr str (ch); // ch �𕶎���ɕϊ�
		tTJSVariant val(str); // str �� tTJSVariant �ɕϊ�
		DicObj->PropSet(
			TJS_MEMBERENSURE, // flags: �����o��������΍쐬���邱�Ƃ���������
			Name_ch.c_str(), // �ݒ肷�郁���o��
			Name_ch.GetHint(), // �ݒ肷�郁���o���̃q���g
			&val, // �ݒ肷�� Variant
			DicObj // objthis: �R���e�L�X�g
			);

		// DicObj ��Ԃ�
		// �Q�ƃJ�E���^�̃C���N�������g�͍s��Ȃ�
		return DicObj;
	}

private:
	/*
		�ی상�\�b�h�Ȃ�
	*/

private:
	/*
		�f�[�^�����o�ł��B�l�C�e�B�u�C���X�^���X�ɂ́A�K�v�ȃf�[�^�����o�����R��
		�������Ƃ��ł��܂��B
	*/
	iTJSDispatch2 * DicObj; // getNext �֐��Ŗ���Ԃ���鎫���z��I�u�W�F�N�g
	iTJSDispatch2 * Dictionary_clear; // Dictionary.clear �֐�

	ttstr Script; // ���̓X�N���v�g��ێ����邽�߂̕�����
	tjs_int Pointer; // ��͈ʒu

	ttstr Name_ch; // "ch" ��ێ����镶����
};
//---------------------------------------------------------------------------
/*
	����� NI_Parser �̃I�u�W�F�N�g���쐬���ĕԂ������̊֐��ł��B
	��q�� TJSCreateNativeClassForPlugin �̈����Ƃ��ēn���܂��B
*/
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_Parser()
{
	return new NI_Parser();
}
//---------------------------------------------------------------------------
/*
	TJS2 �̃l�C�e�B�u�N���X�͈�ӂ� ID �ŋ�ʂ���Ă���K�v������܂��B
	����͌�q�� TJS_BEGIN_NATIVE_MEMBERS �}�N���Ŏ����I�Ɏ擾����܂����A
	���� ID ���i�[����ϐ����ƁA���̕ϐ��������Ő錾���܂��B
	�����l�ɂ͖����� ID ��\�� -1 ���w�肵�Ă��������B
*/
#define TJS_NATIVE_CLASSID_NAME ClassID_Parser
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
/*
	TJS2 �p�́u�N���X�v���쐬���ĕԂ��֐��ł��B
*/
static iTJSDispatch2 * Create_NC_Parser()
{
	/*
		�܂��A�N���X�̃x�[�X�ƂȂ�N���X�I�u�W�F�N�g���쐬���܂��B
		����ɂ� TJSCreateNativeClassForPlugin ��p���܂��B
		TJSCreateNativeClassForPlugin �̑�P�����̓N���X���A��Q������
		�l�C�e�B�u�C���X�^���X��Ԃ��֐����w�肵�܂��B
		�쐬�����I�u�W�F�N�g���ꎞ�I�Ɋi�[���郍�[�J���ϐ��̖��O��
		classobj �ł���K�v������܂��B
	*/
	tTJSNativeClassForPlugin * classobj =
		TJSCreateNativeClassForPlugin(TJS_W("Parser"), Create_NI_Parser);


	/*
		TJS_BEGIN_NATIVE_MEMBERS �}�N���ł��B�����ɂ� TJS2 ���Ŏg�p����N���X��
		���w�肵�܂��B
		���̃}�N���� TJS_END_NATIVE_MEMBERS �}�N���ŋ��܂ꂽ�ꏊ�ɁA�N���X��
		�����o�ƂȂ�ׂ����\�b�h��v���p�e�B�̋L�q�����܂��B
	*/
	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/Parser)

		/*
			��� finalize ���\�b�h��錾���܂��Bfinalize �ɑ������鏈����
			tTJSNativeInstance::Invalidate ���I�[�o�[���C�h���邱�Ƃł������ł�
			�܂��̂ŁA�ʏ�͋�̃��\�b�h�ŏ\���ł��B
		*/
		TJS_DECL_EMPTY_FINALIZE_METHOD

		/*
			(TJS��) �R���X�g���N�^��錾���܂��BTJS �ŃN���X�������Ƃ��A
			�N���X���ŃN���X�Ɠ����̃��\�b�h��錾���Ă��镔���ɑ������܂��B

			TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL �}�N���̂P�Ԗڂ̈����̓l�C�e�B�u
			�C���X�^���X�Ɋ��蓖�Ă�ϐ����ŁA�Q��ʖڂ̈����͂��̕ϐ��̌^���ł��B
			���̃u���b�N���ł� NI_Parser * _this �Ƃ����ϐ������p�\�ŁA
			�l�C�e�B�u�C���X�^���X�ɃA�N�Z�X���邱�Ƃ��ł��܂��B
			�}�N���̂R�Ԗڂ̈����́ATJS ���Ŏg�p����N���X�����w�肵�܂��B
			TJS_END_NATIVE_CONSTRUCTOR_DECL �}�N���̈��������l�ł��B
			�������A�R���X�g���N�^�ɑ������鏈���� tTJSNativeInstance::Construct
			���I�[�o�[���C�h���鎖�Ŏ����ł���̂ŁA�����ł͉��������� S_OK ���
			���܂��B
		*/
		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_Parser,
			/*TJS class name*/Parser)
		{
			// NI_Parser::Construct �ɂ����e���L�q�ł���̂�
			// �����ł͉������Ȃ�
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/Parser)

		/*
			print ���\�b�h��錾���܂��B���\�b�h����
			TJS_BEGIN_NATIVE_METHOD_DECL �� TJS_END_NATIVE_METHOD_DECL �̗��}�N
			���ɓ������̂��w�肷��K�v������܂��B���̃}�N�����Ŏg�p�\�ȕϐ���
			tjs_int numparams �� tTJSVariant **param �������āA���ꂼ��A�n����
			�������̐��ƈ����������Ă��܂��B���̃��\�b�h�ł͂����͎g�p���Ă���
			����BTJS_GET_NATIVE_INSTANCE�́A�I�u�W�F�N�g����l�C�e�B�u�C���X�^��
			�X�����o�����߂̃}�N���ł��B�����ł́A_this �Ƃ��� NI_Parser *
			�^�̕ϐ��Ƀl�C�e�B�u�C���X�^���X�����o���A�Ƃ����Ӗ��ɂȂ�܂��B
			�ȍ~�A_this �Ƃ����ϐ��Ńl�C�e�B�u�C���X�^���X�ɃA�N�Z�X�ł��܂��B
		*/
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/load) // load ���\�b�h
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
				/*var. type*/NI_Parser);

			if(numparams < 1) return TJS_E_BADPARAMCOUNT; // �����̌��`�F�b�N

			_this->Load(*param[0]);

			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/load)


		/*
			getNext ���\�b�h
		*/
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getNext) // getNext ���\�b�h
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
				/*var. type*/NI_Parser);

			iTJSDispatch2 * obj = _this->GetNext();

			if(result) *result = obj; // result �� null �ɂȂ蓾��̂Œ���

			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/getNext)

	TJS_END_NATIVE_MEMBERS

	/*
		���̊֐��� classobj ��Ԃ��܂��B
	*/
	return classobj;
}
//---------------------------------------------------------------------------
/*
	TJS_NATIVE_CLASSID_NAME �͈ꉞ undef ���Ă������ق����悢�ł��傤
*/
#undef TJS_NATIVE_CLASSID_NAME
//---------------------------------------------------------------------------






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

	tTJSVariant val;

	// TJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();


	//-----------------------------------------------------------------------
	// 1 �܂��N���X�I�u�W�F�N�g���쐬
	iTJSDispatch2 * tjsclass = Create_NC_Parser();

	// 2 tjsclass �� tTJSVariant �^�ɕϊ�
	val = tTJSVariant(tjsclass);

	// 3 ���ł� val �� tjsclass ��ێ����Ă���̂ŁAtjsclass ��
	//   Release ����
	tjsclass->Release();


	// 4 global �� PropSet ���\�b�h��p���A�I�u�W�F�N�g��o�^����
	global->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		TJS_W("Parser"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
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
extern "C" HRESULT _stdcall _export V2Unlink()
{
	// �g���g��������A�v���O�C����������悤�Ƃ���Ƃ��ɌĂ΂��֐��B

	// �������炩�̏����Ńv���O�C��������ł��Ȃ��ꍇ��
	// ���̎��_�� E_FAIL ��Ԃ��悤�ɂ���B
	// �����ł́ATVPPluginGlobalRefCount �� GlobalRefCountAtInit ����
	// �傫���Ȃ��Ă���Ύ��s�Ƃ������Ƃɂ���B
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL ���A��ƁAPlugins.unlink ���\�b�h�͋U��Ԃ�


	/*
		�������A�N���X�̏ꍇ�A�����Ɂu�I�u�W�F�N�g���g�p���ł���v�Ƃ������Ƃ�
		�m�邷�ׂ�����܂���B��{�I�ɂ́APlugins.unlink �ɂ��v���O�C���̉����
		�댯�ł���ƍl���Ă������� (�������� Plugins.link �Ń����N������A�Ō��
		�Ńv���O�C������������A�v���O�����I���Ɠ����Ɏ����I�ɉ��������̂��g)�B
	*/

	// TJS �̃O���[�o���I�u�W�F�N�g�ɓo�^���� Parser �N���X�Ȃǂ��폜����

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
			TJS_W("Parser"), // �����o��
			NULL, // �q���g
			global // �R���e�L�X�g
			);
	}

	// - global �� Release ����
	if(global) global->Release();

	// �X�^�u�̎g�p�I��(�K���L�q����)
	TVPUninitImportStub();

	return S_OK;
}
//---------------------------------------------------------------------------

