#include <windows.h>
#include <activscp.h>
#include <stdio.h>
#include <map>

#include "ncbind/ncbind.hpp"

#define GLOBAL L"kirikiri"

// ���O�o�͗p
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

#include "../win32ole/IDispatchWrapper.hpp"

//---------------------------------------------------------------------------

/*
 * Windows Script Host �����p�l�C�e�B�u�C���X�^���X
 */
class WindowsScriptHost : IActiveScriptSite
{
public:
	static WindowsScriptHost *singleton;

	static void initSingleton() {
		singleton = new WindowsScriptHost();
	}

	static void removeSingleton() {
		if (singleton) {
			delete singleton;
			singleton = NULL;
		}
	}
	
protected:
	/// tjs global �ێ��p
	IDispatchEx *global;

	// �R���e�L�X�g���
	map<DWORD, ttstr> contextMap;
	DWORD contextCount;
	
	DWORD getContextCookie(const tjs_char *filename) {
		DWORD ret = contextCount++;
		contextMap[ret] = ttstr(filename);
		return ret;
	}

	ttstr getContextCookieName(DWORD cookie) {
		if (cookie > 0) {
			map<DWORD,ttstr>::const_iterator it = contextMap.find(cookie);
			if (it != contextMap.end()) {
				return it->second;
			}
		}
		return "";
	}
	
	// ------------------------------------------------------
	// IUnknown ����
	// ------------------------------------------------------
protected:
	ULONG refCount;
public:
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject) {
		*ppvObject = NULL;
		return E_NOTIMPL;
	}
	virtual ULONG _stdcall AddRef(void) {
		return ++refCount;
	}
	virtual ULONG _stdcall Release(void) {
		if(--refCount <= 0) return 0;
		return refCount;
	}

	// ------------------------------------------------------
	// IActiveScriptSite ����
	// ------------------------------------------------------
public:
	virtual HRESULT __stdcall GetLCID(LCID *plcid) {
		return S_OK;
	}

	virtual HRESULT __stdcall GetItemInfo(LPCOLESTR pstrName,
										  DWORD dwReturnMask, IUnknown **ppunkItem, ITypeInfo **ppti) {
		if (ppti) {
			*ppti = NULL;
		}
		if (ppunkItem) {
			*ppunkItem = NULL;
			if (dwReturnMask & SCRIPTINFO_IUNKNOWN) {
				if (!_wcsicmp(GLOBAL, pstrName)) {
					global->AddRef();
					*ppunkItem = global;
				}
			}
		}
		return S_OK;
	}
	
	virtual HRESULT __stdcall GetDocVersionString(BSTR *pbstrVersion) {
		return S_OK;
	}
	
	virtual HRESULT __stdcall OnScriptTerminate(const VARIANT *pvarResult, const EXCEPINFO *ei) {
		return S_OK;
	}
	
	virtual HRESULT __stdcall OnStateChange(SCRIPTSTATE ssScriptState) {
		return S_OK;
	}
	
	virtual HRESULT __stdcall OnScriptError(IActiveScriptError *pscriptError) {
		log(TJS_W("OnScriptError"));
		ttstr errMsg;
		BSTR sourceLine;
		if (pscriptError->GetSourceLineText(&sourceLine) == S_OK) {
			log(TJS_W("source:%ls"), sourceLine);
			::SysFreeString(sourceLine);
		}
		DWORD sourceContext;
		ULONG lineNumber;
		LONG charPosition;
		if (pscriptError->GetSourcePosition(
			&sourceContext,
			&lineNumber,
			&charPosition) == S_OK) {
			log(TJS_W("context:%ls lineNo:%d pos:%d"), getContextCookieName(sourceContext).c_str(), lineNumber, charPosition);
		}		
		EXCEPINFO ei;
		memset(&ei, 0, sizeof ei);
		if (pscriptError->GetExceptionInfo(&ei) == S_OK) {
			log(TJS_W("exception code:%x desc:%ls"), ei.wCode, ei.bstrDescription);
		}
		return S_OK;
	}

	virtual HRESULT __stdcall OnEnterScript(void) {
		return S_OK;
	}
	
	virtual HRESULT __stdcall OnLeaveScript(void) {
		return S_OK;
	}

	// ------------------------------------------------------
	// ������
	// ------------------------------------------------------

