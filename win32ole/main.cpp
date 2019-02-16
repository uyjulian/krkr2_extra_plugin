#include <windows.h>
#include <DispEx.h>
#include <stdio.h>

#define KRKRDISPWINDOWCLASS _T("TScrollBox")

// ATL
#if _MSC_VER == 1200
// Microsoft SDK �̂��̂Ƃ��������̂Ŕr��
#define __IHTMLControlElement_INTERFACE_DEFINED__
#endif

#include "ncbind/ncbind.hpp"

#include <atlbase.h>
static CComModule _Module;
#include <atlwin.h>
#include <atlcom.h>
#include <atliface.h>
#define _ATL_DLL
#include <atlhost.h>
#include <ExDispID.h>

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

//---------------------------------------------------------------------------

#include "IDispatchWrapper.hpp"

/**
 * OLE -> �g���g�� �C�x���g�f�B�X�p�b�`��
 * sender (IUnknown) ���� DIID �̃C�x���g���󗝂��A
 * receiver (tTJSDispatch2) �ɑ��M����B
 */ 
class EventSink : public IDispatch
{
protected:
	int refCount;
	REFIID diid;
	ITypeInfo *pTypeInfo;
	iTJSDispatch2 *receiver;

public:
	EventSink(GUID diid, ITypeInfo *pTypeInfo, iTJSDispatch2 *receiver) : diid(diid), pTypeInfo(pTypeInfo), receiver(receiver) {
		refCount = 1;
		if (pTypeInfo) {
			pTypeInfo->AddRef();
		}
		if (receiver) {
			receiver->AddRef();
		}
	}

	~EventSink() {
		if (receiver) {
			receiver->Release();
		}
		if (pTypeInfo) {
			pTypeInfo->Release();
		}
	}

	//----------------------------------------------------------------------------
	// IUnknown ����
	
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
											 void __RPC_FAR *__RPC_FAR *ppvObject) {
		if (riid == IID_IUnknown ||
			riid == IID_IDispatch||
			riid == diid) {
			if (ppvObject == NULL)
				return E_POINTER;
			*ppvObject = this;
			AddRef();
			return S_OK;
		} else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}

	ULONG STDMETHODCALLTYPE AddRef() {
		refCount++;
		return refCount;
	}

	ULONG STDMETHODCALLTYPE Release() {
		int ret = --refCount;
		if (ret <= 0) {
			delete this;
			ret = 0;
		}
		return ret;
	}
	
	// -------------------------------------
	// IDispatch �̎���
