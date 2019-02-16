#include "HttpConnection.h"
#include "ncbind/ncbind.hpp"
#include <vector>
using namespace std;
#include <process.h>

// ���b�Z�[�W�R�[�h
#define	WM_HTTP_READYSTATE	(WM_APP+6)	// �X�e�[�g�ύX
#define	WM_HTTP_PROGRESS	(WM_APP+7)	// �v���O���X���

// �G�[�W�F���g��
#define AGENT_NAME _T("KIRIKIRI")
#define DEFAULT_ENCODING _T("UTF-8")
#define CTYPE_URLENCODED _T("application/x-www-form-urlencoded")

// �G���R�[�f�B���O������R�[�h�y�[�W���擾
extern void initEncoding();
extern void doneEncoding();
extern int getEncoding(const wchar_t *encoding);
extern UINT getWCToMBLen(int enc, const wchar_t *wc, UINT wclen);
extern void convWCToMB(int enc, const wchar_t *wc, UINT *wclen, char *mb, UINT *mblen);
extern UINT getMBToWCLen(int enc, const char *mb, UINT mblen);
extern void convMBToWC(int enc, const char *mb, UINT *mblen, wchar_t *wc, UINT *wclen);

// �J�E���^
static std::map<iTJSDispatch2 *, int> sRefCount;

/**
 * HttpRequest �N���X
 */
class HttpRequest {

public:

	enum ReadyState {
		READYSTATE_UNINITIALIZED,
		READYSTATE_OPEN,
		READYSTATE_SENT,
		READYSTATE_RECEIVING,
		READYSTATE_LOADED
	};

	/**
	 * �R���X�g���N�^
	 * @param objthis ���ȃI�u�W�F�N�g
	 * @param window �e�E�C���h�E
	 * @param cert HTTP�ʐM���ɏؖ����`�F�b�N���s��
	 */
	HttpRequest(iTJSDispatch2 *objthis, iTJSDispatch2 *window, bool cert, const tjs_char *agentName)
		 : objthis(objthis), window(window), http(agentName, cert),
		   threadHandle(NULL), canceled(false),
		   outputStream(NULL), outputLength(0), inputStream(NULL), inputLength(0),
		   readyState(READYSTATE_UNINITIALIZED), statusCode(0)
	{
		window->AddRef();
        if (sRefCount[window]++ == 0) {
          setReceiver(true);
        }
	}
	
	/**
	 * �f�X�g���N�^
	 */
	~HttpRequest() {
		abort();
        if (--sRefCount[window] <= 0) {
          setReceiver(false);
          sRefCount.erase(window);
        }
		window->Release();
	}

	/**
	 * �w�肵�����\�b�h�Ŏw��URL�Ƀ��N�G�X�g����
	 * ����ɔ񓯊��ł̌Ăяo���ɂȂ�܂�
	 * @param method GET|PUT|POST �̂����ꂩ
	 * @param url ���N�G�X�g���URL
	 * @param userName ���[�U���B�w�肷��ƔF�؃w�b�_�����܂�
	 * @param password �p�X���[�h
	 */
	void _open(const tjs_char *method, const tjs_char *url, const tjs_char *userName, const tjs_char *password) {
		abort();
		if (http.open(method, url, userName, password)) {
			onReadyStateChange(READYSTATE_OPEN);
		} else {
			TVPThrowExceptionMessage(http.getErrorMessage());
		}
	}

	static tjs_error open(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		if (numparams < 2) {
			return TJS_E_BADPARAMCOUNT;
		}
		self->_open(params[0]->GetString(), params[1]->GetString(), numparams > 2 ? params[2]->GetString() : NULL, numparams > 3 ? params[3]->GetString() : NULL);
		return TJS_S_OK;
	}
	
	/**
	 * ���M���ɑ�����w�b�_�[��ǉ�����
	 * @param name �w�b�_��
	 * @param value �l
	 */
	void setRequestHeader(const tjs_char *name, const tjs_char *value) {
		checkRunning();
		http.addHeader(name, value);
	}

