#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "ncbind/ncbind.hpp"
#include <map>

#define BASENAME L"mem"

/**
 * �t�@�C�����ێ��p�N���X
 */
class FileInfo {

public:
	typedef std::map<ttstr,FileInfo*> Directory;

	/**
	 * �R���X�g���N�^
	 * @param pParent �e�f�B���N�g��
	 * @param name ���O
	 * @param directory �f�B���N�g���Ȃ� true
	 */
	FileInfo(FileInfo *pParent, const ttstr &name, bool directory=false) : pParent(pParent), name(name), hBuffer(0), pDirectory(0) {
		if (directory) {
			pDirectory = new Directory();
		}
	}

	/**
	 * �f�X�g���N�^
	 */
	~FileInfo() {
		if (pDirectory) {
			Directory::iterator it = pDirectory->begin();
			while (it != pDirectory->end()) {
				delete it->second;
				it++;
			}
			delete pDirectory;
		}
		if (hBuffer) {
			::GlobalFree(hBuffer);
			hBuffer = 0;
		}
	}

	// -----------------------------------------------------
	// �t�@�C�����Q��
	// -----------------------------------------------------

	/**
	 * @return �f�B���N�g�����ǂ���
	 */
	bool isDirectory() const {
		return pDirectory != NULL;
	}
	
	/**
	 * �t�@�C�����f�[�^�Ƃ��ĕԂ�
	 * @return octet�f�[�^
	 */
	tTJSVariant getData() const {
		tTJSVariant ret;
		if (hBuffer) {
			unsigned char* pBuffer = (unsigned char*)::GlobalLock(hBuffer);
			if (pBuffer) {
				ret = tTJSVariant(pBuffer, GlobalSize(hBuffer));
				::GlobalUnlock(hBuffer);
			}
		}
		return ret;
	}

	/**
	 * @return �t�@�C���T�C�Y
	 */
	DWORD getSize() const {
		return hBuffer ? GlobalSize(hBuffer) : 0;
	}
	
	/**
	 * �t�@�C������Ԃ�
	 * @return �t�@�C����� %[name:���O, size:�T�C�Y, isDirectory:�f�B���N�g���Ȃ�true]
	 */
	tTJSVariant getInfo() const {
		iTJSDispatch2 *dict = TJSCreateDictionaryObject();
		if (dict != NULL) {
			tTJSVariant name(this->name);
			tTJSVariant size((tjs_int64)getSize());
			tTJSVariant isDirectory(isDirectory() ? 1 : 0);
			dict->PropSet(TJS_MEMBERENSURE, L"name",  NULL, &name, dict);
			dict->PropSet(TJS_MEMBERENSURE, L"size",  NULL, &size, dict);
			dict->PropSet(TJS_MEMBERENSURE, L"isDirectory",  NULL, &isDirectory, dict);
			tTJSVariant ret(dict, dict);
			dict->Release();
			return ret;
		}
		return tTJSVariant();
	}

	// -----------------------------------------------------
	// �f�B���N�g�����
	// -----------------------------------------------------

	/**
	 * @return �f�B���N�g���G���g���̐�
	 */
	int getDirectoryCount() const {
		return pDirectory ? pDirectory->size() : 0;
	}
	
	/**
	 * �f�B���N�g�����擾
	 * @param lister �i�[��
	 */
	void getDirectory(iTVPStorageLister * lister) const {
		if (pDirectory) {
			Directory::const_iterator it = pDirectory->begin();
			while (it != pDirectory->end()) {
				if (!it->second->isDirectory()) {
					lister->Add(it->first);
				}
				it++;
			}
		}
	}

	/**
	 * �f�B���N�g�����擾
	 * @param array �i�[��
	 */
	void getDirectory(iTJSDispatch2 *array) const {
		if (pDirectory) {
			Directory::const_iterator it = pDirectory->begin();
			while (it != pDirectory->end()) {
				tTJSVariant result = it->second->getInfo(), *param = &result;
				array->FuncCall(0, TJS_W("add"), NULL, 0, 1, &param, array);
				it++;
			}
		}
	}

