#include <windows.h>
#include "tp_stub.h"
#include <stdio.h>
#include <string>

using namespace std;

/**
 * ���O�o�͗p
 */
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

// -----------------------------------------------------------------

class IFile {
public:
	virtual ~IFile() {};
	virtual bool getNextLine(ttstr &str) = 0;
};

class IFileStorage : public IFile {

	IStream *in;
	char buf[8192];
	ULONG pos;
	ULONG len;
	bool eofFlag;
	int codepage;
	
public:
	IFileStorage(tTJSVariantString *filename, int codepage) : codepage(codepage) {
		in = TVPCreateIStream(filename, TJS_BS_READ);
		if(!in) {
			TVPThrowExceptionMessage((ttstr(TJS_W("cannot open : ")) + *filename).c_str());
		}
		pos = 0;
		len = 0;
		eofFlag = false;
	}

	~IFileStorage() {
		if (in) {
			in->Release();
			in = NULL;
		}
	}
	
	int getc() {
		if (pos < len) {
			return buf[pos++];
		} else {
			if (!in || eofFlag) {
				return EOF;
			} else {
				pos = 0;
				if (in->Read(buf, sizeof buf, &len) == S_OK) {
					eofFlag = len < sizeof buf;
				} else {
					eofFlag = true;
					len = 0;
				}
				return getc();
			}
		}
	}

	void ungetc() {
		if (pos > 0) {
			pos--;
		}
	}

	bool eof() {
		return pos >= len && eofFlag;
	}

	/**
	 * ���s�`�F�b�N
	 */
	bool endOfLine(int c) {
		bool eol = (c =='\r' || c == '\n');
		if (c == '\r'){
			c = getc();
			if (!eof() && c != '\n') {
				ungetc();
			}
		}
		return eol;
	}
	
	bool getNextLine(ttstr &str) {
		int c;
		string mbline;
		while ((c = getc()) != EOF && !endOfLine(c)) {
			mbline += c;
		}
		int l = mbline.length();
		if (l > 0 || c != EOF) {
			wchar_t *buf = new wchar_t[l + 1];
			l = MultiByteToWideChar(codepage, 0,
									mbline.data(),
									mbline.length(),
									buf, l);
			buf[l] = '\0';
			str = buf;
			delete buf;
			return true;
		} else {
			return false;
		}
	}
};

class IFileStr : public IFile {

	ttstr dat;
	ULONG pos;

public:
	IFileStr(tTJSVariantString *str) {
		dat = *str;
		pos = 0;
	}

	int getc() {
		return pos < dat.length() ? dat[pos++] : EOF;
	}

	void ungetc() {
		if (pos > 0) {
			pos--;
		}
	}

	bool eof() {
		return pos >= dat.length();
	}

	/**
	 * ���s�`�F�b�N
	 */
	bool endOfLine(tjs_char c) {
		bool eol = (c =='\r' || c == '\n');
		if (c == '\r'){
			c = getc();
			if (!eof() && c != '\n') {
				ungetc();
			}
		}
		return eol;
	}

	bool getNextLine(ttstr &str) {
		str = L"";
		int c;
		while ((c = getc()) != EOF && !endOfLine(c)) {
			str += c;
		}
		if (str.length() > 0 || c != EOF) {
			return true;
		} else {
			return false;
		}
	}
};

// -----------------------------------------------------------------

static void
addMember(iTJSDispatch2 *dispatch, const tjs_char *name, iTJSDispatch2 *member)
{
	tTJSVariant var = tTJSVariant(member);
	member->Release();
	dispatch->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		name, // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
		NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
		&var, // �o�^����l
		dispatch // �R���e�L�X�g
		);
}

static iTJSDispatch2*
getMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	tTJSVariant val;
	if (TJS_FAILED(dispatch->PropGet(TJS_IGNOREPROP,
									 name,
									 NULL,
									 &val,
									 dispatch))) {
		ttstr msg = TJS_W("can't get member:");
		msg += name;
		TVPThrowExceptionMessage(msg.c_str());
	}
	return val.AsObject();
}

static bool
isValidMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	return dispatch->IsValid(TJS_IGNOREPROP,
							 name,
							 NULL,
							 dispatch) == TJS_S_TRUE;
}

static void
delMember(iTJSDispatch2 *dispatch, const tjs_char *name)
{
	dispatch->DeleteMember(
		0, // �t���O ( 0 �ł悢 )
		name, // �����o��
		NULL, // �q���g
		dispatch // �R���e�L�X�g
		);
}

//---------------------------------------------------------------------------

#define TJS_NATIVE_CLASSID_NAME ClassID_LineParser
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;

/**
 * LineParser
 */
class NI_LineParser : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
protected:
	iTJSDispatch2 *target;
	IFile *file;
	tjs_int32 lineNo;
	
