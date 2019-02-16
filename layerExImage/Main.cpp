#include "ncbind/ncbind.hpp"

static const char *copyright = 
"----- CxImage Copyright START -----\n"
"CxImage version 7.0.2 07/Feb/2011\n"
"CxImage : Copyright (C) 2001 - 2011, Davide Pizzolato\n"
"Original CImage and CImageIterator implementation are:\n"
"Copyright (C) 1995, Alejandro Aguilar Sierra (asierra(at)servidor(dot)unam(dot)mx)\n"
"----- CxImage Copyright END -----\n";

#include "LayerExImage.h"

// ----------------------------------- �N���X�̓o�^

NCB_GET_INSTANCE_HOOK(layerExImage)
{
	// �C���X�^���X�Q�b�^
	NCB_INSTANCE_GETTER(objthis) { // objthis �� iTJSDispatch2* �^�̈����Ƃ���
		ClassT* obj = GetNativeInstance(objthis);	// �l�C�e�B�u�C���X�^���X�|�C���^�擾
		if (!obj) {
			obj = new ClassT(objthis);				// �Ȃ��ꍇ�͐�������
			SetNativeInstance(objthis, obj);		// objthis �� obj ���l�C�e�B�u�C���X�^���X�Ƃ��ēo�^����
		}
		obj->reset();
		return obj;
	}
	// �f�X�g���N�^�i���ۂ̃��\�b�h���Ă΂ꂽ��ɌĂ΂��j
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};


// �t�b�N���A�^�b�`
NCB_ATTACH_CLASS_WITH_HOOK(layerExImage, Layer) {
	NCB_METHOD(light);
	NCB_METHOD(colorize);
	NCB_METHOD(modulate);
	NCB_METHOD(noise);
	NCB_METHOD(generateWhiteNoise);
	NCB_METHOD(gaussianBlur);
}

void init()
{
	TVPAddImportantLog(ttstr(copyright));
}

NCB_PRE_REGIST_CALLBACK(init);
