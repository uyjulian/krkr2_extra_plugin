#include "ncbind/ncbind.hpp"

#include "../json/Writer.hpp"

#define OPTION_INDENT  (1<<0)
#define OPTION_CONST   (1<<1)
#define OPTION_KEYSORT (1<<2)
#define OPTION_HIDDEN  (1<<3)

// TJS����J�I�v�V����
#define OPTION_NOPREFIX      (1<<10) // int/real �v���t�B�b�N�X�����Ȃ��i��(const)�w�莞����j
#define OPTION_NOARRAYINDENT (1<<11) // Array�̓C���f���g�w��𖳎�

#define TJS_CONST TJS_W("(const)")

static void getVariantString(const tTJSVariant &var, IWriter *writer, tjs_uint32 option=0);

// Array �N���X�����o
static iTJSDispatch2 *ArrayClass       = NULL;   // Array
static iTJSDispatch2 *ArrayCountProp   = NULL;   // Array.count
static iTJSDispatch2 *ArrayAddFunc     = NULL;   // Array.add
static iTJSDispatch2 *ArraySortFunc    = NULL;   // Array.sort

// Array �����o�I�u�W�F�N�g�擾
static iTJSDispatch2* getArrayChildObject(const tjs_char *name)
{
	if (!ArrayClass) {
		// TJSCreateArrayObject��Array�N���X�I�u�W�F�N�g������̂ł���𗘗p����
		iTJSDispatch2 *dummy = TJSCreateArrayObject(&ArrayClass);
		dummy->Release();
		if (!ArrayClass) TVPThrowExceptionMessage(L"can't get Array object");
	}

	tTJSVariant val;
	if (TJS_FAILED(ArrayClass->PropGet(TJS_IGNOREPROP, name, NULL, &val, ArrayClass)))
		TVPThrowExceptionMessage(L"can't get Array.%1 member", ttstr(name));

	return val.AsObject();
}

// Array.count�擾
static tjs_int getArrayCount(iTJSDispatch2 *array) {
	tjs_int count = 0;
	tTJSVariant result;
	if (!ArrayCountProp) {
		ArrayCountProp = getArrayChildObject(TJS_W("count"));
		if (!ArrayCountProp) return 0;
	}
	if (TJS_SUCCEEDED(ArrayCountProp->PropGet(0, NULL, NULL, &result, array))) {
		count = result;
	}
	return count;
}

// Array.func �Ăяo��
static inline tjs_error invokeArrayFunc(iTJSDispatch2 *array, iTJSDispatch2* &func, tjs_char *const method,
								 tTJSVariant *result, tjs_int numparams, tTJSVariant **param)
{
	if (!func) {
		func = getArrayChildObject(method);
		if (!func) return TJS_E_MEMBERNOTFOUND;
	}
	return func->FuncCall(0, NULL, NULL, result, numparams, param, array);
}

/**
 * �����̓��e�\���p�̌Ăяo�����W�b�N
 */
class DictMemberDispCaller : public tTJSDispatch /** EnumMembers �p */
{
protected:
	IWriter *writer;
	tjs_uint32 option;
	bool first;
	iTJSDispatch2 *keys;
public:
	DictMemberDispCaller(IWriter *writer, tjs_uint32 option)
		: writer(writer), option(option), first(true), keys(0) {;}

