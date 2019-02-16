#if !defined _MSC_VER
#error "�S���i�T�C�l VC �c�J�e �N�_�T�C"
#endif
#if _MSC_VER < 1200
#error "�R���p�C���ɂ� MS-VC6.0 �ȍ~���K�v�ł��B"
#endif

#define CLINKAGE	extern "C"

#if _MSC_VER <= 1200
// FIXME:
// _export ���� BCC �̃L�[���[�h����Ȃ��̂�����H
// �悭�킩��񂯂ǁA���������Ȃ��Ă��̂ł��̂܂܂ɂ��Ă���
#  define EXPORT_DLL	_stdcall _export
#else
#  define EXPORT_DLL	_stdcall
#endif

#if _MSC_VER >= 1400
#  define MSVC_HAS_SECURE_CRT
#endif

#include <windows.h>
#include "tp_stub.h"
#include <stdio.h>

static const char *copyright = 
"----- EXPAT Copyright START -----\n"
"Copyright (c) 1998, 1999, 2000 Thai Open Source Software Center Ltd\n"
"                               and Clark Cooper\n"
"Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006 Expat maintainers.\n"
"\n"
"Permission is hereby granted, free of charge, to any person obtaining\n"
"a copy of this software and associated documentation files (the\n"
"\"Software\"), to deal in the Software without restriction, including\n"
"without limitation the rights to use, copy, modify, merge, publish,\n"
"distribute, sublicense, and/or sell copies of the Software, and to\n"
"permit persons to whom the Software is furnished to do so, subject to\n"
"the following conditions:\n"
"\n"
"The above copyright notice and this permission notice shall be included\n"
"in all copies or substantial portions of the Software.\n"
"\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
"EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
"IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY\n"
"CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,\n"
"TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE\n"
"SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
"----- EXPAT Copyright END -----\n";

/**
 * ���O�o�͗p
 */
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
#if defined MSVC_HAS_SECURE_CRT
	_vsnwprintf_s(msg, sizeof(msg), _TRUNCATE, format, args);
#else
	_vsnwprintf(msg, 1024, format, args);
#endif
	TVPAddLog(msg);
	va_end(args);
}

#define XML_UNICODE_WCHAR_T
#include "expat.h"

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

