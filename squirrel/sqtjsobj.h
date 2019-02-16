#ifndef __SQTJS_OBJ_H_
#define __SQTJS_OBJ_H_

#include <sqobjectclass.h>

/**
 * �g���g���I�u�W�F�N�g��ێ�����squirrel�N���X
 */
class TJSObject : public sqobject::Object, iTJSNativeInstance {

public:
	/**
	 * �������p
	 */
	static void init(HSQUIRRELVM vm);

	/**
	 * �p���p
	 */
	static void done(HSQUIRRELVM vm);

	// ---------------------------------------------------------------

	/**
	 * call �����p�̌�
	 * TJS�C���X�^���X����squirrel�C���X�^���X�̃��\�b�h�𒼐ڌĂяo��
	 */
	static tjs_error call(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
						  tTJSVariant *result,
						  tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
	
	/**
	 * missing �����p�̌�
	 * TJS�C���X�^���X�Ƀ����o�����݂��Ȃ������ꍇ�� squirrel�C���X�^���X���Q�Ƃ���
	 */
	static tjs_error missing(tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
							 tTJSVariant *result,
							 tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);

	// ---------------------------------------------------------------

	
	/**
	 * �X�^�b�N����̋g���g���I�u�W�F�N�g�̎擾
	 * @param v squirrelVM
	 * @param idx �C���f�b�N�X
	 * @param variant �i�[��
	 * @return �i�[���������� true
	 */
	static bool getVariant(HSQUIRRELVM v, SQInteger idx, tTJSVariant *variant);

	/**
	 * �X�^�b�N�ւ̋g���g���I�u�W�F�N�g�̓o�^
	 * @parma variant �I�u�W�F�N�g
	 * @return �o�^���������� true
	 */
	static bool pushVariant(HSQUIRRELVM v, tTJSVariant &variant);

	// ---------------------------------------------------------------
	
	/**
	 * �g���g���N���X���� squirrel �N���X�𐶐�
	 * @param v squirrelVM
	 */
	static SQRESULT createTJSClass(HSQUIRRELVM v);

	// ---------------------------------------------------------------
	
	/**
	 * TJS�I�u�W�F�N�g�p�̃��\�b�h
	 * ����1 �I�u�W�F�N�g
	 * ����2�`�z��
	 * ���R�ϐ�1 ���\�b�h
	 */
	static SQRESULT tjsInvoker(HSQUIRRELVM v);

	/**
	 * TJS�I�u�W�F�N�g�p�̃v���p�e�B�Q�b�^�[
	 * ����1 �I�u�W�F�N�g
	 * ���R�ϐ�1 �v���p�e�B
	 */
	static SQRESULT tjsGetter(HSQUIRRELVM v);

	/**
	 * TJS�I�u�W�F�N�g�p�̃v���p�e�B�Z�b�^�[
	 * ����1 �I�u�W�F�N�g
	 * ����2 �ݒ�l
	 * ���R�ϐ�1 �v���p�e�B
	 */
	static SQRESULT tjsSetter(HSQUIRRELVM v);

	/**
	 * TJS�I�u�W�F�N�g�p�̐ÓI���\�b�h
	 * ����1 �I�u�W�F�N�g
	 * ����2�`�z��
	 * ���R�ϐ�1 �N���X
	 * ���R�ϐ�2 �����o
	 */
	static SQRESULT tjsStaticInvoker(HSQUIRRELVM v);

	/**
	 * TJS�I�u�W�F�N�g�p�̐ÓI�v���p�e�B�Q�b�^�[
	 * ����1 �I�u�W�F�N�g
	 * ���R�ϐ�1 �N���X
	 * ���R�ϐ�2 �v���p�e�B
	 */
	static SQRESULT tjsStaticGetter(HSQUIRRELVM v);

	/**
	 * TJS�I�u�W�F�N�g�p�̐ÓI�v���p�e�B�Z�b�^�[
	 * ����1 �I�u�W�F�N�g
	 * ����2 �ݒ�l
	 * ���R�ϐ�1 �N���X
	 * ���R�ϐ�2 �v���p�e�B
	 */
	static SQRESULT tjsStaticSetter(HSQUIRRELVM v);

	/**
	 * TJS�I�u�W�F�N�g�̗L���m�F
	 * ����1 �I�u�W�F�N�g
	 */
	static SQRESULT tjsIsValid(HSQUIRRELVM v);

	/**
	 * TJS�I�u�W�F�N�g�ɑ΂���I�[�o���C�h����
	 * ����1 �I�u�W�F�N�g
	 * ����2 ���O
	 * ����3 squirrel�N���[�W��
	 */
	static SQRESULT tjsOverride(HSQUIRRELVM v);

protected:
	/**
	 * �R���X�g���N�^
	 * @param v squirrel VM
	 * @param idx �I�u�W�F�N�g�Q�ƌ��C���f�b�N�X
	 * @param instance �o�C���h�Ώۂ�TJS�I�u�W�F�N�g
	 */
	TJSObject(HSQUIRRELVM v, int idx, tTJSVariant &instance);
	
	// �f�X�g���N�^
	~TJSObject();

	/**
	 * �j������
	 */
	void invalidate();
	
	/**
	 * �I�u�W�F�N�g�̃����[�T
	 */
	static SQRESULT release(SQUserPointer up, SQInteger size);

	/**
	 * TJS�I�u�W�F�N�g�̃R���X�g���N�^
	 * TJS�I�u�W�F�N�g�̃R���X�g���N�^
	 * ����1 �I�u�W�F�N�g
	 * ����2�` ����
	 * ���R�ϐ�1 �N���X��
	 */
	static SQRESULT tjsConstructor(HSQUIRRELVM v);

private:
	// NativeClass ID
	static int classId;
	
	// �����ΏۃI�u�W�F�N�g
	tTJSVariant instance;
	
public:
	// NativeInstance �Ή��p�����o
	virtual tjs_error TJS_INTF_METHOD Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	virtual void TJS_INTF_METHOD Invalidate();
	virtual void TJS_INTF_METHOD Destruct();
};

#endif
