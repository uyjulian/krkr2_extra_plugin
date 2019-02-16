#pragma comment(lib, "strmiids.lib")
#include "layerExMovie.hpp"
#include "ncbind/ncbind.hpp"

// ----------------------------------- �N���X�̓o�^

NCB_GET_INSTANCE_HOOK(layerExMovie)
{
	// �C���X�^���X�Q�b�^
	NCB_INSTANCE_GETTER(objthis) { // objthis �� iTJSDispatch2* �^�̈����Ƃ���
		ClassT* obj = GetNativeInstance(objthis);	// �l�C�e�B�u�C���X�^���X�|�C���^�擾
		if (!obj) {
			obj = new ClassT(objthis);				// �Ȃ��ꍇ�͐�������
			SetNativeInstance(objthis, obj);		// objthis �� obj ���l�C�e�B�u�C���X�^���X�Ƃ��ēo�^����
		}
		return obj;
	}

	// �f�X�g���N�^�i���ۂ̃��\�b�h���Ă΂ꂽ��ɌĂ΂��j
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};


// �t�b�N���A�^�b�`
NCB_ATTACH_CLASS_WITH_HOOK(layerExMovie, Layer) {
	NCB_METHOD(openMovie);
	NCB_METHOD(startMovie);
	NCB_METHOD(stopMovie);
	NCB_METHOD(isPlayingMovie);
}

// ----------------------------------- �N���E�J������

static bool coInitialized;

/**
 * �o�^�����O
 */
void PreRegistCallback()
{
	coInitialized = SUCCEEDED(CoInitialize(0));
	if (coInitialized) {
		TVPAddLog("����������");
	} else {
		TVPAddLog("���������s");
	}
}

/**
 * �J��������
 */
void PostUnregistCallback()
{
	if (coInitialized) {
		CoUninitialize();
	}
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