public:
	STDMETHOD (GetTypeInfoCount) (UINT* pctinfo)
	{
		return	E_NOTIMPL;
	}

	STDMETHOD (GetTypeInfo) (UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
	{
		return	E_NOTIMPL;
	}

	STDMETHOD (GetIDsOfNames) (REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
	{
		return	E_NOTIMPL;
	}

	STDMETHOD (Invoke) (DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{
		BSTR bstr = NULL;
		if (pTypeInfo) {
			unsigned int len;
			pTypeInfo->GetNames(dispid, &bstr, 1, &len);
		}
		HRESULT hr = IDispatchWrapper::InvokeEx(receiver, bstr, wFlags, pdispparams, pvarResult, pexcepinfo);
		if (hr == DISP_E_MEMBERNOTFOUND) {
			//log(L"member not found:%ws", bstr);
			hr = S_OK;
		}
		if (bstr) {
			SysFreeString(bstr);
		}
		return hr;
	}
};

//---------------------------------------------------------------------------

/*
 * WIN32OLE �l�C�e�B�u�C���X�^���X
 */
class WIN32OLE // �l�C�e�B�u�C���X�^���X
{
public:
	iTJSDispatch2 *objthis; //< �������g
	IDispatch *pDispatch; //< �ێ����Ă�C���X�^���X

protected:

	struct EventInfo {
		IID diid;
		DWORD cookie;
		EventInfo(REFIID diid, DWORD cookie) : diid(diid), cookie(cookie) {};
	};
	vector<EventInfo> events;

	/**
	 * �C�x���g���̏���
	 */
	void clearEvent() {
		if (pDispatch) {
			vector<EventInfo>::iterator i = events.begin();
			while (i != events.end()) {
				AtlUnadvise(pDispatch, i->diid, i->cookie);
				i++;
			}
			events.clear();
		}
	}

	/**
	 * �o�^���̏���
	 */
	void clear() {
		clearEvent();
		if (pDispatch) {
			pDispatch->Release();
			pDispatch = NULL;
		}
	}

public:
	/**
	 * �R���X�g���N�^
	 * @param objthis TJS2�C���X�^���X
	 * @param progIdorCLSID
	 */
	WIN32OLE(iTJSDispatch2 *objthis, const tjs_char *progIdOrCLSID) : objthis(objthis), pDispatch(NULL) {
		if (progIdOrCLSID) {
			HRESULT hr;
			CLSID   clsid;
			OLECHAR *oleName = SysAllocString(progIdOrCLSID);
			if (FAILED(hr = CLSIDFromProgID(oleName, &clsid))) {
				hr = CLSIDFromString(oleName, &clsid);
			}
			SysFreeString(oleName);
			if (SUCCEEDED(hr)) {
				// COM �ڑ�����IDispatch ���擾����
				/* get IDispatch interface */
				hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&pDispatch);
				if (!SUCCEEDED(hr)) {
					log(L"CoCreateInstance failed %ws", progIdOrCLSID);
				}
			} else {
				log(L"bad CLSID %ws", progIdOrCLSID);
			}
		}
		tTJSVariant name(TJS_W("missing"));
		objthis->ClassInstanceInfo(TJS_CII_SET_MISSING, 0, &name);
	}
	
	// �I�u�W�F�N�g�������������Ƃ��ɌĂ΂��
	virtual ~WIN32OLE()	{
		clear();
	}

	/**
	 * �C���X�^���X�����t�@�N�g��
	 */
	static tjs_error factory(WIN32OLE **result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (result) {
			*result = new WIN32OLE(objthis, param[0]->GetString());
		}
		return S_OK;
	}

protected:

	/**
	 * ���\�b�h���s
	 * �����o���𒼐ڎw��
	 * @param wFlag ���s�t���O
	 * @param membername �����o��
	 * @param result ����
	 * @param numparams �����̐�
	 * @param param ����
	 */
	tjs_error invoke(DWORD wFlags,
					 const tjs_char *membername,
					 tTJSVariant *result,
					 tjs_int numparams,
					 tTJSVariant **param) {
		if (pDispatch) {
			return iTJSDispatch2Wrapper::Invoke(pDispatch,
												wFlags,
												membername,
												result,
												numparams,
												param);
		}
		return TJS_E_FAIL;
	}
	
	/**
	 * ���\�b�h���s
	 * �p�����[�^�̂P�ڂ����\�b�h��
	 * @param wFlag ���s�t���O
	 * @param result ����
	 * @param numparams �����̐�
	 * @param param ����
	 */
	tjs_error invoke(DWORD wFlags,
					 tTJSVariant *result,
					 tjs_int numparams,
					 tTJSVariant **param) {
		//log(L"native invoke %d", numparams);
		if (pDispatch) {
			if (numparams > 0) {
				if (param[0]->Type() == tvtString) {
					return iTJSDispatch2Wrapper::Invoke(pDispatch,
														wFlags,
														param[0]->GetString(),
														result,
														numparams - 1,
														param ? param + 1 : NULL);
				} else {
					return TJS_E_INVALIDPARAM;
				}
			} else {
				return TJS_E_BADPARAMCOUNT;
			}
		}
		return TJS_E_FAIL;
	}

	/**
	 * ���\�b�h���s
	 */
	tjs_error missing(tTJSVariant *result, tjs_int numparams, tTJSVariant **param) {
		
		if (numparams < 3) {return TJS_E_BADPARAMCOUNT;};
		bool ret = false;
		const tjs_char *membername = param[1]->GetString();
		if ((int)*param[0]) {
			// put
			ret = TJS_SUCCEEDED(invoke(DISPATCH_PROPERTYPUT, membername, NULL, 1, &param[2]));
		} else {
			// get
			tTJSVariant result;
			tjs_error err;
			ret = TJS_SUCCEEDED(err = invoke(DISPATCH_PROPERTYGET|DISPATCH_METHOD, membername, &result, 0, NULL));
			if (err == TJS_E_BADPARAMCOUNT) {
				result = new iTJSDispatch2WrapperForMethod(pDispatch, membername);
				ret = true;
			}
			if (ret) {
				iTJSDispatch2 *value = param[2]->AsObject();
				if (value) {
					value->PropSet(0, NULL, NULL, &result, NULL);
					value->Release();
				}
			}
		}
		if (result) {
			*result = ret;
		}
		return TJS_S_OK;
	}

public:
	static tjs_error invokeMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, WIN32OLE *self) {
		return self->invoke(DISPATCH_PROPERTYGET|DISPATCH_METHOD, result, numparams, param);
	}
	
	static tjs_error setMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, WIN32OLE *self) {
		return self->invoke(DISPATCH_PROPERTYPUT, result, numparams, param);
	}
	
	static tjs_error getMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, WIN32OLE *self) {
		return self->invoke(DISPATCH_PROPERTYGET, result, numparams, param);
	}

	static tjs_error missingMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, WIN32OLE *self) {
		return self->missing(result, numparams, param);
	}
	
