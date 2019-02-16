#pragma comment(lib, "gdi32.lib")

#if defined(_WIN32_WINNT) && _WIN32_WINNT < 0x0500
#error "_WIN32_WINNT < 0x0500 : AddFontResourceEx undefined"
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <wingdi.h>
#include "ncbind/ncbind.hpp"

#include <vector>
using namespace std;

// �o�^�ς݃t�H���g�ꗗ
static vector<ttstr>  fontlist;
static vector<ttstr>  tempfontlist;
static vector<HANDLE> memfontlist;

struct FontEx
{
	/**
	 * �v���C�x�[�g�t�H���g�̒ǉ�
	 * @param fontFileName �t�H���g�t�@�C����
	 * @param extract �A�[�J�C�u����e���|�����W�J����
	 * @return void:�t�@�C�����J���̂Ɏ��s 0:�t�H���g�o�^�Ɏ��s ���l:�o�^�����t�H���g�̐�
	 */
	static tjs_error TJS_INTF_METHOD addFont(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis) {
		if (numparams < 1) return TJS_E_BADPARAMCOUNT;

		ttstr filename = TVPGetPlacedPath(*param[0]);
		if (filename.length()) {
			// ��TVPGetLocallyAccessibleName��rev.5221�ȍ~��tp_stub���K�v
			ttstr localname(TVPGetLocallyAccessibleName(filename));
			if (localname.length()) {
				// ���t�@�C�������݂����ꍇ
				int ret;
				if ((ret =  AddFontResourceEx(localname.c_str(), FR_PRIVATE, NULL)) > 0) {
					fontlist.push_back(localname);
				}
				if (result) {
					*result = ret;
				}
				return TJS_S_OK;
			} else {
				if (numparams > 1 && (int)*param[1]) {
					// �������Ƀ��[�h���ēW�J
					IStream *in = TVPCreateIStream(filename, TJS_BS_READ);
					if (in) {
						// �e���|�����t�@�C���쐬
						ttstr tempFile = TVPGetTemporaryName();
						HANDLE hFile;
						if ((hFile = CreateFile(tempFile.c_str(), GENERIC_WRITE, 0, NULL,
												CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL)) == INVALID_HANDLE_VALUE) {
							in->Release();
							return TJS_S_OK;
						}
						// �t�@�C�����R�s�[
						BYTE buffer[1024*16];
						DWORD size;
						while (in->Read(buffer, sizeof buffer, &size) == S_OK && size > 0) {			
							WriteFile(hFile, buffer, size, &size, NULL);
						}
						CloseHandle(hFile);
						in->Release();
						int ret;
						if ((ret =  AddFontResourceEx(tempFile.c_str(), FR_PRIVATE, NULL)) > 0) {
							tempfontlist.push_back(tempFile);
						} else {
							DeleteFile(tempFile.c_str());
						}
						if (result) {
							*result = ret;
						}
						return TJS_S_OK;
					}
				} else {
					// �������Ƀ��[�h���ēW�J
					IStream *in = TVPCreateIStream(filename, TJS_BS_READ);
					if (in) {
						STATSTG stat;
						in->Stat(&stat, STATFLAG_NONAME);
						DWORD ret = 0;
						// �T�C�Y���ӂꖳ�� XXX
						ULONG size = (ULONG)(stat.cbSize.QuadPart);
						char *data = new char[size];
						if (in->Read(data, size, &size) == S_OK) {
							HANDLE handle = AddFontMemResourceEx((void*)data, size, NULL, &ret);
							if (handle) {
								memfontlist.push_back(handle);
							}
						}
						delete[] data;
						if (result) {
							*result = (int)ret;
						}
						return TJS_S_OK;
					}
				}
			}
		}
		return TJS_S_OK;
	}
};

// �t�b�N���A�^�b�`
NCB_ATTACH_CLASS(FontEx, System) {
	RawCallback("addFont", &FontEx::addFont, TJS_STATICMEMBER);
}

// ----------------------------------- �N���E�J������

/**
 * �J��������
 */
void PostUnregistCallback()
{
	for (vector<ttstr>::iterator i = fontlist.begin(); i != fontlist.end(); i++) {
		RemoveFontResourceEx(i->c_str(), FR_PRIVATE, NULL);
	}
	for (vector<ttstr>::iterator i = tempfontlist.begin(); i != tempfontlist.end(); i++) {
		RemoveFontResourceEx(i->c_str(), FR_PRIVATE, NULL);
		DeleteFile(i->c_str());
	}
	for (vector<HANDLE>::iterator i = memfontlist.begin(); i != memfontlist.end(); i++) {
		RemoveFontMemResourceEx(*i);
	}
}

NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
