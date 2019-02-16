#include "ncbind/ncbind.hpp"
#include <vector>
#include <algorithm>

/**
 * ���\�b�h�ǉ��p
 */
class ScriptsAdd {

public:
	ScriptsAdd(){};

	/**
	 * �����o���ꗗ�̎擾
	 */
	static tjs_error TJS_INTF_METHOD getKeys(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis);
	/**
	 * �����o�̌��̎擾
	 */
	static tjs_error TJS_INTF_METHOD getCount(tTJSVariant *result,
											  tjs_int numparams,
											  tTJSVariant **param,
											  iTJSDispatch2 *objthis);
	/**
	 * �R���e�L�X�g�̎擾
	 */
	static tTJSVariant getObjectContext(tTJSVariant obj);

	/**
	 * �R���e�L�X�g�� null ���ǂ�������
	 */
	static bool isNullContext(tTJSVariant obj);
	
	//----------------------------------------------------------------------
	// �\���̔�r�֐�
	static bool equalStruct(tTJSVariant v1, tTJSVariant v2);

	//----------------------------------------------------------------------
	// �\���̔�r�֐�(�����̔�r�͂�邢)
	static bool equalStructNumericLoose(tTJSVariant v1, tTJSVariant v2);

	//----------------------------------------------------------------------
	// �S�z��E��������
	static tjs_error TJS_INTF_METHOD foreach(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis);

	//----------------------------------------------------------------------
	// hash�l�擾
	static tjs_error TJS_INTF_METHOD getMD5HashString(tTJSVariant *result,
													  tjs_int numparams,
													  tTJSVariant **param,
													  iTJSDispatch2 *objthis);


	//----------------------------------------------------------------------
	// �I�u�W�F�N�g����
	static tTJSVariant clone(tTJSVariant v1);

	//----------------------------------------------------------------------
	// �t���O�w����v���p�e�B����
	static tjs_error TJS_INTF_METHOD propSet(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis);
	static tjs_error TJS_INTF_METHOD propGet(tTJSVariant *result,
											 tjs_int numparams,
											 tTJSVariant **param,
											 iTJSDispatch2 *objthis);

	//----------------------------------------------------------------------
	// (const)�������^�z������S�ɕ]��
	static tjs_error TJS_INTF_METHOD safeEvalStorage(tTJSVariant *result,
													 tjs_int numparams,
													 tTJSVariant **param,
													 iTJSDispatch2 *objthis);

private:
		/**
	 * �����o���ꗗ�̎擾
	 */
	static void _getKeys(tTJSVariant *result, tTJSVariant &obj);
};

/**
 * �����̃L�[�ꗗ�擾�p
 */
class DictMemberGetCaller : public tTJSDispatch /** EnumMembers �p */
{
public:
	DictMemberGetCaller(iTJSDispatch2 *array) : array(array) {};
	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (numparams > 1) {
			tTVInteger flag = param[1]->AsInteger();
			static tjs_uint addHint = NULL;
			if (!(flag & TJS_HIDDENMEMBER)) {
				array->FuncCall(0, TJS_W("add"), &addHint, 0, 1, &param[0], array);
			}
		}
		if (result) {
			*result = true;
		}
		return TJS_S_OK;
	}
protected:
	iTJSDispatch2 *array;
};


//----------------------------------------------------------------------
// �������쐬
tTJSVariant createDictionary(void)
{
	iTJSDispatch2 *obj = TJSCreateDictionaryObject();
	tTJSVariant result(obj, obj);
	obj->Release();
	return result;
}

//----------------------------------------------------------------------
// �z����쐬
tTJSVariant createArray(void)
{
	iTJSDispatch2 *obj = TJSCreateArrayObject();
	tTJSVariant result(obj, obj);
	obj->Release();
	return result;
}

//----------------------------------------------------------------------
// �����̗v�f��S��r����Caller
class DictMemberCompareCaller : public tTJSDispatch
{
public:
	tTJSVariantClosure &target;
	bool match;

	DictMemberCompareCaller(tTJSVariantClosure &_target)
		 : target(_target)
		   , match(true) {
		   }

	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (result)
			*result = true;
		if (numparams > 1) {
			if ((int)*param[1] != TJS_HIDDENMEMBER) {
				const tjs_char *key = param[0]->GetString();
				tTJSVariant value = *param[2];
				tTJSVariant targetValue;
				if (target.PropGet(TJS_MEMBERMUSTEXIST, key, NULL, &targetValue, NULL)
					== TJS_S_OK) {
					match = match && ScriptsAdd::equalStruct(value, targetValue);
					if (result)
						*result = match;
				} else {
					match = false;
					if (result) {
						*result = match;
					}
				}
			}
		}
		return TJS_S_OK;
	}
};