protected:

	/**
	 * �f�t�H���g�� IID ��T��
	 * @param pitf ���O
	 * @param piid �擾����IID�̊i�[��
	 * @param ppTypeInfo �֘A��������
	 */
	
	HRESULT findDefaultIID(IID *piid, ITypeInfo **ppTypeInfo) {

		HRESULT hr;

		IProvideClassInfo2 *pProvideClassInfo2;
		hr = pDispatch->QueryInterface(IID_IProvideClassInfo2, (void**)&pProvideClassInfo2);
		if (SUCCEEDED(hr)) {
			hr = pProvideClassInfo2->GetGUID(GUIDKIND_DEFAULT_SOURCE_DISP_IID, piid);
			pProvideClassInfo2->Release();
			ITypeInfo *pTypeInfo;
			if (SUCCEEDED(hr = pDispatch->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &pTypeInfo))) {
				ITypeLib *pTypeLib;
				unsigned int index;
				if (SUCCEEDED(hr = pTypeInfo->GetContainingTypeLib(&pTypeLib, &index))) {
					hr = pTypeLib->GetTypeInfoOfGuid(*piid, ppTypeInfo);
				}
			}
			return hr;
		}

		IProvideClassInfo *pProvideClassInfo;
		if (SUCCEEDED(hr = pDispatch->QueryInterface(IID_IProvideClassInfo, (void**)&pProvideClassInfo))) {
			ITypeInfo *pTypeInfo;
			if (SUCCEEDED(hr = pProvideClassInfo->GetClassInfo(&pTypeInfo))) {
				
				TYPEATTR *pTypeAttr;
				if (SUCCEEDED(hr = pTypeInfo->GetTypeAttr(&pTypeAttr))) {
					int i;
					for (i = 0; i < pTypeAttr->cImplTypes; i++) {
						int iFlags;
						if (SUCCEEDED(hr = pTypeInfo->GetImplTypeFlags(i, &iFlags))) {
							if ((iFlags & IMPLTYPEFLAG_FDEFAULT) &&	(iFlags & IMPLTYPEFLAG_FSOURCE)) {
								HREFTYPE hRefType;
								if (SUCCEEDED(hr = pTypeInfo->GetRefTypeOfImplType(i, &hRefType))) {
									if (SUCCEEDED(hr = pTypeInfo->GetRefTypeInfo(hRefType, ppTypeInfo))) {
										break;
									}
								}
							}
						}
					}
					pTypeInfo->ReleaseTypeAttr(pTypeAttr);
				}
				pTypeInfo->Release();
			}
			pProvideClassInfo->Release();
		}

		if (!*ppTypeInfo) {
			if (SUCCEEDED(hr)) {
				hr = E_UNEXPECTED;
			}
		} else {
			TYPEATTR *pTypeAttr;
			hr = (*ppTypeInfo)->GetTypeAttr(&pTypeAttr);
			if (SUCCEEDED(hr)) {
				*piid = pTypeAttr->guid;
				(*ppTypeInfo)->ReleaseTypeAttr(pTypeAttr);
			} else {
				(*ppTypeInfo)->Release();
				*ppTypeInfo = NULL;
			}
		}
		return hr;
	}
	
	/**
	 * IID ��T��
	 * @param pitf ���O
	 * @param piid �擾����IID�̊i�[��
	 * @param ppTypeInfo �֘A��������
	 */
	HRESULT findIID(const tjs_char *pitf, IID *piid, ITypeInfo **ppTypeInfo) {

		if (pitf == NULL) {
			return findDefaultIID(piid, ppTypeInfo);
		}

		HRESULT hr;
		ITypeInfo *pTypeInfo;
		if (SUCCEEDED(hr = pDispatch->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &pTypeInfo))) {
			ITypeLib *pTypeLib;
			unsigned int index;
			if (SUCCEEDED(hr = pTypeInfo->GetContainingTypeLib(&pTypeLib, &index))) {
				bool found = false;
				unsigned int count = pTypeLib->GetTypeInfoCount();
				for (index = 0; index < count; index++) {
					ITypeInfo *pTypeInfo;
					if (SUCCEEDED(pTypeLib->GetTypeInfo(index, &pTypeInfo))) {
						TYPEATTR *pTypeAttr;
						if (SUCCEEDED(pTypeInfo->GetTypeAttr(&pTypeAttr))) {
							if (pTypeAttr->typekind == TKIND_COCLASS) {
								int type;
								for (type = 0; !found && type < pTypeAttr->cImplTypes; type++) {
									HREFTYPE RefType;
									if (SUCCEEDED(pTypeInfo->GetRefTypeOfImplType(type, &RefType))) {
										ITypeInfo *pImplTypeInfo;
										if (SUCCEEDED(pTypeInfo->GetRefTypeInfo(RefType, &pImplTypeInfo))) {
											BSTR bstr = NULL;
											if (SUCCEEDED(pImplTypeInfo->GetDocumentation(-1, &bstr, NULL, NULL, NULL))) {
												if (wcscmp(pitf, bstr) == 0) {
													TYPEATTR *pImplTypeAttr;
													if (SUCCEEDED(pImplTypeInfo->GetTypeAttr(&pImplTypeAttr))) {
														found = true;
														*piid = pImplTypeAttr->guid;
														if (ppTypeInfo) {
															*ppTypeInfo = pImplTypeInfo;
															(*ppTypeInfo)->AddRef();
														}
														pImplTypeInfo->ReleaseTypeAttr(pImplTypeAttr);
													}
												}
												SysFreeString(bstr);
											}
											pImplTypeInfo->Release();
										}
									}
								}
							}
							pTypeInfo->ReleaseTypeAttr(pTypeAttr);
						}
						pTypeInfo->Release();
					}
					if (found) {
						break;
					}
				}
				if (!found) {
					hr = E_NOINTERFACE;
				}
				pTypeLib->Release();
			}
			pTypeInfo->Release();
		}
		return hr;
	}

	/**
	 * �C�x���g��o�^
	 */
	bool addEvent(const tjs_char *diidName, iTJSDispatch2 *receiver) {
		bool ret = false;
		IID diid;
		ITypeInfo *pTypeInfo;
		if (SUCCEEDED(findIID(diidName, &diid, &pTypeInfo))) {
			EventSink *sink = new EventSink(diid, pTypeInfo, receiver);
			DWORD cookie;
			if (SUCCEEDED(AtlAdvise(pDispatch, sink, diid, &cookie))) {
				events.push_back(EventInfo(diid, cookie));
				ret = true;
			}
			sink->Release();
			if (pTypeInfo) {
				pTypeInfo->Release();
			}
		}
		return ret;
	}

	/**
	 * �C�x���g�o�^
	 */
	static tjs_error _addEventMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis, WIN32OLE *self) {
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (!self) {
			return TJS_E_NATIVECLASSCRASH;
		}
		bool success = false;
		const tjs_char *diidName = param[0]->GetString();
		if (numparams > 1) {
			iTJSDispatch2 *receiver = param[1]->AsObject();
			if (receiver) {
				success = self->addEvent(diidName, receiver);
				receiver->Release();
			}
		} else {
			success = self->addEvent(diidName, objthis);
		}
		if (!success) {
			log(L"�C�x���g[%ws]�̓o�^�Ɏ��s���܂���", diidName);
		}
		return TJS_S_OK;
	}

	/**
	 * �萔�̎擾
	 * @param pTypeInfo TYPEINFO
	 * @param target �i�[��
	 */
	void getConstant(ITypeInfo *pTypeInfo, iTJSDispatch2 *target) {
		// �����
		TYPEATTR  *pTypeAttr = NULL;
		if (SUCCEEDED(pTypeInfo->GetTypeAttr(&pTypeAttr))) {
			for (int i=0; i<pTypeAttr->cVars; i++) {
				VARDESC *pVarDesc = NULL;
				if (SUCCEEDED(pTypeInfo->GetVarDesc(i, &pVarDesc))) {
					if (pVarDesc->varkind == VAR_CONST &&
						!(pVarDesc->wVarFlags & (VARFLAG_FHIDDEN | VARFLAG_FRESTRICTED | VARFLAG_FNONBROWSABLE))) {
						BSTR bstr = NULL;
						unsigned int len;
						if (SUCCEEDED(pTypeInfo->GetNames(pVarDesc->memid, &bstr, 1, &len)) && len >= 0 && bstr) {
							//log(L"const:%s", bstr);
							tTJSVariant result;
							IDispatchWrapper::storeVariant(result, *(pVarDesc->lpvarValue));
							target->PropSet(TJS_MEMBERENSURE|TJS_IGNOREPROP,
											bstr,
											NULL,
											&result,
											target
											);
							SysFreeString(bstr);
						}
					}
					pTypeInfo->ReleaseVarDesc(pVarDesc);
				}
			}
			pTypeInfo->ReleaseTypeAttr(pTypeAttr);
		}
	}

	/**
	 * �萔�̎擾
	 * @param pTypeLib TYPELIB
	 * @param target �i�[��
	 */
	void getConstant(ITypeLib *pTypeLib, iTJSDispatch2 *target) {
		unsigned int count = pTypeLib->GetTypeInfoCount();
		for (unsigned int i=0; i<count; i++) {
			ITypeInfo *pTypeInfo = NULL;
			if (SUCCEEDED(pTypeLib->GetTypeInfo(i, &pTypeInfo))) {
				getConstant(pTypeInfo, target);
				pTypeInfo->Release();
			}
		}
	}

	/**
	 * �萔�̎擾
	 * @param target �i�[��
	 */
	void getConstant(iTJSDispatch2 *target) {
		if (pDispatch) {
			if (target) {
				ITypeInfo *pTypeInfo = NULL;
				if (SUCCEEDED(pDispatch->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &pTypeInfo))) {
					unsigned int index = 0;
					ITypeLib *pTypeLib = NULL;
					if (SUCCEEDED(pTypeInfo->GetContainingTypeLib(&pTypeLib, &index))) {
						getConstant(pTypeLib, target);
						pTypeLib->Release();
					}
					pTypeInfo->Release();
				}
			}
		}
	}

	/**
	 * ���\�b�h���s
	 */
	static tjs_error _getConstantMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis, WIN32OLE *self) {
		if (!self) {
			return TJS_E_NATIVECLASSCRASH;
		}
		if (numparams > 0) {
			iTJSDispatch2 *store = param[0]->AsObject();
			if (store) {
				self->getConstant(store);
				store->Release();
			}
		} else {
			self->getConstant(objthis);
		}
		return TJS_S_OK;
	}