	/**
	 * ���M����
	 * @param ���M�f�[�^
	 * @param sendStorage ���M�t�@�C��
	 * @param saveStorage �ۑ���t�@�C��
	 */
	void _send(tTJSVariant *data, const tjs_char *sendStorage, const tjs_char *saveStorage, bool async=true) {
		checkRunning();
		checkOpen();
		if (saveStorage) {
			outputStream = TVPCreateIStream(ttstr(saveStorage), TJS_BS_WRITE);
			if (outputStream == NULL) {
				TVPThrowExceptionMessage(L"saveStorage open failed");
			}
		}
		if (data) {
			switch (data->Type()) {
			case tvtString:
				{
					tTJSVariantString *str = data->AsStringNoAddRef();
					int enc = getEncoding(http.getRequestEncoding());
					inputLength = ::getWCToMBLen(enc, *str, str->GetLength());
					inputData.resize(inputLength);
					if (inputLength) {
					  UINT wlen = str->GetLength();
					  UINT blen = inputData.size();
					  ::convWCToMB(enc, *str, &wlen, (char*)&inputData[0], &blen);
					}
				}
				break;
			case tvtOctet:
				{
					tTJSVariantOctet *octet = data->AsOctetNoAddRef();
					if (octet) {
						inputLength = octet->GetLength();
						inputData.resize(inputLength);
						memcpy(&inputData[0], octet->GetData(), inputLength);
					}
				}
				break;
			}
			inputLength = inputData.size();
		} else if (sendStorage) {
			inputStream = TVPCreateIStream(ttstr(sendStorage), TJS_BS_READ);
			if (inputStream == NULL) {
				TVPThrowExceptionMessage(L"sendStorage open failed");
			}
			STATSTG stat;
			inputStream->Stat(&stat, STATFLAG_NONAME);
			inputLength = (DWORD)stat.cbSize.QuadPart;
		}
		if (inputLength > 0) {
            tTJSVariant val = tjs_int64(inputLength);
            ttstr len(val);
			http.addHeader(_T("Content-Length"), len.c_str());
		}
	    if (async) {
		    startThread();
		} else {
		    threadMain(false);
		}
	}
	
	/**
	 * ���N�G�X�g�̑��M
	 */
	static tjs_error send(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		self->_send(numparams > 0 ? params[0] : NULL, NULL, numparams > 1 ? params[1]->GetString() : NULL);
		return TJS_S_OK;
	}

	/**
	 * ���N�G�X�g�̑��M
	 */
	static tjs_error sendSync(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		self->_send(numparams > 0 ? params[0] : NULL, NULL, numparams > 1 ? params[1]->GetString() : NULL, false);
	    if (result) {
		  *result = self->statusCode;
		}
	    return TJS_S_OK;
	}
  
	/**
	 * ���N�G�X�g�̑��M
	 */
	static tjs_error sendStorage(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		self->_send(NULL, params[0]->GetString(), numparams > 1 ? params[1]->GetString() : NULL);
		return TJS_S_OK;
	}
  
	/**
	 * ���N�G�X�g�̑��M
	 */
	static tjs_error sendStorageSync(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		self->_send(NULL, params[0]->GetString(), numparams > 1 ? params[1]->GetString() : NULL, false);
	    if (result) {
		  *result = self->statusCode;
		}
		return TJS_S_OK;
	}

	void clearInput() {
		if (inputStream) {
			inputStream->Release();
			inputStream = NULL;
		}
		inputData.clear();
		inputSize = inputLength = 0;
	}

	void closeOutput() {
		if (outputStream) {
			outputStream->Release();
			outputStream = NULL;
		}
	}
	
	void clearOutput() {
		closeOutput();
		outputData.clear();
		outputSize = outputLength = 0;
	}
	

	/**
	 * ���ݎ��s���̑���M�̃L�����Z��
	 */
	void abort() {
		stopThread();
		clearInput();
		clearOutput();
	}
	
