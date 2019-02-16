#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "ncbind/ncbind.hpp"
#include <map>

#define BASENAME L"var"

// �������ǂ����̔���
static bool isDirectory(tTJSVariant &base) {
	return base.Type() == tvtObject && base.AsObjectNoAddRef() != NULL;
}

// �t�@�C�����ǂ����̔���
static bool isFile(tTJSVariant &file) {
	return file.Type() == tvtOctet;
}

/**
 * Variant�Q�ƌ^�X�g���[��
 */
class VariantStream : public IStream {

public:
	/**
	 * �R���X�g���N�^
	 */
	VariantStream(tTJSVariant &parent) : refCount(1), parent(parent), hBuffer(0), stream(0), cur(0) {};

	/**
	 * �t�@�C�����J��
	 */
	bool open(const ttstr &name, tjs_uint32 flags) {
		close();
		this->name = name;

		// �ǂݍ��݂݂̂̏ꍇ
		if (flags == TJS_BS_READ) {
			parent.AsObjectClosureNoAddRef().PropGet(0, name.c_str(), NULL, &value, NULL);
			return isFile(value);
		}

		// �������݂��K�v�ȏꍇ
		hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, 0);
		if (FAILED(::CreateStreamOnHGlobal(hBuffer, FALSE, &stream))) {
			::GlobalFree(hBuffer);
			hBuffer = 0;
			return false;
		}

		// �I�u�W�F�N�g�̓��e�𕡐�
		if (flags == TJS_BS_UPDATE || flags == TJS_BS_APPEND) {
			parent.AsObjectClosureNoAddRef().PropGet(0, name.c_str(), NULL, &value, NULL);
			if (isFile(value)) {
				stream->Write(value.AsOctetNoAddRef()->GetData(), value.AsOctetNoAddRef()->GetLength(), NULL);
				LARGE_INTEGER n;
				n.QuadPart = 0;
				stream->Seek(n, flags == TJS_BS_UPDATE ? STREAM_SEEK_SET : STREAM_SEEK_END, NULL);
			}
		}
		return true;
	}
	
	// IUnknown
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) {
		if (riid == IID_IUnknown || riid == IID_ISequentialStream || riid == IID_IStream) {
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

	ULONG STDMETHODCALLTYPE AddRef(void) {
		refCount++;
		return refCount;
	}
	
	ULONG STDMETHODCALLTYPE Release(void) {
		int ret = --refCount;
		if (ret <= 0) {
			delete this;
			ret = 0;
		}
		return ret;
	}

	// ISequentialStream
	HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead) {
		if (stream) {
			return stream->Read(pv, cb, pcbRead);
		} else {
			const tjs_uint8 *base = getBase();
			tTVInteger size = getSize() - cur;
			if (base && cb > 0 && size > 0) {
				if (cb > size) {
					cb = (ULONG)size;
				}
				memcpy(pv, base + cur, cb);
				cur += cb;
				if (pcbRead) {
					*pcbRead = cb;
				}
				return S_OK;
			} else {
				if (pcbRead) {
					*pcbRead = 0;
				}
				return S_FALSE;
			}
		}
	}

	HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten) {
		if (stream) {
			return stream->Write(pv, cb, pcbWritten);
		} else {
			return E_NOTIMPL;
		}
	}

	// IStream
	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove,	DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) {
		if (stream) {
			return stream->Seek(dlibMove, dwOrigin, plibNewPosition);
		} else {
			switch (dwOrigin) {
			case STREAM_SEEK_CUR:
				cur += dlibMove.QuadPart;
				break;
			case STREAM_SEEK_SET:
				cur = dlibMove.QuadPart;
				break;
			case STREAM_SEEK_END:
				cur = getSize();
				cur += dlibMove.QuadPart;
				break;
			}
			if (plibNewPosition) {
				plibNewPosition->QuadPart = cur;
			}
			return S_OK;
		}
	}
	
	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize) {
		return stream ? stream ->SetSize(libNewSize) : E_NOTIMPL;
	}
	
	HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten) {
		return stream ? stream->CopyTo(pstm, cb, pcbRead, pcbWritten) : E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags) {
		return stream ? stream->Commit(grfCommitFlags) : E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE Revert(void) {
		return stream ? stream->Revert() : E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) {
		return stream ? stream->LockRegion(libOffset, cb, dwLockType) : E_NOTIMPL;
	}
	
	HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) {
		return stream ? stream->UnlockRegion(libOffset, cb, dwLockType) : E_NOTIMPL;
	}
	
	HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag) {
		return stream ? stream->Stat(pstatstg, grfStatFlag) : E_NOTIMPL;
	}

	HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm) {
		return stream ? stream->Clone(ppstm) : E_NOTIMPL;
	}