public:
	static tjs_error addEventMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		WIN32OLE *self = ncbInstanceAdaptor<WIN32OLE>::GetNativeInstance(objthis);
		return _addEventMethod(result, numparams, param, objthis, self);
	}
	
	static tjs_error getConstantMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		WIN32OLE *self = ncbInstanceAdaptor<WIN32OLE>::GetNativeInstance(objthis);
		return _getConstantMethod(result, numparams, param, objthis, self);
	}
};

NCB_REGISTER_CLASS(WIN32OLE) {
	Factory(&ClassT::factory);
	RawCallback("invoke",  &ClassT::invokeMethod,  0);
	RawCallback("set",     &ClassT::setMethod,     0);
	RawCallback("get",     &ClassT::getMethod,     0);
	RawCallback("missing", &ClassT::missingMethod, 0);
	RawCallback("addEvent", &ClassT::addEventMethod, 0);
	RawCallback("getConstant", &ClassT::getConstantMethod, 0);
};

//---------------------------------------------------------------------------

/**
 * DHTML�̊O���ďo��(window.external)�Ɗe��UI�������󗝂��邽�߂̃C���^�[�t�F�[�X�N���X�B
 * ���ꂼ��N���X���̑Ή����郁�\�b�h���Ăяo���Ă���B�p�����Ē��g���L�q���邱�Ƃ�
 * �������ύX�ł���B�p�����[�^��COM�X�^�C���̂���(VARIANT)�ɂȂ�̂Œ���
 * XXX TJS �̃��\�b�h���Ăяo���ł���悤�Ɍ�ŉ��Ǘ\��B���݂̂��̂́u���������Ȃ��v
 * ���߂̂��̂ɂȂ��Ă���B
 */ 