static iTJSDispatch2*
getMember(iTJSDispatch2 *dispatch, int num)
{
	tTJSVariant val;
	if (TJS_FAILED(dispatch->PropGetByNum(TJS_IGNOREPROP,
										  num,
										  &val,
										  dispatch))) {
		ttstr msg = TJS_W("can't get array index:");
		msg += num;
		TVPThrowExceptionMessage(msg.c_str());
	}
	return val.AsObject();
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
// �n���h���Q
//---------------------------------------------------------------------------

/**
 * �v�f�J�n
 */
static void startElement(void *userData,
						 const XML_Char *name,
						 const XML_Char **atts)
{
	iTJSDispatch2 *obj    = (iTJSDispatch2*)userData;
	iTJSDispatch2 *method = getMember(obj, L"startElement");

	// ����1 ���O�i������j
	tTJSVariant var1 = tTJSVariant(name);
	
	// ����2 �����i�����j
	iTJSDispatch2 *dict = TJSCreateDictionaryObject();
	const XML_Char **p = atts;
	while (*p) {
		dict->PropSet(TJS_MEMBERENSURE, p[0], NULL, &tTJSVariant(p[1]), dict);
		p += 2;
	}
	tTJSVariant var2 = tTJSVariant(dict);
	dict->Release();
	
	tTJSVariant *vars[2];
	vars[0] = &var1;
	vars[1] = &var2;

	method->FuncCall(0, NULL, NULL, NULL, 2, vars, obj);
	method->Release();
}

/**
 * �v�f�I��
 */
static void endElement(void *userData,
					   const XML_Char *name)
{
	iTJSDispatch2 *obj    = (iTJSDispatch2*)userData;
	iTJSDispatch2 *method = getMember(obj, L"endElement");

	// ����1 ���O�i������j
	tTJSVariant var1 = tTJSVariant(name);
	
	tTJSVariant *vars[1];
	vars[0] = &var1;

	method->FuncCall(0, NULL, NULL, NULL, 1, vars, obj);
	method->Release();
}

/**
 * �����f�[�^
 */
static void characterData(void *userData,
						  const XML_Char *s,
						  int len)
{
	iTJSDispatch2 *obj    = (iTJSDispatch2*)userData;
	iTJSDispatch2 *method = getMember(obj, L"characterData");

	// ����1 �e�L�X�g�i������j
	tTJSVariant var1 = tTJSVariant(ttstr(s,len));

	tTJSVariant *vars[1];
	vars[0] = &var1;
	
	method->FuncCall(0, NULL, NULL, NULL, 1, vars, obj);
	method->Release();
}

/**
 * �C���X�g���N�V����
 */
static void
processingInstruction(void *userData,
					  const XML_Char *target,
					  const XML_Char *data)
{
	iTJSDispatch2 *obj    = (iTJSDispatch2*)userData;
	iTJSDispatch2 *method = getMember(obj, L"processingInstruction");

	// ����1 �e�L�X�g�i������j
	tTJSVariant var1 = tTJSVariant(target);
	tTJSVariant var2 = tTJSVariant(data);
	
	tTJSVariant *vars[2];
	vars[0] = &var1;
	vars[1] = &var2;
	
	method->FuncCall(0, NULL, NULL, NULL, 2, vars, obj);
	method->Release();
}

/**
 * �R�����g
 */
static void
comment(void *userData,
		const XML_Char *data)
{
	iTJSDispatch2 *obj    = (iTJSDispatch2*)userData;
	iTJSDispatch2 *method = getMember(obj, L"comment");
	
	// ����1 �e�L�X�g�i������j
	tTJSVariant var1 = tTJSVariant(data);
	
	tTJSVariant *vars[1];
	vars[0] = &var1;
	
	method->FuncCall(0, NULL, NULL, NULL, 1, vars, obj);
	method->Release();
}

static void
startCdataSection(void *userData)
{
	iTJSDispatch2 *obj    = (iTJSDispatch2*)userData;
	iTJSDispatch2 *method = getMember(obj, L"startCdataSection");
	method->FuncCall(0, NULL, NULL, NULL, 0, NULL, obj);
	method->Release();
}

static void
endCdataSection(void *userData)
{
	iTJSDispatch2 *obj    = (iTJSDispatch2*)userData;
	iTJSDispatch2 *method = getMember(obj, L"endCdataSection");
	method->FuncCall(0, NULL, NULL, NULL, 0, NULL, obj);
	method->Release();
}

static void
defaultHandler(void *userData,
			   const XML_Char *s,
			   int len)
{
	iTJSDispatch2 *obj    = (iTJSDispatch2*)userData;
	iTJSDispatch2 *method = getMember(obj, L"defaultHandler");

	// ����1 �f�[�^��
	tTJSVariant var1 = tTJSVariant(ttstr(s,len));

	tTJSVariant *vars[1];
	vars[0] = &var1;
	
	method->FuncCall(0, NULL, NULL, NULL, 1, vars, obj);
	method->Release();
}

static void
defaultHandlerExpand(void *userData,
					 const XML_Char *s,
					 int len)
{
	iTJSDispatch2 *obj    = (iTJSDispatch2*)userData;
	iTJSDispatch2 *method = getMember(obj, L"defaultHandlerExpand");

	// ����1 �f�[�^��
	tTJSVariant var1 = tTJSVariant(ttstr(s,len));

	tTJSVariant *vars[1];
	vars[0] = &var1;
	
	method->FuncCall(0, NULL, NULL, NULL, 1, vars, obj);
	method->Release();
}

//---------------------------------------------------------------------------

#define TJS_NATIVE_CLASSID_NAME ClassID_XMLParser
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;

#define entryHandler(setter,name) if (isValidMember(target,L#name)) { XML_Set##setter##Handler(parser,name); };
#define entryHandler2(setter,name) if (isValidMember(target,L#name)) { XML_Set##setter(parser,name); };

/**
 * XMLParser
 */
class NI_XMLParser : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
protected:
	XML_Parser parser;
	iTJSDispatch2 *target;
	
public:

	/**
	 * �R���X�g���N�^
	 */
	NI_XMLParser() {
		parser = NULL;
		target = NULL;
	}

	/**
	 * TJS �R���X�g���N�^
	 * @param numparams �p�����[�^��
	 * @param param
	 * @param tjs_obj this �I�u�W�F�N�g
	 */
	tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) {

		parser = XML_ParserCreate(NULL);
		// �n���h���o�^����
		if (parser) {
			if (numparams > 0) {
				target = param[0]->AsObject();
			}
		}
		return S_OK;
	}

	/**
	 * �p�[�T�̏���������
	 */
	void init(iTJSDispatch2 *objthis) {
		iTJSDispatch2 *target = this->target ? this->target : objthis;
		XML_ParserReset(parser, L"UTF-8");
		XML_SetUserData(parser, target);
		entryHandler(StartElement, startElement);
		entryHandler(EndElement, endElement);
		entryHandler(CharacterData, characterData);
		entryHandler(ProcessingInstruction, processingInstruction);
		entryHandler(Comment, comment);
		entryHandler(StartCdataSection, startCdataSection);
		entryHandler(EndCdataSection, endCdataSection);
		entryHandler2(DefaultHandler, defaultHandler);
		entryHandler2(DefaultHandlerExpand, defaultHandlerExpand);
	}
	
	/**
	 * TJS invalidate
	 */
	void TJS_INTF_METHOD Invalidate() {
		if (parser) {
			XML_ParserFree(parser);
			parser = NULL;
		}
		if (target) {
			target->Release();
			target = NULL;
		}
	}

	XML_Parser getXMLParser() {
		return parser;
	}
	
	/**
	 * @param objthis �I�u�W�F�N�g
	 * @return XMLParser �C���X�^���X�B�擾���s������ NULL
	 */
	static XML_Parser getXMLParser(iTJSDispatch2 *objthis) {
		if (!objthis) return NULL;
		NI_XMLParser *_this;
		if (TJS_SUCCEEDED(objthis->NativeInstanceSupport(TJS_NIS_GETINSTANCE,
														 TJS_NATIVE_CLASSID_NAME, (iTJSNativeInstance**)&_this))) {
			return _this->parser;
		}
		return NULL;
	}

	// -----------------------------------------------------------

	/**
	 * �p�[�X�̎��s
	 * @param text �p�[�X�Ώۂ̃e�L�X�g
	 * @return ���������true
	 */
	bool parse(tTJSVariantString *text, iTJSDispatch2 *objthis) {
		bool ret = false;
		if (parser) {

			init(objthis);
			
			// UTF-8 ������ɖ߂�
			int len = ::WideCharToMultiByte(CP_UTF8, 0, *text, text->GetLength(), NULL, 0, NULL, NULL);
			char *buf = new char[len + 1];
			::WideCharToMultiByte(CP_UTF8, 0, *text, text->GetLength(), buf, len, NULL, NULL);
			buf[len] = '\0';
			ret = XML_Parse(parser, buf, len, TRUE) == XML_STATUS_OK;
			delete[] buf;
		}
		return ret;
	}

	/**
	 * �p�[�X�̎��s
	 * @param filename �p�[�X�Ώۂ̃t�@�C��
	 * @return ���������true
	 */
	bool parseStorage(tTJSVariantString *filename, iTJSDispatch2 *objthis) {

		bool ret = false;
		if (parser) {

			init(objthis);

			IStream *in = TVPCreateIStream(filename, TJS_BS_READ);
			if(!in) {
				TVPThrowExceptionMessage((ttstr(TJS_W("cannot open : ")) + *filename).c_str());
			}
			
			try {
				do {
					char buf[8192];
					ULONG len;
					if (in->Read(buf, sizeof buf, &len) == S_OK) {
						bool last = len < sizeof buf;
						ret = XML_Parse(parser, buf, len, last) == XML_STATUS_OK;
						if (last) {
							break;
						}
					} else {
						ret = false;
					}
				} while (ret);
			} catch (...) {
				in->Release();
				throw;
			}
			in->Release();
		}
		return ret;
	}

	// --------------------------------------------------------------------------

	XML_Error getErrorCode() {
		return parser ? XML_GetErrorCode(parser) : XML_ERROR_NONE;
	}

	tTVInteger getCurrentByteIndex() {
		return parser ? XML_GetCurrentByteIndex(parser) : 0;
	}

	tTVInteger getCurrentLineNumber() {
		return parser ? XML_GetCurrentLineNumber(parser) : 0;
	}

	tTVInteger getCurrentColumnNumber() {
		return parser ? XML_GetCurrentColumnNumber(parser) : 0;
	}

	tTVInteger getCurrentByteCount() {
		return parser ? XML_GetCurrentByteCount(parser) : 0;
	}
};

