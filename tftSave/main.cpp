#include <windows.h>
#include "ncbind.hpp"

////////////////////////////////////////////////////////////////

// �����_�����O�ς݃t�H���g�t�@�C���̕ۑ�/���ǂݎ�菈��

////////////////////////////////////////////////////////////////


//--------------------------------------------------------------
// �t�@�C������N���X(����)

struct PFontFile
{
	PFontFile(tjs_char const *storage, tjs_uint32 flags) : stream(0), storage(storage), commit(false)
	{
		stream = TVPCreateIStream(storage, flags);
		if (!stream) error(TJS_W("can't open storage"));
	}

	virtual ~PFontFile() {
		if (stream) {
			if (commit) stream->Commit(STGC_DEFAULT);
			stream->Release();
		}
		stream = 0;
	}

	void error(tjs_char const *message) const {
		ttstr mes(message);
		mes += TJS_W(":");
		mes += storage;
		TVPThrowExceptionMessage(mes.c_str());
	}

	typedef ULONG SizeType;

	void write(void const *buf, SizeType length) {
		if (!stream) return;
		SizeType written = 0;
		if (stream->Write(buf, length, &written) != S_OK || written != length)
			error(TJS_W("can't write storage"));
		commit = true;
	}

	void read(void *buf, SizeType length) {
		if (!stream) return;
		SizeType readed = 0;
		if (stream->Read(buf, length, &readed) != S_OK || readed != length)
			error(TJS_W("can't read storage"));
	}

	void seek(SizeType pos) {
		if (!stream) return;
		LARGE_INTEGER lpos;
		ULARGE_INTEGER newpos;
		lpos.QuadPart  = pos;
		newpos.QuadPart = 0;
		stream->Seek(lpos, STREAM_SEEK_SET, &newpos);
	}

	SizeType getPos() const {
		if (!stream) return 0;
		LARGE_INTEGER lpos;
		ULARGE_INTEGER curpos;
		lpos.QuadPart  = 0;
		curpos.QuadPart = 0;
		stream->Seek(lpos, STREAM_SEEK_CUR, &curpos);
		return (SizeType)curpos.QuadPart;
	}

	template <typename typename T>
	SizeType align(T t) {
		SizeType pos = getPos();
		write(&t, sizeof(T) - (pos % sizeof(T)));
		return getPos();
	}

protected:
	IStream *stream;
	ttstr storage;
	bool commit;

	static const char*    headerText;
	static const SizeType headerLength;
};
const char*               PFontFile::headerText   = "TVP pre-rendered font\x1a\x01\x02";
const PFontFile::SizeType PFontFile::headerLength = 24;

//--------------------------------------------------------------
// �t�@�C������N���X(��������)

struct PFontSaver : public PFontFile
{
	PFontSaver(tjs_char const *storage) : PFontFile(storage, TJS_BS_WRITE)
	{
		write(headerText, headerLength);
		write("            ", 12); // dummy index
	}
	virtual ~PFontSaver() {}

	void writeHeader(tjs_uint32 count, SizeType chindexpos, SizeType indexpos) {
		seek(headerLength);
		write(&count,      4);
		write(&chindexpos, 4);
		write(&indexpos,   4);
	}

	// �t�H���g�C���[�W�i65�i�K�j�̃��������O�X���k�ۑ�
	void writeCompress65(unsigned char *buf, int size) {
		if (!size) return;

		unsigned char *newbuf = new unsigned char [size];
		int newsize = 0;
		int count = 0;
		unsigned char last = 0xff;

		int i;
		for (i = 0; i < size; i++) {
			if (last == buf[i]) count++;
			else {
				newsize = _writeRunLength65(last, newbuf, newsize, count);
				newbuf[newsize++] = buf[i];
				count = 0;
			}
			last = buf[i];
		}
		newsize = _writeRunLength65(last, newbuf, newsize, count);
		try {
			write(newbuf, (SizeType)newsize);
		} catch(...) {
			delete [] newbuf;
			throw;
		}
		delete [] newbuf;
	}
private:
	inline int _writeRunLength65(unsigned char last, unsigned char *newbuf, int newsize, int count) {
		if(count >= 2) {
			while (count) {
				int len = count > 190 ? 190 : count;
				newbuf[newsize++] = 0x40 + len; // running
				count -= len;
			}
		} else {
			while(count--) newbuf[newsize++] = last;
		}
		return newsize;
	}
};