	// -----------------------------------------------------
	// �t�@�C������
	// -----------------------------------------------------

	/**
	 * �t�@�C���܂��̓t�@�C����T��
	 * @param name �t�@�C����
	 * @return �������ꂽ�t�@�C�����
	 */
	const FileInfo *find(const ttstr &name) const {
		if (pDirectory) {
			const tjs_char *p = name.c_str();
			const tjs_char *q;
			if ((q = wcschr(p, '/'))) {
				if (q == p) {
					return NULL;
				}
				Directory::const_iterator it = pDirectory->find(ttstr(p, q-p));
				if (it != pDirectory->end()) {
					FileInfo *info = it->second;
					q++;
					if (*q) {
						if (info->isDirectory()) {
							return info->find(ttstr(q));
						}
					} else {
						return info;
					}
				}
			} else {
				Directory::const_iterator it = pDirectory->find(name);
				if (it != pDirectory->end()) {
					return it->second;
				}
			}
		}
		return NULL;
	}

	/**
	 * �t�@�C����T��
	 * @param name �t�@�C����
	 * @return �������ꂽ�t�@�C�����
	 */
	const FileInfo *findFile(const ttstr &name) const {
		if (name.GetLastChar() != '/') {
			const FileInfo *file = find(name);
			if (file && !file->isDirectory()) {
				return file;
			}
		}
		return NULL;
	}
	
	/**
	 * �f�B���N�g����T��
	 * @param name �t�@�C����
	 * @return �������ꂽ�t�@�C�����
	 */
	const FileInfo *findDirectory(const ttstr &name) const {
		const FileInfo *file = find(name);
		return file && file->isDirectory() ? file : NULL;
	}

	// -----------------------------------------------------
	// �f�B���N�g������
	// -----------------------------------------------------

	/**
	 * �t�@�C�����J��
	 * @param name ���΃p�X�ł̃t�@�C����
	 * @param flags �I�[�v���t���O
	 * @return �J�����X�g���[��
	 */
	tTJSBinaryStream *open(const ttstr &name, tjs_uint32 flags) {
		if (pDirectory && name != "" && name.GetLastChar() != '/') {
			const tjs_char *p = name.c_str();
			const tjs_char *q;
			if ((q = wcschr(p, '/'))) {
				if (q == p) {
					return NULL;
				}
				ttstr dirname(p, q-p);
				Directory::iterator it = pDirectory->find(dirname);
				FileInfo *dir = it != pDirectory->end() ? it->second : _mkdir(dirname);
				if (dir && dir->isDirectory()) {
					return dir->open(ttstr(q+1), flags);
				}
			} else {
				return _open(name, flags);
			}
		}
		return NULL;
	}
	
	/**
	 * �w�肳�ꂽ�f�B���N�g�����쐬
	 * @return �쐬�����G���g��
	 */
	const FileInfo *mkdir(const ttstr &name) {
		if (pDirectory && name != "") {
			const tjs_char *p = name.c_str();
			const tjs_char *q;
			if ((q = wcschr(p, '/'))) {
				if (q == p) {
					return NULL;
				}
				ttstr dirname(p, q-p);
				Directory::const_iterator it = pDirectory->find(dirname);
				FileInfo *dir = it != pDirectory->end() ? it->second : _mkdir(dirname);
				if (dir && dir->isDirectory()) {
					return dir->mkdir(ttstr(q+1));
				}
			} else {
				return _mkdir(name);
			}
		}
		return false;
	}

	/**
	 * �w�肳�ꂽ�t�@�C�����폜����
	 * @param name �t�@�C����
	 * @return �폜������ true
	 */
	bool remove(const ttstr &name) {
		const FileInfo *file = findFile(name);
		if (file) {
			file = file->pParent->_remove(name);
			if (file) {
				delete file;
				return true;
			}
		}
		return false;
	}

