/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */

#ifndef __SQOBJECTINFO_H__
#define __SQOBJECTINFO_H__

#include <stdio.h>
#include <squirrel.h>
#include <sqstdstring.h>
#include <string>

#ifndef SQHEAPDEFINE
#define SQHEAPDEFINE \
	static void* operator new(size_t size) { return sq_malloc(size); }\
	static void* operator new[](size_t size) { return sq_malloc(size); }\
	static void  operator delete(void* p) { sq_free(p, 0); }\
	static void  operator delete[](void* p) { sq_free(p, 0); }
#endif

// ���O�o�͗p
#define SQPRINT(v,msg) {\
	SQPRINTFUNCTION print = sq_getprintfunc(v);\
	if (print) {\
		print(v,msg);\
	}\
}

namespace sqobject {

/// ������
HSQUIRRELVM init();
/// �I��
extern void done();
/// ���ێ��p�O���[�o��VM�̎擾
extern HSQUIRRELVM getGlobalVM();

class ObjectInfo;

typedef std::basic_string<SQChar> sqstring;

// �l�� push
void pushValue(HSQUIRRELVM v, bool value);
void pushValue(HSQUIRRELVM v, SQInteger value);
void pushValue(HSQUIRRELVM v, SQFloat value);
void pushValue(HSQUIRRELVM v, const SQChar *value);
void pushValue(HSQUIRRELVM v, SQUserPointer value);
void pushValue(HSQUIRRELVM v, const ObjectInfo &obj);
void pushValue(HSQUIRRELVM v, const sqstring &value);
void pushValue(HSQUIRRELVM v, SQFUNCTION func);
void pushValue(HSQUIRRELVM v, HSQOBJECT obj);


// �l�̎擾
SQRESULT getValue(HSQUIRRELVM v, bool *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, SQInteger *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, SQFloat *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, const SQChar **value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, SQUserPointer *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, sqstring *value, int idx=-1);
SQRESULT getValue(HSQUIRRELVM v, ObjectInfo *value, int idx=-1);


// �l�̋���������
void clearValue(bool *value);
void clearValue(SQInteger *value);
void clearValue(SQFloat *value);
void clearValue(const SQChar **value);
void clearValue(SQUserPointer *value);
void clearValue(ObjectInfo *value);
void clearValue(sqstring *value);


// �l�̎擾�F��{ getValue �̃R�s�y�B������͈��S�łȂ��ꍇ������̂Ŕr������K�v����
SQRESULT getResultValue(HSQUIRRELVM v, bool *value);
SQRESULT getResultValue(HSQUIRRELVM v, SQInteger *value);
SQRESULT getResultValue(HSQUIRRELVM v, SQFloat *value);
SQRESULT getResultValue(HSQUIRRELVM v, SQUserPointer *value);
SQRESULT getResultValue(HSQUIRRELVM v, ObjectInfo *value);
SQRESULT getResultValue(HSQUIRRELVM v, sqstring *value);

// ---------------------------------------------------------
// ObjectInfo
// ---------------------------------------------------------

/**
 * squirrel �I�u�W�F�N�g�ێ��p�N���X
 * ��Q�Ƃ��ێ��\
 */
class ObjectInfo {
public:
#ifdef SQOBJHEAP
	SQHEAPDEFINE;
#endif
	// roottable �̎擾
	static ObjectInfo getRoot();
	// �z��̍쐬
	static ObjectInfo createArray(SQInteger size=0);
	// �����̍쐬
	static ObjectInfo createTable();

	// ���e����
	void clear();

	// �X�^�b�N����擾
	void getStack(HSQUIRRELVM v, SQInteger idx);

    // �X�^�b�N�����Q�ƂƂ��Ď擾
	void getStackWeak(HSQUIRRELVM v, SQInteger idx);

	// �I�u�W�F�N�g����Q�ƂƂ��Ď擾
	void getWeak(const ObjectInfo &src);
	
	// �R���X�g���N�^
	ObjectInfo();

	// �R���X�g���N�^
	ObjectInfo(HSQOBJECT obj);
	