//--------------------------------------------------------------
// �t�@�C������N���X(�ǂݎ��)

struct PFontLoader : public PFontFile
{
	PFontLoader(tjs_char const *storage) : PFontFile(storage, TJS_BS_READ)
	{
		if (stream && !check(headerText, headerLength))
			error(TJS_W("invalid tft header"));
	}
	virtual ~PFontLoader() {}

	void readHeader(tjs_uint32 &count, SizeType &chindexpos, SizeType &indexpos) {
		seek(headerLength);
		read(&count,      4);
		read(&chindexpos, 4);
		read(&indexpos,   4);
	}

	bool check(void const *buf, SizeType length) {
		char * checkbuf = new char[length];
		try {
			read(checkbuf, length);
		} catch (...) {
			delete [] checkbuf;
			throw;
		}
		bool r = !memcmp(checkbuf, buf, length);
		delete [] checkbuf;
		return r;
	}
};

//--------------------------------------------------------------
// �O���t���ێ����C���[�W�ϊ��N���X

class PFontImage
{
	tjs_uint32 offset;
	tjs_char   code;
	tjs_uint16 width, height;
	tjs_int16  origin_x, origin_y, inc_x, inc_y, inc;
public:
	PFontImage()
		:   offset(0),
			code(0),
			width(0), height(0),
			origin_x(0), origin_y(0), inc_x(0), inc_y(0), inc(0)
		{}

	struct GetInfoWork {
		tTJSVariant         result;
		tTJSVariantClosure *closure;
		tjs_error           error;
		tjs_int             ch;

		GetInfoWork(tjs_int n, tTJSVariantClosure *c) : closure(c), error(TJS_E_FAIL), ch(n) {}
		iTJSDispatch2 *callback() {
			TVPDoTryBlock(TryBlock, CatchBlock, 0, this);
			return error == TJS_S_OK ? result.AsObjectNoAddRef() : 0;
		}
		void doTry() {
			tTJSVariant num = ch;
			tTJSVariant *param[] = { &num };
			error = closure->FuncCall(0, 0, 0, &result, 1, param, 0);
		}
		static void TJS_USERENTRY TryBlock(void *p) { ((GetInfoWork*)p)->doTry(); }
		static bool TJS_USERENTRY CatchBlock(void *p, const tTVPExceptionDesc &d) { return true; }
	};

	void saveImage(PFontSaver &saver, tjs_char ch, tTJSVariantClosure *closure) {
		code = ch;
		GetInfoWork wk((tjs_int)ch, closure);
		iTJSDispatch2 *layer = wk.callback();
		if (!layer) saver.error(TJS_W("invalid callback result"));

		ncbPropAccessor info(layer);
		if (!info.HasValue(TJS_W("blackbox_x")) ||
			!info.HasValue(TJS_W("blackbox_y")) ||
			!info.HasValue(TJS_W("origin_x")) ||
			!info.HasValue(TJS_W("origin_y")) ||
			!info.HasValue(TJS_W("inc_x")) ||
			!info.HasValue(TJS_W("inc_y")) ||
			!info.HasValue(TJS_W("inc")) ||
			!info.getIntValue(TJS_W("hasImage")))
			saver.error(TJS_W("no layer info"));
		int w    = (int)        info.getIntValue(TJS_W("blackbox_x"));
		int h    = (int)        info.getIntValue(TJS_W("blackbox_y"));
		origin_x = (tjs_int16)  info.getIntValue(TJS_W("origin_x"));
		origin_y = (tjs_int16)  info.getIntValue(TJS_W("origin_y"));
		inc_x    = (tjs_int16)  info.getIntValue(TJS_W("inc_x"));
		inc_y    = (tjs_int16)  info.getIntValue(TJS_W("inc_y"));
		inc      = (tjs_int16)  info.getIntValue(TJS_W("inc"));
		if (w < 0) w = 0;
		if (h < 0) h = 0;
		width    = (tjs_uint16) w;
		height   = (tjs_uint16) h;

		offset   = saver.getPos();

		if (width > 0 && height > 0) {
			unsigned char *buf = new unsigned char[w * h];
			try {
				copyAlphaImage65(info, buf, w, h);
				saver.writeCompress65(buf, w * h);
			} catch (...) {
				delete [] buf;
				throw;
			}
			delete buf;
		}
	}
	void copyAlphaImage65(ncbPropAccessor &lay, unsigned char *buf, int w, int h) {
		ZeroMemory(buf, w*h);
		int  sw    = (int) lay.getIntValue(TJS_W("imageWidth"));
		int  sh    = (int) lay.getIntValue(TJS_W("imageHeight"));
		long pitch = (long)lay.getIntValue(TJS_W("mainImageBufferPitch"));
		unsigned char *img = (unsigned char*)lay.getIntValue(TJS_W("mainImageBuffer"));

		if (sw > w) sw = w;
		if (sh > h) sh = h;

		// �x�����Ǔ����΂悢
		for (int y = 0; y < sh; y++) {
			unsigned char *line = buf + (y * w);
			for (int x = 0; x < sw; x++) {
				*line++ = (unsigned char)((unsigned long)(img[x*4 + y*pitch + 3]) * 64 / 255);
			}
		}
	}