protected:

	/**
	 * �t�@�C������鏈��
	 */
	void close() {
		if (stream) {
			stream->Release();
			stream = NULL;
		}
		if (hBuffer) {
			// �����߂�����
			if (name != "") {
				unsigned char* pBuffer = (unsigned char*)::GlobalLock(hBuffer);
				if (pBuffer) {
					value = tTJSVariant(pBuffer, GlobalSize(hBuffer));
					parent.AsObjectClosureNoAddRef().PropSet(TJS_MEMBERENSURE, name.c_str(), NULL, &value, NULL);
					::GlobalUnlock(hBuffer);
				}
			}
			::GlobalFree(hBuffer);
			hBuffer = 0;
		}
		value.Clear();
		cur = 0;
	}

    /**
	 * �f�X�g���N�^
	 */
	virtual ~VariantStream() {
		close();
	}

	// �ǂݍ��ݗp�������̈�擾
	const tjs_uint8 *getBase() {
		return isFile(value) ? value.AsOctetNoAddRef()->GetData() : NULL;
	}

	// �ǂݍ��ݗp�������T�C�Y�擾
	tTVInteger getSize() {
		return isFile(value) ? value.AsOctetNoAddRef()->GetLength() : 0;
	}

private:
	int refCount;
	tTJSVariant parent;
	ttstr name;
	tTJSVariant value;
	HGLOBAL hBuffer;
	IStream *stream;
	tTVInteger cur;
};

/**
 * �����o�o�^�����p
 */
class GetLister : public tTJSDispatch /** EnumMembers �p */
{

public:
	// �R���X�g���N�^
	GetLister(iTVPStorageLister *lister) : lister(lister) {};

	// EnumMember�p�J��Ԃ����s��
	// param[0] �����o��
	// param[1] �t���O
	// param[2] �����o�̒l
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
			if (!(flag & TJS_HIDDENMEMBER) && isFile(*param[2])) {
				lister->Add(ttstr(param[0]->GetString()));
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}

private:
	iTVPStorageLister *lister;
};


/**
 * Var�X�g���[�W
 */
class VarStorage : public iTVPStorageMedia
{

public:
	/**
	 * �R���X�g���N�^
	 */
	VarStorage() : refCount(1) {
	}

	// -----------------------------------
	// iTVPStorageMedia Intefaces
	// -----------------------------------

	virtual void TJS_INTF_METHOD AddRef() {
		refCount++;
	};

	virtual void TJS_INTF_METHOD Release() {
		if (refCount == 1) {
			delete this;
		} else {
			refCount--;
		}
	};

	// returns media name like "file", "http" etc.
	virtual void TJS_INTF_METHOD GetName(ttstr &name) {
		name = BASENAME;
	}

	//	virtual ttstr TJS_INTF_METHOD IsCaseSensitive() = 0;
	// returns whether this media is case sensitive or not

	// normalize domain name according with the media's rule
	virtual void TJS_INTF_METHOD NormalizeDomainName(ttstr &name) {
		// nothing to do
	}

	// normalize path name according with the media's rule
	virtual void TJS_INTF_METHOD NormalizePathName(ttstr &name) {
		// nothing to do
	}

	// check file existence
	virtual bool TJS_INTF_METHOD CheckExistentStorage(const ttstr &name) {
		return isFile(getFile(name));
	}

	// open a storage and return a tTJSBinaryStream instance.
	// name does not contain in-archive storage name but
	// is normalized.
	virtual tTJSBinaryStream * TJS_INTF_METHOD Open(const ttstr & name, tjs_uint32 flags) {
		tTJSBinaryStream *ret = NULL;
		ttstr fname;
		tTJSVariant parent = getParentName(name, fname);
		if (isDirectory(parent) && fname.length() > 0) {
			VariantStream *stream = new VariantStream(parent);
			if (stream) {
				if (stream->open(fname, flags)) {
					ret = TVPCreateBinaryStreamAdapter(stream);
				}
				stream->Release();
			}
		}
		if (!ret) {
			TVPThrowExceptionMessage(TJS_W("cannot open memfile:%1"), name);
		}
		return ret;
	}

