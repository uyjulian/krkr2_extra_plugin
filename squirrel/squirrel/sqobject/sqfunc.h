#ifndef __SQFUNC__H_
#define __SQFUNC__H_

#include <squirrel.h>
#include <new>

extern SQRESULT ERROR_CREATE(HSQUIRRELVM v);
extern SQRESULT ERROR_BADINSTANCE(HSQUIRRELVM v);
extern SQRESULT ERROR_BADMETHOD(HSQUIRRELVM v);

// ���N���X�_�~�[�p
struct BaseClass {};

// �N���X�^���ێ��p
template <typename T>
struct SQClassType {
public:
	static inline HSQOBJECT& ClassObject() {
		static HSQOBJECT classObj = {OT_NULL};
		return classObj;
	}

	static void done(HSQUIRRELVM vm) {
		HSQOBJECT &classObj = ClassObject();
		sq_release(vm, &classObj);
		sq_resetobject(&classObj);
	}

	static T* getInstance(HSQUIRRELVM vm, SQInteger idx=1) {
		SQUserPointer typetag = NULL;
		sq_getobjtypetag(&ClassObject(),&typetag);
		SQUserPointer up;
		if (SQ_SUCCEEDED(sq_getinstanceup(vm, idx, &up, typetag))) {
			return (T*)up;
		}
		return NULL;
	}

	// XXX sqobject::Object �O��R�[�h����
	
	static void pushInstance(HSQUIRRELVM v, T *value) {
		sq_pushobject(v, ClassObject());
		if (SQ_SUCCEEDED(sq_createinstance(v, -1))) {
			sq_remove(vm, -2);
			value->initSelf(vm, idx);
			if (SQ_SUCCEEDED(sq_setinstanceup(v, -1, value))) {;
				sq_setreleasehook(v, -1, release);
			} else {
				delete value;
				sq_pop(vm, 1);
				sq_pushnull(v);
			}
		} else {
			delete value;
			sq_pop(v, 1);
			sq_pushnull(v);
		}
	}

	static SQRESULT release(SQUserPointer up, SQInteger size) {
		if (up) {
			T* self = (T*)up;
			if (self) {
				self->destructor();
				delete self;
			}
		}
		return SQ_OK;
	}
};


/**
 * �N���X�o�^�p�e���v���[�g
 * @param T �o�^�Ώی^
 * @param P �e�̌^
 */
template <typename T, typename P>
class SQTemplate {

private:
	HSQUIRRELVM v;
	
public:

	/**
	 * �N���X���`����
	 * @param v squirrelVM
	 * @param typeName �o�^�^��
	 */
	SQTemplate(HSQUIRRELVM v, const SQChar *typeName=NULL) : v(v) {
		
		HSQOBJECT& classObj  = SQClassType<T>::ClassObject();
		HSQOBJECT& parentObj = SQClassType<P>::ClassObject();
		
		if (typeName) {
			sq_pushroottable(v); // root
			sq_pushstring(v, typeName, -1); // typeName
		}
		if (!sq_isnull(parentObj)) {
			// �e�N���X���w�肳��Ă�ꍇ�͌p������
			sq_pushobject(v, parentObj);
			sq_newclass(v, true);
		} else {
			// �p���Ȃ��ŃN���X����
			sq_newclass(v, false);
		}
		// �^�O��o�^
		sq_settypetag(v, -1, (SQUserPointer)&classObj);
		// �N���X�I�u�W�F�N�g�擾
		sq_getstackobj(v, -1, &classObj);
		sq_addref(v, &classObj); // must addref before the pop!

		if (typeName) {
			sq_createslot(v, -3);
			sq_pop(v, 1); // root
		} else {
			sq_pop(v, 1); // classobject
		}
		
		// �R���X�g���N�^�E�f�X�g���N�^��o�^
		Register(destructor, _SC("destructor"));
	}
	
	/*
	 * �l�C�e�B�u�I�u�W�F�N�g�̃����[�T�B
	 */
	static SQRESULT release(SQUserPointer up, SQInteger size) {
		if (up) {
			T* self = (T*)up;
			if (self) {
				self->destructor();
				self->~T();
				sq_free(up, size);
			}
		}
		return SQ_OK;
	}
	