	void saveCode(PFontSaver &saver) {
		saver.write(&code, sizeof(code));
	}
	void saveInfo(PFontSaver &saver) {
		saver.write(&offset,   4);
		saver.write(&width,    2);
		saver.write(&height,   2);
		saver.write(&origin_x, 2);
		saver.write(&origin_y, 2);
		saver.write(&inc_x,    2);
		saver.write(&inc_y,    2);
		saver.write(&inc,      2);

		tjs_int16 reserved = 0;
		saver.write(&reserved, 2);
	}

	////////////////////////////////////////////////
	tjs_char loadCode(PFontLoader &loader) {
		loader.read(&code, sizeof(code));
		return code;
	}
	void loadInfo(PFontLoader &loader) {
		loader.read(&offset,   4);
		loader.read(&width,    2);
		loader.read(&height,   2);
		loader.read(&origin_x, 2);
		loader.read(&origin_y, 2);
		loader.read(&inc_x,    2);
		loader.read(&inc_y,    2);
		loader.read(&inc,      2);

		tjs_int16 reserved = 0;
		loader.read(&reserved, 2);
	}
	void loadImage(PFontLoader &loader, tTJSVariantClosure *closure) {
		loader.seek(offset);
		// [TODO] convert tft-image to layer
	}
};

//--------------------------------------------------------------
// �ۑ�����

static void savePreRenderedFont(tjs_char const *storage, tTJSVariant characters, tTJSVariant callback)
{
	PFontSaver saver(storage);

	ncbPropAccessor charray(characters);
	tTJSVariantClosure closure = callback.AsObjectClosureNoAddRef();

	// �\�[�g����
	charray.FuncCall(0, TJS_W("sort"), 0, NULL);

	// �L������
	tjs_uint32 count = charray.GetArrayCount();
	if (!count) saver.error(TJS_W("empty characters"));

	// ���������L���������p��
	PFontImage *images = new PFontImage[count];

	typedef PFontFile::SizeType SizeType;
	SizeType chindexpos = 0;
	SizeType indexpos   = 0;
	SizeType padding    = 0;
	try {
		tjs_uint32 i;
		for (i = 0; i < count; i++) images[i].saveImage(saver, charray.getIntValue((tjs_int32)i), &closure);

		chindexpos = saver.align(padding);
		for (i = 0; i < count; i++) images[i].saveCode(saver);

		indexpos = saver.align(padding);
		for (i = 0; i < count; i++) images[i].saveInfo(saver);

		saver.writeHeader(count, chindexpos, indexpos);

	} catch (...) {
		delete [] images;
		throw;
	}
	delete [] images;
}

NCB_ATTACH_FUNCTION(savePreRenderedFont, System, savePreRenderedFont);

//--------------------------------------------------------------
// �ǂݍ��ݏ���

static void loadPreRenderedFont(tjs_char const *storage, tTJSVariant characters, tTJSVariant callback)
{
	PFontLoader loader(storage);

	ncbPropAccessor charray(characters);
	tTJSVariantClosure closure;
	bool encb = (callback.Type() == tvtObject);
	if (encb) closure = callback.AsObjectClosureNoAddRef();

	typedef PFontFile::SizeType SizeType;
	SizeType chindexpos = 0;
	SizeType indexpos   = 0;
	SizeType imagepos   = 0;

	tjs_uint32 count = 0;
	loader.readHeader(count, chindexpos, indexpos);
	if (!count) loader.error(TJS_W("empty characters"));

	// ���������L���������p��
	PFontImage *images = new PFontImage[count];

	try {
		tjs_uint32 i;
		imagepos = loader.getPos();
		loader.seek(chindexpos);
		for (i = 0; i < count; i++) {
			tjs_char ch = images[i].loadCode(loader);
			charray.SetValue(i, (tjs_int)ch);
		}

		if (encb) {
			loader.seek(indexpos);
			for (i = 0; i < count; i++) images[i].loadInfo(loader);

			loader.seek(imagepos);
			for (i = 0; i < count; i++) {
				images[i].loadImage(loader, &closure);
			}
		}
	} catch (...) {
		delete [] images;
		throw;
	}
	delete [] images;
}

NCB_ATTACH_FUNCTION(loadPreRenderedFont, System, loadPreRenderedFont);



////////////////////////////////////////////////////////////////

// �O���t���擾���`��p�g��

////////////////////////////////////////////////////////////////

struct LayerGlyphEx
{
	LayerGlyphEx(iTJSDispatch2 *self) : hdc(0), hfont(0), obj(self), font(0), format(GGO_GRAY8_BITMAP) {
		hdc = ::CreateCompatibleDC(NULL);
	}
	~LayerGlyphEx() {
		if (hfont) ::DeleteObject(hfont);
		::DeleteDC(hdc);
	}

	int getGlyphOutlineInfo(int ncode, GLYPHMETRICS &gm, iTJSDispatch2 *info = 0) {
		ZeroMemory(&gm, sizeof(gm));
		int size;
		SIZE incsz;
		tjs_char code = ncode;

		updateFont();
		size = ::GetGlyphOutlineW(hdc, ncode, format, &gm, 0, NULL, &no_transform_affin_matrix);
		::GetTextExtentPoint32W(hdc, &code, 1, &incsz);
		if (info) {
			ncbPropAccessor p(info);
			p.SetValue(TJS_W("blackbox_x"), (tjs_int)(size?gm.gmBlackBoxX:0));
			p.SetValue(TJS_W("blackbox_y"), (tjs_int)(size?gm.gmBlackBoxY:0));
			p.SetValue(TJS_W("origin_x"),   (tjs_int)gm.gmptGlyphOrigin.x);
			p.SetValue(TJS_W("origin_y"),   (tjs_int)gm.gmptGlyphOrigin.y);
			p.SetValue(TJS_W("inc_x"),      (tjs_int)gm.gmCellIncX);
			p.SetValue(TJS_W("inc_y"),      (tjs_int)gm.gmCellIncY);
			p.SetValue(TJS_W("inc"),        (tjs_int)incsz.cx);
		}
		return size;
	}
	void setGlyphInfo(int ncode) {
		GLYPHMETRICS gm;
		getGlyphOutlineInfo(ncode, gm, obj);
	}
	void drawGlyph(int ncode) {
		GLYPHMETRICS gm;
		int size = getGlyphOutlineInfo(ncode, gm, obj);
		int w = gm.gmBlackBoxX;
		int h = gm.gmBlackBoxY;
		if (size > 0 && w > 0 && h > 0) {
			int pitch = (size / h) & ~0x03L;
			long dstpch = 0;
			DWORD *dst = setupWriteImage(w, h, dstpch);
			unsigned char *buf = new unsigned char[size];
			try {
				::GetGlyphOutlineW(hdc, ncode, format, &gm, size, buf, &no_transform_affin_matrix);
				unsigned char *p = buf;
				for (int y = 0; y < h; y++, p+=pitch) {
					DWORD *q = dst + y * dstpch;
					for (int x = 0; x < w; x++) *q++ = convPixel(p[x]);
				}
			} catch (...) {
				delete [] buf;
				throw;
			}
			delete [] buf;
		}
	}
	inline DWORD convPixel(unsigned char px) { return !px ? 0 : px >= 64 ? 0xFFFFFFFF : (0x00FFFFFF | (((DWORD)px) << (2+24))); }
	DWORD* setupWriteImage(int w, int h, long &pch) {
		ncbPropAccessor p(obj);
		p.FuncCall(0, TJS_W("setSize"), 0, NULL, w, h);
		pch = (long)p.getIntValue(TJS_W("mainImageBufferPitch")) / 4;
		return (DWORD*)p.getIntValue(TJS_W("mainImageBufferForWrite"));
	}