	/**
	 * ���ׂĂ� HTTP�w�b�_���擾����
	 * @return HTTP�w�b�_���i�[���ꂽ����
	 */
	tTJSVariant getAllResponseHeaders() {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		tstring name;
		tstring value;
		http.initRH();
		while (http.getNextRH(name, value)) {
			tTJSVariant v(value.c_str());
			dict->PropSet(TJS_MEMBERENSURE, name.c_str(), NULL, &v, dict);
		}
		tTJSVariant ret(dict,dict);
		dict->Release();
		return ret;
	}

	/**
	 * �w�肵��HTTP�w�b�_���擾����
	 * @param name �w�b�_���x����
	 * @return �w�b�_�̒l
	 */
	const tjs_char *getResponseHeader(const tjs_char *name) {
		return http.getResponseHeader(name);
	}

	/**
	 * �ʐM��ԁB�ǂݍ��ݐ�p
	 * @return ���݂̒ʐM���
	 * 0: �������
	 * 1: �ǂݍ��ݒ�
	 * 2: �ǂݍ���
	 * 3: ��͒�
	 * 4: ����
	 */
	int getReadyState() const {
		return readyState;
	}

	/**
	 * ���X�|���X���e�L�X�g�̌`�ŕԂ�
	 * @param encoding �G���R�[�f�B���O�w��
	 */
	tTJSString _getResponseText(const tjs_char *encoding) {
		tTJSString ret;
		if (encoding == NULL) {
			encoding = http.getEncoding();
		}
		if (outputData.size() > 0) {
			DWORD size = outputData.size();
			const char *data = (const char*)&outputData[0];
			UINT dlen = outputData.size();
			int enc = getEncoding(encoding);
			UINT l = ::getMBToWCLen(enc, data, dlen);
			if (l > 0) {
				tjs_char *str = ret.AllocBuffer(l);
				::convMBToWC(enc, data, &dlen, str, &l);
			}
		}
		return ret;
	}

	static tjs_error getResponseText(tTJSVariant *result, tjs_int numparams, tTJSVariant **params, HttpRequest *self) {
		if (result) {
			*result = self->_getResponseText(numparams > 0 ? params[0]->GetString() : NULL);
		}
		return TJS_S_OK;
	}
	
	/**
	 * ���X�|���X�f�[�^�B�ǂݍ��ݐ�p
	 * @return ���X�|���X�f�[�^
	 */
	tTJSVariant getResponse() {
		const TCHAR *contentType = http.getContentType();
		if (_tcsncmp(http.getContentType(), _T("text/"), 5) == 0) {
			return _getResponseText(http.getEncoding());
//		} else if (_tcscmp(contentType, CTYPE_URLENCODED) == 0) {
//			// URLENCODED�ȃf�[�^����͂��Ď������\�z
		} else if (outputData.size() > 0) {
			return tTJSVariant((const tjs_uint8 *)&outputData[0], outputData.size());
		}
		return tTJSVariant();
	}
	
	/**
	 * ���X�|���X�f�[�^�B�ǂݍ��ݐ�p
	 * @return ���X�|���X�f�[�^
	 */
	tTJSVariant getResponseData() {
		if (outputData.size() > 0) {
			return tTJSVariant((const tjs_uint8 *)&outputData[0], outputData.size());
		}
		return tTJSVariant();
	}

	/**
	 * ���X�|���X�� HTTP�X�e�[�^�X�R�[�h�B�ǂݍ��ݐ�p
	 * @return �X�e�[�^�X�R�[�h
	 */
	int getStatus() {
		return statusCode;
	}
	
	/**
	 * ���X�|���X�� HTTP�X�e�[�^�X�̕�����
	 * @return ���X�|���X������
	 */
	const tjs_char *getStatusText() {
		return statusText.c_str();
	}

	const tjs_char *getContentType() {
		return http.getContentType();
	}

	const tjs_char *getContentTypeEncoding() {
		return http.getEncoding();
	}

	DWORD getContentLength() {
		return http.getContentLength();
	}
	
	/**
	 * �C���X�^���X�����t�@�N�g��
	 */
	static tjs_error factory(HttpRequest **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		iTJSDispatch2 *window = params[0]->AsObjectNoAddRef();
		if (window->IsInstanceOf(0, NULL, NULL, L"Window", window) != TJS_S_TRUE) {
			TVPThrowExceptionMessage(L"InvalidObject");
		}
		*result = new HttpRequest(objthis, window, numparams >= 2 ? params[1]->AsInteger() != 0 : true, AGENT_NAME);
		return S_OK;
	}
	

protected:

