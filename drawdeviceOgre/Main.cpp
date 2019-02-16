#include "ncbind/ncbind.hpp"
#include "OgreDrawDevice.h"

// Ogre ��{���
static OgreInfo *ogreInfo = NULL;

/**
 * Ogre �x�[�X DrawDevice �̃N���X
 */
class OgreDrawDevice {
public:
	// �f�o�C�X���
	iTVPDrawDevice *device;
public:
	/**
	 * �R���X�g���N�^
	 */
	OgreDrawDevice() {
		device = new tTVPOgreDrawDevice(ogreInfo);
	}

	/**
	 * �f�X�g���N�^
	 */
	~OgreDrawDevice() {
		if (device) {
			device->Destruct();
			device = NULL;
		}
	}
	
	/**
	 * @return �f�o�C�X���
	 */
	tjs_int64 GetDevice() {
		return reinterpret_cast<tjs_int64>(device);
	}

	// ---------------------------------------------
	// �ȉ� Ogre �𐧌䂷�邽�߂̋@�\�������ǉ��\��
	// ---------------------------------------------
	
};


NCB_REGISTER_CLASS(OgreDrawDevice) {
	NCB_CONSTRUCTOR(());
	NCB_PROPERTY_RO(interface, GetDevice);
}

/**
 * �o�^�����O
 */
static void
PreRegistCallback()
{
	// OGRE �̊�{��񐶐�
	ogreInfo = new OgreInfo();
	if (!ogreInfo->config()) {
		delete ogreInfo;
		ogreInfo = NULL;
		TVPThrowExceptionMessage(TJS_W("can't init OGRE."));
	}
}

/**
 * �J��������
 */
static void PostUnregistCallback()
{
	// ogre �I��
	delete ogreInfo;
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
