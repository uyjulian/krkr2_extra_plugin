#include "psdclass.h"

#ifndef LOAD_MEMORY

#include "psdparse/psdparse.h"

/**
 * Stream �p�� Iterator
 */
class PSDIterator : public std::iterator<std::random_access_iterator_tag, const unsigned char>
{
public:

	typedef size_t diff_t;

	PSDIterator() : _psd(0), _pos(0), _size(0) {
	}

	/**
	 * �R���X�g���N�^
	 * @param srream �Q�ƃX�g���[��
	 * @param begin true �Ȃ�`���Afalse �Ȃ疖��
	 */
	PSDIterator(PSD* psd, bool begin) : _psd(psd), _pos(0), _size(0) {
		_size = psd->mStreamSize;
		_pos = begin ? 0 : _size;
	}

	// �R�s�[�R���X�g���N�^
	PSDIterator(const PSDIterator& o) {
		_psd = o._psd;
		_pos = o._pos;
		_size = o._size;
	}

	// ������Z�q
	PSDIterator &operator=(const PSDIterator &o) {
		_psd  = o._psd;
		_pos  = o._pos;
		_size = o._size;
		return *this;
	}
	
	// �f�X�g���N�^
	virtual ~PSDIterator() {
	}

	// �C�e���[�^��i�߂�(�O�u)
	PSDIterator& operator++() {
		_pos++;
		if (_pos > _size) _pos = _size;
		return *this;
	}

	// �C�e���[�^��i�߂�i��u)
	PSDIterator operator++(int) {
		PSDIterator ret = *this;
		_pos++;
		if (_pos > _size) _pos = _size;
		return ret;
	}

	PSDIterator& operator+=(diff_t n) {
		_pos += n;
		if (_pos > _size) _pos = _size;
		return *this;
	}

	PSDIterator operator+(diff_t n) {
		PSDIterator ret = *this;
		ret += n;
		return ret;
	}
	
	// �C�e���[�^��߂�(�O�u)
	PSDIterator& operator--() {
		_pos--;
		if (_pos < 0) _pos = 0;
		return *this;
	}

	// �C�e���[�^��߂��i��u)
	PSDIterator operator--(int) {
		PSDIterator ret = *this;
		_pos--;
		if (_pos < 0) _pos = 0;
		return ret;
	}

	PSDIterator& operator-=(diff_t n) {
		_pos -= n;
		if (_pos < 0) _pos = 0;
		return *this;
	}

	PSDIterator operator-(diff_t n) {
		PSDIterator ret = *this;
		ret -= n;
		return ret;
	}

	// �ǂݎ��
	const unsigned char& operator*() {
		return _psd->getStreamValue(_pos);
	}

	void copyToBuffer(uint8_t *buf, int size) {
		_psd->copyToBuffer(buf, _pos, size);
		_pos += size;
	}
	
	// ����
	diff_t operator-(const PSDIterator& b) const {
		return (diff_t)(_pos - b._pos);
	}
	
	// �C�e���[�^�̈�v����
	bool operator==(const PSDIterator& o) const {
		return o._pos == _pos;
	}
	// �C�e���[�^�̈�v����
	bool operator!=(const PSDIterator& o) const {
		return o._pos != _pos;
	}

	bool operator<(const PSDIterator& o) const {
		return _pos < o._pos;
	}
	bool operator>(const PSDIterator& o) const {
		return _pos > o._pos;
	}
	bool operator<=(const PSDIterator& o) const {
		return _pos <= o._pos;
	}
	bool operator>=(const PSDIterator& o) const {
		return _pos >= o._pos;
	}
	

	
private:
	PSD *_psd;
	tTVInteger _size; //< �X�g���[���T�C�Y�ێ��p
	tTVInteger _pos;  //< �Q�ƈʒu
};

namespace psd {

	inline void copyToBuffer(uint8_t *buffer, PSDIterator &cur, int size) {
		cur.copyToBuffer(buffer, size);
	}

	inline void getShortLE(uint8_t *buffer, PSDIterator &cur) {
		cur.copyToBuffer(buffer, 2);
	}
	
	inline void getLongLE(uint8_t *buffer, PSDIterator &cur) {
		cur.copyToBuffer(buffer, 4);
	}

	inline void getLongLongLE(uint8_t *buffer, PSDIterator &cur) {
		cur.copyToBuffer(buffer, 8);
	}
}

bool
PSD::loadStream(const ttstr &filename)
{
	clearData();

	// �X�g���[���̂܂�
	isLoaded = false;
	pStream = TVPCreateIStream(filename, TJS_BS_READ);
	if (pStream) {

		STATSTG stat;
		pStream->Stat(&stat, STATFLAG_NONAME);
		mStreamSize = stat.cbSize.QuadPart;

		PSDIterator begin(this, true);
		PSDIterator end(this, false);
		psd::Parser<PSDIterator> parser(*this);
		bool r = parse(begin , end,  parser);
		if (r && begin == end) {
			dprint("succeeded\n");
			isLoaded = processParsed();
		}
		if (!isLoaded) {
			clearData();
		}
	}
	return isLoaded;	
}

#endif