	/**
	 * �R���X�g���N�^
	 * �l�C�e�B�u�I�u�W�F�N�g�̃R���X�g���N�^�Ɉ����Ƃ��� HSQUIRRELVM ��n���B
	 */
	static SQRESULT constructor(HSQUIRRELVM v) {
		T *self = (T*)sq_malloc(sizeof *self);
		new (self) T();
		if (self) {
			SQRESULT result;
			if (SQ_SUCCEEDED(result = sq_setinstanceup(v, 1, self))) {;
				sq_setreleasehook(v, 1, release);
			} else {
				self->~T();
				sq_free(self, sizeof *self);
			}
			return result;
		} else {
			return ERROR_CREATE(v);
		}
	}

	/**
	 * �R���X�g���N�^
	 * �l�C�e�B�u�I�u�W�F�N�g�̃R���X�g���N�^�Ɉ����Ƃ��� HSQUIRRELVM ��n���B
	 */
	static SQRESULT vconstructor(HSQUIRRELVM v) {
		T *self = (T*)sq_malloc(sizeof *self);
		new (self) T(v);
		if (self) {
			SQRESULT result;
			if (SQ_SUCCEEDED(result = sq_setinstanceup(v, 1, self))) {;
				sq_setreleasehook(v, 1, release);
			} else {
				self->~T();
				sq_free(self, sizeof *self);
			}
			return result;
		} else {
			return ERROR_CREATE(v);
		}
	}
	
	/**
	 * �f�X�g���N�^
	 * �Ȃɂ����Ȃ��BObject ��������ɌĂяo���̂ŃG���[���łȂ��悤�ɑ΍�
	 */
	static SQRESULT destructor(HSQUIRRELVM v) {
		return SQ_OK;
	}
	
	// -------------------------------------------------
	// �X�^�e�B�b�N�֐��̓o�^
	// -------------------------------------------------

	// SQFUNCTION �o�^
	void Register(SQFUNCTION func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		sq_newclosure(v, func, 0);
		sq_createslot(v, -3);
		sq_pop(v,1);
	}

	// -------------------------------------------------
	// �����o�֐��̓o�^�p
	// -------------------------------------------------

	// �C���X�^���X�擾
	static T *getInstance(HSQUIRRELVM v, int idx=1) {
		return SQClassType<T>::getInstance(v, idx);
	}
	
	// �֐��|�C���^�擾
	template <typename Func>
	static void getFunc(HSQUIRRELVM v, Func **func) {
		SQUserPointer x = NULL;
		sq_getuserdata(v,sq_gettop(v),&x,NULL);
		*func = (Func*)x;
	}

	// -------------------------------------------------

	// �A��l int �ň��������̊֐�
	typedef void (T::*VoidFunc)();

