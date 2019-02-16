#include <vector>

#include "Base64.h"

/*
 * Basic �F�؂ŕK�v��Base64 �̃G���R�[�h�������������Ă��܂��B
 * �f�R�[�h�͌���K�v�Ȃ��̂Ŏ������Ă��܂���B
 */

using namespace std;

// Base64 �ϊ��R�[�h�e�[�u��
static TCHAR *code = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

#ifdef BASE64_TEST
#include <iostream>
#define TEST_STR "TESTTEST"
int _tmain(int argc, TCHAR * argv[]) {
	cout << base64encode((const unsigned TCHAR *)argv[1], tcslen(argv[1]));
	return 0;
}
#endif

inline void
encode_block(vector<TCHAR> &buf, const TCHAR *p) {
	buf.push_back(code[((unsigned int)p[0] & 0xfc) >> 2]);
	buf.push_back(code[((unsigned int)p[0] & 0x03) << 4 | (p[1] & 0xf0) >> 4]);
	buf.push_back(code[((unsigned int)p[1] & 0x0f) << 2 | (p[2] & 0xc0) >> 6]);
	buf.push_back(code[((unsigned int)p[2] & 0x3f)]);
}

tstring 
base64encode(const TCHAR *input, int len) {
	vector<TCHAR> buf;
	const TCHAR *p = input;

	// 3�������ϊ�
	if (len >= 3) {
		for (int i = 0; i <= len - 3; i += 3) {
			encode_block(buf, p);
			p += 3;
		}
	}

	// 3�����ɖ����Ȃ��Ō�̗]����ϊ�
	// ��U3�����ɂ��ĕϊ�������A�]�v�ȕ������I�['='�ŏ㏑������
	int odd = len % 3;
	if (odd != 0) {
		TCHAR last[3] = {0x0, 0x0, 0x0};
		for (int i = 0; i < odd; i++ ) {
			last[i] = *p++;
		}
		encode_block(buf, last);
		
		// �I�[��ǉ�
		for (int i = (2 - odd); i >= 0; i--) {
			buf[buf.size() - i - 1] = '=';
		}
	}
	buf.push_back('\0');
	
	return tstring((TCHAR *)&buf[0]);
}