class CExternalUI : public IDocHostUIHandlerDispatch {

protected:
	IDispatchEx *dispatchEx;
	
public:
	CExternalUI() {
		iTJSDispatch2 * global = TVPGetScriptDispatch();
		dispatchEx = new IDispatchWrapper(global);
		global->Release();
	}

	~CExternalUI() {
		dispatchEx->Release();
	}

	//----------------------------------------------------------------------------
	// IUnknown ����
	
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
											 void __RPC_FAR *__RPC_FAR *ppvObject) {
		if (dispatchEx && (riid == IID_IUnknown || riid == IID_IDispatch || riid == IID_IDispatchEx)) {
			//log(L"get dispatchEx");
			if (ppvObject == NULL)
				return E_POINTER;
			dispatchEx->AddRef();
			*ppvObject = dispatchEx;
			return S_OK;
		} else if (riid == IID_IUnknown || riid == IID_IDispatch) {
			if (ppvObject == NULL)
				return E_POINTER;
			*ppvObject = this;
			AddRef();
			return S_OK;
		} else {
			*ppvObject = 0;
			return E_NOINTERFACE;
		}
	}

	// XXX MSHTML ����̌Ăяo�������������̂ŊJ�����Ȃ��悤�ɂ��Ă���
	ULONG STDMETHODCALLTYPE AddRef() {
		return 1;
	}

	// XXX MSHTML ����̌Ăяo�������������̂ŊJ�����Ȃ��悤�ɂ��Ă���
	ULONG STDMETHODCALLTYPE Release() {
		return 1;
	}

	//----------------------------------------------------------------------------
	// IDispatch ����

	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) {
		return E_NOTIMPL;
	}

	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) {
		return E_NOTIMPL;
	}

	/**
	 * ���\�b�h����ID�̑Ή����Ƃ郁�\�b�h
	 * regszNames �Ƀ��\�b�h���̔z�񂪂���̂ŁA
	 * rgdispid �ɑΉ����� dispid ��Ԃ��Ă��
	 */
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
							 LCID lcid, DISPID* rgdispid) {
		return E_NOTIMPL;
	}

	/**
	 * ���\�b�h���s
	 * dispidMember �Ń��\�b�h���w�肳���B������ pdispparams �� VARIANT �̔z��
	 * �̌`�ł킽�����̂ł�����g��
	 */
	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
					  LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
					  EXCEPINFO* pexcepinfo, UINT* puArgErr) {
		return E_NOTIMPL;
	}

	//----------------------------------------------------------------------------
	// IDocHostUIHandlerDispatch �̎���

	
	/**
	 * �R���e�L�X�g���j���[����
	 * �������Ȃ����ƂŃ��j���[�������Ă���
	 */
	HRESULT STDMETHODCALLTYPE ShowContextMenu( 
		/* [in] */ DWORD dwID,
		/* [in] */ DWORD x,
		/* [in] */ DWORD y,
		/* [in] */ IUnknown __RPC_FAR *pcmdtReserved,
		/* [in] */ IDispatch __RPC_FAR *pdispReserved,
		/* [retval][out] */ HRESULT __RPC_FAR *dwRetVal) {
		*dwRetVal = S_OK;      //This is what the WebBrowser control is looking for.
		//You can show your own context menu here.
		return S_OK;        
	}

	HRESULT STDMETHODCALLTYPE GetHostInfo( 
		/* [out][in] */ DWORD __RPC_FAR *pdwFlags,
		/* [out][in] */ DWORD __RPC_FAR *pdwDoubleClick) {
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE ShowUI( 
		/* [in] */ DWORD dwID,
		/* [in] */ IUnknown __RPC_FAR *pActiveObject,
		/* [in] */ IUnknown __RPC_FAR *pCommandTarget,
		/* [in] */ IUnknown __RPC_FAR *pFrame,
		/* [in] */ IUnknown __RPC_FAR *pDoc,
		/* [retval][out] */ HRESULT __RPC_FAR *dwRetVal) {
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE HideUI( void) {
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE UpdateUI( void) {
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE EnableModeless(
		/* [in] */ VARIANT_BOOL fEnable) {
		return E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE OnDocWindowActivate( 
		/* [in] */ VARIANT_BOOL fActivate) {
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE OnFrameWindowActivate(
		/* [in] */ VARIANT_BOOL fActivate) {
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE ResizeBorder( 
		/* [in] */ long left,
		/* [in] */ long top,
		/* [in] */ long right,
		/* [in] */ long bottom,
		/* [in] */ IUnknown __RPC_FAR *pUIWindow,
		/* [in] */ VARIANT_BOOL fFrameWindow) {
		return E_NOTIMPL;
	}
	
	HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
		/* [in] */ DWORD hWnd,
		/* [in] */ DWORD nMessage,
		/* [in] */ DWORD wParam,
		/* [in] */ DWORD lParam,
		/* [in] */ BSTR bstrGuidCmdGroup,
		/* [in] */ DWORD nCmdID,
		/* [retval][out] */ HRESULT __RPC_FAR *dwRetVal) {
		return E_NOTIMPL;
	}
	
	HRESULT STDMETHODCALLTYPE GetOptionKeyPath( 
		/* [out] */ BSTR __RPC_FAR *pbstrKey,
		/* [in] */ DWORD dw) {
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE GetDropTarget( 
		/* [in] */ IUnknown __RPC_FAR *pDropTarget,
		/* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppDropTarget) {
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE GetExternal( 
		/* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDispatch) {
		*ppDispatch = this;
		return S_OK;
	}
        
	HRESULT STDMETHODCALLTYPE TranslateUrl( 
		/* [in] */ DWORD dwTranslate,
		/* [in] */ BSTR bstrURLIn,
		/* [out] */ BSTR __RPC_FAR *pbstrURLOut) {
		return E_NOTIMPL;
	}
        
	HRESULT STDMETHODCALLTYPE FilterDataObject( 
		/* [in] */ IUnknown __RPC_FAR *pDO,
		/* [out] */ IUnknown __RPC_FAR *__RPC_FAR *ppDORet) {
		return E_NOTIMPL;
	}
};

/*
 * ActiveX �l�C�e�B�u�C���X�^���X
 */
class ActiveX : public WIN32OLE, public CWindowImpl<ActiveX, CAxWindow>
{
protected:
	iTJSDispatch2 *window;  //< �I�u�W�F�N�g���̎Q��
	ttstr progId;
	int left;
	int top;
	int width;
	int height;

	CExternalUI *externalUI;

	// �C�x���g����
	static bool __stdcall messageHandler(void *userdata, tTVPWindowMessage *Message);

	// ���[�U���b�Z�[�W���V�[�o�̓o�^/����
	void setReceiver(tTVPWindowMessageReceiver receiver, bool enable) {
		tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)this;
		tTJSVariant *p[] = {&mode, &proc, &userdata};
		if (window->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, window) != TJS_S_OK) {
			if (enable) {
				TVPThrowExceptionMessage(L"can't regist user message receiver");
			}
		}
	}

	/**
	 * �E�C���h�E�𐶐�
	 * @param krkr �g���g���̃E�C���h�E
	 * @return ���������� true
	 */
	void createWindow(HWND krkr) {
		RECT rect;
		rect.left   = left;
		rect.top    = top;
		rect.right  = left + width;
		rect.bottom = top  + height;
		
		HRESULT hr;
		HWND parent;
		if (krkr) {
			if (!(parent = FindWindowEx(krkr, NULL, KRKRDISPWINDOWCLASS, NULL))) {
				// KRKRZ �̏ꍇ�́��̑�������
				parent = krkr;
			}
			if (width == -1 || height == -1) {
				::GetClientRect(parent, &rect);
			}
			Create(parent, rect, NULL, WS_CHILD|WS_CLIPCHILDREN);
		} else {
			Create(0, rect, NULL, WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN);
		}
		if (m_hWnd) {
			// �R���g���[������
			OLECHAR *oleName = SysAllocString(progId.c_str());
			hr = CreateControl(oleName);
			if (SUCCEEDED(hr)) {
				// �O���f�B�X�p�b�`����o�^
				{
					IDispatchEx *dispatchEx = new IDispatchWrapper(objthis);
					SetExternalDispatch(dispatchEx);
					dispatchEx->Release();
				}
				// �O��UI�n���h����o�^
				if (externalUI) {
					SetExternalUIHandler(externalUI);
				}
				// IDispatch�擾
				hr = QueryControl(IID_IDispatch, (void**)&pDispatch);
			} else {
				log(L"CreateControl failed %ws", progId.c_str());
			}
			SysFreeString(oleName);
		}
		// XXX ���������㏈���Ăяo��
		// if (SUCCEEDED(hr)) { onCreate(); }
	}
		
	/**
	 * ����j��
	 */
	void clear() {
		// XXX �j���O�����Ăяo��
		// if (m_hWnd) {onDestroy();};
		WIN32OLE::clear();
		if (m_hWnd) {
			DestroyWindow();
			m_hWnd = 0;
		}
	}

	// �z�u�����p
	void _setPos() {
		if (m_hWnd) {
			SetWindowPos(0, left, top, width, height, 0);
		}
	}
public:
	static tjs_error invokeMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, ActiveX *self) {
		return self->invoke(DISPATCH_PROPERTYGET|DISPATCH_METHOD, result, numparams, param);
	}
	
	static tjs_error setMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, ActiveX *self) {
		return self->invoke(DISPATCH_PROPERTYPUT, result, numparams, param);
	}
	
	static tjs_error getMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, ActiveX *self) {
		return self->invoke(DISPATCH_PROPERTYGET, result, numparams, param);
	}

	static tjs_error missingMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, ActiveX *self) {
		return self->missing(result, numparams, param);
	}

	static tjs_error addEventMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		ActiveX *self = ncbInstanceAdaptor<ActiveX>::GetNativeInstance(objthis);
		return _addEventMethod(result, numparams, param, objthis, self);
	}
	
	static tjs_error getConstantMethod(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		ActiveX *self = ncbInstanceAdaptor<ActiveX>::GetNativeInstance(objthis);
		return _getConstantMethod(result, numparams, param, objthis, self);
	}
	
