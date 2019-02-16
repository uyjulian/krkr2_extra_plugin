/*
 * zip/unzip plugin
 * Copyright (C) 2007 Go Watanabe
 * 
 * original copyright
 *  minizip.c
 *  miniunz.c
 *  Version 1.01e, February 12th, 2005
 * Copyright (C) 1998-2005 Gilles Vollant
*/

static const char *copyright = 
"\n----- MiniZip Copyright START -----\n"
"MiniZip/MiniUnz 1.01b, demo of zLib + Zip package written by Gilles Vollant\n"
"more info at http://www.winimage.com/zLibDll/minizip.html\n"
"----- MiniZip Copyright END -----\n";

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <direct.h>
#include <io.h>

#include "ncbind/ncbind.hpp"
#include "crypt.h"
#include "unzip.h"
#include "zip.h"

#include "narrow.h"

#define BUFFERSIZE (16384)
#define CASESENSITIVITY (0)
#define MAXFILENAME (256)

// UTF8�ȃt�@�C�������ǂ����̃t���O
#define FLAG_UTF8 (1<<11)

// �t�@�C���A�N�Z�X�p
extern zlib_filefunc64_def TVPZlibFileFunc;

// Date �N���X�����o
static iTJSDispatch2 *dateClass = NULL;    // Date �̃N���X�I�u�W�F�N�g
static iTJSDispatch2 *dateSetTime = NULL;  // Date.setTime ���\�b�h

/**
 * int �Ŏ�������v�f���擾
 */
static int
getIntProp(tTJSVariant &options, const tjs_char *name, int defaultValue)
{
	if (options.Type() == tvtObject) {
		iTJSDispatch2 *obj = options.AsObjectNoAddRef();
		tTJSVariant var;
		if (obj->PropGet(0, name, NULL, &var, obj) == TJS_S_OK) {
			return (tjs_int)var;
		}
	}
	return defaultValue;
}

/**
 * int �Ŕz�񂩂�v�f���擾
 */
static int
getIntProp(tTJSVariant &options, int num, int defaultValue)
{
	if (options.Type() == tvtObject) {
		iTJSDispatch2 *obj = options.AsObjectNoAddRef();
		tTJSVariant var;
		if (obj->PropGetByNum(0, num, &var, obj) == TJS_S_OK) {
			return (tjs_int)var;
		}
	}
	return defaultValue;
}

/**
 * ������Ŏ�������v�f���擾
 */
static ttstr
getStrProp(tTJSVariant &options, const tjs_char *name, ttstr &defaultValue)
{
	if (options.Type() == tvtObject) {
		iTJSDispatch2 *obj = options.AsObjectNoAddRef();
		tTJSVariant var;
		if (obj->PropGet(0, name, NULL, &var, obj) == TJS_S_OK) {
			return (ttstr)var;
		}
	}
	return defaultValue;
}

/**
 * ������Ŕz�񂩂�v�f���擾
 */
static ttstr
getStrProp(tTJSVariant &options, int num, ttstr &defaultValue)
{
	if (options.Type() == tvtObject) {
		iTJSDispatch2 *obj = options.AsObjectNoAddRef();
		tTJSVariant var;
		if (obj->PropGetByNum(0, num, &var, obj) == TJS_S_OK) {
			return (ttstr)var;
		}
	}
	return defaultValue;
}

// �I�u�W�F�N�g�ɐ��l���i�[
static void setIntProp(iTJSDispatch2 *obj, const tjs_char *name, int value)
{
	tTJSVariant var = value;
	obj->PropSet(TJS_MEMBERENSURE, name,  NULL, &var, obj);
}

// �I�u�W�F�N�g�ɕ�������i�[
static void setStrProp(iTJSDispatch2 *obj, const tjs_char *name, ttstr &value)
{
	tTJSVariant var = value;
	obj->PropSet(TJS_MEMBERENSURE, name,  NULL, &var, obj);
}

// �I�u�W�F�N�g�ɓ������i�[
static void setDateProp(iTJSDispatch2 *obj, const tjs_char *name, FILETIME &filetime)
{
	// �t�@�C��������
	tjs_uint64 ft = filetime.dwHighDateTime * 0x100000000 | filetime.dwLowDateTime;
	if (ft > 0) {
		iTJSDispatch2 *date;
		if (TJS_SUCCEEDED(dateClass->CreateNew(0, NULL, NULL, &date, 0, NULL, obj))) {
			// UNIX TIME �ɕϊ�
			tjs_int64 unixtime = (ft - 0x19DB1DED53E8000 ) / 10000;
			tTJSVariant time(unixtime);
			tTJSVariant *param[] = { &time };
			dateSetTime->FuncCall(0, NULL, NULL, NULL, 1, param, date);

			tTJSVariant var = date;
			date->Release();
			obj->PropSet(TJS_MEMBERENSURE, name, NULL, &var, obj);
		}
	}
}