	/**
	 * �w�肳�ꂽ�f�B���N�g�����폜����
	 * @param name �f�B���N�g����
	 * @return �폜������ true
	 */
	bool rmdir(const ttstr &name) {
		const FileInfo *dir = findDirectory(name);
		if (dir && dir->getDirectoryCount() == 0) {
			dir = dir->pParent->_remove(name);
			if (dir) {
				delete dir;
				return true;
			}
		}
		return false;
	}

protected:

	// -----------------------------------------------------
	// �t�@�C���p����
	// -----------------------------------------------------
	
	/**
	 * @return �t�@�C���X�g���[����Ԃ�
	 */
	IStream *getStream() {
		if (!pDirectory) {
			if (hBuffer == 0) {
				hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, 0);
			}
			if (hBuffer) {
				IStream *pStream;
				::CreateStreamOnHGlobal(hBuffer, FALSE, &pStream);
				return pStream;
			}
		}
		return NULL;
	}

	// -----------------------------------------------------
	// �f�B���N�g���p����
	// -----------------------------------------------------
	
	/**
	 * �t�@�C�����J��
	 * @param name �t�@�C����
	 * @param flags 
	 * @return �X�g���[��
	 */
	tTJSBinaryStream *_open(const ttstr &name, tjs_uint32 flags) {
		if (pDirectory) {
			IStream *stream = NULL;
			Directory::const_iterator it = pDirectory->find(name);
			if (it != pDirectory->end()) {
				stream = it->second->getStream();
			} else {
				if (flags == TJS_BS_WRITE) {
					FileInfo *file = new FileInfo(this, name);
					if (file) {
						(*pDirectory)[name] = file;
						stream = file->getStream();
					}
				}
			}
			if (stream) {
				if (flags == TJS_BS_APPEND) {
					LARGE_INTEGER n;
					n.QuadPart = 0;
					stream->Seek(n, STREAM_SEEK_END, NULL);
				}
				tTJSBinaryStream *bstream =TVPCreateBinaryStreamAdapter(stream);
				stream->Release();
				return bstream;
			}
		}
		return NULL;
	}

	/**
	 * �f�B���N�g���G���g���̍쐬
	 * @param name �t�@�C����
	 * @return ���������炻�̃G���g��
	 */
	FileInfo *_mkdir(const ttstr &name) {
		FileInfo *info = NULL;
		if (pDirectory) {
			Directory::const_iterator it = pDirectory->find(name);
			if (it == pDirectory->end()) {
				info = new FileInfo(this, name, true);
				(*pDirectory)[name] = info;
			}
		}
		return info;
	}

	/**
	 * �t�@�C�����̍폜
	 * @param name ���O
	 */
	const FileInfo *_remove(const ttstr &name) {
		FileInfo *info = NULL;
		if (pDirectory) {
			Directory::iterator it = pDirectory->find(name);
			if (it != pDirectory->end()) {
				info = it->second;
				pDirectory->erase(it);
			}
		}
		return info;
	}
	
private:
    FileInfo *pParent;
    ttstr name;
	HGLOBAL hBuffer;
	Directory *pDirectory;
};

class MemStorage : public iTVPStorageMedia
{

public:
	MemStorage() : refCount(1), root(NULL, ttstr("root"), true) {}

	~MemStorage() {;}

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

	void getFilename(const ttstr &name, ttstr &dname, ttstr &fname) {
		const tjs_char *p = name.c_str();
		const tjs_char *q;
		if ((q = wcschr(p, '/'))) {
			dname = ttstr(p, q-p);
			fname = ttstr(q+1);
			if (dname != L".") {
				TVPThrowExceptionMessage(TJS_W("no such domain:%1"), dname);
			}
		} else {
			TVPThrowExceptionMessage(TJS_W("invalid path:%1"), name);
		}
	}
	
	// check file existence
	virtual bool TJS_INTF_METHOD CheckExistentStorage(const ttstr &name) {
		ttstr dname, fname;
		getFilename(name, dname, fname);
		return root.findFile(fname) != NULL;
	}