static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_XMLParser()
{
	return new NI_XMLParser();
}

static iTJSDispatch2 * Create_NC_XMLParser()
{
	tTJSNativeClassForPlugin * classobj = TJSCreateNativeClassForPlugin(TJS_W("XMLParser"), Create_NI_XMLParser);

	TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/XMLParser)

		TJS_DECL_EMPTY_FINALIZE_METHOD

		TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
			/*var.name*/_this,
			/*var.type*/NI_XMLParser,
			/*TJS class name*/XMLParser)
		{
			return TJS_S_OK;
		}
		TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/XMLParser)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parse)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_XMLParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			iTJSDispatch2 *target = numparams > 1 ? *param[1] : objthis;
			bool ret = _this->parse(param[0]->AsStringNoAddRef(), target);
			if (result) {
				*result = ret;
			}
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/parse)

		TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/parseStorage)
		{
			TJS_GET_NATIVE_INSTANCE(/*var. name*/_this, /*var. type*/NI_XMLParser);
			if (numparams < 1) return TJS_E_BADPARAMCOUNT;
			iTJSDispatch2 *target = numparams > 1 ? *param[1] : objthis;
			bool ret = _this->parseStorage(param[0]->AsStringNoAddRef(), target);
			if (result) {
				*result = ret;
			}
			return TJS_S_OK;
		}
		TJS_END_NATIVE_METHOD_DECL(/*func. name*/parseStorage)

		TJS_BEGIN_NATIVE_PROP_DECL(errorCode)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_XMLParser);
				*result = (tTVInteger)_this->getErrorCode();
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER
			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(errorCode)

		TJS_BEGIN_NATIVE_PROP_DECL(errorString)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_XMLParser);
				*result = XML_ErrorString(_this->getErrorCode());
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER
			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(errorString)

		TJS_BEGIN_NATIVE_PROP_DECL(currentByteIndex)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_XMLParser);
				*result = _this->getCurrentByteIndex();
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER
			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(currentByteIndex)

		TJS_BEGIN_NATIVE_PROP_DECL(currentLineNumber)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_XMLParser);
				*result = _this->getCurrentLineNumber();
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER
			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(currentLineNumber)

		TJS_BEGIN_NATIVE_PROP_DECL(currentColumnNumber)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_XMLParser);
				*result = _this->getCurrentColumnNumber();
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER
			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(currentColumnNumber)

		TJS_BEGIN_NATIVE_PROP_DECL(currentByteCount)
		{
			TJS_BEGIN_NATIVE_PROP_GETTER
			{
				TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,	/*var. type*/NI_XMLParser);
				*result = _this->getCurrentByteCount();
				return TJS_S_OK;
			}
			TJS_END_NATIVE_PROP_GETTER
			TJS_DENY_NATIVE_PROP_SETTER
		}
		TJS_END_NATIVE_PROP_DECL(currentByteCount)
			
	TJS_END_NATIVE_MEMBERS

	// �萔�̓o�^

	/*
		���̊֐��� classobj ��Ԃ��܂��B
	*/
	return classobj;
}

#undef TJS_NATIVE_CLASSID_NAME

//---------------------------------------------------------------------------

#if _MSC_VER <= 1200
// FIXME:
// ���̃v���O�}���� BCC �́c�c�i��
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
#else
int WINAPI DllMain(HINSTANCE hinst, unsigned long reason, void* lpReserved)
#endif
{
	return 1;
}

//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
CLINKAGE HRESULT EXPORT_DLL V2Link(iTVPFunctionExporter *exporter)
{
	// �X�^�u�̏�����(�K���L�q����)
	TVPInitImportStub(exporter);

	TVPAddImportantLog(ttstr(copyright));
	
	// TJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();
	
	if (global) {
		addMember(global, L"XMLParser", Create_NC_XMLParser());
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
CLINKAGE HRESULT EXPORT_DLL V2Unlink()
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
		delMember(global, L"XMLParser");
		global->Release();
	}

	// �X�^�u�̎g�p�I��(�K���L�q����)
	TVPUninitImportStub();

	return S_OK;
}
