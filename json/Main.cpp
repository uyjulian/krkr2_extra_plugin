/**
 * copyright (c) 2007 Go Watanabe
 */

#include <windows.h>
#include "tp_stub.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>

using namespace std;

#define UNICODE_BOM (0xfeff)

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

// ----------------------------------------------------------------------

class IFileStorage  {

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
	
	bool addNextLine(ttstr &str) {
		int c;
		string mbline;
		while ((c = getc()) != EOF && !endOfLine(c)) {
			mbline += c;
		}
		int l = (int)mbline.length();
		if (l > 0 || c != EOF) {
			wchar_t *buf = new wchar_t[l + 1];
			l = MultiByteToWideChar(codepage, 0,
									mbline.data(),
									(int)mbline.length(),
									buf, l);
			buf[l] = '\0';
			str += buf;
			delete buf;
			return true;
		} else {
			return false;
		}
	}
};


// -----------------------------------------------------------------
class JSONTextReadStream : public iTJSTextReadStream
{
public:
  IFileStorage *Storage;
  tjs_int codepage;
  ttstr buf;
  tjs_int pos;

  JSONTextReadStream(tTJSVariantString *filename, tjs_int codepage) {
    Storage = new IFileStorage(filename, codepage);
    pos = 0;
  }

  ~JSONTextReadStream(void) {
    delete Storage;
  }

  virtual tjs_uint TJS_INTF_METHOD Read(tTJSString & targ, tjs_uint size) {
    tjs_uint readSize = 0;
    while (readSize < size) {
      if (pos >= buf.length()) {
	buf.Clear();
	pos = 0;
	if (! Storage->addNextLine(buf))
	  break;
      }
      tjs_uint n = min(tjs_uint(buf.length() - pos), size - readSize);
      readSize += n;
      while (n > 0) {
	targ += buf[pos];
	pos++;
	n--;
      }
    }
    return readSize;
  }

  virtual void TJS_INTF_METHOD Destruct() {
    delete this;
  }
};

// -----------------------------------------------------------------

class IReader {
public:

	virtual int getc() = 0;
	virtual void ungetc() = 0;
	virtual void close() = 0;

	bool isError;

	/**
	 * �R���X�g���N�^
	 */
	IReader() {
		isError = false;
	}

	virtual ~IReader() {};
	
	/**
	 * �G���[����
	 */
	void error(const tjs_char *msg) {
		isError = true;
		log(msg);
	}
	
	/**
	 * �s���܂œǂݔ�΂�
	 */
    void toEOL() {
        int c;
        do {
            c = getc();
		} while (c != EOF && c != '\n' && c != '\r');
    }

    /**
     * �󔒂ƃR�����g���������Ď��̕�����Ԃ�
     */
	int next() {
        for (;;) {
			int c = getc();
			if (c == '#') {
				toEOL();
			} else if (c == '/') {
				switch (getc()) {
				case '/':
					toEOL();
					break;
                case '*':
					for (;;) {
						c = getc();
						if (c == EOF) {
							error(L"�R�����g�����Ă��܂���");
							return EOF;
                        }
                        if (c == '*') {
                            if (getc() == '/') {
                                break;
                            }
                            ungetc();
                        }
                    }
                    break;
                default:
                    ungetc();
                    return '/';
                }
			} else if (c != UNICODE_BOM && (c == EOF || c > ' ')) {
				return c;
			}
		}
	}

    /**
     * �w�肳�ꂽ���������̕�������擾
     * @param str ������̊i�[��
     * @param n ������
     */
    void next(ttstr &str, int n) {
		str = "";
		while (n > 0) {
			int c = getc();
			if (c == EOF) {
				break;
			}
			str += c;
			n--;
		}
	}
	