	void getFontInfo(ttstr &face, int &h, int &a, int &f) {
		ncbPropAccessor p(font);
		face = p.getStrValue(TJS_W("face"));
		h = p.getIntValue(TJS_W("height"));
		a = p.getIntValue(TJS_W("angle"));
		f = (   (p.getIntValue(TJS_W("bold"     )) ? TVP_TF_BOLD      : 0) |
				(p.getIntValue(TJS_W("italic"   )) ? TVP_TF_ITALIC    : 0) |
				(p.getIntValue(TJS_W("strikeout")) ? TVP_TF_STRIKEOUT : 0) |
				(p.getIntValue(TJS_W("underline")) ? TVP_TF_UNDERLINE : 0) );
	}

	void updateFont() {
		if (!font) {
			tTJSVariant val;
			obj->PropGet(0, TJS_W("font"), 0, &val, obj);
			font = val.AsObjectNoAddRef();
			getFontInfo(f_face, f_height, f_angle, f_flags);
		} else {
			ttstr face;
			int height, angle, flags;
			getFontInfo(face, height, angle, flags);

			if (f_face   == face   &&
				f_height == height &&
				f_angle  == angle  &&
				f_flags  == flags) return;

			f_face   = face;
			f_height = height;
			f_angle  = angle;
			f_flags  = flags;
		}

		LOGFONT lf;
		ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight     =  f_height < 0 ? f_height : -f_height;
		lf.lfItalic     = (f_flags & TVP_TF_ITALIC   ) ? TRUE:FALSE;
		lf.lfWeight     = (f_flags & TVP_TF_BOLD     ) ?  700:  400;
		lf.lfUnderline  = (f_flags & TVP_TF_UNDERLINE) ? TRUE:FALSE;
		lf.lfStrikeOut  = (f_flags & TVP_TF_STRIKEOUT) ? TRUE:FALSE;
		lf.lfEscapement = lf.lfOrientation = f_angle;

		lf.lfCharSet        = DEFAULT_CHARSET; //SHIFTJIS_CHARSET;
		lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
		lf.lfQuality        = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		wcscpy_s(lf.lfFaceName, LF_FACESIZE, f_face.c_str());

		if (hfont) ::DeleteObject(hfont);
		hfont = ::CreateFontIndirect(&lf);
		::SelectObject(hdc, hfont);
	}
private:
	HDC hdc;
	HFONT hfont;
	iTJSDispatch2 *obj, *font;
	UINT format;
	ttstr f_face;
	int   f_height, f_angle, f_flags;

	static MAT2 no_transform_affin_matrix;

};
MAT2 LayerGlyphEx::no_transform_affin_matrix = { {0,1}, {0,0}, {0,0}, {0,1} };


NCB_GET_INSTANCE_HOOK(LayerGlyphEx)
{
	/**/  NCB_GET_INSTANCE_HOOK_CLASS () {}
	/**/ ~NCB_GET_INSTANCE_HOOK_CLASS () {}
	NCB_INSTANCE_GETTER(objthis) {
		ClassT* obj = GetNativeInstance(objthis);
		if (!obj) SetNativeInstance(objthis, (obj = new ClassT(objthis)));
		return obj;
	}
};
NCB_ATTACH_CLASS_WITH_HOOK(LayerGlyphEx, Layer)
{
	Method(TJS_W("setGlyphInfo"), &Class::setGlyphInfo);
	Method(TJS_W("drawGlyph"), &Class::drawGlyph);
}