	void checkRunning() {
		if (threadHandle) {
			TVPThrowExceptionMessage(TJS_W("already running"));
		}
	}

	void checkOpen() {
		if (!http.isValid()) {
			TVPThrowExceptionMessage(TJS_W("not open"));
		}
	}

	/**
	 * readyState ���ω������ꍇ�̃C�x���g����
	 * @param readyState �V�����X�e�[�g
	 */
	void onReadyStateChange(int readyState) {
		this->readyState = readyState;
		if (readyState == READYSTATE_LOADED) {
			stopThread();
		}
		tTJSVariant param(readyState);
		static ttstr eventName(TJS_W("onReadyStateChange"));
		TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_POST, 1, &param);
	}
	
	/**
	 * �f�[�^�ǂݍ��ݒ��̃C�x���g����
	 * @param upload ���M��
	 * @param percent �i��
	 */
	void onProgress(bool upload, tjs_real percent) {
		tTJSVariant params[2];
		params[0] = upload;
		params[1] = percent;
		static ttstr eventName(TJS_W("onProgress"));
		TVPPostEvent(objthis, objthis, eventName, 0, TVP_EPT_POST, 2, params);
	}
	
	// ���[�U���b�Z�[�W���V�[�o�̓o�^/����
	void setReceiver(bool enable) {
		tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
		tTJSVariant userdata = (tTVInteger)0;
		tTJSVariant *p[] = {&mode, &proc, &userdata};
		if (window->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, objthis) != TJS_S_OK) {
			TVPThrowExceptionMessage(L"can't regist user message receiver");
		}
	}

	/**
	 * �C�x���g��M����
	 */
	static bool __stdcall receiver(void *userdata, tTVPWindowMessage *Message) {
        HttpRequest *self = (HttpRequest*)Message->WParam;
		switch (Message->Msg) {
		case WM_HTTP_READYSTATE:
          self->onReadyStateChange((ReadyState)Message->LParam);
          return true;
		case WM_HTTP_PROGRESS:
          int lparam = (int)Message->LParam;
          self->onProgress((lparam & 0x8000)!=0, tjs_real(lparam & 0x7fff) / 100.0);
          return true;
		}
		return false;
	}

	// -----------------------------------------------
	// �X���b�h����
	// -----------------------------------------------

	/**
	 * �t�@�C�����M�����������߂�
	 */
        void rewindUpload(void) {
	  if (inputStream) {
	    LARGE_INTEGER pos;
	    pos.QuadPart = 0;
	    inputStream->Seek(pos, STREAM_SEEK_SET, NULL);
	  }
	  inputSize = 0;
	}
  
        /**
	 * ���M�����߂��̃R�[���o�b�N����
	 */
        static void rewindUploadCallback(void *context) {
	  HttpRequest *self = (HttpRequest*)context;
	  if (self)
	    self->rewindUpload();
	}
    
	/**
	 * �t�@�C�����M����
	 * @param buffer �ǂݎ��o�b�t�@
	 * @param size �ǂݏo�����T�C�Y
	 */
	bool upload(void *buffer, DWORD &size, bool async=true) {
		if (inputStream) {
			// �t�@�C������ǂݍ���
			inputStream->Read(buffer, size, &size);
		} else {
			// ����������ǂݍ���
			DWORD s = inputData.size() - inputSize;
			if (s < size) {
				size = s;
			}
			if (size > 0) {
				memcpy(buffer, &inputData[inputSize], size);
			}
		}
		if (size > 0) {
			inputSize += size;
		    if (async) {
			  int bp = (inputLength > 0) ? (DWORDLONG)inputSize * 10000 / inputLength : 0;
			  ::PostMessage(hwnd, WM_HTTP_PROGRESS, (WPARAM)this, 0x8000 | bp);
			}
		}
		return !canceled;
	}

	/**
	 * �ʐM���̃R�[���o�b�N����
	 * @return �L�����Z���Ȃ� false
	 */
	static bool uploadCallback(void *context, void *buffer, DWORD &size) {
		HttpRequest *self = (HttpRequest*)context;
		return self ? self->upload(buffer, size) : false;
	}

	/**
	 * �ʐM���̃R�[���o�b�N����
	 * @return �L�����Z���Ȃ� false
	 */
	static bool uploadCallbackSync(void *context, void *buffer, DWORD &size) {
		HttpRequest *self = (HttpRequest*)context;
		return self ? self->upload(buffer, size, false) : false;
	}

  /**
	 * �t�@�C���ǂݎ�菈��
	 * @param buffer �ǂݎ��o�b�t�@
	 * @param size �ǂݏo�����T�C�Y
	 * @param async �񓯊�����true
	 */
	bool download(const void *buffer, DWORD size, bool async=true) {
		if (outputStream) {
			if (buffer) {
				DWORD n = 0;
				DWORD s = size;
				while (s > 0) {
					DWORD l;
					if (outputStream->Write((BYTE*)buffer+n, s, &l) == S_OK) {
						s -= l;
						n += l;
					} else {
						break;
					}
				}
			} else {
				outputStream->Release();
				outputStream = NULL;
			}
		} else {
			outputData.resize(outputSize + size);
			memcpy(&outputData[outputSize], buffer, size);
		}
		outputSize += size;
	    if (async) {
		  int bp = (outputLength > 0) ? (DWORDLONG)outputSize * 10000 / outputLength : 0;
		  ::PostMessage(hwnd, WM_HTTP_PROGRESS, (WPARAM)this, bp);
		}
		return !canceled;
	}
	
	/**
	 * �ʐM���̃R�[���o�b�N����
	 * @return �L�����Z���Ȃ� false
	 */
	static bool downloadCallback(void *context, const void *buffer, DWORD size) {
		HttpRequest *self = (HttpRequest*)context;
		return self ? self->download(buffer, size) : false;
	}

	/**
	 * �ʐM���̃R�[���o�b�N����(�����p)
	 * @return �L�����Z���Ȃ� false
	 */
	static bool downloadCallbackSync(void *context, const void *buffer, DWORD size) {
		HttpRequest *self = (HttpRequest*)context;
		return self ? self->download(buffer, size, false) : false;
	}

  
	/**
	 * �o�b�N�O���E���h�Ŏ��s���鏈��
	 */
	void threadMain(bool async=true) {

		{
			tTJSVariant val;
			window->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
			hwnd = reinterpret_cast<HWND>((tjs_int)(val));
		}

	    if (async) ::PostMessage(hwnd, WM_HTTP_READYSTATE, (WPARAM)this, (LPARAM)READYSTATE_SENT);
		inputSize = 0;
		int errorCode;
		if (canceled) {
			errorCode = HttpConnection::ERROR_CANCEL;
			clearInput();
		} else {
		    if ((errorCode = http.request(async ? uploadCallback : uploadCallbackSync, rewindUploadCallback, (void*)this)) == HttpConnection::ERROR_NONE) {
				clearInput();
				if (canceled) {
					errorCode = HttpConnection::ERROR_CANCEL;
					clearOutput();
				} else {
					http.queryInfo();
					outputSize = 0;
					outputLength = http.getContentLength();
					if (async) ::PostMessage(hwnd, WM_HTTP_READYSTATE, (WPARAM)this, (LPARAM)READYSTATE_RECEIVING);
				    if ((errorCode = http.response(async ? downloadCallback : downloadCallbackSync, (void*)this)) == HttpConnection::ERROR_NONE) {
						closeOutput();
					} else {
						clearOutput();
					}
				}
			} else {
				clearInput();
			}
		}
		switch (errorCode) {
		case HttpConnection::ERROR_NONE:
			statusCode = http.getStatusCode();
			statusText = http.getStatusText();
			break;
		case HttpConnection::ERROR_CANCEL:
			statusCode = -1;
			statusText = L"aborted";
			break;
		default:
			statusCode = 0;
			statusText = http.getErrorMessage();
			break;
		}
		if (async) ::PostMessage(hwnd, WM_HTTP_READYSTATE, (WPARAM)this, (LPARAM)READYSTATE_LOADED);
	}

	// ���s�X���b�h
	static unsigned __stdcall threadFunc(void *data) {
		((HttpRequest*)data)->threadMain();
		_endthreadex(0);
		return 0;
	}

	// �X���b�h�����J�n
	void startThread() {
		stopThread();
		canceled = false;
		threadHandle = (HANDLE)_beginthreadex(NULL, 0, threadFunc, this, 0, NULL);
	}

	// �X���b�h�����I��
	void stopThread() {
		if (threadHandle) {
			canceled = true;
			WaitForSingleObject(threadHandle, INFINITE);
			CloseHandle(threadHandle);
			threadHandle = 0;
		}
	}
	