    void parseObject(tTJSVariant &var) {

        // �����𐶐�
        iTJSDispatch2 *dict = TJSCreateDictionaryObject();
        var = tTJSVariant(dict, dict);
	dict->Release();

        int c;

        for (;;) {

            tTJSVariant key;

            c = next();
			if (c == EOF) {
				error(L"�I�u�W�F�N�g�� '}' �ŏI������K�v������܂�");
				return;
			} else if (c == '}') {
				return;
			} else if (c == ',' || c == ';') {
				ungetc();
			} else {

				ungetc();
				parse(key);

				c = next();
				if (c == '=') {
					if (getc() != '>') {
						ungetc();
					}
				} else if (c != ':') {
					error(L"�L�[�̌�ɂ� ':' �܂��� '=' �܂��� '=>' ���K�v�ł�");
					return;
				}

				// �����o�o�^
				tTJSVariant value;
				parse(value);
				
				dict->PropSet(TJS_MEMBERENSURE, key.GetString(), NULL, &value, dict);
			}
			
			switch (next()) {
			case ';':
			case ',':
				break;
			case '}':
				return;
			default:
				error(L" ',' �܂��� ';' �܂��� '}' ���K�v�ł�");
				return;
			}
        }
    }

	void parseArray(tTJSVariant &var) {
        
        // �z��𐶐�
		iTJSDispatch2 *array = TJSCreateArrayObject();
		var = tTJSVariant(array, array);
		array->Release();

        tjs_int cnt = 0;

        for (;;) {
            int ch = next();
			switch (ch) {
			case EOF:
				error(L"�z��� ']' �ŏI������K�v������܂�");
				return;
			case ']':
				return;
			case ',':
			case ';':
				{
					ungetc();
					// ��̃J������o�^
					tTJSVariant value;
					array->PropSetByNum(TJS_MEMBERENSURE, cnt++, &value, array);
				}
				break;
			default:
				ungetc();
				tTJSVariant value;
				parse(value);
				array->PropSetByNum(TJS_MEMBERENSURE, cnt++, &value, array);
            }

            switch (next()) {
            case ';':
            case ',':
                break;
            case ']':
                return;
            default:
				error(L" ',' �܂��� ';' �܂��� ']' ���K�v�ł�");
                return;
            }
        }
    }

    /**
	 * �N�I�[�g������̃p�[�X
     * @param quote �N�I�[�g����
     * @param var �i�[��
     */
	void parseQuoteString(int quote, tTJSVariant &var) {
		int c;
		ttstr str;
		for (;;) {
			c = getc();
			switch (c) {
			case 0:
			case '\n':
			case '\r':
				error(L"�����񂪏I�[���Ă��܂���");
				return;
			case '\\':
				c = getc();
				switch (c) {
				case 'b':
					str += '\b';
					break;
				case 'f':
					str += '\f';
					break;
				case 't':
					str += '\t';
					break;
				case 'r':
					str += '\r';
					break;
				case 'n':
					str += '\n';
					break;
				case 'u':
					{
						ttstr work;
						next(work, 4);
						str += (tjs_char)wcstol(work.c_str(), NULL, 16);
					}
					break;
				case 'x' :
					{
						ttstr work;
						next(work, 2);
						str += (tjs_char)wcstol(work.c_str(), NULL, 16);
					};
					break;
				default:
					str += c;
				}
				break;
			default:
				if (c == quote) {
					var = str;
					return;
				}
				str += c;
			}
		}
	}

	/**
	 * �w�肵�����������l�̂P�����ڂ̍\���v�f���ǂ���
	 */
	bool isNumberFirst(int ch) {
		return (ch >= '0' && ch <= '9') || ch == '.' || ch == '-' || ch == '+';
	}

	/**
	 * �w�肵�����������l�̍\���v�f���ǂ���
	 */
	bool isNumber(int ch) {
		return (ch >= '0' && ch <= '9') || ch == '.' || ch == '-' || ch == '+' || ch == 'e' || ch == 'E';
	}

	/**
	 * �w�肵��������������̍\���v�f���ǂ���
	 */
	bool isString(int ch) {
		return ch > 0x80 || ch > ' ' && wcschr(L",:]}/\\\"[{;=#", ch) == NULL;
	}

