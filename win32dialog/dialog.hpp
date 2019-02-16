#include <windows.h>

#include "dialog_config.hpp"

struct DialogTemplate {
	// �ݒ肩��^���G�C���A�X����
	typedef DialogConfig::SizeT   SizeT;
	typedef DialogConfig::NameT   NameT;
	typedef DialogConfig::StringT StringT;

	virtual ~DialogTemplate() {}

	// �f�[�^�̏����o��
	// c �ɕK�v�T�C�Y�����Z����i�A���C�����̗\�������]���ɉ��Z����邱�Ƃ�����j
	// p == 0 �Ȃ�f�[�^�͏����Ȃ�
	virtual void Write(BYTE* &p, SizeT &c) = 0;

	// �o�b�t�@�o�́��T�C�Y�v�Z�p �w���p�֐�

	// 1�o�C�g�����o��
	static inline void WriteByte(BYTE* &p, SizeT &c, BYTE v) {
		c++;
		if (p) *p++ = v;
	}
	// 2�o�C�g�����o��
	static inline void WriteWord(BYTE* &p, SizeT &c, WORD v) {
		c += 2;
		if (p) {
			*p++ = (BYTE)( v     & 0xFF);
			*p++ = (BYTE)((v>>8) & 0xFF);
		}
	}
	// 4�o�C�g�����o��
	static inline void WriteDWord(BYTE* &p, SizeT &c, DWORD v) {
		c += 4;
		if (p) {
			*p++ = (BYTE)( v      & 0xFF);
			*p++ = (BYTE)((v>> 8) & 0xFF);
			*p++ = (BYTE)((v>>16) & 0xFF);
			*p++ = (BYTE)((v>>24) & 0xFF);
		}
	}

	// �����񏑂��o��
	static inline void WriteString(BYTE* &p, SizeT &c, NameT r) {
		while (*r) WriteWord(p, c, *r++);
		/**/       WriteWord(p, c, 0);
	}

	// �A���C�������g
	static inline void Alignment(BYTE* &p, SizeT &c, SizeT al) {
		c += al; // �傫�����ɂ͖��Ȃ��̂ŃT�C�Y�v�Z�p�ɂ̓A���C�����𑫂�
		if (p) {
			ULONG n = al-1;
			p = (BYTE*) (((ULONG)p + n) & ~n);
		}
	}

	// sz_Or_Ord �^�F������ID�̂ǂ��炩
	//  0x0000 : �v�f�Ȃ�
	//  0xFFFF 0x???? : ID
	//  ���̑� : Null�����ŏI���Unicode������

	typedef enum { SZORD_NONE, SZORD_ID, SZORD_STR } SZORD;
	struct sz_Or_Ord{
		SZORD sel;
		WORD id;
		StringT str;
		sz_Or_Ord() : sel(SZORD_NONE) {}
		inline void SetString(NameT s) { sel = SZORD_STR; str = s; }
		inline void SetID(WORD n)      { sel = SZORD_ID;  id  = n; }
		inline void Write(BYTE* &p, SizeT &c) {
			switch (sel) {
			case SZORD_ID:  WriteWord(  p, c, 0xFFFF); WriteWord(p, c, id); break;
			case SZORD_STR: WriteString(p, c, str.c_str()); break;
			default:        WriteWord(  p, c, 0); break;
			}
		}
	};
};

struct DialogHeader : public DialogTemplate {
	// DLGTEMPLATEEX; // �z�u�͕K�� DWORD �A���C���̂���
	WORD dlgVer;			// �o�[�W�����i���1�j
	WORD signature;			// �V�O�l�`���i���0xFFFF�j
	DWORD helpID;			// �R���e�L�X�g�w���vID
	DWORD exStyle;			// ex�X�^�C��
	DWORD style;			// �X�^�C��
	WORD dlgItems;			// �A�C�e���̌�
	short x;				// x (dialog unit)
	short y;				// y (dialog unit)
	short cx;				// w (dialog unit)
	short cy;				// h (dialog unit)

