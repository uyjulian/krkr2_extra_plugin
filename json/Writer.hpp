/**
 * �o�͏����p�C���^�[�t�F�[�X
 */
class IWriter {
protected:
	const tjs_char *newlinestr;
public:
	int indent;
        bool hex;
	IWriter(int newlinetype=0) {
		indent = 0;
                hex = false;
		switch (newlinetype) {
		case 1:
			newlinestr = L"\n";
			break;
		default:
			newlinestr  = L"\r\n";
			break;
		}
	}
	virtual ~IWriter(){};
	virtual void write(const tjs_char *str) = 0;
	virtual void write(tjs_char ch) = 0;
	virtual void write(tTVReal) = 0;
	virtual void write(tTVInteger) = 0;

	inline void newline() {
		write(newlinestr);
		for (int i=0;i<indent;i++) {
			write((tjs_char)' ');
		}
	}

	inline void addIndent() {
		indent++;
		newline();
	}

	inline void delIndent() {
		indent--;
		newline();
	}
};

/**
 * ������o��
 */
class IStringWriter : public IWriter {

public:
	ttstr buf;
	/**
	 * �R���X�g���N�^
	 */
	IStringWriter(int newlinetype=0) : IWriter(newlinetype) {};

	virtual void write(const tjs_char *str) {
		buf += str;
	}

	virtual void write(tjs_char ch) {
		buf += ch;
	}

	virtual void write(tTVReal num) {
               if (hex) {
                 tTJSVariantString *str = TJSRealToHexString(num);
                 buf += str;
                 str->Release();
                 buf += L" /* ";
                 str = TJSRealToString(num);
                 buf += str;
                 str->Release();
                 buf += L" */";
               } else {
                 tTJSVariantString *str = TJSRealToString(num);
                 buf += str;
                 str->Release();
               }
	}

	virtual void write(tTVInteger num) {
		tTJSVariantString *str = TJSIntegerToString(num);
		buf += str;
		str->Release();
	}
};

/**
 * �t�@�C���o��
 */
class IFileWriter : public IWriter {

	/// �o�̓o�b�t�@
	ttstr buf;
	/// �o�̓X�g���[��
	IStream *stream;
	bool utf;
	char *dat;
	int datlen;
	
public:

	/**
	 * �R���X�g���N�^
	 */
	IFileWriter(const tjs_char *filename, bool utf=false, int newlinetype=0) : IWriter(newlinetype) {
		stream = TVPCreateIStream(filename, TJS_BS_WRITE);
		this->utf = utf;
		dat = NULL;
		datlen = 0;
	}

	/**
	 * �f�X�g���N�^
	 */
	~IFileWriter() {
		if (stream) {
			if (buf.length() > 0) {
				output();
			}
			stream->Commit(STGC_DEFAULT);
			stream->Release();
		}
		if (dat) {
			free(dat);
		}
	}
	
	void output() {
		if (stream) {
			ULONG s;
			if (utf) {
				// UTF-8 �ŏo��
				int maxlen = buf.length() * 6 + 1;
				if (maxlen > datlen) {
					datlen = maxlen;
					dat = (char*)realloc(dat, datlen);
				}
				if (dat != NULL) {
					int len = TVPWideCharToUtf8String(buf.c_str(), dat);
					stream->Write(dat, len, &s);
				}
			} else {
				// ���݂̃R�[�h�y�[�W�ŏo��
				int len = buf.GetNarrowStrLen() + 1;
				if (len > datlen) {
					datlen = len;
					dat = (char*)realloc(dat, datlen);
				}
				if (dat != NULL) {
					buf.ToNarrowStr(dat, len-1);
					stream->Write(dat, len-1, &s);
				}
			}
		}
		buf.Clear();
	}
	
	virtual void write(const tjs_char *str) {
		if (stream) {
			buf += str;
			if (buf.length() >= 1024) {
				output();
			}
		}
	}

	virtual void write(tjs_char ch) {
		buf += ch;
	}

	virtual void write(tTVReal num) {
               if (hex) {
                 tTJSVariantString *str = TJSRealToHexString(num);
                 buf += str;
                 str->Release();
                 buf += L" /* ";
                 str = TJSRealToString(num);
                 buf += str;
                 str->Release();
                 buf += L" */";
               } else {
                 tTJSVariantString *str = TJSRealToString(num);
                 buf += str;
                 str->Release();
               }
	}

	virtual void write(tTVInteger num) {
		tTJSVariantString *str = TJSIntegerToString(num);
		buf += str;
		str->Release();
	}
};