	// �R���X�g���N�^
	ObjectInfo(HSQUIRRELVM v, SQInteger idx);

	// �R�s�[�R���X�g���N�^
	ObjectInfo(const ObjectInfo &orig);

	// ���
	ObjectInfo & operator=(const ObjectInfo &orig);

	// �C�ӌ^�̃R���X�g���N�^
	template<typename T>
	ObjectInfo(T value) {
		sq_resetobject(&obj);
		setValue(value);
	}

	// �C�ӌ^�̑��
	template<typename T>
	ObjectInfo & operator=(T value) {
		setValue(value);
		return *this;
	}

	// �C�ӌ^�ւ̃L���X�g
	// �擾�ł��Ȃ������ꍇ�̓N���A�l�ɂȂ�
	template<typename T>
	operator T() const
	{
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		T value;
		if (SQ_FAILED(getValue(gv, &value))) {
			clearValue(&value);
		}
		sq_pop(gv, 1);
		return value;
	}
	
	// �l�̐ݒ�
	template<typename T>
	void setValue(T value) {
		HSQUIRRELVM gv = getGlobalVM();
		pushValue(gv, value);
		getStack(gv, -1);
		sq_pop(gv, 1);
	}
	
	// �f�X�g���N�^
	virtual ~ObjectInfo();

	// ��Q�Ƃ��H
	bool isWeak() {
		return sq_isweakref(obj);
	}
	
	// null ���H
	bool isNull() const {
		return type() == OT_NULL;
	}
	
	// �����X���b�h���H
	bool isSameThread(const HSQUIRRELVM v) const;

	// �X���b�h���擾
	operator HSQUIRRELVM() const;
	
	// �I�u�W�F�N�g��PUSH
	void push(HSQUIRRELVM v) const;

	/// ������o�^(���s������NULL)
	void pushClone(HSQUIRRELVM v) const;
	
	// ---------------------------------------------------
	// ��r�֐��Q
	// ---------------------------------------------------

	template<typename T>
	bool operator ==(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) == 0;
		sq_pop(v,2);
		return cmp;
	}