	sz_Or_Ord menu;			// ���j���[ID
	sz_Or_Ord windowClass;	// �E�B���h�E�N���X
	StringT title;			// �_�C�A���O�{�b�N�X�̃^�C�g��(Unicode)

	// �ȉ��� DS_SETFONT ���w�肳�ꂽ�ꍇ�̂ݑ���
	short pointSize;		// �t�H���g�̃T�C�Y
	short weight;			// �t�H���g�̃E�F�C�g(0�`1000)
	BYTE  italic;			// �t�H���g�̃C�^���b�N(TRUEorFALSE)
	BYTE  charset;			// �t�H���g�̃L�����Z�b�g
	StringT typeFace;		// �t�H���g�̃^�C�v�t�F�C�X

	// �����l
	DialogHeader()
		:   dlgVer(1),
			signature(0xFFFF),
			helpID(0),
			exStyle(0),
			style(0),
			dlgItems(0),
			x(0),
			y(0),
			cx(0),
			cy(0),
			pointSize(0),
			weight(0),
			italic(0),
			charset(0)
		{}

	// �����o��
	void Write(BYTE* &p, SizeT &c) {
		Alignment( p, c, 4);

		WriteWord( p, c, dlgVer);
		WriteWord( p, c, signature);
		WriteDWord(p, c, helpID);
		WriteDWord(p, c, exStyle);
		WriteDWord(p, c, style);
		WriteWord( p, c, dlgItems);
		WriteWord( p, c, (WORD)x);
		WriteWord( p, c, (WORD)y);
		WriteWord( p, c, (WORD)cx);
		WriteWord( p, c, (WORD)cy);
		menu       .Write(p, c);
		windowClass.Write(p, c);
		WriteString(p, c, title.c_str());

		if (style & DS_SETFONT) {
			WriteWord( p, c, (WORD)pointSize);
			WriteWord( p, c, (WORD)weight);
			WriteByte( p, c, (BYTE)italic);
			WriteByte( p, c, (BYTE)charset);
			WriteString(p, c, typeFace.c_str());
		}
	}
};

struct DialogItems : public DialogTemplate {
	// DLGITEMTEMPLATEEX; // �z�u�͕K�� DWORD �A���C���̂���
	DWORD helpID;			// �R���e�L�X�g�w���vID
	DWORD exStyle;			// ex�X�^�C��
	DWORD style;			// �X�^�C��
	short x;				// x (dialog unit)
	short y;				// y (dialog unit)
	short cx;				// w (dialog unit)
	short cy;				// h (dialog unit)
	DWORD id;				// ���̃R���g���[��ID

	sz_Or_Ord windowClass;	// �E�B���h�E�N���X
	sz_Or_Ord title;		// �^�C�g��
	WORD extraCount;		// �g���f�[�^�̃T�C�Y�iWORD�A���C���j
	BYTE const *extraData;

	DialogItems()
		:   helpID(0),
			exStyle(0),
			style(0),
			x(0),
			y(0),
			cx(0),
			cy(0),
			id(0),
			extraCount(0),
			extraData(0)
		{}

	// �����o��
	void Write(BYTE* &p, SizeT &c) {
		Alignment( p, c, 4);

		WriteDWord(p, c, helpID);
		WriteDWord(p, c, exStyle);
		WriteDWord(p, c, style);
		WriteWord( p, c, (WORD)x);
		WriteWord( p, c, (WORD)y);
		WriteWord( p, c, (WORD)cx);
		WriteWord( p, c, (WORD)cy);
		WriteDWord(p, c, id);
		windowClass.Write(p, c);
		title      .Write(p, c);
		WriteWord(p, c, extraCount);
		if (extraCount > 0) {
			BYTE const *r = extraData;
			for (long i = 0; i < (long)extraCount; i++)
				WriteByte(p, c, *r++);

			Alignment( p, c, 2);
		}
	}
};

