#include "OgreInfo.h"

/**
 * �R���X�g���N�^
 */
OgreInfo::OgreInfo()
{
	// Ogre ���[�g�N���X
	root = new Ogre::Root();
	
	// �R���t�B�O������
	ConfigFile cf;
	cf.load("resources.cfg");
	// Go through all sections & settings in the file
	ConfigFile::SectionIterator seci = cf.getSectionIterator();
	
	String secName, typeName, archName;
	while (seci.hasMoreElements()) {
		secName = seci.peekNextKey();
		ConfigFile::SettingsMultiMap *settings = seci.getNext();
		ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i) {
			typeName = i->first;
			archName = i->second;
			ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}
}

/**
 * �f�X�g���N�^
 */
OgreInfo::~OgreInfo()
{
	stop();
	delete root;
}

/**
 * Ogre Config �Ăяo��
 */
bool
OgreInfo::config()
{
	if (root->showConfigDialog()) {
		root->initialise(false);
		return true;
	}
	return false;
}

/**
 * Ogre �Ăяo�������J�n
 */
void
OgreInfo::start()
{
	stop();
	TVPAddContinuousEventHook(this);
}

/**
 * Ogre �Ăяo��������~
 */
void
OgreInfo::stop()
{
	TVPRemoveContinuousEventHook(this);
}

/**
 * Continuous �R�[���o�b�N
 * �g���g�����ɂȂƂ��ɏ�ɌĂ΂��
 */
void TJS_INTF_METHOD
OgreInfo::OnContinuousCallback(tjs_uint64 tick)
{
	root->renderOneFrame();
};