	/**
	 * �p�[�X�̎��s
	 * @param var ���ʊi�[��
	 */
	void parse(tTJSVariant &var) {
		
		int ch = next();

		switch (ch) {
		case '"':
		case '\'':
			// �N�I�[�g������
			parseQuoteString(ch, var);
			break;
		case '{':
			// �I�u�W�F�N�g
			parseObject(var);
			break;
		case '[':
			parseArray(var);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '.':
		case '-':
		case '+':
			{
				// ���l
				bool doubleValue = false;
				
				ttstr s;
				while (isNumber(ch)) {
					if (ch == '.') {
						doubleValue = true; // �Ђǂ��������i��)
					}
					s += ch;
					ch = getc();
				}
				ungetc();

				// ���l
				if (doubleValue) {
					double value = wcstod(s.c_str(), NULL);
					var = value;
				} else {
					tjs_int64 value = _wcstoi64(s.c_str(), NULL, 0);
					var = value;
				}
			}
			break;
		default:
			if (ch >= 'a' && ch <= 'z') {
			
				// ������𒊏o
				ttstr s;
				while (ch >= 'a' && ch <= 'z') {
					s += ch;
					ch = getc();
				}
				ungetc();
				
				// ���ʎq
				if (s == L"true") {
					var = true;
				} else if (s == L"false") {
					var = false;
				} else if (s == L"null") {
					var.Clear();
				} else if (s == L"void") {
					var.Clear();
				} else {
					ttstr msg = L"�s���ȃL�[���[�h�ł�:";
					msg += s;
					error(msg.c_str());
				}
			} else {
				ttstr msg = L"�s���ȕ����ł�:";
				error(msg.c_str());
			}
		}
    }
};

class IFileReader : public IReader {

	/// ���̓o�b�t�@
	ttstr buf;
	/// ���̓X�g���[��
	iTJSTextReadStream *stream;
	
	ULONG pos;
	bool eofFlag;
	
public:
	IFileReader(tTJSVariantString *filename, tjs_int codepage) {
		stream = new JSONTextReadStream(filename, codepage);
		pos = 0;
		eofFlag = false;
	}

	virtual void close() {
		if (stream) {
			stream->Destruct();
			stream = NULL;
		}
	}
	
	~IFileReader() {
		close();
	}

	
	int getc() {
		if (pos < buf.length()) {
			return buf.c_str()[pos++];
		} else {
			if (!stream || eofFlag) {
				return EOF;
			} else {
				pos = 0;
				buf.Clear();
				eofFlag = stream->Read(buf, 1024) < 1024;
				return getc();
			}
		}
	}

	void ungetc() {
		if (pos > 0) {
			pos--;
		}
	}

};

class IStringReader : public IReader {

	ttstr dat;
	const tjs_char *p;
	ULONG length;
	ULONG pos;

public:
	IStringReader(const tjs_char *str) {
		dat = str;
		p = dat.c_str();
		length = dat.length();
		pos = 0;
	}

	void close() {
	}
	
	int getc() {
		return pos < length ? p[pos++] : EOF;
	}
	
	void ungetc() {
		if (pos > 0) {
			pos--;
		}
	}
};

// -----------------------------------------------------------------

#include "Writer.hpp"

//---------------------------------------------------------------------------

// Array �N���X�����o
static iTJSDispatch2 *ArrayCountProp   = NULL;   // Array.count

// -----------------------------------------------------------------

static void
addMethod(iTJSDispatch2 *dispatch, const tjs_char *methodName, tTJSDispatch *method)
{
	tTJSVariant var = tTJSVariant(method);
	method->Release();
	dispatch->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		methodName, // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
		NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
		&var, // �o�^����l
		dispatch // �R���e�L�X�g
		);
}