private:
	iTJSDispatch2 *objthis; ///< ���ȃI�u�W�F�N�g���̎Q��
	iTJSDispatch2 *window; ///< �E�C���h�E�I�u�W�F�N�g���̎Q��(�C�x���g�擾�ɕK�v)
	HWND hwnd; ///< �E�C���h�E�n���h���B���C���X���b�h��~���� Window �ɃA�N�Z�X����ƌł܂�̂ŏ����O�ɂƂ��Ă���
	
	// HTTP�ʐM�����p
	HttpConnection http;

	// �X���b�h�����p
	HANDLE threadHandle; ///< �X���b�h�̃n���h��
	bool canceled; ///< �L�����Z�����ꂽ

	// ���N�G�X�g
	IStream *inputStream;   ///< ���M�p�X�g���[��
	vector<BYTE>inputData;  ///< ���M�p�f�[�^
	DWORD inputLength; ///< ���M�f�[�^�T�C�Y
	DWORD inputSize;   ///< ���M�ς݃f�[�^�T�C�Y

	// ���X�|���X
	IStream *outputStream;  ///< ��M�p�X�g���[��
	vector<BYTE>outputData; ///< ��M�p�f�[�^
	DWORD outputLength; ///< ��M�f�[�^�T�C�Y
	DWORD outputSize;   ///< ��M�ς݃f�[�^�T�C�Y

	int readyState;
	int statusCode; ///< HTTP�X�e�[�^�X�R�[�h
	ttstr statusText; ///< HTTP�X�e�[�^�X�e�L�X�g
};

