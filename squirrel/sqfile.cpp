#include <windows.h>
#include <squirrel.h>
#include "tp_stub.h"

/**
 * �t�@�C���ǂݍ��ݗp�f�[�^�\��
 */
class SQFileInfo {

public:
	/// �R���X�g���N�^
	SQFileInfo(const SQChar *filename, bool binary) : is(NULL), buffer(NULL), size(0), readed(0), binary(binary) {
		is = TVPCreateIStream(filename, TJS_BS_READ);
		if (is) {
			if (binary) {
				STATSTG stat;
				is->Stat(&stat, STATFLAG_NONAME);
				size = (ULONG)stat.cbSize.QuadPart;
				buffer = new char[size];
			} else {
				DWORD len;
				unsigned short us = 0;
				is->Read(&us, 2, &len);
				if (us == SQ_BYTECODE_STREAM_TAG) { //BYTECODE
					LARGE_INTEGER move = {0};
					is->Seek(move,STREAM_SEEK_SET,NULL);
					STATSTG stat;
					is->Stat(&stat, STATFLAG_NONAME);
					size = (ULONG)stat.cbSize.QuadPart;
					buffer = new char[size];
				} else {
					is->Release();
					is = NULL;
					ttstr data;
					iTJSTextReadStream *rs = TVPCreateTextStreamForRead(filename, L"");
					rs->Read(data, 0);
					rs->Destruct();
					readed = size = data.length() * sizeof tjs_char + 2;
					buffer = new char [size];
					memcpy(buffer, "\xFF\xFE", 2); // Little Endian BOM
					memcpy(buffer+2, (void*)data.c_str(), data.length()*sizeof tjs_char); // converted string
				}
			}
		}
	}

	/// �f�X�g���N�^
	~SQFileInfo() {
		if (buffer) {
			delete [] buffer;
		}
		if (is) {
			is->Release();
		}
	}

	/// @return �ǂݍ��݊��������� true
	bool check() {
		if (buffer) {
			if (readed < size) {
				ULONG len;
				if (is->Read(buffer+readed,size-readed,&len) == S_OK) {
					readed += len;
				}
			}
			return readed >= size;
		} else {
			return true;
		}
	}

	/// @return �o�b�t�@
	const char *getBuffer() {
		return (const char*)buffer;
	}

	/// @return �T�C�Y
	int getSize() {
		return (int)size;
	}

private:
	IStream *is;  ///< ���̓X�g���[��
	char *buffer; ///< ���̓f�[�^�̃o�b�t�@
	ULONG size;   ///< �ǂݍ��݃T�C�Y
	ULONG readed; ///< �ǂݍ��ݍς݃T�C�Y
	bool binary;
};

/**
 * �t�@�C����񓯊��ɊJ��
 * @param filename �X�N���v�g�t�@�C����
 * @return �t�@�C���n���h��
 */
void *sqobjOpenFile(const SQChar *filename, bool binary)
{
	return (void*) new SQFileInfo(filename, binary);
}

/**
 * �t�@�C�����J���ꂽ���ǂ����̃`�F�b�N
 * @param handler �t�@�C���n���h��
 * @param dataPtr �f�[�^�i�[��A�h���X(�o��)
 * @param dataSize �f�[�^�T�C�Y(�o��)
 * @return ���[�h�������Ă����� true
 */
bool sqobjCheckFile(void *handler, const char **dataAddr, int *dataSize)
{
	SQFileInfo *file = (SQFileInfo*)handler;
	if (file) {
		if (file->check()) {
			*dataAddr = file->getBuffer();
			*dataSize = file->getSize();
			return true;
		} else {
			return false;
		}
	} else {
		*dataAddr = NULL;
		*dataSize = 0;
		return true;
	}
}

/**
 * �t�@�C�������
 * @param handler �t�@�C���n���h��
 */
void sqobjCloseFile(void *handler)
{
	SQFileInfo *file = (SQFileInfo*)handler;
	if (file) {
		delete file;
	}
}