BEGIN_MSG_MAP(ActiveX)
END_MSG_MAP()
	
	/**
	 * �R���X�g���N�^
	 * @param objthis TJS2�C���X�^���X
	 * @param numparams �p�����[�^��
	 * @param param �p�����[�^�z��
	 */
	ActiveX(iTJSDispatch2 *objthis, tjs_int numparams, tTJSVariant **param) : WIN32OLE(objthis, NULL), window(NULL), externalUI(NULL), left(0), top(0), width(-1), height(-1) {
		
		progId = param[0]->GetString();
		
		if (numparams >= 6) {
			left   = *param[2];
			top    = *param[3];
			width  = *param[4];
			height = *param[5];
		}
		
		HWND handle = 0;
		if (numparams >= 2 && param[1]->Type() == tvtObject) {
			// �E�C���h�E���w��
			iTJSDispatch2 *win = param[1]->AsObjectNoAddRef();
			if (win->IsInstanceOf(0, NULL, NULL, L"Window", win) == TJS_S_TRUE) {
				window = win;
				window->AddRef();
				setReceiver(messageHandler, true);
				tTJSVariant hwnd;
				if (win->PropGet(0, TJS_W("HWND"), NULL, &hwnd, win) == TJS_S_OK) {
					HWND handle = (HWND)(int)hwnd;
					if (handle) {
						// ���ɐ����ς�
						createWindow(handle);
					}
				}
			} else {
				TVPThrowExceptionMessage(L"must set window object");
			}
		} else {
			// �Ɨ��E�C���h�E
			createWindow(0);
		}
	}

	/**
	 * �f�X�g���N�^
	 */
	~ActiveX() {
		clear();
		if (externalUI) {
			delete externalUI;
			externalUI = NULL;
		}
		if (window) {
			setReceiver(messageHandler, false);
			window->Release();
			window = NULL;
		}
	}

	/**
	 * �C���X�^���X�����t�@�N�g��
	 * @param name  ���ʖ�
	 * @param left  �\���ʒu
	 * @param top   �\���ʒu
	 * @param width  �\���T�C�Y
	 * @param height �����w��
	 */
	static tjs_error factory(ActiveX **result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {
		// �p�����[�^�͍Œ�ЂƂK�{
		if (numparams < 1) {
			return TJS_E_BADPARAMCOUNT;
		}
		if (result) {
			*result = new ActiveX(objthis, numparams, param);
		}
		return TJS_S_OK;
	}

	/**
	 * �O���g���n���h���̓o�^
	 * �������� IE ��p�̏���
	 */
	void setExternalUI() {
		if (externalUI == NULL) {
			externalUI = new CExternalUI();
			if (m_hWnd) {
				SetExternalUIHandler(externalUI);
			}
		}
	}

	void setVisible(bool visible) {
		if (m_hWnd) {
			if (visible) {
				_setPos();
			}
			ShowWindow(visible);
		}
	}

	bool isValidWindow() {
		return m_hWnd != 0;
	}
	
	bool getVisible() {
		return m_hWnd && IsWindowVisible();
	}

	void setLeft(int l) {
		left = l;
		_setPos();
	}

	int getLeft() {
		return left;
	}

	void setTop(int t) {
		top = t;
		_setPos();
	}

	int getTop() {
		return top;
	}
	
	void setWidth(int w) {
		width = w;
		_setPos();
	}

	int getWidth() {
		return width;
	}

	void setHeight(int h) {
		height = h;
		_setPos();
	}

	int getHeight() {
		return height;
	}
	
	/**
	 * ���ꏊ�w��
	 */	
	void setPos(int l, int t) {
		left = l;
		top  = t;
		_setPos();
	}

	/**
	 * ���T�C�Y�w��
	 */	
	void setSize(int w, int h) {
		width = w;
		height = h;
		_setPos();
	}
};