public:

	/**
	 * �R���X�g���N�^
	 */
	NI_LineParser() {
		target = NULL;
		file = NULL;
		lineNo = 0;
	}

	/**
	 * TJS �R���X�g���N�^
	 * @param numparams �p�����[�^��
	 * @param param
	 * @param tjs_obj this �I�u�W�F�N�g
	 */
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {
		if (numparams > 0) {
			target = param[0]->AsObject();
		}
		return S_OK;
	}

	/**
	 * �t�@�C���N���[�Y����
	 */
	void clear() {
		if (file) {
			delete file;
			file = NULL;
		}
	}
	
	/**
	 * TJS invalidate
	 */
	void TJS_INTF_METHOD Invalidate() {
		if (target) {
			target->Release();
			target = NULL;
		}
		clear();
	}

	/**
	 * �p�[�T�̏���������
	 */
	void init(tTJSVariantString *text) {
		clear();
		file = new IFileStr(text);
		lineNo = 0;
	}

	/**
	 * ����������
	 */
	void initStorage(tTJSVariantString *filename, bool utf8=false) {
		clear();
		file = new IFileStorage(filename, utf8 ? CP_UTF8 : CP_ACP);
		lineNo = 0;
	}

	/**
	 * �s�̎擾
	 * @param line �s
	 * @return ���������true
	 */
	bool getNextLine(ttstr &line) {
		if (file) {
			if (file->getNextLine(line)) {
				lineNo++;
				return true;
			} else {
				clear();
			}
		}
		return false;
	}

	/**
	 * ���݂̍s�ԍ��̎擾
	 * @return �s�ԍ�
	 */
	tjs_int32 getLineNumber() {
		return lineNo;
	}
	
	/**
	 * �p�[�X�̎��s
	 */
	void parse(iTJSDispatch2 *objthis) {
		iTJSDispatch2 *target = this->target ? this->target : objthis;
		if (file && isValidMember(target, L"doLine")) {
			iTJSDispatch2 *method = getMember(target, L"doLine");
			ttstr line;
			while (getNextLine(line)) {
				tTJSVariant var1 = tTJSVariant(line);
				tTJSVariant var2 = tTJSVariant(lineNo);
				tTJSVariant *vars[2];
				vars[0] = &var1;
				vars[1] = &var2;
				method->FuncCall(0, NULL, NULL, NULL, 2, vars, target);
			}
			method->Release();
			clear();
		}
	}

};

static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_LineParser()
{
	return new NI_LineParser();
}

static iTJSDispatch2 * Create_NC_LineParser()
{
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("LineParser"), Create_NI_LineParser);

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/LineParser)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_LineParser,
			/*TJS class name*/LineParser)
		{
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/LineParser)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/init)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			_this->init(param[0]->AsStringNoAddRef());
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/init)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/initStorage)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			_this->initStorage(param[0]->AsStringNoAddRef(), numparams > 1 && (tjs_int)*param[1] != 0);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/initStorage)
			
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getNextLine)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
			ttstr ret;
			if (_this->getNextLine(ret) && result){
				*result = ret;
			}
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/getNextLine)
			
		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parse)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
			if (numparams > 0) {
				_this->init(param[0]->AsStringNoAddRef());
			}
			_this->parse(objthis);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/parse)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parseStorage)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_LineParser);
			if (numparams > 0) {
				_this->initStorage(param[0]->AsStringNoAddRef(), numparams > 1 && (tjs_int)*param[1] != 0);
			}
			_this->parse(objthis);
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/parseStorage)

		TJS_BEGIN_NATIVE_PROP_DECL(currentLineNumber)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_LineParser);
				*result = _this->getLineNumber();
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER
			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(currentLineNumber)

	TJS_END_NATIVE_MEMBERS

	// �萔�̓o�^

	/*
	 * ���̊֐��� classobj ��Ԃ��܂��B
	 */
	return classobj;
}

#undef TJS_NATIVE_CLASSID_NAME

//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}

//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall V2Link(iTVPFunctionExporter *exporter)
{
	// �X�^�u�̏�����(�K���L�q����)
	TVPInitImportStub(exporter);

	// TJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	
	if (global) {
		addMember(global, L"LineParser", Create_NC_LineParser());
		global->Release();
	}
			
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
extern "C" HRESULT _stdcall V2Unlink()
{
	// �g���g��������A�v���O�C����������悤�Ƃ���Ƃ��ɌĂ΂��֐��B

	// �������炩�̏����Ńv���O�C��������ł��Ȃ��ꍇ��
	// ���̎��_�� E_FAIL ��Ԃ��悤�ɂ���B
	// �����ł́ATVPPluginGlobalRefCount �� GlobalRefCountAtInit ����
	// �傫���Ȃ��Ă���Ύ��s�Ƃ������Ƃɂ���B
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL ���A��ƁAPlugins.unlink ���\�b�h�͋U��Ԃ�

	// - �܂��ATJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global �� DeleteMember ���\�b�h��p���A�I�u�W�F�N�g���폜����
	if (global)	{
		delMember(global, L"LineParser");
		global->Release();
	}

	// �X�^�u�̎g�p�I��(�K���L�q����)
	TVPUninitImportStub();

	return S_OK;
}