	virtual ~DictMemberDispCaller() {
		if (keys) keys->Release();
	}

	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (numparams > 1) {
			tTVInteger flag = param[1]->AsInteger();
			if (!(flag & TJS_HIDDENMEMBER) || (option & OPTION_HIDDEN)) {
				if (option & OPTION_KEYSORT) {
					if (!keys) keys = TJSCreateArrayObject();
					invokeArrayFunc(keys, ArrayAddFunc, TJS_W("add"), NULL, 1, param); // keys.add(param[0]) 
				} else {
					write(*param[0], *param[2]);
				}
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}

	void write(const tTJSVariant &key, const tTJSVariant &value) {
		if (first) {
			first = false;
			if (option & OPTION_INDENT) writer->addIndent(); // �C���f���g�����͗v�f���P�ȏ゠�鎞�̂ݓK�p
		} else {
			writer->write((tjs_char)',');
			writer->newline();
		}
		ttstr name(key.GetString()), escape;
		name.EscapeC(escape);

		writer->write((tjs_char)'"');
		writer->write(escape.c_str());
		writer->write(L"\"=>");
		getVariantString(value, writer, option);
	}

	void cleanup(iTJSDispatch2 *dict) {
		if (keys) {
			// �\�[�g�����̂��߂�2�p�X��
			invokeArrayFunc(keys, ArraySortFunc, TJS_W("sort"), NULL, 0, NULL); // keys.sort()
			tjs_int count = getArrayCount(keys);
			for (tjs_int i=0; i<count; i++) {
				tTJSVariant key, value;
				if (TJS_SUCCEEDED(keys->PropGetByNum(TJS_IGNOREPROP, i, &key, keys)) &&
					TJS_SUCCEEDED(dict->PropGet(TJS_IGNOREPROP, key.GetString(), key.GetHint(), &value, dict)))
				{
					write(key, value);
				}
			}
		}
		if (!first && (option & OPTION_INDENT)) writer->delIndent(); // �C���f���g������
	}
};

static void getDictString(iTJSDispatch2 *dict, IWriter *writer, tjs_uint32 option=0)
{
	if (option & OPTION_CONST) writer->write(TJS_CONST);

	writer->write(L"%[");
	DictMemberDispCaller *caller = new DictMemberDispCaller(writer, option);
	tTJSVariantClosure closure(caller);
	tjs_uint32 novalue = (option & OPTION_KEYSORT) ? TJS_ENUM_NO_VALUE : 0; // �\�[�g����������ꍇ��value�s�v
	dict->EnumMembers(TJS_IGNOREPROP | novalue, &closure, dict);
	caller->cleanup(dict);
	caller->Release();
	writer->write((tjs_char)']');
}

static void getArrayString(iTJSDispatch2 *array, IWriter *writer, tjs_uint32 option=0)
{
	if (option & OPTION_CONST) writer->write(TJS_CONST);

	writer->write((tjs_char)'[');
	tjs_int count = getArrayCount(array);
	bool indent = (count > 0) && (option & OPTION_INDENT) && !(option & OPTION_NOARRAYINDENT); // �C���f���g�����t���O
	if (indent) writer->addIndent();
	for (tjs_int i=0; i<count; i++) {
		if (i != 0) {
			writer->write((tjs_char)',');
			if (indent) writer->newline();
		}
		tTJSVariant result;
		if (array->PropGetByNum(TJS_IGNOREPROP, i, &result, array) == TJS_S_OK) {
			getVariantString(result, writer, option);
		}
	}
	if (indent) writer->delIndent();
	writer->write((tjs_char)']');
}

static void
getVariantString(const tTJSVariant &var, IWriter *writer, tjs_uint32 option)
{
	tTJSVariantType type = var.Type();

	if (type == tvtObject)
	{
		iTJSDispatch2 *obj = var.AsObjectNoAddRef();
		if (obj == NULL) {
			writer->write(L"null");
		} else if (obj->IsInstanceOf(TJS_IGNOREPROP,NULL,NULL,L"Array",obj) == TJS_S_TRUE) {
			getArrayString(obj, writer, option);
		} else {
			getDictString(obj, writer, option);
		}
	}
	else
	{
		if (!(option & (OPTION_CONST|OPTION_NOPREFIX))) { // (const)���̓G���[�ɂȂ�̂ł��Ȃ�
			if      (type == tvtInteger) writer->write(L"int ");
			else if (type == tvtReal)    writer->write(L"real ");
		}
		writer->write(TJSVariantToExpressionString(var).c_str());
	}
}

//---------------------------------------------------------------------------

/**
 * ���\�b�h�ǉ��p
 */
class ArrayAdd {

public:
	ArrayAdd(){;}