	// open a storage and return a tTJSBinaryStream instance.
	// name does not contain in-archive storage name but
	// is normalized.
	virtual tTJSBinaryStream * TJS_INTF_METHOD Open(const ttstr & name, tjs_uint32 flags) {
		ttstr dname, fname;
		getFilename(name, dname, fname);
		tTJSBinaryStream *ret = root.open(fname, flags);
		if (!ret) {
			TVPThrowExceptionMessage(TJS_W("cannot open memfile:%1"), name);
		}
		return ret;
	}

	// list files at given place
	virtual void TJS_INTF_METHOD GetListAt(const ttstr &name, iTVPStorageLister * lister) {
		ttstr dname, fname;
		getFilename(name, dname, fname);
		const FileInfo *directory = root.findDirectory(fname);
		if (directory) {
			directory->getDirectory(lister);
		}
	}

	// basically the same as above,
	// check wether given name is easily accessible from local OS filesystem.
	// if true, returns local OS native name. otherwise returns an empty string.
	virtual void TJS_INTF_METHOD GetLocallyAccessibleName(ttstr &name) {
		name = "";
	}

public:

	/**
	 * �w�肳�ꂽ�f�B���N�g�����쐬
	 * @return �쐬������ true
	 */
	bool mkdir(const ttstr &name) {
		return root.mkdir(name) != NULL;
	}
	
	/**
	 * �w�肳�ꂽ�t�@�C���̑��݊m�F
	 * @param name �t�@�C����
	 * @return ���݂����� true
	 */
	bool isExistFile(const ttstr &name) {
		return root.findFile(name) != NULL;
	}

	/**
	 * �w�肳�ꂽ�t�@�C���̑��݊m�F
	 * @param name �t�@�C����
	 * @return ���݂����� true
	 */
	bool isExistDirectory(const ttstr &name) {
		return root.findDirectory(name) != NULL;
	}
	
	/**
	 * �w�肳�ꂽ�t�@�C�����폜����
	 * @param name �t�@�C����
	 * @return �폜������ true
	 */
	bool remove(const ttstr &name) {
		return root.remove(name);
	}

	/**
	 * �w�肳�ꂽ�f�B���N�g�����폜����
	 * @param name �f�B���N�g����
	 * @return �폜������ true
	 */
	bool rmdir(const ttstr &name) {
		return root.rmdir(name);
	}

	/**
	 * �w�肳�ꂽ�t�@�C���̓��e�� octet �ŕԂ�
	 * @param name �t�@�C����
	 * @return �t�@�C����� %[name:���O, size:�T�C�Y, isDirectory:�f�B���N�g���Ȃ�true]
	 */
	tTJSVariant getInfo(const ttstr &name) {
		const FileInfo *file = root.findFile(name);
		if (file) {
			return file->getInfo();
		}
		return tTJSVariant();
	}
	
	/**
	 * �w�肳�ꂽ�t�@�C���̓��e�� octet �ŕԂ�
	 * @param name �t�@�C����
	 * @return �f�[�^
	 */
	tTJSVariant getData(const ttstr &name) {
		const FileInfo *file = root.findFile(name);
		if (file) {
			return file->getData();
		}
		return tTJSVariant();
	}

	/**
	 * �w�肳�ꂽ�f�B���N�g���̃t�@�C�����ꗗ���擾
	 * @param name �f�B���N�g����
	 * @return �t�@�C�����̔z�� %[name:���O, size:�T�C�Y, isDirectory:�f�B���N�g���Ȃ�true]
	 */
	tTJSVariant getDirectory(const ttstr &name) {
		const FileInfo *dir = root.findDirectory(name);
		if (dir) {
			iTJSDispatch2 *array = TJSCreateArrayObject();
			if (array) {
				dir->getDirectory(array);
				tTJSVariant ret(array, array);
				array->Release();
				return ret;
			}
		}
		return tTJSVariant();
	}
	
private:
	tjs_uint refCount;
	FileInfo root;
};