	// list files at given place
	virtual void TJS_INTF_METHOD GetListAt(const ttstr &name, iTVPStorageLister * lister) {
		tTJSVariant base = getFile(name);
		if (isDirectory(base)) {
			tTJSVariantClosure closure(new GetLister(lister));
			base.AsObjectClosureNoAddRef().EnumMembers(TJS_IGNOREPROP, &closure, NULL);
			closure.Release();
		}
	}

	// basically the same as above,
	// check wether given name is easily accessible from local OS filesystem.
	// if true, returns local OS native name. otherwise returns an empty string.
	virtual void TJS_INTF_METHOD GetLocallyAccessibleName(ttstr &name) {
		name = "";
	}

protected:

	/**
	 * �f�X�g���N�^
	 */
	virtual ~VarStorage() {
	}
	
	/*
	 * �e�t�H���_�ƃp�X��Ԃ�
	 * @param name �t�@�C����
	 * @param fname �t�@�C������Ԃ�
	 * @return �e�t�H���_
	 */
	tTJSVariant getParentName(const ttstr &name, ttstr &fname) {
		// �h���C�����𕪗�
		const tjs_char *p = name.c_str();
		const tjs_char *q;
		if ((q = wcschr(p, '/'))) {
			ttstr dname = ttstr(p, q-p);
			if (dname != L".") {
				TVPThrowExceptionMessage(TJS_W("no such domain:%1"), dname);
			}
		} else {
			TVPThrowExceptionMessage(TJS_W("invalid path:%1"), name);
		}
		// �p�X��
		ttstr path = ttstr(q+1);
		iTJSDispatch2 *global = TVPGetScriptDispatch();
		tTJSVariant base(global, global);
		while (path.length() > 0) {
			p = path.c_str();
			q = wcschr(p, '/');
			if (q == NULL) {
				// �t�@�C��
				break;
			} else if (q == p) {
				// �t�H���_������
				base.Clear();
				break;
			} else {
				// �t�H���_
				ttstr member = ttstr(p, q-p);
				tTJSVariant value;
				tTJSVariantClosure &o = base.AsObjectClosureNoAddRef();
				if (((o.IsInstanceOf(0, NULL, NULL, L"Array", NULL) == TJS_S_TRUE &&
					  TJS_SUCCEEDED(o.PropGetByNum(0, (tjs_int)TJSStringToInteger(member.c_str()), &value, NULL))) ||
					 (TJS_SUCCEEDED(o.PropGet(0, member.c_str(), NULL, &value, NULL)))) && isDirectory(value)) {
					base = value;
					path = ttstr(q+1);
				} else {
					base.Clear();
					break;
				}
			}
		}
		fname = path;
		return base;
	}
	
	/*
	 * �t�@�C�����ɍ��v����ϐ���T���ĕԂ�
	 * @param name �t�@�C����
	 * @return ���������t�@�C���܂��̓t�H���_�B������Ȃ��ꍇ�� tvtVoid
	 */
	tTJSVariant getFile(const ttstr &name) {
		ttstr fname;
		tTJSVariant base = getParentName(name, fname);
		if (isDirectory(base) && fname.length() > 0) {
			// �t�@�C��
			tTJSVariant value;
			if (TJS_SUCCEEDED(base.AsObjectClosureNoAddRef().PropGet(0, fname.c_str(), NULL, &value, NULL))) {
				base = value;
			} else {
				base.Clear();
			}
		}
		return base;
	}

private:
	tjs_uint refCount; //< ���t�@�����X�J�E���g
};

VarStorage *var = NULL;

/**
 * �J��������
 */
static void PreRegistCallback()
{
	if (var == NULL) {
		var = new VarStorage();
		TVPRegisterStorageMedia(var);
	}
}

/**
 * �J��������
 */
static void PostUnregistCallback()
{
	if (var != NULL) {
		TVPUnregisterStorageMedia(var);
		var->Release();
		var = NULL;
	}
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