//----------------------------------------------------------------------
// �����̗v�f��S��r����Caller(�����̔�r�͂�邢)
class DictMemberCompareNumericLooseCaller : public tTJSDispatch
{
public:
	tTJSVariantClosure &target;
	bool match;

	DictMemberCompareNumericLooseCaller(tTJSVariantClosure &_target)
		 : target(_target)
		   , match(true) {
		   }

	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		if (result)
			*result = true;
		if (numparams > 1) {
			if ((int)*param[1] != TJS_HIDDENMEMBER) {
				const tjs_char *key = param[0]->GetString();
				tTJSVariant value = *param[2];
				tTJSVariant targetValue;
				if (target.PropGet(0, key, NULL, &targetValue, NULL)
					== TJS_S_OK) {
					match = match && ScriptsAdd::equalStructNumericLoose(value, targetValue);
					if (result)
						*result = match;
				}
			}
		}
		return TJS_S_OK;
	}
};

//----------------------------------------------------------------------
// ���������񂷂�caller
class DictIterateCaller : public tTJSDispatch
{
public:
	iTJSDispatch2 *func;
	iTJSDispatch2 *functhis;
	tTJSVariant **paramList;
	tjs_int paramCount;
	tTJSVariant breakResult;

	DictIterateCaller(iTJSDispatch2 *func,
					  iTJSDispatch2 *functhis,
					  tTJSVariant **_paramList,
					  tjs_int _paramCount)
		 : func(func), functhis(functhis)
		   , paramList(_paramList)
		   , paramCount(_paramCount) {
		   }

	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		breakResult.Clear();
		if (numparams > 1) {
			if ((int)*param[1] != TJS_HIDDENMEMBER) {
				paramList[0] = param[0];
				paramList[1] = param[2];
				(void)func->FuncCall(0, NULL, NULL, &breakResult, paramCount, paramList, functhis);
			}
		}
		if (result) {
			*result = breakResult.Type() == tvtVoid;
		}
		return TJS_S_OK;
	}
};

//----------------------------------------------------------------------
// �ϐ�
tjs_uint32 countHint;

void
ScriptsAdd::_getKeys(tTJSVariant *result, tTJSVariant &obj)
{
	if (result) {
		iTJSDispatch2 *array = TJSCreateArrayObject();
		DictMemberGetCaller *caller = new DictMemberGetCaller(array);
		tTJSVariantClosure closure(caller);
		obj.AsObjectClosureNoAddRef().EnumMembers(TJS_IGNOREPROP|TJS_ENUM_NO_VALUE, &closure, NULL);
		caller->Release();
		static tjs_uint sortHint = NULL;
		// �Ԃ��L�[�̓\�[�g����
		array->FuncCall(0, TJS_W("sort"), &sortHint, 0, 0, 0, array);
		*result = tTJSVariant(array, array);
		array->Release();
	}
}

	/**
	 * �����o���ꗗ�̎擾
	 */
tjs_error TJS_INTF_METHOD
ScriptsAdd::getKeys(tTJSVariant *result,
					tjs_int numparams,
					tTJSVariant **param,
					iTJSDispatch2 *objthis)
{
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;
	_getKeys(result, *param[0]);
	return TJS_S_OK;
}

/**
 * �����o�̌��̎擾
 */
tjs_error TJS_INTF_METHOD
ScriptsAdd::getCount(tTJSVariant *result,
					 tjs_int numparams,
					 tTJSVariant **param,
					 iTJSDispatch2 *objthis)
{
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;
	if (result) {
		tjs_int count;
		param[0]->AsObjectClosureNoAddRef().GetCount(&count, NULL, NULL, NULL);
		*result = count;
	}
	return TJS_S_OK;
}


/**
 * �R���e�L�X�g�̎擾
 */
tTJSVariant
ScriptsAdd::getObjectContext(tTJSVariant obj)
{
	iTJSDispatch2 *objthis = obj.AsObjectClosureNoAddRef().ObjThis;
	return tTJSVariant(objthis, objthis);
}

/**
 * �R���e�L�X�g�� null ���ǂ�������
 */
bool
ScriptsAdd::isNullContext(tTJSVariant obj)
{
	return obj.AsObjectClosureNoAddRef().ObjThis == NULL;
}

