#ifndef OGREDRAWDEVICE_H
#define OGREDRAWDEVICE_H

#include "OgreInfo.h"
#include <string>
using namespace std;

#include "BasicDrawDevice.h"

/**
 * Ogre �x�[�X�� DrawDevice
 */
class tTVPOgreDrawDevice : public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;

	/// OGRE �֌W���
	OgreInfo *ogreInfo;
	string windowName;
	RenderWindow* _renderWindow;
	SceneManager* _sceneManager;
	
public:
	tTVPOgreDrawDevice(OgreInfo *info); //!< �R���X�g���N�^
private:
	virtual ~tTVPOgreDrawDevice(); //!< �f�X�g���N�^

	void attach(HWND hwnd);
	void detach();

	int width;
	int height;

	unsigned char *destBuffer;
	int destWidth;
	int destHeight;
	int destPitch;
	
public:

	//---- �`��ʒu�E�T�C�Y�֘A
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd);

	//---- LayerManager ����̉摜�󂯓n���֘A
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);

	//---------------------------------------------------------------------------

	// �e�X�g�p����
	void init();
};

#endif
