#ifndef OGREINFO_H
#define OGREINFO_H

#include <windows.h>
#include "tp_stub.h"

#include "Ogre.h"
#include "OgreConfigFile.h"
using namespace Ogre;

/**
 * Ogre ��{���N���X
 * �g���g���̃��C�����[�v����펞��΂�邱�Ƃ�z��
 */
class OgreInfo : public tTVPContinuousEventCallbackIntf
{
public:
	// ���[�g���
	Ogre::Root *root;

	/**
	 * �R���X�g���N�^
	 */
	OgreInfo();

	/**
	 * �f�X�g���N�^
	 */
	virtual ~OgreInfo();

public:

	/**
	 * Ogre Config �Ăяo��
	 */
	bool config();

	/**
	 * Ogre �Ăяo�������J�n
	 */
	void start();

	/**
	 * Ogre �Ăяo���������f
	 */
	void stop();
	
	/**
	 * Continuous �R�[���o�b�N
	 * �g���g�����ɂȂƂ��ɏ�ɌĂ΂��
	 * �h�蒼������
	 */
	virtual void TJS_INTF_METHOD OnContinuousCallback(tjs_uint64 tick);
};

#endif
