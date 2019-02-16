/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#ifndef __SQOBJECT_H__
#define __SQOBJECT_H__

// �^��
#ifndef SQOBJECT
#define SQOBJECT Object
#define SQOBJECTNAME _SC("Object")
#endif

// �I�u�W�F�N�g�o�C���h�p����
#ifndef NOUSESQRAT
#include <sqrat.h>
#else
#include "sqfunc.h"
#endif

#include "sqobjectinfo.h"

namespace sqobject {

const SQChar *getString(HSQUIRRELVM v, SQInteger idx);

/**
 * �I�u�W�F�N�g�p
 */
class Object {

protected:
	// squirrel�I�u�W�F�N�g�̎��ȎQ��
	ObjectInfo self;
	// ���̃I�u�W�F�N�g��҂��Ă�X���b�h�̈ꗗ
	ObjectInfo _waitThreadList;
	// delegate
	ObjectInfo delegate;

public:
#ifdef SQOBJHEAP
	SQHEAPDEFINE;
#endif
	bool isInit() {
		return !self.isNull();
	}
	void push(HSQUIRRELVM v) {
		self.push(v);
	}
	
	/**
	 * �I�u�W�F�N�g�҂��̓o�^
	 * @param thread �X���b�h
	 */
	void addWait(ObjectInfo &thread);
	
	/**
	 * �I�u�W�F�N�g�҂��̉���
	 * @param thread �X���b�h
	 */
	void removeWait(ObjectInfo &thread);
	
	/**
	 * �R���X�g���N�^
	 */
	Object();
	
	/**
	 * �R���X�g���N�^
	 * @param v squirrelVM
	 * @param delegateIdx �f���Q�[�g���i�[����Ă�����ԍ�
	 */
	Object(HSQUIRRELVM v, int delegateIdx=2);

	/**
	 * �f�X�g���N�^
	 */
	virtual ~Object();

	/**
	 * ���ȎQ�Ə������p
	 * �f�t�H���g�R���X�g���N�^�ŏ��������ꍇ�͕K�����̏�������Ԃ���
	 * @param v SQUIRREL vm
	 * @param idx �����̃I�u�W�F�N�g������C���f�b�N�X
	 */
	void initSelf(HSQUIRRELVM v, int idx=1);

	/**
	 * �j�������p
	 */
	void destructor();

public:
	
	// ------------------------------------------------------------------

	/**
	 * ���̃I�u�W�F�N�g��҂��Ă���P�X���b�h�̑҂�������
	 */
	void notify();
	
	/**
	 * ���̃I�u�W�F�N�g��҂��Ă���S�X���b�h�̑҂�������
	 */
	void notifyAll();
	
	/**
	 * �v���p�e�B����l���擾
	 * @param name �v���p�e�B��
	 * @return �v���p�e�B�l
	 */
	SQRESULT _get(HSQUIRRELVM v);

	/**
	 * �v���p�e�B�ɒl��ݒ�
	 * @param name �v���p�e�B��
	 * @param value �v���p�e�B�l
	 */
	SQRESULT _set(HSQUIRRELVM v);

	/**
	 * set�v���p�e�B�̑��݊m�F
	 * @param name �v���p�e�B��
	 * @return set�v���p�e�B�����݂����� true
	 */
	SQRESULT hasSetProp(HSQUIRRELVM v);
	
	/**
	 * �Ϗ��̐ݒ�
	 */
	SQRESULT setDelegate(HSQUIRRELVM v);

	/**
	 * �Ϗ��̎擾
	 */
	SQRESULT getDelegate(HSQUIRRELVM v);

public:
	/**
	 * squirrel �N���X�o�^
	 */
	static void registerClass();


protected:

	/**
	 * ���ȃI�u�W�F�N�g�C�x���g�Ăяo���i��������)
	 * C++���� squirrel �̎w�胁�\�b�h���C�x���g�Ƃ��ăR�[���o�b�N�ł��܂��B
	 * @param eventName �C�x���g��
	 */
	SQRESULT callEvent(const SQChar *eventName) {
		return self.callMethod(eventName);
	}

	/**
	 * ���ȃI�u�W�F�N�g�C�x���g�Ăяo���i����1��)
	 * @param eventName �C�x���g��
	 * @param p1 ����
	 */
	template<typename T1> SQRESULT callEvent(const SQChar *eventName, T1 p1) {
		return self.callMethod(eventName, p1);
	}
	
	/**
	 * ���ȃI�u�W�F�N�g�C�x���g�Ăяo���i����2��)
	 * @param eventName �C�x���g��
	 * @param p1 ����
	 * @param p2 ����2
	 */
	template<typename T1, typename T2> SQRESULT callEvent(const SQChar *eventName, T1 p1, T2 p2) {
		return self.callMethod(eventName, p1, p2);
	}
	
	/**
	 * �Ԓl�L�莩�ȃI�u�W�F�N�g�C�x���g�Ăяo���i��������)
	 * @param r �A��l�|�C���^
	 * @param eventName �C�x���g��
	 */
	template<typename R> SQRESULT callEventResult(R* r, const SQChar *eventName) {
		return self.callMethodResult(r, eventName);
	}

	/**
	 * �Ԓl���莩�ȃI�u�W�F�N�g�C�x���g�Ăяo���i����1��)
	 * @param r �A��l�|�C���^
	 * @param eventName �C�x���g��
	 * @param p1 ����
	 */
	template<typename R, typename T1> SQRESULT callEventResult(R* r, const SQChar *eventName, T1 p1) {
		return self.callMethodResult(r, eventName, p1);
	}
	