	/**
	 * save �`���ł̎����܂��͔z��̕ۑ�
	 * @param filename �t�@�C����
	 * @param utf true �Ȃ� UTF-8 �ŏo��
	 * @param newline ���s�R�[�h 0:CRLF 1:LF
	 * @return ���s����
	 */
	static tjs_error TJS_INTF_METHOD save2(tTJSVariant *result,
										   tjs_int numparams,
										   tTJSVariant **param,
										   iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		IFileWriter writer(param[0]->GetString(),
						   numparams > 1 ? (int)*param[1] != 0: false,
						   numparams > 2 ? (int)*param[2] : 0
						   );
		writer.hex = true;
		tjs_int count = getArrayCount(objthis);
		for (tjs_int i=0; i<count; i++) {
			tTJSVariant result;
			if (objthis->PropGetByNum(TJS_IGNOREPROP, i, &result, objthis) == TJS_S_OK) {
				writer.write(result.GetString());
				writer.newline();
			}
		}
		return TJS_S_OK;
	}

	/**
	 * saveStruct �`���ł̃I�u�W�F�N�g�̕ۑ�
	 * @param filename �t�@�C����
	 * @param utf true �Ȃ� UTF-8 �ŏo��
	 * @param newline ���s�R�[�h 0:CRLF 1:LF
	 * @param option ���`�I�v�V����bitmask 1:�C���f���g���s��, 2:(const)����, 4:�����L�[�\�[�g
	 * @return ���s����
	 */
	static tjs_error TJS_INTF_METHOD saveStruct2(tTJSVariant *result,
												 tjs_int numparams,
												 tTJSVariant **param,
												 iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		IFileWriter writer(param[0]->GetString(),
						   numparams > 1 ? (int)*param[1] != 0: false,
						   numparams > 2 ? (int)*param[2] : 0
						   );
		writer.hex = true;
		getArrayString(objthis, &writer, numparams > 3 ? (int)*param[3] : 0);
		return TJS_S_OK;
	}
	
	/**
	 * saveStruct �`���ŕ�����
	 * @param newline ���s�R�[�h 0:CRLF 1:LF
	 * @param option ���`�I�v�V����bitmask 1:�C���f���g���s��, 2:(const)����, 4:�����L�[�\�[�g
	 * @return ���s����
	 */
	static tjs_error TJS_INTF_METHOD toStructString(tTJSVariant *result,
													tjs_int numparams,
													tTJSVariant **param,
													iTJSDispatch2 *objthis) {
		if (result) {
			IStringWriter writer(TJS_PARAM_EXIST(0) ? (int)*param[0] : 1);
			writer.hex = true;
			getArrayString(objthis, &writer, numparams > 1 ? (int)*param[1] : 0);
			*result = writer.buf;
		}
		return TJS_S_OK;
	}
};

NCB_ATTACH_CLASS(ArrayAdd, Array) {
	RawCallback("save2", &ArrayAdd::save2, 0);
	RawCallback("saveStruct2", &ArrayAdd::saveStruct2, 0);
	RawCallback("toStructString", &ArrayAdd::toStructString, 0);
};

/**
 * ���\�b�h�ǉ��p
 */
class DictAdd {

public:
	DictAdd(){;}

	/**
	 * saveStruct �`���ł̃I�u�W�F�N�g�̕ۑ�
	 * @param filename �t�@�C����
	 * @param utf true �Ȃ� UTF-8 �ŏo��
	 * @param newline ���s�R�[�h 0:CRLF 1:LF
	 * @param option ���`�I�v�V����bitmask 1:�C���f���g���s��, 2:(const)����, 4:�����L�[�\�[�g
	 * @return ���s����
	 */
	static tjs_error TJS_INTF_METHOD saveStruct2(tTJSVariant *result,
												 tjs_int numparams,
												 tTJSVariant **param,
												 iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		IFileWriter writer(param[0]->GetString(),
						   numparams > 1 ? (int)*param[1] != 0: false,
						   numparams > 2 ? (int)*param[2] : 0
						   );
		writer.hex = true;
		getDictString(objthis, &writer, numparams > 3 ? (int)*param[3] : 0);
		return TJS_S_OK;
	}
	