static void
delMethod(iTJSDispatch2 *dispatch, const tjs_char *methodName)
{
	dispatch->DeleteMember(
		0, // �t���O ( 0 �ł悢 )
		methodName, // �����o��
		NULL, // �q���g
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

// -----------------------------------------------------------------

static tjs_error eval(IReader &file, tTJSVariant *result)
{
	tjs_error ret = TJS_S_OK;
	if (result) {
		file.parse(*result);
	}
	file.close();
	if (file.isError) {
		TVPThrowExceptionMessage(L"JSON�t�@�C�� �̃p�[�X�Ɏ��s���܂���");
	}
	return ret;
}

//---------------------------------------------------------------------------

/**
 * JSON �𕶎��񂩂�ǂݎ��
 * @param text JSON �̕�����\��
 */
class tEvalJSON : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		if (membername) return TJS_E_MEMBERNOTFOUND;
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;

		eval(IStringReader(param[0]->GetString()), result);
		return TJS_S_OK;
	}
};

/**
 * JSON ���t�@�C������ǂݎ��
 * @param filename �t�@�C����
 */
class tEvalJSONStorage : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		if (membername) return TJS_E_MEMBERNOTFOUND;
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;

		tTJSVariantString *filename = param[0]->AsStringNoAddRef();
		bool utf8 = numparams >= 2 ? (int*)param[1] != 0 : false;
		tjs_int codepage = utf8 ? CP_UTF8 : CP_ACP;

		eval(IFileReader(filename, codepage), result);
		return TJS_S_OK;
	}
};

static void
quoteString(const tjs_char *str, IWriter *writer)
{
	if (str) {
		writer->write((tjs_char)'"');
		const tjs_char *p = str;
		int ch;
		while ((ch = *p++)) {
			if (ch == '"') {
				writer->write(L"\\\"");
			} else if (ch == '\\') {
			  writer->write(L"\\\\");
			} else if (ch == 0x08) {
			  writer->write(L"\\b");
			} else if (ch == 0x0c) {
			  writer->write(L"\\f");
			} else if (ch == 0x0a) {
			  writer->write(L"\\n");
			} else if (ch == 0x0d) {
			  writer->write(L"\\r");
			} else if (ch == 0x09) {
			  writer->write(L"\\t");
			} else if (ch < 0x20) {
			  wchar_t buf[256];
			  swprintf(buf, 255, L"\\u%04x", ch);
			  writer->write(buf);
			} else {
				writer->write((tjs_char)ch);
			}
		}
		writer->write((tjs_char)'"');
	} else {
		writer->write(L"\"\"");
	}
}

static void getVariantString(tTJSVariant &var, IWriter *writer);

/**
 * �����̓��e�\���p�̌Ăяo�����W�b�N
 */
class DictMemberDispCaller : public tTJSDispatch /** EnumMembers �p */
{
protected:
	IWriter *writer;
	bool first;
public:
	DictMemberDispCaller(IWriter *writer) : writer(writer) { first = true; };
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
			if (!(flag & TJS_HIDDENMEMBER)) {
				if (first) {
					first = false;
				} else {
					writer->write((tjs_char)',');
					writer->newline();
				}
				quoteString(param[0]->GetString(), writer);
				writer->write((tjs_char)':');
				getVariantString(*param[2], writer);
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}
};

static void getDictString(iTJSDispatch2 *dict, IWriter *writer)
{
	writer->write((tjs_char)'{');
	writer->addIndent();
	DictMemberDispCaller *caller = new DictMemberDispCaller(writer);
	tTJSVariantClosure closure(caller);
	dict->EnumMembers(TJS_IGNOREPROP, &closure, dict);
	caller->Release();
	writer->delIndent();
	writer->write((tjs_char)'}');
}

static void getArrayString(iTJSDispatch2 *array, IWriter *writer)
{
	writer->write((tjs_char)'[');
	writer->addIndent();
	tjs_int count = 0;
	{
		tTJSVariant result;
		if (TJS_SUCCEEDED(ArrayCountProp->PropGet(0, NULL, NULL, &result, array))) {
			count = result.AsInteger();
		}
	}
	for (tjs_int i=0; i<count; i++) {
		if (i != 0) {
			writer->write((tjs_char)',');
			writer->newline();
		}
		tTJSVariant result;
		if (array->PropGetByNum(TJS_IGNOREPROP, i, &result, array) == TJS_S_OK) {
			getVariantString(result, writer);
		}
	}
	writer->delIndent();
	writer->write((tjs_char)']');
}

