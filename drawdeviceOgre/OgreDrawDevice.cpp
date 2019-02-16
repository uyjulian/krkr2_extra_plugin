#include <windows.h>

#include "OgreDrawDevice.h"

/**
 * �R���X�g���N�^
 */
tTVPOgreDrawDevice::tTVPOgreDrawDevice(OgreInfo *info)
{
	ogreInfo = info;
	_renderWindow = NULL;
	_sceneManager = ogreInfo->root->createSceneManager(ST_GENERIC, "ExampleSMInstance");
}

/**
 * �f�X�g���N�^
 */
tTVPOgreDrawDevice::~tTVPOgreDrawDevice()
{
	detach();
}

/**
 * �E�C���h�E�̍Đݒ�
 * @param hwnd �n���h��
 */
void
tTVPOgreDrawDevice::attach(HWND hwnd)
{
	// �T�C�Y���
	RECT rect;
	GetClientRect(hwnd, &rect);
	width  = rect.right - rect.left;
	height = rect.bottom - rect.top;

	// �n���h���𕶎���
	char hwndName[100];
	snprintf(hwndName, sizeof hwndName, "%d", hwnd);

	// �E�C���h�E����
	windowName = "window";
	windowName += hwndName;
	
	// �������p�����[�^
	NameValuePairList params;
	params["parentWindowHandle"] = hwndName;
	params["left"] = "0";
	params["top"] = "0";
	
	// �E�C���h�E����
	_renderWindow = ogreInfo->root->createRenderWindow(windowName,
													   width,
													   height,
													   false,
													   &params);

	// XX �e�X�g�p
	init();
	
	// ogre �쓮�J�n
	ogreInfo->start();
}

/**
 * �E�C���h�E�̉���
 */
void
tTVPOgreDrawDevice::detach()
{
	ogreInfo->stop();
	if (_renderWindow) {
		_sceneManager->destroyAllCameras();
		_renderWindow->removeAllViewports();
		_renderWindow->removeAllListeners();
		ogreInfo->root->getRenderSystem()->destroyRenderWindow(windowName);
		_renderWindow = NULL;
	}
}

/***
 * �E�C���h�E�̎w��
 * @param wnd �E�C���h�E�n���h��
 */
void TJS_INTF_METHOD
tTVPOgreDrawDevice::SetTargetWindow(HWND wnd)
{
	detach();
	if (wnd != NULL) {
		attach(wnd);
	}
}

/**
 * �r�b�g�}�b�v�R�s�[�����J�n
 */
void TJS_INTF_METHOD
tTVPOgreDrawDevice::StartBitmapCompletion(iTVPLayerManager * manager)
{
	// bitmap�����J�n
}

/**
 * �r�b�g�}�b�v�R�s�[����
 */
void TJS_INTF_METHOD
tTVPOgreDrawDevice::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	// bits, bitmapinfo �ŕ\�����r�b�g�}�b�v�� cliprect �̗̈���Ax, y �ɕ`�悷��B
}

/**
 * �r�b�g�}�b�v�R�s�[�����I��
 */
void TJS_INTF_METHOD
tTVPOgreDrawDevice::EndBitmapCompletion(iTVPLayerManager * manager)
{
	// bitmap �����I��
}

//---------------------------------------------------------------------------

/**
 * �e�X�g�p����������
 */
void
tTVPOgreDrawDevice::init()
{
	// �J����������
	Camera *camera = _sceneManager->createCamera("Player");
	camera->setPosition(Vector3(0,0,500));
	camera->lookAt(Vector3(0,0,-300));
	camera->setNearClipDistance(5);

	// �r���[�|�[�g������
	Viewport* vp = _renderWindow->addViewport(camera);
	vp->setBackgroundColour(ColourValue(0,0,0));

	// ���\�[�X������
	ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	
	// �e�X�g�p�ɃI�u�W�F�N�g��z�u���Ă݂�
	_sceneManager->setAmbientLight(ColourValue(0.5, 0.5, 0.5));
	Entity *ent = _sceneManager->createEntity("head", "ogrehead.mesh");
	_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(ent);
	
	// Green nimbus around Ogre
	ParticleSystem* pSys1 = _sceneManager->createParticleSystem("Nimbus", 
																"Examples/GreenyNimbus");
	_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(pSys1);
	
	// Create a rainstorm 
	ParticleSystem* pSys4 = _sceneManager->createParticleSystem("rain", 
																"Examples/Rain");
	SceneNode* rNode = _sceneManager->getRootSceneNode()->createChildSceneNode();
	rNode->translate(0,1000,0);
	rNode->attachObject(pSys4);
	// Fast-forward the rain so it looks more natural
	pSys4->fastForward(5);
	
	// Aureola around Ogre perpendicular to the ground
	ParticleSystem* pSys5 = _sceneManager->createParticleSystem("Aureola", 
																"Examples/Aureola");
	_sceneManager->getRootSceneNode()->createChildSceneNode()->attachObject(pSys5);
	
	// Set nonvisible timeout
	ParticleSystem::setDefaultNonVisibleUpdateTimeout(5);
}
