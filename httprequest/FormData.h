#ifndef __FORMDATA_H__
#define __FORMDATA_H__

#include <vector>

/**
 * application/x-www-form-urlencoded �p�̃f�[�^�𐶐�����
 * ���o�^���ꂽ�f�[�^�� UTF-8 �ŃG���R�[�f�B���O
 */
class FormData {

public:
	/**
	 * �R���X�g���N�^
	 */
	FormData() : hasParam(false) {}

	/**
	 * key �� value �̑g�� URL-encode ���Ēǉ�
	 * @param key �L�[
	 * @param value �o�^����l
	 */
	void addParam(const TCHAR *key, const TCHAR *value) {
		if (hasParam) {
			data.push_back('&');
		} 
		addEncodedString(key);
		data.push_back('=');
		addEncodedString(value);
		hasParam = true;
	}

	/**
	 * key �� value �̑g�� URL-encode ���Ēǉ�
	 * @param key �L�[
	 * @param num �o�^����l�i���l�j
	 */
	void addParam(const TCHAR *key, int num) {
		TCHAR value[100];
		_sntprintf_s(value, sizeof value, _T("%d"), num);
		addParam(key, value);
	}

	/**
	 * key �� value �̑g�� URL-encode ���Ēǉ�
	 * @param key �L�[
	 * @param value �o�^����l�iBOOL)
	 */
	void addParam(const TCHAR *key, bool value) {
		addParam(key, value ? _T("1") : _T("0"));
	}

	// HTTP �ő���f�[�^���擾
	const BYTE *getData() const {
		if (data.size() <= 0) {
			return NULL;
		} else {
			return &data[0];
		}
	}

	// URL �G���R�[�h������������f�[�^�Ƃ��ĒǋL����
	void _addEncodedString(const char *p) {
		int ch;
		while ((ch = *p++)) {
			if (ch >= '0' && ch <= '9' ||
				ch >= 'a' && ch <= 'z' ||
				ch >= 'A' && ch <= 'Z') {
				data.push_back(ch);
			} else {
				data.push_back('%');
				data.push_back("0123456789ABCDEF"[ch / 16]);
				data.push_back("0123456789ABCDEF"[ch % 16]);
			}
		}
	}
	
protected:
	// URL �G���R�[�h������������f�[�^�Ƃ��ĒǋL����
	void addEncodedString(const TCHAR *str) {
#ifdef _UNICODE
		// UTF-8 ������ɖ߂�
		int len = _tcslen(str);
		int mblen = ::WideCharToMultiByte(CP_UTF8, 0, str, len, NULL, 0, NULL, NULL);
		char *buf = new char[mblen + 1];
		::WideCharToMultiByte(CP_UTF8, 0, str, len, buf, mblen, NULL, NULL);
		buf[mblen] = '\0';
		_addEncodedString(str);
		delete[] buf;
#else
		// ���̂܂܏���
		_addEncodedString(str);
#endif
	}
	
private:
	std::vector<BYTE>data;	    ///< put/post �̃f�[�^(GET�̃p�����[�^�����˂�)
	bool hasParam;	        ///< �p�����[�^(�f�[�^)���Z�b�g���ꂽ��Ԃ�
};

#endif