#define ENUM(n) Variant(#n, (int)HttpRequest::READYSTATE_ ## n)

NCB_REGISTER_CLASS(HttpRequest) {
	Factory(&ClassT::factory);
	ENUM(UNINITIALIZED);
	ENUM(OPEN);
	ENUM(SENT);
	ENUM(RECEIVING);
	ENUM(LOADED);
	RawCallback(TJS_W("open"), &Class::open, 0);
	NCB_METHOD(setRequestHeader);
	RawCallback(TJS_W("send"), &Class::send, 0);
	RawCallback(TJS_W("sendSync"), &Class::sendSync, 0);
	RawCallback(TJS_W("sendStorage"), &Class::sendStorage, 0);
	RawCallback(TJS_W("sendStorageSync"), &Class::sendStorageSync, 0);
	NCB_METHOD(abort);
	NCB_METHOD(getAllResponseHeaders);
	NCB_METHOD(getResponseHeader);
	RawCallback(TJS_W("getResponseText"), &Class::getResponseText, 0);
	NCB_PROPERTY_RO(readyState, getReadyState);
	NCB_PROPERTY_RO(response, getResponse);
	NCB_PROPERTY_RO(responseData, getResponseData);
	NCB_PROPERTY_RO(status, getStatus);
	NCB_PROPERTY_RO(statusText, getStatusText);
	NCB_PROPERTY_RO(contentType, getContentType);
	NCB_PROPERTY_RO(contentTypeEncoding, getContentTypeEncoding);
	NCB_PROPERTY_RO(contentLength, getContentLength);
}

NCB_PRE_REGIST_CALLBACK(initEncoding);
NCB_POST_UNREGIST_CALLBACK(doneEncoding);