/**
 * ZIP���k�����N���X
 */
class Zip {

protected:
	zipFile zf;
	
public:

	/**
	 * �R���X�g���N�^
	 */
	Zip() : zf(NULL) {
	}

	/**
	 * �f�X�g���N�^
	 */
	~Zip(){
		close();
	}

public:

	/**
	 * ZIP�t�@�C�����J��
	 * @param filename �t�@�C����
	 * @param overwrite �㏑���w�� 1:�㏑�� 2:�ǉ�
	 */
	static tjs_error TJS_INTF_METHOD open(tTJSVariant *result,
										  tjs_int numparams,
										  tTJSVariant **param,
										  Zip *self) {

		if (numparams < 1) return TJS_E_BADPARAMCOUNT;
		
		ttstr filename        = *param[0];
		int overwrite = numparams > 1 ? (int)*param[1] : 0;
		
		if (overwrite == 2) { // �ǋL
			ttstr path = TVPGetPlacedPath(filename);
			if (!path.length()) {
				overwrite = 1;
			} 
		} else if (overwrite == 0) {
			ttstr path = TVPGetPlacedPath(filename);
			if (path.length()) {
				// ���ɑ��݂��Ă���
				ttstr msg = filename + " exists.";
				TVPThrowExceptionMessage(msg.c_str());
			}
		}

		if ((self->zf = zipOpen2_64((const void*)filename.c_str(), (overwrite==2) ? 2 : 0, NULL, &TVPZlibFileFunc)) == NULL) {
			// �I�[�v�����s
			ttstr msg = filename + " can't open.";
			TVPThrowExceptionMessage(msg.c_str());
		}

		return TJS_S_OK;
	}

	/**
	 * �t�@�C�������
	 */
	void close() {
		if (zf) {
			zipClose(zf,NULL);
			zf = NULL;
		}
	}
	
	/**
	 * �t�@�C���̒ǉ�
	 * @param srcname  �ǉ�����t�@�C��
	 * @param destname �o�^���i�p�X���܂ށj
	 * @param compressLevel ���k���x��
	 * @param password �p�X���[�h�w��
	 * @return �ǉ��ɐ��������� true
	 */
	static tjs_error TJS_INTF_METHOD add(tTJSVariant *result,
										 tjs_int numparams,
										 tTJSVariant **param,
										 Zip *self) {

		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		if (!self->zf) {
			TVPThrowExceptionMessage(L"don't open zipfile");
		}			
		
		ttstr srcname  = *param[0];
		ttstr destname = *param[1];
		int   compressLevel = numparams > 2 ? (int)*param[2] : Z_DEFAULT_COMPRESSION;
		bool usePassword = false;
		ttstr password;
		if (numparams > 3 && param[3]->Type() == tvtString) {
			usePassword = true;
			password = *param[3];
		}

		// �t�@�C����
		ttstr filename = TVPGetPlacedPath(srcname);
		if (filename.length() == 0) {
			ttstr msg = srcname + " not exists.";
			TVPThrowExceptionMessage(msg.c_str());
		}
		
		// �t�@�C���������擾
		zip_fileinfo zi;
		memset(&zi, 0, sizeof zi);
		{
			SYSTEMTIME time;
			GetLocalTime(&time); // ���ݎ���
			ttstr name(TVPGetLocallyAccessibleName(filename));
			if (name.length() > 0) {
				// ���t�@�C�������݂���ꍇ�͎����𔲂��Ă���
				HANDLE hFile;
				if ((hFile = CreateFileW(name.c_str(), GENERIC_READ, 0, NULL ,
										 OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL)) != INVALID_HANDLE_VALUE) {
					FILETIME c, a, m;
					if (GetFileTime(hFile , &c, &a, &m)) {
						FILETIME local;
						FileTimeToLocalFileTime(&m, &local);
						FileTimeToSystemTime(&local, &time);
					}
					CloseHandle(hFile);
				}
			}
			zi.tmz_date.tm_year = time.wYear;
			zi.tmz_date.tm_mon  = time.wMonth - 1;
			zi.tmz_date.tm_mday = time.wDay;
			zi.tmz_date.tm_hour = time.wHour;
			zi.tmz_date.tm_min  = time.wMinute;
			zi.tmz_date.tm_sec  = time.wSecond;
		}

		bool ret;
		
		IStream *in = TVPCreateIStream(filename, TJS_BS_READ);
		if (in) {
			
			// CRC�v�Z
			unsigned long crcFile=0;
			if (usePassword) {
				char buf[BUFFERSIZE];
				DWORD size;
				while (in->Read(buf, sizeof buf, &size) == S_OK && size > 0) {
					crcFile = crc32(crcFile, (const Bytef *)buf, size);
				}
				// �ʒu�����ǂ�
				LARGE_INTEGER move = {0};
				ULARGE_INTEGER newposition;
				in->Seek(move, STREAM_SEEK_CUR, &newposition);
			}
			// �t�@�C���̒ǉ�
			// UTF8�Ŋi�[����
			if (zipOpenNewFileInZip4(self->zf, NarrowString(destname, true), &zi,
									 NULL,0,NULL,0,NULL /* comment*/,
									 (compressLevel != 0) ? Z_DEFLATED : 0,
									 compressLevel, 0,
									 -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
									 usePassword ? (const char*)NarrowString(password) : NULL,
									 crcFile, 0, FLAG_UTF8) == ZIP_OK) {
				char buf[BUFFERSIZE];
				DWORD size;
				while (in->Read(buf, sizeof buf, &size) == S_OK && size > 0) {
					zipWriteInFileInZip (self->zf, buf, size);
				}
				zipCloseFileInZip(self->zf);
				ret = true;
			} else {
				ret = false;
			}
			in->Release();
		}

		if (result) {
			*result = ret;
		}

		return TJS_S_OK;
	}
};

