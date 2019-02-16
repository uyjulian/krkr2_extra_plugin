#include "sqcont.h"
#include "sqthread.h"

namespace sqobject {

ObjectInfo *beforeContinuousList;
ObjectInfo *afterContinuousList;

// ���X�g�ɒǉ�(���ɓo�^����Ă�ꍇ�͖���)
static void add(ObjectInfo &list, ObjectInfo &info)
{
	SQInteger max = list.len();
	for (SQInteger i=0;i<max;i++) {
		ObjectInfo f = list.get(i);
		if (f == info) {
			return;
		}
	}
	list.append(info);
}

static void call(ObjectInfo &list, int tick, int diff)
{
	SQInteger max = list.len();
	SQInteger i=0;
	for (SQInteger i=0;i<max;i++) {
		if (SQ_FAILED(list.get(i).call(tick, diff))) {
			list.remove(i);
			max--;
		}
	}
}

/// �n���h���o�^
static SQRESULT addContinuousHandler(HSQUIRRELVM v)
{
	ObjectInfo info(v,2);
	SQInteger type;
	if (sq_gettop(v) >= 3) {
		sq_getinteger(v, 3, &type);
	} else {
		type=1;
	}
	if (type == 0) {
		add(*beforeContinuousList, info);
	} else if (type == 1) {
		add(*afterContinuousList, info);
	}
	return SQ_OK;
}

/// �n���h���폜
static SQRESULT removeContinuousHandler(HSQUIRRELVM v)
{
	ObjectInfo info(v,2);
	SQInteger type;
	if (sq_gettop(v) >= 3) {
		sq_getinteger(v, 3, &type);
	} else {
		type=1;
	}
	if (type == 0) {
		beforeContinuousList->removeValue(info, true);
	} else if (type == 1) {
		afterContinuousList->removeValue(info, true);
	}
	return SQ_OK;
}

static SQRESULT clearContinuousHandler(HSQUIRRELVM v)
{
	beforeContinuousList->clearData();
	afterContinuousList->clearData();
	delete beforeContinuousList;
	delete afterContinuousList;
	return SQ_OK;
}


/// �@�\�o�^
void registerContinuous()
{
#define REGISTERMETHOD(name, n, type) \
	sq_pushstring(v, _SC(#name), -1);\
	sq_newclosure(v, name, 0);\
	sq_setparamscheck(v, n, type);\
	sq_createslot(v, -3);

	HSQUIRRELVM v = getGlobalVM();
	sq_pushroottable(v); // root
	REGISTERMETHOD(addContinuousHandler, -2, _SC(".cn"));
	REGISTERMETHOD(removeContinuousHandler, -2, _SC(".cn"));
	REGISTERMETHOD(clearContinuousHandler, 0, _SC(""));
	sq_pop(v,1);

	beforeContinuousList = new ObjectInfo();
	afterContinuousList  = new ObjectInfo();
	beforeContinuousList->initArray();
	afterContinuousList->initArray();
}

/// �n���h�������Ăяo���BThread::main �̑O�ŌĂяo��
void beforeContinuous()
{
	call(*beforeContinuousList, Thread::currentTick, Thread::diffTick);
}

/// �n���h�������Ăяo���BThread::main �̌�ŌĂяo��
void afterContinuous()
{
	call(*afterContinuousList, Thread::currentTick, Thread::diffTick);
}

/// �@�\�I��
void doneContinuous()
{
	beforeContinuousList->clearData();
	beforeContinuousList->clear();
	afterContinuousList->clearData();
	afterContinuousList->clear();
}

};