	template<typename T>
	bool operator !=(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) != 0;
		sq_pop(v,2);
		return cmp;
	}
	
	template<typename T>
	bool operator <(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) < 0;
		sq_pop(v,2);
		return cmp;
	}
	
	template<typename T>
	bool operator <=(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) <= 0;
		sq_pop(v,2);
		return cmp;
	}

	template<typename T>
	bool operator >(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) > 0;
		sq_pop(v,2);
		return cmp;
	}
	
	template<typename T>
	bool operator >=(T &value) {
		HSQUIRRELVM v = getGlobalVM();
		push(v);
		pushValue(v, value);
		bool cmp = sq_cmp(v) >= 0;
		sq_pop(v,2);
		return cmp;
	}

	// ---------------------------------------------------
	// delegate �����p
	// ---------------------------------------------------

	// delegate �Ƃ��ċ@�\���邩�ǂ���
	bool isDelegate() const;

	// bindenv �����邩�ǂ���
	bool isBindDelegate() const;

	// ---------------------------------------------------
	// �f�[�^�擾
	// ---------------------------------------------------

	const SQChar *getString();
	
	// ---------------------------------------------------
	// �z��E���������p
	// ---------------------------------------------------

	/// �z��Ƃ��ď�����
	void initArray(SQInteger size=0);
	
	/// @return �z��Ȃ� true
	bool isArray() const { return type() == OT_ARRAY; }

	/// �z��Ƃ��ď�����
	void initTable();

	/// @return �z��Ȃ� true
	bool isTable() const { return type() == OT_TABLE; }
	
	/// �z��ɒl��ǉ�
	SQRESULT append(HSQUIRRELVM v, SQInteger idx);

	/// �z��ɔz���ǉ�
	SQRESULT appendArray(ObjectInfo &array);
	
	/// �z��ɒl��ǉ�
	template<typename T>
	SQRESULT append(T value) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, value);
		SQRESULT ret = sq_arrayappend(gv, -2);
		sq_pop(gv,1);
		return ret;
	}

	/// �z��ɒl��}��
	template<typename T>
	SQRESULT insert(SQInteger index, T value) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, value);
		SQRESULT ret = sq_arrayinsert(gv, -2, index);
		sq_pop(gv,1);
		return ret;
	}

	/// �z�񂩂�w�肳�ꂽ�C���f�b�N�X�̒l���폜
	SQRESULT remove(SQInteger index) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		SQRESULT ret = sq_arrayremove(gv, -1, index);
		sq_pop(gv,1);
		return ret;
	}

	/// �z�񂩂�w�肳�ꂽ�l���폜
	template<typename T>
	SQRESULT removeValue(T value, bool all=false) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, value);
		SQRESULT ret = sq_arrayremovevalue(gv, -2, all ? SQTrue : SQFalse);
		sq_pop(gv,1);
		return ret;
	}
	
	/// �z��/�����ɒl���i�[
	template<typename K, typename T>
	SQRESULT set(K key, T value) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		pushValue(gv, value);
		SQRESULT ret = sq_set(gv, -3);
		sq_pop(gv,1);
		return ret;
	}

	/// �����ɒl��V�K�i�[
	template<typename K, typename T>
	SQRESULT create(K key, T value) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		pushValue(gv, value);
		SQRESULT ret = sq_newslot(gv, -3, SQFalse);
		sq_pop(gv,1);
		return ret;
	}

	/// ��������l���폜
	template<typename K>
	SQRESULT deleteslot(K key) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		SQRESULT ret = sq_deleteslot(gv, -2, SQFalse);
		sq_pop(gv,1);
		return ret;
	}

	/// @return �z��/�����̒��� key �����݂���Ȃ� true ��Ԃ�
	template<typename T>
	bool has(T key) const {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		if (sq_exists(gv, -2)) {
			sq_pop(gv, 1);
			return true;
		} else {
			sq_pop(gv, 1);
			return false;
		}
	}
	
	/// �z��/�����̒l���擾�B���݂��Ȃ��ꍇ�� null ��Ԃ�
	template<typename K>
	ObjectInfo get(K key) const {
		HSQUIRRELVM gv = getGlobalVM();
		pushData(gv, key);
		ObjectInfo ret(gv, -1);
		sq_pop(gv,1);
		return ret;
	}

	/**
	 * �z��Q�Ɨp
	 */
	class ObjectInfoReference {

	public:
		// �R���X�g���N�^
		ObjectInfoReference(HSQOBJECT obj, int key) : obj(obj), intKey(key), type(0) {};
		ObjectInfoReference(HSQOBJECT obj, const SQChar *key) : obj(obj), strKey(key), type(1) {};
		ObjectInfoReference(HSQOBJECT obj, sqstring &key) : obj(obj), strKey(key.c_str()), type(1) {};
		
		// �C�ӌ^�̑��
		// �ݒ�ł��Ȃ������ꍇ�Ŏ����̏ꍇ�͍쐬���Ă��܂�
		template<typename T>
		const T &operator=(const T &value) {
			ObjectInfo o(obj);
			if (o.type() == OT_ARRAY) {
				if (type == 0) {
					// �T�C�Y���͂��ĂȂ��̂� null �ł��߂�
					if (o.len() <= intKey) {
						HSQUIRRELVM gv = getGlobalVM();
						o.push(gv);
						while (o.len() <= intKey) {
							sq_pushnull(gv);
							sq_arrayappend(gv, -2);
						}
						sq_pop(gv,1);
					}
					o.set(intKey, value);
				}
			} else {
				SQRESULT ret;
				if (type == 0) {
					ret = o.set(intKey, value);
				} else {
					ret = o.set(strKey, value);
				}
				if (SQ_FAILED(ret)) {
					if (o.type() == OT_TABLE || o.type() == OT_CLASS) {
						if (type == 0) {
							o.create(intKey, value);
						} else {
							o.create(strKey, value);
						}
					}
				}
			}
			return value;
		}
		
		// �C�ӌ^�ւ̃L���X�g
		// �ϊ��ł��Ȃ������ꍇ�͏����l(0)�ł̃N���A�ɂȂ�
		template<typename T>
		operator T() const {
			ObjectInfo o(obj);
			T value;
			SQRESULT ret;
			if (type == 0) {
				ret = o.get(intKey, &value);
			} else {
				ret = o.get(strKey, &value);
			}
			if (SQ_FAILED(ret)) {
				clearValue(&value);
			}
			return value;
		}

		template<typename K>
		ObjectInfoReference operator[](K key) const {
			HSQOBJECT target;
			HSQUIRRELVM gv = getGlobalVM();
			pushData(gv);
			sq_getstackobj(gv, -1, &target);
			ObjectInfoReference ret = ObjectInfoReference(target, key);
			sq_pop(gv, 1);
			return ret;
		}
		
		// �f�[�^�擾�p
		void pushData(HSQUIRRELVM v) const {
			ObjectInfo o(obj);
			if (type == 0) {
				o.pushData(v, intKey);
			} else {
				o.pushData(v, strKey);
			}
		}
		
	protected:
		HSQOBJECT obj;
		int intKey;
		const SQChar *strKey;
		int type;
	};
	
	// �l�̐ݒ�
	// �Q�Ə�����������ꍇ�p
	void setValue(ObjectInfoReference &ref) {
		HSQUIRRELVM gv = getGlobalVM();
		ref.pushData(gv);
		getStack(gv, -1);
		sq_pop(gv, 1);
	}
	
	/**
	 * �z��/�����Q�Ɨp���t�@�����X���擾
	 * �����A�N�Z�X�����ꍇ��
	 * �E������s�����玩���ō쐬
	 * �E�擾���s�����珉���l(0)��Ԃ�
	 * �z��A�N�Z�X�����ꍇ��
	 * �E�T�C�Y�������������玩���I�ɂ��̃T�C�Y�܂ő��₷(null�����߂�)
	 * �E�擾���s�����珉���l(0)��Ԃ�
	 * �Ƃ��������������I�ɍs���܂�
	 */
	template<typename K>
	ObjectInfoReference operator[](K key) const {
		return ObjectInfoReference(obj, key);
	}

	/// �z��/�����̒l���擾����
	template<typename K, typename T>
	SQRESULT get(K key, T *value) const {
		SQRESULT ret;
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		pushValue(gv, key);
		if (SQ_SUCCEEDED(ret = sq_get(gv, -2))) {
			ret = getValue(gv, value);
			sq_pop(gv,1);
		}
		sq_pop(gv,1);
		return ret;
	}
	
	/// �z��/�����̒l��push�B������Ȃ����null��push
	template<typename T>
	void pushData(HSQUIRRELVM v, T key) const {
		push(v);
		pushValue(v, key);
		if (SQ_FAILED(sq_get(v, -2))) {
			sq_pushnull(v);
		}
		sq_remove(v,-2);
	}
	
	/// �z��E�����̒��g���N���A
	void clearData() {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		sq_clear(gv,-1);
		sq_pop(gv,1);
	}

	/// @return �I�u�W�F�N�g�̌^
	SQObjectType type() const {
		SQObjectType type;
		if (sq_isweakref(obj)) {
			HSQUIRRELVM gv = getGlobalVM();
			sq_pushobject(gv, obj);
			sq_getweakrefval(gv, -1);
			type = sq_gettype(gv, -1);
			sq_pop(gv, 2);
		} else {
			type = sq_type(obj);
		}
		return type;
	}
	
	/// @return �z��̒���/�����̃T�C�Y/������̒���
	SQInteger len() const;

	/**
	 * �z��̓��e��S��PUSH
	 * @param v squirrelVM
	 * @return push ������
	 */
	SQInteger pushArray(HSQUIRRELVM v) const;

	// ---------------------------------------------------
	// �֐������p
	// ---------------------------------------------------

	/// @return �z��Ȃ� true
	bool isClosure() const { SQObjectType t = type(); return t == OT_CLOSURE || t == OT_NATIVECLOSURE; }

	// �Ăяo������
	SQRESULT call(ObjectInfo *self=NULL);

	template<typename T1> SQRESULT call(T1 p1, ObjectInfo *self=NULL) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		pushValue(gv, p1);
		SQRESULT ret = sq_call(gv, 2, SQFalse, SQTrue);
		sq_pop(gv, 1);
		return ret;
	}

	template<typename T1, typename T2> SQRESULT call(T1 p1, T2 p2, ObjectInfo *self=NULL) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		pushValue(gv, p1);
		pushValue(gv, p2);
		SQRESULT ret = sq_call(gv, 3, SQFalse, SQTrue);
		sq_pop(gv, 1);
		return ret;
	}
	
	template<typename R> SQRESULT callResult(R* r, ObjectInfo *self=NULL) {
		SQRESULT ret;
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		if (SQ_SUCCEEDED(ret = sq_call(gv, 1, SQTrue, SQTrue))) {
			ret = getResultValue(gv, r);
			sq_pop(gv, 1);
		}
		sq_pop(gv, 1);
		return ret;
	}
	template<typename R, typename T1> SQRESULT callResult(R* r, T1 p1, ObjectInfo *self=NULL) {
		SQRESULT ret;
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		pushValue(gv, p1);
		if (SQ_SUCCEEDED(ret = sq_call(gv, 2, SQTrue, SQTrue))) {
			ret = getResultValue(gv, r);
			sq_pop(gv, 1);
		}
		sq_pop(gv, 1);
		return ret;
	}
	template<typename R, typename T1, typename T2> SQRESULT callResult(R* r, T1 p1, T2 p2, ObjectInfo *self=NULL) {
		SQRESULT ret;
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		if (self) {
			self->push(gv);
		} else {
			sq_pushroottable(gv); // root
		}
		pushValue(gv, p1);
		pushValue(gv, p2);
		if (SQ_SUCCEEDED(ret = sq_call(gv, 3, SQTrue, SQTrue))) {
			ret = getResultValue(gv, r);
			sq_pop(gv, 1);
		}
		sq_pop(gv, 1);
		return ret;
	}

	/**
	 * ���ȃI�u�W�F�N�g���\�b�h�Ăяo���i��������)
	 * @param methodName ���\�b�h��
	 */
	SQRESULT callMethod(const SQChar *methodName) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.call(this);
			}
		}
		return SQ_ERROR;
	}

	/**
	 * ���ȃI�u�W�F�N�g���\�b�h�Ăяo���i����1��)
	 * @param methodName ���\�b�h��
	 * @param p1 ����
	 */
	template<typename T1> SQRESULT callMethod(const SQChar *methodName, T1 p1) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.call(p1, this);
			}
		}
		return SQ_ERROR;
	}
	
	/**
	 * ���ȃI�u�W�F�N�g���\�b�h�Ăяo���i����2��)
	 * @param methodName ���\�b�h��
	 * @param p1 ����
	 * @param p2 ����2
	 */
	template<typename T1, typename T2> SQRESULT callMethod(const SQChar *methodName, T1 p1, T2 p2) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.call(p1, p2, this);
			}
		}
		return SQ_ERROR;
	}
	
	/**
	 * �Ԓl�L�莩�ȃI�u�W�F�N�g���\�b�h�Ăяo���i��������)
	 * @param r �A��l�|�C���^
	 * @param methodName ���\�b�h��
	 */
	template<typename R> SQRESULT callMethodResult(R* r, const SQChar *methodName) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.callResult(r, this);
			}
		}
		return SQ_ERROR;
	}

	/**
	 * �Ԓl���莩�ȃI�u�W�F�N�g���\�b�h�Ăяo���i����1��)
	 * @param r �A��l�|�C���^
	 * @param methodName ���\�b�h��
	 * @param p1 ����
	 */
	template<typename R, typename T1> SQRESULT callMethodResult(R* r, const SQChar *methodName, T1 p1) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.callResult(r, p1, this);
			}
		}
		return SQ_ERROR;
	}
	
	/**
	 * �Ԓl�L�莩�ȃI�u�W�F�N�g���\�b�h�Ăяo���i����2��)
	 * @param r �A��l�|�C���^
	 * @param methodName ���\�b�h��
	 * @param p1 ����
	 * @param p2 ����2
	 */
	template<typename R, typename T1, typename T2> SQRESULT callMethodResult(R* r, const SQChar *methodName, T1 p1, T2 p2) {
		if (!isNull()) {
			ObjectInfo method = get(methodName);
			if (method.isClosure()) {
				return method.callResult(r, p1, p2, this);
			}
		}
		return SQ_ERROR;
	}

	// ---------------------------------------------------
	// �N���X�����p
	// ---------------------------------------------------

	// �N���X�I�u�W�F�N�g��
	bool isClass() const;


	// ---------------------------------------------------
	// �C�e���[�^����
	// ---------------------------------------------------

	/**
	 * �C�e���[�^�Ăяo���֐�
	 * @param key �L�[
	 * @param value �l
	 * @param userData ���[�U�f�[�^
	 */
	typedef void (*foreachFunc)(const ObjectInfo &key, const ObjectInfo &value, void *userData);

	/**
	 * �֐��ŃC�e���[�^��������
	 * @param func �Ăяo���֐�
	 * @param userData ���[�U�f�[�^
	 */
	void foreach(foreachFunc func, void *userData=NULL) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		sq_pushnull(gv);
		while (SQ_SUCCEEDED(sq_next(gv,-2))) {
			func(ObjectInfo(gv,-2), ObjectInfo(gv,-1), userData);
			sq_pop(gv,2);
		}
		sq_pop(gv,1); // null
	}

	/**
	 * �֐��I�u�W�F�N�g�ŃC�e���[�^�����B�ȉ������������֐��I�u�W�F�N�g��n��
	 * anytype �� ObjectInfo ����L���X�g�\�Ȍ^�Ȃ�Ȃ�ł�OK
	 * struct Func {
	 *  void operator()(anytype key, anytype value) {
	 * }
	 */
	template<class F>
	void foreach(F &func) {
		HSQUIRRELVM gv = getGlobalVM();
		push(gv);
		sq_pushnull(gv);
		while (SQ_SUCCEEDED(sq_next(gv,-2))) {
			func(ObjectInfo(gv,-2), ObjectInfo(gv, -1));
			sq_pop(gv,2);
		}
		sq_pop(gv,1); // null
	}

	// ---------------------------------------------------
	// �����񏈗���
	// ---------------------------------------------------
	
	/**
	 * ������\�L��Ԃ�
	 * tostring() �����̏����B
	 */
	sqstring toString() const;