#include <vector>

// �t�@�C�����ϊ�����
void
storeFilename(ttstr &name, const char *narrowName, bool utf8)
{
	if (utf8) {
		int	len = ::MultiByteToWideChar(CP_UTF8, 0, narrowName, -1, NULL, 0);
		if (len > 0) {
			std::vector<tjs_char> outbuf( len+1, 0 );
			int ret = ::MultiByteToWideChar(CP_UTF8, 0, narrowName, -1, &(outbuf[0]), len);
			if (ret) {
				outbuf[ret] = '\0';
				name = &(outbuf[0]);
			}
		}
	} else {
		name = narrowName;
	}
}

/**
 * Zip �W�J�N���X
 */
class Unzip {

protected:
	unzFile uf;
	bool utf8;
	
public:
	Unzip() : uf(NULL), utf8(false) {
	}
	
	~Unzip() {
		close();
	}

	/**
	 * ZIP�t�@�C�����J��
	 * @param filename �t�@�C����
	 */
	void open(const tjs_char *filename) {
		if ((uf = unzOpen2_64((const void*)filename, &TVPZlibFileFunc)) == NULL) {
			ttstr msg = filename;
			msg += L" can't open.";
			TVPThrowExceptionMessage(msg.c_str());
		}
		// UTF8�ȃt�@�C�������ǂ����̔���B�ŏ��̃t�@�C���Ō��߂�
		unzGoToFirstFile(uf);
		unz_file_info file_info;
		if (unzGetCurrentFileInfo(uf,&file_info, NULL,0,NULL,0,NULL,0) == UNZ_OK) {
			utf8 = (file_info.flag & FLAG_UTF8) != 0;
		}
	}

	/**
	 * ZIP �t�@�C�������
	 */
	void close() {
		if (uf) {
			unzClose(uf);
			uf = NULL;
		}
	}