	/**
	 * saveStruct �`���ŕ�����
	 * @param newline ���s�R�[�h 0:CRLF 1:LF
	 * @param option ���`�I�v�V����bitmask 1:�C���f���g���s��, 2:(const)����, 4:�����L�[�\�[�g
	 * @return ���s����
	 */
	static tjs_error TJS_INTF_METHOD toStructString(tTJSVariant *result,
													tjs_int numparams,
													tTJSVariant **param,
													iTJSDispatch2 *objthis) {
		if (result) {
			IStringWriter writer(TJS_PARAM_EXIST(0) ? (int)*param[0] : 1);
			writer.hex = true;
			getDictString(objthis, &writer, numparams > 1 ? (int)*param[1] : 0);
			*result = writer.buf;
		}
		return TJS_S_OK;
	}
};

NCB_ATTACH_CLASS(DictAdd, Dictionary) {
	RawCallback("saveStruct2", &DictAdd::saveStruct2, TJS_STATICMEMBER);
	RawCallback("toStructString", &DictAdd::toStructString, TJS_STATICMEMBER);
};

/**
 * ���\�b�h�ǉ��p
 */
class ScriptsAdd {
	
public:
	ScriptsAdd(){;}

	/**
	 * saveStruct �`���ŕ�����
	 * @param target �����񉻂���Ώ�
	 * @param newline ���s�R�[�h 0:CRLF 1:LF
	 * @param option ���`�I�v�V����bitmask 1:�C���f���g���s��, 2:(const)����, 4:�����L�[�\�[�g
	 * @return ���s����
	 */
	static tjs_error TJS_INTF_METHOD toStructString(tTJSVariant *result,
													tjs_int numparams,
													tTJSVariant **param,
													iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		if (result) {
			IStringWriter writer(TJS_PARAM_EXIST(1) ? (int)*param[1] : 1);
			writer.hex = true;
			getVariantString(*param[0], &writer, numparams > 2 ? (int)*param[2] : 0);
			*result = writer.buf;
		}
		return TJS_S_OK;
	}
};

NCB_ATTACH_CLASS(ScriptsAdd, Scripts) {
	RawCallback("toStructString", &ScriptsAdd::toStructString, TJS_STATICMEMBER);
};


/**
 * �o�^������
 */
static void PostRegistCallback()
{
	// sso�I�v�V�����l�o�^
	iTJSDispatch2 *global = TVPGetScriptDispatch();
	if (global) {
		global->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, TJS_W("ssoIndent"), NULL, &tTJSVariant(OPTION_INDENT),  global);
		global->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, TJS_W("ssoConst"),  NULL, &tTJSVariant(OPTION_CONST),   global);
		global->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, TJS_W("ssoSort"),   NULL, &tTJSVariant(OPTION_KEYSORT), global);
		global->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP, TJS_W("ssoHidden"), NULL, &tTJSVariant(OPTION_HIDDEN), global);
		global->Release();
	}
}

/**
 * �J�������O
 */
static void PreUnregistCallback()
{
	// sso�I�v�V�����l�o�^
	iTJSDispatch2 *global = TVPGetScriptDispatch();
	if (global) {
		global->DeleteMember(0, TJS_W("ssoIndent"), NULL, global);
		global->DeleteMember(0, TJS_W("ssoConst"),  NULL, global);
		global->DeleteMember(0, TJS_W("ssoSort"),   NULL, global);
		global->DeleteMember(0, TJS_W("ssoHidden"), NULL, global);
		global->Release();
	}

	// Array�����o�Q�ƊJ��
	if (ArraySortFunc)  ArraySortFunc  -> Release();
	if (ArrayAddFunc)   ArrayAddFunc   -> Release();
	if (ArrayCountProp) ArrayCountProp -> Release();
	if (ArrayClass)     ArrayClass     -> Release();

	ArrayClass     = NULL;
	ArrayCountProp = NULL;
	ArrayAddFunc   = NULL;
	ArraySortFunc  = NULL;
}

NCB_POST_REGIST_CALLBACK(PostRegistCallback);
NCB_PRE_UNREGIST_CALLBACK(PreUnregistCallback);
