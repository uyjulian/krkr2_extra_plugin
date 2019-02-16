#include <tchar.h>
#include <string>
#include <regex>

typedef std::basic_string<TCHAR> tstring;
typedef std::tr1::basic_regex<TCHAR> tregex;
typedef std::tr1::match_results<tstring::const_iterator> tmatch;

// Content-Type ���^�^�O���}�b�`���O���鐳�K�\���B�啶���������͖���
static tregex regctype(_T("<meta[ \\t]+http-equiv=(\\\"content-type\\\"|'content-type'|content-type)[ \\t]+content=(\\\"[^\\\"]*\\\"|'[^']*'|[^ \\t>]+).*>"), tregex::icase);

/**
 * text ������ Content-Type �̃��^�^�O��T���āA�w�肳��Ă�l (content=) ��Ԃ��B
 * @param text �T���Ώ�
 * @param ctype ���ʊi�[��
 */
bool
matchContentType(tstring &text, tstring &ctype)
{
	tmatch result;
	if (std::tr1::regex_search(text, result, regctype)) {
		tstring str = result.str(2);
		int len = str.size();
		const TCHAR *buf = str.c_str();
		if (len > 0) {
			if (buf[0] == '\'' || buf[0] == '"') {
				// �N�I�[�g����Ă�ꍇ�͂������菜��
				ctype = tstring(buf+1, len-2);
			} else {
				ctype = tstring(buf, len);
			}
			return true;
		}
	}
	return false;
}