//----------------------------------------------------------------------
// �\���̔�r�֐�
bool
ScriptsAdd::equalStruct(tTJSVariant v1, tTJSVariant v2)
{
	// �^�C�v���I�u�W�F�N�g�Ȃ���ꔻ��
	if (v1.Type() == tvtObject
		&& v2.Type() == tvtObject) {
		if (v1.AsObjectNoAddRef() == v2.AsObjectNoAddRef())
			return true;

		tTJSVariantClosure &o1 = v1.AsObjectClosureNoAddRef();
		tTJSVariantClosure &o2 = v2.AsObjectClosureNoAddRef();

		// �֐��ǂ����Ȃ���ʈ����Ŋ֐���r
		if (o1.IsInstanceOf(0, NULL, NULL, L"Function", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Function", NULL)== TJS_S_TRUE)
			return v1.DiscernCompare(v2);

		// Array�ǂ����Ȃ�S���ڂ��r
		if (o1.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE) {
			// ��������v���Ă��Ȃ���Δ�r���s
			tTJSVariant o1Count, o2Count;
			(void)o1.PropGet(0, L"count", &countHint, &o1Count, NULL);
			(void)o2.PropGet(0, L"count", &countHint, &o2Count, NULL);
			if (! o1Count.DiscernCompare(o2Count))
				return false;
			// �S���ڂ����Ԃɔ�r
			tjs_int count = o1Count;
			tTJSVariant o1Val, o2Val;
			for (tjs_int i = 0; i < count; i++) {
				(void)o1.PropGetByNum(TJS_IGNOREPROP, i, &o1Val, NULL);
				(void)o2.PropGetByNum(TJS_IGNOREPROP, i, &o2Val, NULL);
				if (! equalStruct(o1Val, o2Val))
					return false;
			}
			return true;
		}

		// Dictionary�ǂ����Ȃ�S���ڂ��r
		if (o1.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE) {
			// �L�[�ꗗ����v���ĂȂ���Δ�r���s
			tTJSVariant k1, k2;
			_getKeys(&k1, v1);
			_getKeys(&k2, v2);
			if (!equalStruct(k1, k2)) {
				return false;
			}
			// �S���ڂ����Ԃɔ�r
			DictMemberCompareCaller *caller = new DictMemberCompareCaller(o2);
			tTJSVariantClosure closure(caller);
			tTJSVariant(o1.EnumMembers(TJS_IGNOREPROP, &closure, NULL));
			bool result = caller->match;
			caller->Release();
			return result;
		}
	}

	return v1.DiscernCompare(v2);
}

//----------------------------------------------------------------------
// �\���̔�r�֐�(�����̔�r�͂�邢)
bool
ScriptsAdd::equalStructNumericLoose(tTJSVariant v1, tTJSVariant v2)
{
	// �^�C�v���I�u�W�F�N�g�Ȃ���ꔻ��
	if (v1.Type() == tvtObject
		&& v2.Type() == tvtObject) {
		if (v1.AsObjectNoAddRef() == v2.AsObjectNoAddRef())
			return true;

		tTJSVariantClosure &o1 = v1.AsObjectClosureNoAddRef();
		tTJSVariantClosure &o2 = v2.AsObjectClosureNoAddRef();

		// �֐��ǂ����Ȃ���ʈ����Ŋ֐���r
		if (o1.IsInstanceOf(0, NULL, NULL, L"Function", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Function", NULL)== TJS_S_TRUE)
			return v1.DiscernCompare(v2);

		// Array�ǂ����Ȃ�S���ڂ��r
		if (o1.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE) {
			// ��������v���Ă��Ȃ���Δ�r���s
			tTJSVariant o1Count, o2Count;
			(void)o1.PropGet(0, L"count", &countHint, &o1Count, NULL);
			(void)o2.PropGet(0, L"count", &countHint, &o2Count, NULL);
			if (! o1Count.DiscernCompare(o2Count))
				return false;
			// �S���ڂ����Ԃɔ�r
			tjs_int count = o1Count;
			tTJSVariant o1Val, o2Val;
			for (tjs_int i = 0; i < count; i++) {
				(void)o1.PropGetByNum(TJS_IGNOREPROP, i, &o1Val, NULL);
				(void)o2.PropGetByNum(TJS_IGNOREPROP, i, &o2Val, NULL);
				if (! equalStructNumericLoose(o1Val, o2Val))
					return false;
			}
			return true;
		}

		// Dictionary�ǂ����Ȃ�S���ڂ��r
		if (o1.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE
			&& o2.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE) {
			// ���ڐ�����v���Ă��Ȃ���Δ�r���s
			tjs_int o1Count, o2Count;
			(void)o1.GetCount(&o1Count, NULL, NULL, NULL);
			(void)o2.GetCount(&o2Count, NULL, NULL, NULL);
			if (o1Count != o2Count)
				return false;
			// �S���ڂ����Ԃɔ�r
			DictMemberCompareNumericLooseCaller *caller = new DictMemberCompareNumericLooseCaller(o2);
			tTJSVariantClosure closure(caller);
			tTJSVariant(o1.EnumMembers(TJS_IGNOREPROP, &closure, NULL));
			bool result = caller->match;
			caller->Release();
			return result;
		}
	}

	// �����̏ꍇ��
	if ((v1.Type() == tvtInteger || v1.Type() == tvtReal)
		&& (v2.Type() == tvtInteger || v2.Type() == tvtReal))
		return v1.NormalCompare(v2);

	return v1.DiscernCompare(v2);
}

//----------------------------------------------------------------------
// �S�z��E��������
tjs_error TJS_INTF_METHOD
ScriptsAdd::foreach(tTJSVariant *result,
					tjs_int numparams,
					tTJSVariant **param,
					iTJSDispatch2 *objthis)
{
	if (numparams < 2) return TJS_E_BADPARAMCOUNT;
	tTJSVariantClosure &obj = param[0]->AsObjectClosureNoAddRef();
	tTJSVariantClosure &funcClosure = param[1]->AsObjectClosureNoAddRef();

	// ���s�Ώۊ֐���I��
	// �����֐��Ȃ� this �R���e�L�X�g�œ��삳����
	iTJSDispatch2 *func     = funcClosure.Object;
	iTJSDispatch2 *functhis = funcClosure.ObjThis;
	if (functhis == 0) {
		functhis = objthis;
	}

	// �z��̏ꍇ
	if (obj.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE) {

		tTJSVariant key, value;
		tTJSVariant **paramList = new tTJSVariant *[numparams];
		paramList[0] = &key;
		paramList[1] = &value;
		for (tjs_int i = 2; i < numparams; i++)
			paramList[i] = param[i];

		tTJSVariant arrayCount;
		(void)obj.PropGet(0, L"count", &countHint, &arrayCount, NULL);
		tjs_int count = arrayCount;

		tTJSVariant breakResult;
		for (tjs_int i = 0; i < count; i++) {
			key = i;
			breakResult.Clear();
			(void)obj.PropGetByNum(TJS_IGNOREPROP, i, &value, NULL);
			(void)func->FuncCall(0, NULL, NULL, &breakResult, numparams, paramList, functhis);
			if (breakResult.Type() != tvtVoid) {
				break;
			}
		}
		if (result) {
			*result = breakResult;
		}
		
		delete[] paramList;

	} else {

		tTJSVariant **paramList = new tTJSVariant *[numparams];
		for (tjs_int i = 2; i < numparams; i++)
			paramList[i] = param[i];

		DictIterateCaller *caller = new DictIterateCaller(func, functhis, paramList, numparams);
		tTJSVariantClosure closure(caller);
		obj.EnumMembers(TJS_IGNOREPROP, &closure, NULL);
		if (result) {
			*result = caller->breakResult;
		}
		caller->Release();

		delete[] paramList;
	}
	return TJS_S_OK;
}


/**
 * octet �� MD5�n�b�V���l�̎擾
 * @param octet �ΏۃI�N�e�b�g
 * @return �n�b�V���l�i32������16�i���n�b�V��������i�������j�j
 */
tjs_error TJS_INTF_METHOD
ScriptsAdd::getMD5HashString(tTJSVariant *result,
							 tjs_int numparams,
							 tTJSVariant **param,
							 iTJSDispatch2 *objthis) {
	if (numparams < 1) return TJS_E_BADPARAMCOUNT;

	tTJSVariantOctet *octet = param[0]->AsOctetNoAddRef();

	TVP_md5_state_t st;
	TVP_md5_init(&st);
	TVP_md5_append(&st, octet->GetData(), (int)octet->GetLength());
	
	tjs_uint8 buffer[16];
	TVP_md5_finish(&st, buffer);

	tjs_char ret[32+1], *hex = TJS_W("0123456789abcdef");
	for (tjs_int i=0; i<16; i++) {
		ret[i*2  ] = hex[(buffer[i] >> 4) & 0xF];
		ret[i*2+1] = hex[(buffer[i]     ) & 0xF];
	}
	ret[32] = 0;
	if (result) *result = ttstr(ret);
	return TJS_S_OK;
}


//----------------------------------------------------------------------
// �����̗v�f��Sclone����Caller
class DictMemberCloneCaller : public tTJSDispatch
{
public:
	DictMemberCloneCaller(iTJSDispatch2 *dict) : dict(dict) {};
	virtual tjs_error TJS_INTF_METHOD FuncCall( // function invocation
												tjs_uint32 flag,			// calling flag
												const tjs_char * membername,// member name ( NULL for a default member )
												tjs_uint32 *hint,			// hint for the member name (in/out)
												tTJSVariant *result,		// result
												tjs_int numparams,			// number of parameters
												tTJSVariant **param,		// parameters
												iTJSDispatch2 *objthis		// object as "this"
												) {
		tTJSVariant value = ScriptsAdd::clone(*param[2]);
		dict->PropSet(TJS_MEMBERENSURE|(tjs_int)*param[1], param[0]->GetString(), 0, &value, dict);
		if (result)
			*result = true;
		return TJS_S_OK;
	}
protected:
	iTJSDispatch2 *dict;
};

//----------------------------------------------------------------------
// �\���̔�r�֐�
tTJSVariant
ScriptsAdd::clone(tTJSVariant obj)
{
	// �^�C�v���I�u�W�F�N�g�Ȃ�ׂ�������
	if (obj.Type() == tvtObject) {

		tTJSVariantClosure &o1 = obj.AsObjectClosureNoAddRef();
		
		// Array�̕���
		if (o1.IsInstanceOf(0, NULL, NULL, L"Array", NULL)== TJS_S_TRUE) {
			iTJSDispatch2 *array = TJSCreateArrayObject();
			tTJSVariant o1Count;
			(void)o1.PropGet(0, L"count", &countHint, &o1Count, NULL);
			tjs_int count = o1Count;
			tTJSVariant val;
			tTJSVariant *args[] = {&val};
			for (tjs_int i = 0; i < count; i++) {
				(void)o1.PropGetByNum(TJS_IGNOREPROP, i, &val, NULL);
				val = ScriptsAdd::clone(val);
				static tjs_uint addHint = 0;
				(void)array->FuncCall(0, TJS_W("add"), &addHint, 0, 1, args, array);
			}
			tTJSVariant result(array, array);
			array->Release();
			return result;
		}
		
		// Dictionary�̕���
		if (o1.IsInstanceOf(0, NULL, NULL, L"Dictionary", NULL)== TJS_S_TRUE) {
			iTJSDispatch2 *dict = TJSCreateDictionaryObject();
			DictMemberCloneCaller *caller = new DictMemberCloneCaller(dict);
			tTJSVariantClosure closure(caller);
			o1.EnumMembers(TJS_IGNOREPROP, &closure, NULL);
			caller->Release();
			tTJSVariant result(dict, dict);
			dict->Release();
			return result;
		}

		// clone���\�b�h�̌Ăяo���ɐ�������΂����Ԃ�
		tTJSVariant result;
		static tjs_uint cloneHint = 0;
		if (o1.FuncCall(0, L"clone", &cloneHint, &result, 0, NULL, NULL)== TJS_S_TRUE) {
			return result;
		}
	}
	
	return obj;
}

//----------------------------------------------------------------------
// �t���O�w����v���p�e�B����
tjs_error TJS_INTF_METHOD
ScriptsAdd::propSet(tTJSVariant *result,
					tjs_int numparams,
					tTJSVariant **param,
					iTJSDispatch2 *objthis)
{
	if (numparams < 3) return TJS_E_BADPARAMCOUNT;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();

	tjs_uint32 flag = (numparams > 3) ? (tjs_uint32)param[3]->operator tjs_int() : TJS_MEMBERENSURE;
	return ((param[1]->Type() != tvtInteger)
			? Try_iTJSDispatch2_PropSet     (clo.Object, flag, param[1]->GetString(), param[1]->GetHint(), param[2], clo.ObjThis)
			: Try_iTJSDispatch2_PropSetByNum(clo.Object, flag, param[1]->operator tjs_int(),               param[2], clo.ObjThis));
}
tjs_error TJS_INTF_METHOD
ScriptsAdd::propGet(tTJSVariant *result,
					tjs_int numparams,
					tTJSVariant **param,
					iTJSDispatch2 *objthis)
{
	if (numparams < 2) return TJS_E_BADPARAMCOUNT;
	tTJSVariantClosure clo = param[0]->AsObjectClosureNoAddRef();

	tjs_uint32 flag = (numparams > 2) ? (tjs_uint32)param[2]->operator tjs_int() : TJS_MEMBERMUSTEXIST;
	return ((param[1]->Type() != tvtInteger)
			? Try_iTJSDispatch2_PropGet     (clo.Object, flag, param[1]->GetString(), param[1]->GetHint(), result, clo.ObjThis)
			: Try_iTJSDispatch2_PropGetByNum(clo.Object, flag, param[1]->operator tjs_int(),               result, clo.ObjThis));
}

//----------------------------------------------------------------------
// (const)�������^�z������S�ɕ]��
tjs_error TJS_INTF_METHOD
ScriptsAdd::safeEvalStorage(tTJSVariant *result,
							tjs_int numparams,
							tTJSVariant **param,
							iTJSDispatch2 *objthis)
{
	if(numparams < 1) return TJS_E_BADPARAMCOUNT;

	ttstr name = *param[0];

	ttstr modestr;
	if(numparams >=2 && param[1]->Type() != tvtVoid)
		modestr = *param[1];

	iTJSDispatch2 *context = numparams >= 3 && param[2]->Type() != tvtVoid ? param[2]->AsObjectNoAddRef() : NULL;

	ttstr shortname(TVPExtractStorageName(name));

	iTJSTextReadStream * stream = TVPCreateTextStreamForRead(name, modestr);
	ttstr buffer;
	try
	{
		stream->Read(buffer, 0);
	}
	catch(...)
	{
		stream->Destruct();
		throw;
	}
	stream->Destruct();

	/*
	ttstr content(TJS_W("(const)["));
	content += buffer;
	content += TJS_W("]");
	buffer = content;
	 */
	tjs_int length = buffer.length();
	tjs_char *top = buffer.AppendBuffer(8+1); // [MEMO] "(const)[]".length == 9
	memmove(top + 8,         top,               sizeof(tjs_char)*length); // xxxxxxxx<buffer>x
	memcpy (top,             TJS_W("(const)["), sizeof(tjs_char)*8);      // (const)[<buffer>x
	memcpy (top + 8 +length, TJS_W("]"),        sizeof(tjs_char)*1);      // (const)[<buffer>]
	buffer.FixLen();
	//TVPAddLog(buffer);

	tTJSVariant temp;
	TVPExecuteExpression(buffer, shortname, 0, context, &temp);
	if (result) {
		tTJSVariantClosure clo;
		clo = temp.AsObjectClosureNoAddRef();
		if (clo.Object) {
			clo.PropGetByNum(TJS_IGNOREPROP, 0, result, NULL);
		}
	}

	return TJS_S_OK;
}
//----------------------------------------------------------------------
NCB_ATTACH_CLASS(ScriptsAdd, Scripts) {
	RawCallback(TJS_W("getObjectKeys"), &ScriptsAdd::getKeys, TJS_STATICMEMBER);
	RawCallback(TJS_W("getObjectCount"), &ScriptsAdd::getCount, TJS_STATICMEMBER);
	NCB_METHOD(getObjectContext);
	NCB_METHOD(isNullContext);
	NCB_METHOD(equalStruct);
	NCB_METHOD(equalStructNumericLoose);
	RawCallback(TJS_W("foreach"), &ScriptsAdd::foreach, TJS_STATICMEMBER);
	RawCallback(TJS_W("getMD5HashString"), &ScriptsAdd::getMD5HashString, TJS_STATICMEMBER);
	NCB_METHOD(clone);

	RawCallback("propSet", &ScriptsAdd::propSet, TJS_STATICMEMBER);
	RawCallback("propGet", &ScriptsAdd::propGet, TJS_STATICMEMBER);
	Variant(TJS_W("pfMemberEnsure"),    TJS_MEMBERENSURE);
	Variant(TJS_W("pfMemberMustExist"), TJS_MEMBERMUSTEXIST);
	Variant(TJS_W("pfIgnoreProp"),      TJS_IGNOREPROP);
	Variant(TJS_W("pfHiddenMember"),    TJS_HIDDENMEMBER);
	Variant(TJS_W("pfStaticMember"),    TJS_STATICMEMBER);

	RawCallback(TJS_W("safeEvalStorage"), &ScriptsAdd::safeEvalStorage, TJS_STATICMEMBER);

};

NCB_ATTACH_FUNCTION(rehash, Scripts, TJSDoRehash);
