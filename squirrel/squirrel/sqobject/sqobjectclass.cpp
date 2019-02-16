/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#include "sqthread.h"
#include <string.h>
#include <ctype.h>

namespace sqobject {

/**
 * �N���[�W�����ǂ���
 */
static bool
isClosure(SQObjectType type)
{
	return type == OT_CLOSURE || type == OT_NATIVECLOSURE;
}


/**
 * ������擾�p
 * @param v VM
 * @param idx �C���f�b�N�X
 * @return ������
 */
const SQChar *getString(HSQUIRRELVM v, SQInteger idx) {
	const SQChar *x = NULL;
	sq_getstring(v, idx, &x);
	return x;
};

// setter���O����
static void pushSetterName(HSQUIRRELVM v, const SQChar *name)
{
	int len = sizeof(SQChar) * (scstrlen(name) + 4);
	SQChar *buf = (SQChar*)sq_malloc(len);
	SQChar *p = buf;
	*p++ = 's';
	*p++ = 'e';
	*p++ = 't';
	*p++ = toupper(*name++);
	while (*name) { *p++ = *name++; }
	*p++ = '\0';
	sq_pushstring(v, buf, -1);
	sq_free(buf, len);
}

// getter���O����
static void pushGetterName(HSQUIRRELVM v, const SQChar *name)
{
	int len = sizeof(SQChar) * (scstrlen(name) + 4);
	SQChar *buf = (SQChar*)sq_malloc(len);
	SQChar *p = buf;
	*p++ = 'g';
	*p++ = 'e';
	*p++ = 't';
	*p++ = toupper(*name++);
	while (*name) { *p++ = *name++; };
	*p++ = '\0';
	sq_pushstring(v, buf, -1);
	sq_free(buf, len);
}

// ---------------------------------------------------------
// Object
// ---------------------------------------------------------

/**
 * �I�u�W�F�N�g�҂��̓o�^
 * @param thread �X���b�h
 */
void
Object::addWait(ObjectInfo &thread)
{
	_waitThreadList.append(thread);
}

/**
 * �I�u�W�F�N�g�҂��̉���
 * @param thread �X���b�h
 */
void
Object::removeWait(ObjectInfo &thread)
{
	_waitThreadList.removeValue(thread, true);
}

/**
 * �R���X�g���N�^
 */
Object::Object()
{
	_waitThreadList.initArray();
}

/**
 * �R���X�g���N�^
 */
Object::Object(HSQUIRRELVM v, int delegateIdx)
{
	self.getStackWeak(v,1);
	_waitThreadList.initArray();
	if (sq_gettop(v) >= delegateIdx) {
		delegate.getStackWeak(v, delegateIdx);
	}
}

/**
 * �f�X�g���N�^
 */
Object::~Object()
{
	notifyAll();
	delegate.clear();
	_waitThreadList.clear();
	self.clear();
}

/**
 * ���ȎQ�Ə������p
 * �I�u�W�F�N�g������K�����̏�������Ԃ���
 * @param v SQUIRREL vm
 * @param idx �����̃I�u�W�F�N�g������C���f�b�N�X
 */
void
Object::initSelf(HSQUIRRELVM v, int idx)
{
	if (sq_gettop(v) >= idx) {
		self.getStackWeak(v,idx);
	}
}

/**
 * �I��������
 */
void
Object::destructor()
{
	callEvent(_SC("destructor"));
}

/**
 * ���̃I�u�W�F�N�g��҂��Ă���P�X���b�h�̑҂�������
 */
void
Object::notify()
{
	SQInteger max = _waitThreadList.len();
	for (int i=0;i<max;i++) {
		Thread *th = _waitThreadList.get(i);
		if (th && th->notifyObject(self)) {
			_waitThreadList.remove(i);
			return;
		}
	}
}
	
/**
 * ���̃I�u�W�F�N�g��҂��Ă���S�X���b�h�̑҂�������
 */
void
Object::notifyAll()
{
	SQInteger max = _waitThreadList.len();
	for (int i=0;i<max;i++) {
		Thread *th = _waitThreadList.get(i);
		if (th) {
			th->notifyObject(self);
		}
	}
	_waitThreadList.clearData();
}

/**
 * �v���p�e�B����l���擾
 * @param name �v���p�e�B��
 * @return �v���p�e�B�l
 */
SQRESULT
Object::_get(HSQUIRRELVM v)
{
	SQRESULT result = SQ_OK;
	const SQChar *name = getString(v, 2);
	if (name && *name) {
		// delegate�̎Q��
		if (delegate.isDelegate()) {
			delegate.push(v);
			sq_pushstring(v, name, -1);
			if (SQ_SUCCEEDED(result = sq_get(v,-2))) {
				// ���\�b�h�̏ꍇ�͑�������
				if (isClosure(sq_gettype(v,-1)) && delegate.isBindDelegate()) {
					delegate.push(v);
					if (SQ_SUCCEEDED(sq_bindenv(v, -2))) {
						sq_remove(v, -2); // ���̃N���[�W��
					}
				}
				sq_remove(v, -2);
				return 1;
			} else {
				sq_pop(v,1); // delegate
			}
		}
		
		// getter ��T���ăA�N�Z�X
		sq_push(v, 1); // self
		pushGetterName(v,name); // getter��
		if (SQ_SUCCEEDED(result = sq_rawget(v,-2))) {
			sq_push(v, 1); //  self;
			if (SQ_SUCCEEDED(result = sq_call(v,1,SQTrue,SQTrue))) {
				//sqprintf("�Ăяo������:%s\n", name);
				sq_remove(v, -2); // func
				sq_remove(v, -2); // self
				return 1;
			} else {
				sq_pop(v,2); // func, self
			}
		} else {
			sq_pop(v, 1); // self
#if 0
			// �O���[�o���ϐ����Q��
			sq_pushroottable(v);
			sq_pushstring(v, name, -1);
			if (SQ_SUCCEEDED(sq_rawget(v,-2))) {
				sq_remove(v, -2); // root
				return 1;
			} else {
				sq_pop(v,1);
			}
#endif
		}
	}
	return SQ_ERROR;
}

/**
 * �v���p�e�B�ɒl��ݒ�
 * @param name �v���p�e�B��
 * @param value �v���p�e�B�l
 */
SQRESULT
Object::_set(HSQUIRRELVM v)
{
	SQRESULT result = SQ_OK;
	const SQChar *name = getString(v, 2);
	if (name && *name) {
		// delegate�̎Q��
		if (delegate.isDelegate()) {
			delegate.push(v);
			sq_push(v, 2); // name
			sq_push(v, 3); // value
			if (SQ_SUCCEEDED(result = sq_set(v,-3))) {
				sq_pop(v,1); // delegate
				return SQ_OK;
			} else {
				sq_pop(v,1); // delegate
			}
		}
		
		// setter ��T���ăA�N�Z�X
		sq_push(v, 1); // self
		pushSetterName(v, name);
		if (SQ_SUCCEEDED(result = sq_rawget(v,-2))) {
			sq_push(v, 1); // self
			sq_push(v, 3); // value
			if (SQ_SUCCEEDED(result = sq_call(v,2,SQFalse,SQTrue))) {
				//sqprintf("�Ăяo������:%s\n", name);
				sq_pop(v,2); // func, self
				return SQ_OK;
			} else {
				sq_pop(v,2); // func, self
			}
		}
		
	}
	//return result;
	return SQ_ERROR;
}

/**
 * set�v���p�e�B�̑��݊m�F
 * @param name �v���p�e�B��
 * @return set�v���p�e�B�����݂����� true
 */
SQRESULT
Object::hasSetProp(HSQUIRRELVM v)
{
	SQRESULT result = SQ_OK;
	SQBool ret = SQFalse;
	if (sq_gettop(v) > 1) {
		const SQChar *name = getString(v, 2);
		if (name && *name) {
			sq_push(v, 1); // object
			pushSetterName(v, name); // setter��
			if (SQ_SUCCEEDED(result = sq_rawget(v,-2))) {
				sq_pop(v,1);
				ret = SQTrue;
			} else {
				sq_pushstring(v, name, -1);
				if (SQ_SUCCEEDED(result = sq_rawget(v,-2))) {
					sq_pop(v,1);
					ret = SQTrue;
				}
			}
			sq_pop(v,1); // object
		}
	}
	if (SQ_SUCCEEDED(result)) {
		sq_pushbool(v, ret);
		return 1;
	} else {
		return result;
	}
}

/**
 * �Ϗ��̐ݒ�
 */
SQRESULT
Object::setDelegate(HSQUIRRELVM v)
{
	if (sq_gettop(v) > 1) {
		delegate.getStackWeak(v,2);
	} else {
		delegate.clear();
	}
	return SQ_OK;
}

/**
 * �Ϗ��̐ݒ�
 */
SQRESULT
Object::getDelegate(HSQUIRRELVM v)
{
	delegate.push(v);
	return 1;
}

bool
pushObject(HSQUIRRELVM v, Object *obj)
{
	if (obj->isInit()) {
		obj->push(v);
		return true;
	}
	return false;
}

void pushValue(HSQUIRRELVM v, const StackValue &value) { value.push(v); }

};