static void
getVariantString(tTJSVariant &var, IWriter *writer)
{
	switch(var.Type()) {

	case tvtVoid:
		writer->write(L"null");
		break;
		
	case tvtObject:
		{
			iTJSDispatch2 *obj = var.AsObjectNoAddRef();
			if (obj == NULL) {
				writer->write(L"null");
			} else if (obj->IsInstanceOf(TJS_IGNOREPROP,NULL,NULL,L"Array",obj) == TJS_S_TRUE) {
				getArrayString(obj, writer);
			} else {
				getDictString(obj, writer);
			}
		}
		break;
		
	case tvtString:
		quoteString(var.GetString(), writer);
		break;

	case tvtInteger:
		writer->write((tTVInteger)var);
		break;

	case tvtReal: {
	  ttstr str = var;
	  // delete top '+' of number.
	  if (str[0] == L'+') {
	    ttstr src = str;
	    str = src.c_str() + 1;
	  }
	  writer->write(str.c_str());
	  break;
	}

	default:
		writer->write(L"null");
		break;
	};
}

/**
 *
 */
class tSaveJSON : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		IFileWriter writer(param[0]->GetString(), 
			numparams > 2 ? (int)*param[2] != 0 : false, 
			numparams > 3 ? (int)*param[3] : 0);
		getVariantString(*param[1], &writer);
		return TJS_S_OK;
	}
};

/**
 *
 */
class tToJSONString : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		if (result) {
			IStringWriter writer(numparams > 1 ? *param[1] : 0);
			getVariantString(*param[0], &writer);
			*result = writer.buf;
		}
		return TJS_S_OK;
	}
};

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


	// Arary �N���X�����o�[�擾
	{
		tTJSVariant varScripts;
		TVPExecuteExpression(TJS_W("Array"), &varScripts);
		iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
		// �����o�擾
		ArrayCountProp = getMember(dispatch, TJS_W("count"));
	}

	{
		tTJSVariant varScripts;
		TVPExecuteExpression(TJS_W("Scripts"), &varScripts);
		iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
		if (dispatch) {
			addMethod(dispatch, L"evalJSON",        new tEvalJSON());
			addMethod(dispatch, L"evalJSONStorage", new tEvalJSONStorage());
			addMethod(dispatch, L"saveJSON",        new tSaveJSON());
			addMethod(dispatch, L"toJSONString",    new tToJSONString());
		}
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
static void TJS_USERENTRY tryUnlinkScripts(void *data)
{
  tTJSVariant varScripts;
  TVPExecuteExpression(TJS_W("Scripts"), &varScripts);
  iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
  if (dispatch) {
    delMethod(dispatch, L"evalJSON");
    delMethod(dispatch, L"evalJSONStorage");
  }
}

static bool TJS_USERENTRY catchUnlinkScripts(void *data, const tTVPExceptionDesc & desc) {
  return false;
}


extern "C" HRESULT _stdcall V2Unlink()
{
	// �g���g��������A�v���O�C����������悤�Ƃ���Ƃ��ɌĂ΂��֐��B

	// �������炩�̏����Ńv���O�C��������ł��Ȃ��ꍇ��
	// ���̎��_�� E_FAIL ��Ԃ��悤�ɂ���B
	// �����ł́ATVPPluginGlobalRefCount �� GlobalRefCountAtInit ����
	// �傫���Ȃ��Ă���Ύ��s�Ƃ������Ƃɂ���B
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
	// E_FAIL ���A��ƁAPlugins.unlink ���\�b�h�͋U��Ԃ�

	{
          TVPDoTryBlock(&tryUnlinkScripts, &catchUnlinkScripts, NULL, NULL);
	}
	
	if (ArrayCountProp) {
		ArrayCountProp->Release();
		ArrayCountProp = NULL;
	}
	
	// �X�^�u�̎g�p�I��(�K���L�q����)
	TVPUninitImportStub();

	return S_OK;
}
