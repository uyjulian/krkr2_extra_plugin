#ifndef IrrlichtSimpleDevice_H
#define IrrlichtSimpleDevice_H

#include "IrrlichtBase.h"

/**
 * Irrlicht �`��f�o�C�X
 */
class IrrlichtSimpleDevice : public IrrlichtBase
{

protected:
	HWND hwnd; //< �g���g���̃E�C���h�E�n���h��
	iTJSDispatch2 *window; //< �I�u�W�F�N�g���̎Q��(�e�E�C���h�E)
	tjs_int width;  //< Irrlicht ����ʂ̉�ʉ���
	tjs_int height; //< Irrlicht ����ʂ̉�ʏc��
	bool useRender; //< �����_�[�^�[�Q�b�g���g��
	irr::video::ITexture *target; //< �����_�[�^�[�Q�b�g

	tjs_int dwidth;
	tjs_int dheight;
	HBITMAP hbmp; // �`���DIB
	HBITMAP oldbmp;
	HDC destDC; // �`���DC
	void *bmpbuffer;

	void clearDC();
	void updateDC(int dwidth, int dheight);
		
	
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

	/// �f�o�C�X���蓖�Č㏈��
	virtual void onAttach();

	/// �f�o�C�X�j���O����
	virtual void onDetach();

public:
	/**
	 * �R���X�g���N�^
	 * @param widow �e�ɂȂ�E�C���h�E
	 * @param width ����
	 * @param height �c��
	 * @param useRender �����_�[�^�[�Q�b�g���g��(�����L��)
	 */
	IrrlichtSimpleDevice(iTJSDispatch2 *objthis, iTJSDispatch2 *window, int width, int height, bool useRender);
		
	/**
	 * �f�X�g���N�^
	 */
	virtual ~IrrlichtSimpleDevice();

	// -----------------------------------------------------------------------
	// �����t�@�N�g��
	// -----------------------------------------------------------------------

	static tjs_error Factory(IrrlichtSimpleDevice **obj, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);

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
	void _setSize();
	
public:
	void setWidth(int w) {
		if (width != w) {
			width = w;
			_setSize();
		}
	}

	int getWidth() {
		return width;
	}

	void setHeight(int h) {
		if (height != h) {
			height = h;
			_setSize();
		}
	}

	int getHeight() {
		return height;
	}
	
	/**
	 * ���T�C�Y�w��
	 */	
	void setSize(int w, int h) {
		if (width != w || height != h) {
			width = w;
			height = h;
			_setSize();
		}
	}

	/**
	 * ���C���ɑ΂��čX�V�`��
	 * �o�b�N�o�b�t�@����R�s�[���܂��B
	 * @param layer ���C��
	 * @param srcRect �\�[�X�̈�
	 */
	void _updateToLayer(iTJSDispatch2 *layer, irr::core::rect<irr::s32> *srcRect = NULL);

	/**
	 * ���C���ɑ΂��čX�V�`��(�Ăяo���p)
	 */
	static tjs_error updateToLayer(tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
};

#endif