private:
	HSQOBJECT obj; // �I�u�W�F�N�g�Q�Ə��
};

// --------------------------------------------------------------------------------------
// printf����
// --------------------------------------------------------------------------------------

/**
 * printf�����̏���
 * @param format ����������
 * @return �\��������
 */
inline SQInteger printf(const SQChar *format)
{
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	return sqstd_printf(gv, 0);
}

template<typename T1>
inline SQInteger printf(const SQChar *format, T1 p1) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	return sqstd_printf(gv, 1);
}

template<typename T1, typename T2>
inline SQInteger printf(const SQChar *format, T1 p1, T2 p2) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	pushValue(gv, p2);
	return sqstd_printf(gv, 2);
}

template<typename T1, typename T2, typename T3>
inline SQInteger printf(const SQChar *format, T1 p1, T2 p2, T3 p3) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	pushValue(gv, p2);
	pushValue(gv, p3);
	return sqstd_printf(gv, 3);
}

template<typename T1, typename T2, typename T3, typename T4>
inline SQInteger printf(const SQChar *format, T1 p1, T2 p2, T3 p3, T4 p4) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	pushValue(gv, p2);
	pushValue(gv, p3);
	pushValue(gv, p4);
	return sqstd_printf(gv, 4);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
inline SQInteger printf(const SQChar *format, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5) {
	HSQUIRRELVM gv = getGlobalVM();
	sq_pushstring(gv, format, -1);
	pushValue(gv, p1);
	pushValue(gv, p2);
	pushValue(gv, p3);
	pushValue(gv, p4);
	pushValue(gv, p5);
	return sqstd_printf(gv, 5);
}

void pushValue(HSQUIRRELVM v, const ObjectInfo::ObjectInfoReference &obj);

};

#endif
