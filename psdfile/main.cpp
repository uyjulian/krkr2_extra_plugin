#include <ncbind.hpp>

#include "psdclass.h"

// -----------------------------------------------------------------------------
// �X�g���[�W�@�\
// -----------------------------------------------------------------------------

#define BASENAME L"psd"

/**
 * PSD�X�g���[�W
 */
class PSDStorage : public iTVPStorageMedia, tTVPCompactEventCallbackIntf
{
public:

	/**
	 * �R���X�g���N�^
	 */
	PSDStorage() : refCount(1) {
	}

	/**
	 * �f�X�g���N�^
	 */
	virtual ~PSDStorage() {
			clearCache();
	}

	/*
	 * PSD�I�u�W�F�N�g�Q�Ƃ�ǉ�
	 * @param filename �Q�ƃt�@�C����
	 * @param psd PSD�I�u�W�F�N�g
	 */
	void add(ttstr filename, PSD *psd) {
		psdMap[filename] = psd;
	}
	
	/**
	 * PSD�I�u�W�F�N�g�Q�Ƃ̏����v��
	 * @param psd PSD�I�u�W�F�N�g
	 */
	void remove(PSD *psd) {
		PSDMap::iterator it = psdMap.begin();
		while (it != psdMap.end()) {
			if (it->second == psd) {
				it = psdMap.erase(it);
			} else {
				it++;
			}
		}
	}

	/**
	 * �L���b�V�������N���A����
	 */
	void clearCache() {
		cache.Clear();
	}

public:
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
		ttstr fname;
		PSD *psd = getPSD(name, fname);
		if (psd) {
			bool ret = psd->CheckExistentStorage(fname);
			return ret;
		}
		return false;
	}

	// open a storage and return a tTJSBinaryStream instance.
	// name does not contain in-archive storage name but
	// is normalized.
	virtual tTJSBinaryStream * TJS_INTF_METHOD Open(const ttstr & name, tjs_uint32 flags) {
		if (flags == TJS_BS_READ) { // �ǂݍ��݂̂�
			ttstr fname;
			PSD *psd = getPSD(name, fname);
			if (psd) {
				IStream *stream = psd->openLayerImage(fname);
				if (stream) {
					tTJSBinaryStream *ret = TVPCreateBinaryStreamAdapter(stream);
					stream->Release();
					return ret;
				}
			}
		}
		TVPThrowExceptionMessage(TJS_W("%1:cannot open psdfile"), name);
		return NULL;
	}

	// list files at given place
	virtual void TJS_INTF_METHOD GetListAt(const ttstr &name, iTVPStorageLister * lister) {
		ttstr fname;
		PSD *psd = getPSD(name, fname);
		if (psd) {
			psd->GetListAt(fname, lister);
		}
	}

	// basically the same as above,
	// check wether given name is easily accessible from local OS filesystem.
	// if true, returns local OS native name. otherwise returns an empty string.
	virtual void TJS_INTF_METHOD GetLocallyAccessibleName(ttstr &name) {
		name = "";
	}

public:
	// -----------------------------------
	// tTVPCompactEventCallbackIntf
	// -----------------------------------
	virtual void TJS_INTF_METHOD OnCompact(tjs_int level) {
		if (level > TVP_COMPACT_LEVEL_MINIMIZE) {
			clearCache();
		}
	}
	
protected:

	/*
	 * �t�@�C�����ɍ��v���� PSD �����擾
	 * @param name �t�@�C����
	 * @param fname �t�@�C������Ԃ�
	 * @return PSD���
	 */
	PSD *getPSD(ttstr name, ttstr &fname) {
		// �������Ő��K��
		name.ToLowerCase();
		// �h���C�����Ƃ���ȍ~�𕪗�
		ttstr dname;
		const tjs_char *p = name.c_str();
		const tjs_char *q;
		if ((q = wcschr(p, '/'))) {
			dname = ttstr(p, q-p);
			fname = ttstr(q+1);
		} else {
			TVPThrowExceptionMessage(TJS_W("invalid path:%1"), name);
		}

		PSD *psd = 0;

		// ���߂̃L���b�V�������v����ꍇ�͂����Ԃ�
		if (cache.Type() == tvtObject &&
				(psd = ncbInstanceAdaptor<PSD>::GetNativeInstance(cache.AsObjectNoAddRef())) &&
				psd->dname == dname) {
			return psd;
		}

		// PSD�I�u�W�F�N�g�̎�Q�Ƃ���T��
		PSDMap::iterator it = psdMap.find(dname);
		if (it != psdMap.end()) {
			// �����f�[�^������
			cache = it->second->getSelf();
			return it->second;
		}

		// ������open���Ă��̂܂܃L���b�V���Ƃ��Ď���
		TVPExecuteExpression(L"new PSD()", &cache);
		psd = ncbInstanceAdaptor<PSD>::GetNativeInstance(cache.AsObjectNoAddRef());
		if (psd) {
			if (psd->load(dname)) {
				return psd;
			} else {
				clearCache();
			}
		}

		return 0;
	}

private:
	tjs_uint refCount; //< ���t�@�����X�J�E���g

	// PSD�I�u�W�F�N�g�̃L���b�V���Q��
	tTJSVariant cache;
	
	// PSD�I�u�W�F�N�g�̎�Q��
	typedef std::map<ttstr, PSD*> PSDMap;
	PSDMap psdMap;
};

static PSDStorage *psdStorage = 0;

// ��Q�ƒǉ�
void
PSD::addToStorage(const ttstr &filename)
{
	// �o�^�p�x�[�X���𐶐�
	const tjs_char *p = filename.c_str();
	const tjs_char *q;
	if ((q = wcsrchr(p, '/'))) {
		dname = ttstr(q+1);
	} else {
		dname = filename;
	}
	// �������Ő��K��
	dname.ToLowerCase();
	if (psdStorage != NULL) {
		psdStorage->add(dname, this);
	}
}

// ��Q�Ɖ���
void
PSD::removeFromStorage()
{
	if (psdStorage != NULL) {
		psdStorage->remove(this);
	}
}

void
PSD::clearStorageCache()
{
	if (psdStorage != NULL) {
		psdStorage->clearCache();
	}
}

// --------------------------------------------------------------------


void initStorage()
{
	if (psdStorage== NULL) {
		psdStorage = new PSDStorage();
		TVPRegisterStorageMedia(psdStorage);
		TVPAddCompactEventHook((tTVPCompactEventCallbackIntf*)psdStorage);
	}
}

void doneStorage()
{
	if (psdStorage != NULL) {
		TVPRemoveCompactEventHook((tTVPCompactEventCallbackIntf*)psdStorage);
		TVPUnregisterStorageMedia(psdStorage);
		psdStorage->Release();
		psdStorage = NULL;
	}
}

NCB_PRE_REGIST_CALLBACK(initStorage);
NCB_POST_UNREGIST_CALLBACK(doneStorage);