protected:
	/// �g���q��ProgId �̃}�b�s���O
	map<ttstr, ttstr> extMap;
	// CLSID ��r�p
	struct CompareCLSID : public binary_function<CLSID,CLSID,bool> {
		bool operator() (const CLSID &key1, const CLSID &key2) const {
#define CHK(a) if (key1.a!=key2.a) { return key1.a<key2.a; }
			CHK(Data1);
			CHK(Data2);
			CHK(Data3);
			CHK(Data4[0]);
			CHK(Data4[1]);
			CHK(Data4[2]);
			CHK(Data4[3]);
			CHK(Data4[4]);
			CHK(Data4[5]);
			CHK(Data4[6]);
			CHK(Data4[7]);
			return false;
		}
	};
	map<CLSID, IActiveScript*, CompareCLSID> scriptMap;

	/**
	 * �w�肳�ꂽ ActiveScript �G���W�����擾����
	 * @param type �g���q �܂��� progId �܂��� CLSID
	 * @return �G���W���C���^�[�t�F�[�X
	 */
	IActiveScript *getScript(const tjs_char *type) {
		HRESULT hr;
		CLSID   clsid;
		
		// ProgId �܂��� CLSID �̕�����\������G���W���� CLSID �����肷��
		OLECHAR *oleType = ::SysAllocString(type);
		if (FAILED(hr = CLSIDFromProgID(oleType, &clsid))) {
			hr = CLSIDFromString(oleType, &clsid);
		}
		::SysFreeString(oleType);

		if (SUCCEEDED(hr)) {
			map<CLSID, IActiveScript*, CompareCLSID>::const_iterator n = scriptMap.find(clsid);
			if (n != scriptMap.end()) {
				// ���łɎ擾�ς݂̃G���W���̏ꍇ�͂����Ԃ�
				return n->second;
			} else {
				// �V�K�擾
				IActiveScript *pScript;
				hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void**)&pScript);
				if (SUCCEEDED(hr)) {
					IActiveScriptParse *pScriptParse;
					if (SUCCEEDED(pScript->QueryInterface(IID_IActiveScriptParse, (void **)&pScriptParse))) {
						// ActiveScriptSite ��o�^
						pScript->SetScriptSite(this);
						// �O���[�o���ϐ��̖��O��o�^
						pScript->AddNamedItem(GLOBAL, SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);
						// ������
						pScriptParse->InitNew();
						pScriptParse->Release();
						scriptMap[clsid] = pScript;
						return pScript;
					} else {
						log(TJS_W("QueryInterface IActipveScriptParse failed %s"), type);
						pScript->Release();
					}
				} else {
					log(TJS_W("CoCreateInstance failed %s"), type);
				}
			}
		} else {
			log(TJS_W("bad ProgId/CLSID %s"), type);
		}
		return NULL;
	}
	
