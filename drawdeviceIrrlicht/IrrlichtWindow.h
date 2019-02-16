#ifndef IRRLICHTWINDOW_H
#define IRRLICHTWINDOW_H

#include "IrrlichtBase.h"

extern void registerWindowClass();
extern void unregisterWindowClass();

/**
 * Irrlicht �`�悪�\�ȃE�C���h�E
 */
class IrrlichtWindow :	public IrrlichtBase
{
public:
	// �E�C���h�E�v���V�[�W��
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

protected:
	HWND parent; //< �e��(TScrollBox)�̃n���h��
	HWND hwnd;   //< ���݂̃n���h��
	iTJSDispatch2 *window; //< �I�u�W�F�N�g���̎Q��

	// �C�x���g����
	static bool __stdcall messageHandler(void *userdata, tTVPWindowMessage *Message);

	// ���[�U���b�Z�[�W���V�[�o�̓o�^/����
	void setReceiver(tTVPWindowMessageReceiver receiver, bool enable);

	/**
	 * �E�C���h�E�𐶐�
	 * @param krkr �g���g���̃E�C���h�E
	 */
	void createWindow(HWND krkr);

	/**
	 * �E�C���h�E��j��
	 */
	void destroyWindow();

	/**
	 * �g���g�����Ƀ��b�Z�[�W�𑗕t
	 * @param message ���b�Z�[�W
	 * @param wParam WPARAM
	 * @param lParam LPARAM
	 * @param convPosition lParam �̃}�E�X���W�l��e�̂��̂ɕϊ�����
	 */
	void sendMessage(UINT message, WPARAM wParam, LPARAM lParam, bool convPosition=false);
	
public:
	bool transparentEvent; //< �C�x���g����
	

	/**
	 * �R���X�g���N�^
	 */
	IrrlichtWindow(iTJSDispatch2 *objthis, iTJSDispatch2 *win, int left, int top, int width, int height);
		
	/**
	 * �f�X�g���N�^
	 */
	virtual ~IrrlichtWindow();

	// -----------------------------------------------------------------------
	// �����t�@�N�g��
	// -----------------------------------------------------------------------

	static tjs_error Factory(IrrlichtWindow **obj, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
	
	// -----------------------------------------------------------------------
	// continuous handler
	// -----------------------------------------------------------------------
public:
	/**
	 * Continuous �R�[���o�b�N
	 */
	virtual void TJS_INTF_METHOD OnContinuousCallback(tjs_uint64 tick);
	
	// -----------------------------------------------------------------------
	// ���ʃ��\�b�h�Ăяo���p
	// -----------------------------------------------------------------------

public:
	void setEventMask(int mask) {
		IrrlichtBase::setEventMask(mask);
	}

	int getEventMask() {
		return IrrlichtBase::getEventMask();
	}

	irr::video::IVideoDriver *getVideoDriver() {
		return IrrlichtBase::getVideoDriver();
	}

	irr::scene::ISceneManager *getSceneManager() {
		return IrrlichtBase::getSceneManager();
	}

	irr::gui::IGUIEnvironment *getGUIEnvironment() {
		return IrrlichtBase::getGUIEnvironment();
	}

	irr::ILogger *getLogger() {
		return IrrlichtBase::getLogger();
	}
	
	irr::io::IFileSystem *getFileSystem() {
		return IrrlichtBase::getFileSystem();
	}

	// -----------------------------------------------------------------------
	// �ŗL���\�b�h
	// -----------------------------------------------------------------------
	
protected:
	bool visible;
	int left;
	int top;
	int width;
	int height;

	void _setPos();
	
public:
	void setVisible(bool v);
	bool getVisible();

	void setLeft(int l);
	int getLeft();

	void setTop(int t);
	int getTop();
	
	void setWidth(int w);
	int getWidth();

	void setHeight(int h);
	int getHeight();
	
	/**
	 * ���ꏊ�w��
	 */	
	void setPos(int l, int t);

	/**
	 * ���T�C�Y�w��
	 */	
	void setSize(int w, int h);
};

#endif