	// VoidFunc �Ăяo��
	static SQRESULT VoidFuncCaller(HSQUIRRELVM v) {
		int n = sq_gettop(v);
		T *instance = getInstance(v);
		if (instance) {
			VoidFunc *func;
			getFunc(v, &func);
			if (func) {
				sq_settop(v,n-1);
				(instance->*(*func))();
				return 0;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// VoidFunc �o�^
	void Register(VoidFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,VoidFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v,1);
	}

	// -------------------------------------------------

	// �A��l void �ň���int�̊֐�
	typedef void (T::*IntArgFunc)(int);
	typedef void (T::*UIntArgFunc)(unsigned int);

	// IntArgFunc �Ăяo��
	static SQRESULT IntArgFuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			IntArgFunc *func;
			getFunc(v, &func);
			SQInteger arg;
			sq_getinteger(v, 2, &arg);
			if (func) {
				(instance->*(*func))(arg);
				return 0;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// IntArgFunc �o�^
	void Register(IntArgFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,IntArgFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v,1);
	}

	// IntArgFunc �o�^
	void Register(UIntArgFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,IntArgFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v,1);
	}

	// -------------------------------------------------

	// �A��l void �ň���int�̊֐�
	typedef void (T::*RealArgFunc)(SQFloat);

	// IntArgFunc �Ăяo��
	static SQRESULT RealArgFuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			RealArgFunc *func;
			getFunc(v, &func);
			SQFloat arg;
			sq_getfloat(v, 2, &arg);
			if (func) {
				(instance->*(*func))(arg);
				return 0;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// IntArgFunc �o�^
	void Register(RealArgFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,RealArgFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}

	// -------------------------------------------------

	// �A��l void �ň���int�̊֐�
	typedef void (T::*RealArg2Func)(SQFloat, SQFloat);

	// IntArgFunc �Ăяo��
	static SQRESULT RealArg2FuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			RealArg2Func *func;
			getFunc(v, &func);
			SQFloat arg1, arg2;
			sq_getfloat(v, 2, &arg1);
			sq_getfloat(v, 3, &arg2);
			if (func) {
				(instance->*(*func))(arg1,arg2);
				return 0;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// IntArgFunc �o�^
	void Register(RealArg2Func func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,RealArg2FuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}
	
	// -------------------------------------------------

	// �A��l void �ň���int�̊֐�
	typedef void (T::*BoolArgFunc)(bool);

	// BoolArgFunc �Ăяo��
	static SQRESULT BoolArgFuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			BoolArgFunc *func;
			getFunc(v, &func);
			SQBool arg;
			sq_getbool(v, 2, &arg);
			if (func) {
				(instance->*(*func))(arg != 0);
				return 0;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// BoolArgFunc �o�^
	void Register(BoolArgFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,BoolArgFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}

	// �A��l void �ň���int�̊֐�
	typedef void (T::*BoolArg2Func)(bool,bool);

	// BoolArgFunc �Ăяo��
	static SQRESULT BoolArg2FuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			BoolArg2Func *func;
			getFunc(v, &func);
			SQBool arg1,arg2;
			sq_getbool(v, 2, &arg1);
			sq_getbool(v, 3, &arg2);
			if (func) {
				(instance->*(*func))(arg1!=0,arg2!=0);
				return 0;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// BoolArgFunc �o�^
	void Register(BoolArg2Func func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,BoolArg2FuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}

	
	// -------------------------------------------------

	// �A��l void �ň���int�̊֐�
	typedef void (T::*StrArgFunc)(const SQChar *);

	// StrArgFunc �Ăяo��
	static SQRESULT StrArgFuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			StrArgFunc *func;
			getFunc(v, &func);
			const SQChar *arg;
			sq_getstring(v, 2, &arg);
			if (func) {
				(instance->*(*func))(arg);
				return 0;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// StrArgFunc �o�^
	void Register(StrArgFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,StrArgFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}
	
	// -------------------------------------------------
	
	// �A��l int �ň��������̊֐�
	typedef int (T::*IntFunc)();
	typedef unsigned int (T::*UIntFunc)();

	// IntFunc �Ăяo��
	static SQRESULT IntFuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			IntFunc *func;
			getFunc(v, &func);
			if (func) {
				sq_pushinteger(v, (instance->*(*func))());
				return 1;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// IntFunc �o�^
	void Register(IntFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,IntFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}

	// IntFunc �o�^
	void Register(UIntFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,IntFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}
	
	// -------------------------------------------------
	
	// �A��l int �ň��������̊֐�
	typedef SQFloat (T::*RealFunc)();

	// RealFunc �Ăяo��
	static SQRESULT RealFuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			RealFunc *func;
			getFunc(v, &func);
			if (func) {
				sq_pushfloat(v, (instance->*(*func))());
				return 1;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// RealFunc �o�^
	void Register(RealFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,RealFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}

	// -------------------------------------------------
	
	// �A��l int �ň��������̊֐�
	typedef SQFloat (T::*RealFuncConst)() const;

	// RealFunc �o�^
	void Register(RealFuncConst func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,RealFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}
	
	// -------------------------------------------------
	
	// �A��l int �ň��������̊֐�
	typedef int (T::*IntFuncConst)() const;
	typedef unsigned int (T::*UIntFuncConst)() const;

	// IntFunc �o�^
	void Register(IntFuncConst func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,IntFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}

	// IntFunc �o�^
	void Register(UIntFuncConst func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,IntFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}
	
	// -------------------------------------------------
	
	// �A��l int �ň��������̊֐�
	typedef bool (T::*BoolFunc)();

	// IntFunc �Ăяo��
	static SQRESULT BoolFuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			BoolFunc *func;
			getFunc(v, &func);
			if (func) {
				sq_pushbool(v, (instance->*(*func))());
				return 1;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// BoolFunc �o�^
	void Register(BoolFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,BoolFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}

	// -------------------------------------------------
	
	// �A��l int �ň��������̊֐�
	typedef bool (T::*BoolFuncConst)() const;

	// IntFunc �Ăяo��
	static SQRESULT BoolFuncConstCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			BoolFuncConst *func;
			getFunc(v, &func);
			if (func) {
				sq_pushbool(v, (instance->*(*func))());
				return 1;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// BoolFuncConst �o�^
	void Register(BoolFuncConst func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,BoolFuncConstCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}
	
	// -------------------------------------------------
	
	// �A��l int �ň��������̊֐�
	typedef const SQChar *(T::*StrFunc)();

	// IntFunc �Ăяo��
	static SQRESULT StrFuncCaller(HSQUIRRELVM v) {
		T *instance = getInstance(v);
		if (instance) {
			StrFunc *func;
			getFunc(v, &func);
			if (func) {
				sq_pushstring(v, (instance->*(*func))(), -1);
				return 1;
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// BoolFunc �o�^
	void Register(StrFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,StrFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}
	
	// -------------------------------------------------

	// SQFUNCTION �X�^�C���̊֐�
	typedef SQRESULT (T::*VFunc)(HSQUIRRELVM v);

	// VFunc �Ăяo��
	static SQRESULT VFuncCaller(HSQUIRRELVM v) {
		SQInteger n = sq_gettop(v);
		T *instance = getInstance(v);
		if (instance) {
			VFunc *func = NULL;
			getFunc(v, &func);
			if (func) {
				sq_settop(v,n-1);
				return (instance->*(*func))(v);
			}
			return ERROR_BADMETHOD(v);
		}
		return ERROR_BADINSTANCE(v);
	}

	// VFunc �o�^
	void RegisterV(VFunc func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		SQUserPointer up = sq_newuserdata(v,sizeof(func));
		memcpy(up,&func,sizeof(func));
		sq_newclosure(v,VFuncCaller,1);
		sq_createslot(v, -3);
		sq_pop(v, 1);
	}

	// VFunc �o�^
	void RegisterVStatic(SQFUNCTION func, const SQChar *name) {
		sq_pushobject(v, SQClassType<T>::ClassObject());
		sq_pushstring(v, name, -1);
		sq_newclosure(v, func, 0);
		sq_newslot(v, -3, SQTrue);
		sq_pop(v, 1);
	}
};

// ------------------------------------------------------------------
// �N���X�o�^�p�}�N��
// ------------------------------------------------------------------

#define SQCLASS(Class,Parent,Name) SQTemplate<Class,Parent> cls(getGlobalVM(),Name);
#define SQCONSTRUCTOR() cls.Register(cls.constructor, _SC("constructor"));
#define SQVCONSTRUCTOR() cls.Register(cls.vconstructor, _SC("constructor"));
#define SQMYCONSTRUCTOR(Class, Name) cls.Register(&Class::Name, _SC("constructor"));
#define SQFUNC(Class, Name)   cls.Register(&Class::Name, _SC(#Name))
#define SQVFUNC(Class, Name)  cls.RegisterV(&Class::Name, _SC(#Name))
#define SQNFUNC(Class, Name) cls.Register(Class ## _ ## Name, _SC(#Name))
#define SQSVFUNC(Class, Name)  cls.RegisterVStatic(&Class::Name, _SC(#Name))

#endif