public:
	/**
	 * �R���X�g���N�^
	 */
	WindowsScriptHost() : global(NULL), contextCount(1), refCount(1) {
		// global �̎擾
		iTJSDispatch2 * tjsGlobal = TVPGetScriptDispatch();
		global = new IDispatchWrapper(tjsGlobal);
		tjsGlobal->Release();
		// �g���q�ɑ΂��� ProgId �̃}�b�s���O�̓o�^
		extMap["js"]  = "JScript";
		extMap["vbs"] = "VBScript";
		extMap["pl"]  = "PerlScript";
		extMap["pls"] = "PerlScript";
		extMap["rb"]  = "RubyScript";
		extMap["rbs"] = "RubyScript";
	}

	/**
	 * �f�X�g���N�^
	 */
	~WindowsScriptHost() {
		// �G���W���̊J��
		map<CLSID, IActiveScript*, CompareCLSID>::iterator i = scriptMap.begin();
		while (i != scriptMap.end()) {
			i->second->Close();
			i->second->Release();
			i = scriptMap.erase(i);
		}
		// global ���J��
		global->Release();
	}

	/**
	 * �g���q�� ProgId �ɕϊ�����
	 * @param exe �g���q
	 * @return ProgId
	 */
	const tjs_char *getProgId(const tjs_char *ext) {
		ttstr extStr(ext);
		extStr.ToLowerCase();
		map<ttstr, ttstr>::const_iterator n = extMap.find(extStr);
		if (n != extMap.end()) {
			return n->second.c_str();
		}
		return ext;
	}

	/**
	 * �X�N���v�g�̎��s
	 * @param script �X�N���v�g������
	 * @param progId �X�N���v�g�̎��
	 * @param result ���ʊi�[��
	 */
	tjs_error exec(const tjs_char *script, const tjs_char *progId, tTJSVariant *result, DWORD cookie=0) {
		IActiveScript *pScript = getScript(getProgId(progId));
		if (pScript) {
			IActiveScriptParse *pScriptParse;
			if (SUCCEEDED(pScript->QueryInterface(IID_IActiveScriptParse, (void **)&pScriptParse))) {
				
				// ���ʊi�[�p
				HRESULT hr;
				EXCEPINFO ei;
				VARIANT rs;
				memset(&ei, 0, sizeof ei);

				BSTR pParseText = ::SysAllocString(script);
				if (SUCCEEDED(hr = pScriptParse->ParseScriptText(pParseText, GLOBAL, NULL, NULL, cookie, 0, 0L, &rs, &ei))) {
					hr = pScript->SetScriptState(SCRIPTSTATE_CONNECTED);
				}
				::SysFreeString(pParseText);
				
				switch (hr) {
				case S_OK:
					if (result) {
						IDispatchWrapper::storeVariant(*result, rs);
					}
					return TJS_S_OK;
				case DISP_E_EXCEPTION:
					log(TJS_W("exception code:%x desc:%ls"), ei.wCode, ei.bstrDescription);
					TVPThrowExceptionMessage(TJS_W("exception"));
					break;
				case E_POINTER:
					TVPThrowExceptionMessage(TJS_W("memory error"));
					break;
				case E_INVALIDARG:
					return TJS_E_INVALIDPARAM;
				case E_NOTIMPL:
					return TJS_E_NOTIMPL;
				case E_UNEXPECTED:
					return TJS_E_ACCESSDENYED;
				default:
					log(TJS_W("error:%x"), hr);
					return TJS_E_FAIL;
				}
			}
		}
		return TJS_E_FAIL;
	}

	/**
	 * �X�N���v�g�̃t�@�C������̎��s
	 * @param script �X�N���v�g�t�@�C����
	 * @param progId �X�N���v�g�̎��
	 * @param result ���ʊi�[��
	 */
	tjs_error execStorage(const tjs_char *filename, const tjs_char *progId, tTJSVariant *result) {
		
		if (progId == NULL) {
			const tjs_char *ext = wcsrchr(filename, '.');
			if (ext) {
				progId = ext + 1;
			}
		}
		if (!progId) {
			return TJS_E_FAIL;
		}

		DWORD cookie = getContextCookie(filename);
		iTJSTextReadStream * stream = TVPCreateTextStreamForRead(filename, TJS_W(""));
		tjs_error ret;
		try {
			ttstr data;
			stream->Read(data, 0);
			ret = exec(data.c_str(), progId, result, cookie);
		}
		catch(...)
		{
			stream->Destruct();
			throw;
		}
		stream->Destruct();
		return ret;
	}

	/**
	 * �g���q�� ProgId �̑g��ǉ��o�^����
	 * @param exe �g���q
	 * @param progId ProgId
	 */
	void addProgId(const tjs_char *ext, const tjs_char *progId) {
		ttstr extStr(ext);
		extStr.ToLowerCase();
		extMap[extStr] = progId;
	}

	static void addProgIdMethod(const tjs_char *ext, const tjs_char *progId) {
		if (singleton) {
			singleton->addProgId(ext, progId);
		}
	}

	/**
	 * �X�N���v�g�̎��s
	 * @param script �X�N���v�g������
	 * @param progId �X�N���v�g�̎��
	 * @param result ���ʊi�[��
	 */
	static tjs_error execMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		if (numparams < 2) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (singleton) {
			return singleton->exec(param[0]->GetString(), param[1]->GetString(), result);
		}
		return TJS_E_FAIL;
	}

	/**
	 * �X�N���v�g�̃t�@�C������̎��s
	 * @param script �X�N���v�g�t�@�C����
	 * @param progId �X�N���v�g�̎��
	 * @param result ���ʊi�[��
	 */
	static tjs_error execStorageMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (singleton) {
			return singleton->execStorage(param[0]->GetString(), numparams > 1 ? param[1]->GetString() : NULL, result);
		}
		return TJS_E_FAIL;
	}
};

NCB_ATTACH_CLASS(WindowsScriptHost, Scripts) {
	Method("addProgId",           &WindowsScriptHost::addProgId);
	RawCallback("execWSH",        &WindowsScriptHost::execMethod,        TJS_STATICMEMBER);
	RawCallback("execStorageWSH", &WindowsScriptHost::execStorageMethod, TJS_STATICMEMBER);
};

WindowsScriptHost *WindowsScriptHost::singleton = NULL;

//---------------------------------------------------------------------------

static BOOL gOLEInitialized = false;

/**
 * �o�^�����O
 */
static void PreRegistCallback()
{
	if (!gOLEInitialized) {
		if (SUCCEEDED(OleInitialize(NULL))) {
			gOLEInitialized = true;
		} else {
			log(L"OLE ���������s");
		}
	}
}

/**
 * �o�^������
 */
static void PostRegistCallback()
{
	WindowsScriptHost::initSingleton();
}

/**
 * �J�������O
 */
static void PreUnregistCallback()
{
	WindowsScriptHost::removeSingleton();
}

/**
 * �J��������
 */
static void PostUnregistCallback()
{
	if (gOLEInitialized) {
		OleUninitialize();
		gOLEInitialized = false;
	}
}


NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_REGIST_CALLBACK(PostRegistCallback);
NCB_PRE_UNREGIST_CALLBACK(PreUnregistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);

