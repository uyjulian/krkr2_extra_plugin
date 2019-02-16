/**
 * sqrat �ł� sqobject ����
 *
 * sqrat ���g���� Object, Thread �o�^�����̎�����ł��B
 * sqrat �̋@�\�������Čp�����������Ă��܂��B
 */
#include "sqratfunc.h"
#include "sqobjectclass.h"
#include "sqthread.h"

#include <sqstdstring.h>
#include <sqstdmath.h>
#include <sqstdaux.h>

using namespace sqobject;

namespace sqobject {

// global vm
HSQUIRRELVM vm;

/// vm ������
HSQUIRRELVM init() {
	vm = sq_open(1024);
	sq_pushroottable(vm);
	sqstd_register_mathlib(vm);
	sqstd_register_stringlib(vm);
	sqstd_seterrorhandlers(vm);
	sq_pop(vm,1);
	return vm;
}

/// ���ێ��p�O���[�o��VM�̎擾
HSQUIRRELVM getGlobalVM()
{
	return vm;
}

/// vm �I��
void done()
{
	sq_close(vm);
}

// �f�X�g���N�^�o�^�p
static SQRESULT destructor(HSQUIRRELVM v) {
	return SQ_OK;
}

/**
 * Object �N���X�̓o�^
 */
void
Object::registerClass()
{
	Sqrat::Class<Object, sqobject::VMConstructor<Object> > cls(vm, (SQUserPointer)SQOBJECTNAME);
	cls.SquirrelFunc(_SC("destructor"), ::destructor);
	// sqrat �� set/get ���㏑������ sqobject �@�\�Ɛ������Ƃ�
	sqobject::OverrideSetGet<Object>::Func(vm);
	Sqrat::RootTable(vm).Bind(SQOBJECTNAME, cls);

	SQFUNC(Object,notify);
	SQFUNC(Object,notifyAll);
	SQVFUNC(Object,hasSetProp);
	SQVFUNC(Object,setDelegate);
	SQVFUNC(Object,getDelegate);
};

/**
 * Thread �N���X�̓o�^
 */
void
Thread::registerClass()
{
	SQCLASSOBJ_VCONSTRUCTOR(Thread,SQTHREADNAME);
	SQVFUNC(Thread,exec);
	SQVFUNC(Thread,exit);
	SQFUNC(Thread,stop);
	SQFUNC(Thread,run);
	SQFUNC(Thread,getCurrentTick);
	SQFUNC(Thread,getStatus);
	SQVFUNC(Thread,getExitCode);
	SQVFUNC(Thread,wait);
	SQFUNC(Thread,cancelWait);
};

}