	/**
	 * �t�@�C�����X�g�擾
	 * @return �t�@�C�����i�����j�̔z��
	 */
	static tjs_error TJS_INTF_METHOD list(tTJSVariant *result,
										  tjs_int numparams,
										  tTJSVariant **param,
										  Unzip *self) {

		if (!self->uf) {
			TVPThrowExceptionMessage(L"don't open zipfile");
		}

		iTJSDispatch2 *array = TJSCreateArrayObject();

		unzGoToFirstFile(self->uf);
		do {
			char filename_inzip[1024];
			unz_file_info file_info;
			
			if (unzGetCurrentFileInfo(self->uf, &file_info, filename_inzip, sizeof(filename_inzip),NULL,0,NULL,0) == UNZ_OK) {
				
				iTJSDispatch2 *obj;
				if ((obj = TJSCreateDictionaryObject()) != NULL) {
					
					ttstr filename;
					storeFilename(filename, filename_inzip, self->utf8);
					
					setStrProp(obj, L"filename", filename);
					setIntProp(obj, L"uncompressed_size", file_info.uncompressed_size);
					setIntProp(obj, L"compressed_size", file_info.compressed_size);
					setIntProp(obj, L"crypted", (file_info.flag & 1) ? 1 : 0);
					setIntProp(obj, L"deflated", (file_info.compression_method==Z_DEFLATED) ? 1 : 0);
					setIntProp(obj, L"deflateLevel", (file_info.flag & 0x6)/2);
					setIntProp(obj, L"crc", file_info.crc);

					// ���t���
					FILETIME date;
					{
						SYSTEMTIME time;
						memset(&time, 0, sizeof time);
						time.wYear   = file_info.tmu_date.tm_year;
						time.wMonth  = file_info.tmu_date.tm_mon + 1;
						time.wDay    = file_info.tmu_date.tm_mday;
						time.wHour   = file_info.tmu_date.tm_hour;
						time.wMinute = file_info.tmu_date.tm_min;
						time.wSecond = file_info.tmu_date.tm_sec;
						FILETIME filetime;
						SystemTimeToFileTime(&time, &filetime);
						LocalFileTimeToFileTime(&filetime, &date);
					}
					setDateProp(obj, L"date", date);

					tTJSVariant var(obj), *param = &var;
					array->FuncCall(0, TJS_W("add"), NULL, 0, 1, &param, array);
					obj->Release();
				}
			}
		} while (unzGoToNextFile(self->uf) == UNZ_OK);

		if (result) {
			tTJSVariant ret(array,array);
			*result = ret;
		}
		array->Release();

		return TJS_S_OK;
	}
	
	/**
	 * �t�@�C���̓W�J
	 * @param srcname �W�J���t�@�C��
	 * @param destfile �W�J��t�@�C��
	 * @param password �p�X���[�h�w��
	 */
	static tjs_error TJS_INTF_METHOD extract(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 Unzip *self) {
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		if (!self->uf) {
			TVPThrowExceptionMessage(L"don't open zipfile");
		}			
		ttstr srcname  = *param[0];
		ttstr destname = *param[1];
		bool usePassword = false;
		ttstr password;
		if (numparams > 2 && param[2]->Type() == tvtString) {
			usePassword = true;
			password = *param[2];
		}

		bool ret;
		
		if (unzLocateFile(self->uf, NarrowString(srcname, self->utf8), CASESENSITIVITY) == UNZ_OK) {
			int result = usePassword ? unzOpenCurrentFilePassword(self->uf,NarrowString(password))
				: unzOpenCurrentFile(self->uf);
			if (result == UNZ_OK) {
				IStream *out = TVPCreateIStream(destname, TJS_BS_WRITE);
				if (out) {
					char buf[BUFFERSIZE];
					DWORD size;
					while ((size = unzReadCurrentFile(self->uf,buf,sizeof buf)) > 0) {
						out->Write(buf, size, &size);
					}
					out->Release();
				} else {
					unzCloseCurrentFile(self->uf);
					ttstr msg = destname + " can't open.";
					TVPThrowExceptionMessage(msg.c_str());
				}
				unzCloseCurrentFile(self->uf);
				ret = true;
			} else {
				ret = false;
			}
		} else {
			ret = false;
		}

		if (result) {
			*result = ret;
		}

		return TJS_S_OK;
	}

};

NCB_REGISTER_CLASS(Zip) {
	Constructor();
	RawCallback("open", &ClassT::open, 0);
	NCB_METHOD(close);
	RawCallback("add", &ClassT::add, 0);
}

NCB_REGISTER_CLASS(Unzip) {
	Constructor();
	NCB_METHOD(open);
	NCB_METHOD(close);
	RawCallback("list", &ClassT::list, 0);
	RawCallback("extract", &ClassT::extract, 0);
}

extern void initZipStorage();
extern void doneZipStorage();

/**
 * �o�^�����O
 */
static void PreRegistCallback()
{
	TVPAddImportantLog(ttstr(copyright));
	initZipStorage();
}

/**
 * �o�^������
 */
static void PostRegistCallback()
{
	tTJSVariant var;
	TVPExecuteExpression(TJS_W("Date"), &var);
	dateClass = var.AsObject();
	TVPExecuteExpression(TJS_W("Date.setTime"), &var);
	dateSetTime = var.AsObject();
}

#define RELEASE(name) name->Release();name= NULL

/**
 * �J�������O
 */
static void PreUnregistCallback()
{
	RELEASE(dateClass);
	RELEASE(dateSetTime);
}

/**
 * �J��������
 */
static void PostUnregistCallback()
{
	doneZipStorage();
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_REGIST_CALLBACK(PostRegistCallback);
NCB_PRE_UNREGIST_CALLBACK(PreUnregistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