// �C�x���g����
bool __stdcall
ActiveX::messageHandler(void *userdata, tTVPWindowMessage *Message)
{
	ActiveX *self = (ActiveX*)userdata;
	switch (Message->Msg) {
	case TVP_WM_DETACH:
		self->clear();
		break;
	case TVP_WM_ATTACH:
		self->createWindow((HWND)Message->LParam);
		break;
	default:
		break;
	}
	return false;
}

NCB_REGISTER_CLASS(ActiveX) {
	Factory(&ClassT::factory);
	RawCallback("invoke",  &ClassT::invokeMethod,  0);
	RawCallback("set",     &ClassT::setMethod,     0);
	RawCallback("get",     &ClassT::getMethod,     0);
	RawCallback("missing", &ClassT::missingMethod, 0);
	RawCallback("addEvent", &ClassT::addEventMethod, 0);
	RawCallback("getConstant", &ClassT::getConstantMethod, 0);
	NCB_METHOD(setExternalUI);
	NCB_METHOD(setPos);
	NCB_METHOD(setSize);
	NCB_PROPERTY_RO(isValidWindow, isValidWindow);
	NCB_PROPERTY(visible, getVisible, setVisible);
	NCB_PROPERTY(left, getLeft, setLeft);
	NCB_PROPERTY(top, getTop, setTop);
	NCB_PROPERTY(width, getWidth, setWidth);
	NCB_PROPERTY(height, getHeight, setHeight);
};

//---------------------------------------------------------------------------

// �g���g���̃A�[�J�C�u�ɃA�N�Z�X���邽�߂̏���
void registerArchive();
void unregisterArchive();

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
	
	// �A�[�J�C�u����
	registerArchive();
	
	// ATL�֘A������
	_Module.Init(NULL, NULL);
	AtlAxWinInit();
}

/**
 * �o�^������
 */
static void PostRegistCallback()
{
}

/**
 * �J�������O
 */
static void PreUnregistCallback()
{
}

/**
 * �J��������
 */
static void PostUnregistCallback()
{
	// ATL �I��
	_Module.Term();

	// �A�[�J�C�u�I��
	unregisterArchive();
	
	if (gOLEInitialized) {
		OleUninitialize();
		gOLEInitialized = false;
	}
}


NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_REGIST_CALLBACK(PostRegistCallback);
NCB_PRE_UNREGIST_CALLBACK(PreUnregistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