	/**
	 * �Ԓl�L�莩�ȃI�u�W�F�N�g�C�x���g�Ăяo���i����2��)
	 * @param r �A��l�|�C���^
	 * @param eventName �C�x���g��
	 * @param p1 ����
	 * @param p2 ����2
	 */
	template<typename R, typename T1, typename T2> SQRESULT callEventResult(R* r, const SQChar *eventName, T1 p1, T2 p2) {
		return self.callMethodResult(r, eventName, p1, p2);
	}

};

// ---------------------------------------------------
// �I�u�W�F�N�g�o�C���h�����p
// ---------------------------------------------------

// Object�� push����
// @return ���ł� squirrel �p�ɏ������ς݂ŃC���X�^���X�������Ă��� push �ł����� true
bool pushObject(HSQUIRRELVM v, Object *obj);


#ifndef NOUSESQRAT
// �l�̊i�[
// �i�[���s�����Ƃ��̓I�u�W�F�N�g���폜�����̂� delete �̕K�v�͂Ȃ�
// ������ squirrel ���Ő������ꂽ�I�u�W�F�N�g�������ꍇ�͕K����������
template<typename T>
void pushValue(HSQUIRRELVM v, T *value) {
	if (value) {
		if (pushObject(v, value)) {
			return;
		}
		Sqrat::Var<T*>::push(v, value);
		return;
	}
	sq_pushnull(v);
}

// �l�̊i�[���̑��p
template<typename T>
void pushOtherValue(HSQUIRRELVM v, T *value) {
	if (value) {
		Sqrat::Var<T*>::push(v, value);
		return;
	}
	sq_pushnull(v);
}

// �l�̎擾
template<typename T>
SQRESULT getValue(HSQUIRRELVM v, T **value, int idx=-1) {
	*value = Sqrat::Var<T*>(v, idx).value;
	return SQ_OK;
}
#else

// �l�̊i�[
// �i�[���s�����Ƃ��̓I�u�W�F�N�g���폜�����̂� delete �̕K�v�͂Ȃ�
// ������ squirrel ���Ő������ꂽ�I�u�W�F�N�g�������ꍇ�͕K����������
template<typename T>
void pushValue(HSQUIRRELVM v, T *value) {
	if (value) {
		if (pushObject(v, value)) {
			SQClassType<T>::pushInstance(v, value);
			return;
		}
	}
	sq_pushnull(v);
}

// �l�̎擾
template<typename T>
SQRESULT getValue(HSQUIRRELVM v, T **value, int idx=-1) {
	*value = SQClassType<T>::getInstance(v, idx);
	return SQ_OK;
}
#endif

// �l�̋���������
template<typename T>
void clearValue(T **value) {
	*value = 0;
}

// �l�̎擾
template<typename T>
SQRESULT getResultValue(HSQUIRRELVM v, T **value) {
	return getValue(value);
}


// ---------------------------------------------------------
// StackValue
// ---------------------------------------------------------

/**
 * �X�^�b�N�Q�Ɨp
 */
class StackValue {
	
public:
  // �R���X�g���N�^
  StackValue(HSQUIRRELVM v, int idx) : v(v), idx(idx) {};
  
  // �C�ӌ^�ւ̃L���X�g
  // �擾�ł��Ȃ������ꍇ�̓N���A�l�ɂȂ�
  template<typename T>
  operator T() const
  {
	T value;
	if (SQ_FAILED(getValue(v, &value, idx))) {
	  clearValue(&value);
	}
	return value;
  }

  // �I�u�W�F�N�g��PUSH
  void push(HSQUIRRELVM v) const {
	  sq_move(v, this->v, idx);
  }

  // �^��Ԃ�
  SQObjectType type() const {
	return sq_gettype(v, idx);
  }

  // int�l
  SQInteger intValue() const {
	return (SQInteger)*this;
  }

  // flaot�l
  SQFloat floatValue() const {
	return (SQFloat)*this;
  }

private:
  HSQUIRRELVM v;
  int idx;
};

void pushValue(HSQUIRRELVM v, const StackValue &sv);

// --------------------------------------------------------------------------------------

/**
 * ���������p���
 */
class StackInfo {

public:
	/**
	 * �R���X�g���N�^
	 * @param vm VM
	 */
	StackInfo(HSQUIRRELVM vm) : vm(vm) {
		argc = sq_gettop(vm) - 1;
	}
	
	/**
	 * @return �����̐�
	 */
	int len() const {
		return argc;
	}

	/**
	 * @return self�Q��
	 */
	ObjectInfo getSelf() const {
		return ObjectInfo(vm, 1);
	}

	/**
	 * @param n �����ԍ� 0�`
	 * @return �����̌^
	 */
	SQObjectType getType(int n) const {
		if (n < argc) {
			return sq_gettype(vm, n+2);
		}
		return OT_NULL;
	}
	
	/**
	 * @param n �����ԍ� 0�`
	 * @return �����̌^
	 */
	ObjectInfo getArg(int n) const {
		ObjectInfo ret;
		if (n < argc) {
			ret.getStack(vm, n+2);
		}
		return ret;
	}

	/**
	 * �����擾
	 * @param n �����ԍ� 0�`
	 * @return �����I�u�W�F�N�g
	 */
	ObjectInfo operator[](int n) const {
		return getArg(n);
	}

	// ���ʓo�^
	SQRESULT setReturn() const { return 0; }
	template<typename T>
	SQRESULT setReturn(T value) const {
		pushValue(vm, value);
		return 1;
	}

protected:
	HSQUIRRELVM vm; //< VM
	SQInteger argc; //< �����̐�
};

};// namespace

#endif
