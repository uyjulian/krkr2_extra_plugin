#include "layerExCairo.hpp"
#include "ncbind/ncbind.hpp"

// ----------------------------------- �N���X�̓o�^

NCB_GET_INSTANCE_HOOK(layerExCairo)
{
	// �C���X�^���X�Q�b�^
	NCB_INSTANCE_GETTER(objthis) { // objthis �� iTJSDispatch2* �^�̈����Ƃ���
		
		ClassT* obj = GetNativeInstance(objthis);	// �l�C�e�B�u�C���X�^���X�|�C���^�擾
		if (!obj) {
			obj = new ClassT(objthis);				// �Ȃ��ꍇ�͐�������
			SetNativeInstance(objthis, obj);		// objthis �� obj ���l�C�e�B�u�C���X�^���X�Ƃ��ēo�^����
		}
		if (obj) obj->reset();						// ���\�b�h���ĂԑO�ɕK���Ă΂��
		return (_obj = obj);						//< �f�X�g���N�^�Ŏg�p�������ꍇ�̓v���C�x�[�g�ϐ��ɕۑ�
	}

	// �f�X�g���N�^�i���ۂ̃��\�b�h���Ă΂ꂽ��ɌĂ΂��j
	~NCB_GET_INSTANCE_HOOK_CLASS () {
		if (_obj) _obj->redraw();					// ���\�b�h���Ă񂾌�ɕK���Ă΂��
	}

private:
	ClassT *_obj;
}; // ���̂� class ��`�Ȃ̂� ; ��Y��Ȃ��ł�

// �t�b�N���A�^�b�`
NCB_ATTACH_CLASS_WITH_HOOK(layerExCairo, Layer) {
}

// ----------------------------------- �N���E�J������

/**
 * �o�^�����O
 */
void PreRegistCallback()
{
}

/**
 * �o�^������
 */
void PostRegistCallback()
{
}

/**
 * �J�������O
 */
void PreUnregistCallback()
{
}

/**
 * �J��������
 */
void PostUnregistCallback()
{
}

NCB_PRE_REGIST_CALLBACK(   PreRegistCallback);
NCB_POST_REGIST_CALLBACK(  PostRegistCallback);
NCB_PRE_UNREGIST_CALLBACK( PreUnregistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