/**
 * ���\�b�h�ǉ��p
 */
class StoragesMemFile {

public:
	
	static void initMemoryFile() {
		if (mem == NULL) {
			mem = new MemStorage();
			TVPRegisterStorageMedia(mem);
		}
	}

	static void doneMemoryFile() {
		if (mem != NULL) {
			TVPUnregisterStorageMedia(mem);
			mem->Release();
			mem = NULL;
		}
	}

	/**
	 * �������t�@�C���̑��݊m�F
	 * @param filename �Ώۃt�@�C���� (mem:///�͂��Ȃ�)
	 * @return ���݂����� true
	 */
	static bool isExistMemoryFile(ttstr filename) {
		return mem && mem->isExistFile(filename);
	}

	/**
	 * �������f�B���N�g���̑��݊m�F
	 * @param dirname �Ώۃf�B���N�g���� (mem:///�͂��Ȃ�)
	 * @return ���݂����� true
	 */
	static bool isExistMemoryDirectory(ttstr filename) {
		return mem && mem->isExistDirectory(filename);
	}
	
	/**
	 * �������t�@�C�����폜����
	 * @param filename �Ώۃt�@�C���� (mem:///�͂��Ȃ�)
	 * @return �t�@�C�����폜���ꂽ�� true
	 */
	static bool deleteMemoryFile(ttstr filename) {
		return mem && mem->remove(filename);
	}

	/**
	 * �������f�B���N�g�����폜����
	 * @param dirname �Ώۃf�B���N�g���� (mem:///�͂��Ȃ�)
	 * @return �f�B���N�g�����폜���ꂽ�� true
	 */
	static bool deleteMemoryDirectory(ttstr dirname) {
		return mem && mem->rmdir(dirname);
	}

	/**
	 * �������t�@�C�������擾����
	 * @param filename �Ώۃt�@�C���� (mem:///�͂��Ȃ�)
	 * @return �t�@�C����� %[name:���O, size:�T�C�Y, isDirectory:�f�B���N�g���Ȃ�true]
	 */
	static tTJSVariant getMemoryFileInfo(ttstr filename) {
		return mem ? mem->getInfo(filename) : NULL;
	}
	
	/**
	 * �������t�@�C�������擾����
	 * @param filename �Ώۃt�@�C���� (mem:///�͂��Ȃ�)
	 * @return �t�@�C�������݂�������e�� octet �ŕԂ��B�Ȃ���� void
	 */
	static tTJSVariant getMemoryFileData(ttstr filename) {
		return mem ? mem->getData(filename) : NULL;
	}

	/**
	 * �������f�B���N�g�������擾����
	 * @param dirname �Ώۃf�B���N�g���� (mem:///�͂��Ȃ�)
	 * @return �t�@�C�����̔z�� %[name:���O, size:�T�C�Y, isDirectory:�f�B���N�g���Ȃ�true]
	 */
	static tTJSVariant getMemoryDirectory(ttstr dirname) {
		return mem ? mem->getDirectory(dirname) : NULL;
	}

protected:
	static MemStorage *mem;
};

MemStorage *StoragesMemFile::mem = NULL;

NCB_ATTACH_CLASS(StoragesMemFile, Storages) {
	NCB_METHOD(isExistMemoryFile);
	NCB_METHOD(isExistMemoryDirectory);
	NCB_METHOD(deleteMemoryFile);
	NCB_METHOD(deleteMemoryDirectory);
	NCB_METHOD(getMemoryFileInfo);
	NCB_METHOD(getMemoryFileData);
	NCB_METHOD(getMemoryDirectory);
};

/**
 * �J��������
 */
static void PreRegistCallback()
{
	StoragesMemFile::initMemoryFile();
}


/**
 * �J��������
 */
static void PostUnregistCallback()
{
	StoragesMemFile::doneMemoryFile();
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
