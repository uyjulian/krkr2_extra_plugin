#ifndef IRRLICHTDRAWDEVICE_H
#define IRRLICHTDRAWDEVICE_H

#include "IrrlichtBase.h"
#include "BasicDrawDevice.h"

/**
 * Irrlicht �x�[�X�� DrawDevice
 */
class IrrlichtDrawDevice : public IrrlichtBase, public tTVPDrawDevice
{
	typedef tTVPDrawDevice inherited;

protected:
	bool zoomMode;
	tjs_int width;        //< ���[�U�w��̉�ʉ���
	tjs_int height;       //< ���[�U�w��̉�ʏc��
	tjs_int destWidth;    //< ���`��̈�̉���
	tjs_int destHeight;   //< ���`��̈�̏c��

	tjs_int screenWidth;  //< Irrlicht ����ʂ̉�ʉ���
	tjs_int screenHeight; //< Irrlicht ����ʂ̉�ʏc��
	irr::core::rect<irr::s32> screenRect;
	irr::core::rect<irr::s32> destRect;

public:
	// �R���X�g���N�^
	IrrlichtDrawDevice(iTJSDispatch2 *objthis, int width, int height);
	// �f�X�g���N�^
	virtual ~IrrlichtDrawDevice();

	// -----------------------------------------------------------------------
	// �����t�@�N�g��
	// -----------------------------------------------------------------------

	static tjs_error Factory(IrrlichtDrawDevice **obj, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis);
	
	// -----------------------------------------------------------------------
	// continuous handler
	// -----------------------------------------------------------------------
public:
	/**
	 * Continuous �R�[���o�b�N
	 */
	virtual void TJS_INTF_METHOD OnContinuousCallback(tjs_uint64 tick);
	
protected:
	// �f�o�C�X���蓖�Č㏈��
	virtual void onAttach();

	// �f�o�C�X����O����
	virtual void onDetach();
	
	/**
	 * Device��Irrlicht�����̍��W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformToIrrlicht(tjs_int &x, tjs_int &y);

	/** Irrlicht��Device�����̍��W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformFromIrrlicht(tjs_int &x, tjs_int &y);

	/**
	 * Device�����C���}�l�[�W���̍��W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformToManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);

	/** ���C���}�l�[�W����Device�����̍��W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformFromManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);

	/**
	 * Device���W�����W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformTo(tjs_int &x, tjs_int &y);
	
	/** �W�����W��Device�����̍��W�̕ϊ����s��
	 * @param		x		X�ʒu
	 * @param		y		Y�ʒu
	 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
	 */
	void transformFrom(tjs_int &x, tjs_int &y);
	
	// ------------------------------------------------------------
	// �X�V����
	// ------------------------------------------------------------
protected:
	/**
	 * �N���X�ŗL�X�V����
	 * �V�[���}�l�[�W���̏�����AGUI�̏����O�ɌĂ΂��
	 */
	void update(irr::video::IVideoDriver *driver);
	
public:
	//---- LayerManager �̊Ǘ��֘A
	virtual void TJS_INTF_METHOD AddLayerManager(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD RemoveLayerManager(iTVPLayerManager * manager);

	//---- �`��ʒu�E�T�C�Y�֘A
	virtual void TJS_INTF_METHOD SetTargetWindow(HWND wnd, bool is_main);
	virtual void TJS_INTF_METHOD SetDestRectangle(const tTVPRect & rect);
	virtual void TJS_INTF_METHOD GetSrcSize(tjs_int &w, tjs_int &h);
	virtual void TJS_INTF_METHOD NotifyLayerResize(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyLayerImageChange(iTVPLayerManager * manager) {}

	//---- ���[�U�[�C���^�[�t�F�[�X�֘A
	// window �� drawdevice
	virtual void TJS_INTF_METHOD OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags);
	virtual void TJS_INTF_METHOD OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y);

	virtual void TJS_INTF_METHOD GetCursorPos(iTVPLayerManager * manager, tjs_int &x, tjs_int &y);
	virtual void TJS_INTF_METHOD SetCursorPos(iTVPLayerManager * manager, tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD RequestInvalidation(const tTVPRect & rect);
	
	//---- �ĕ`��֘A
	virtual void TJS_INTF_METHOD Show();
	
	//---- LayerManager ����̉摜�󂯓n���֘A
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager);
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager);

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
	
public:
	/**
	 * @return �f�o�C�X���
	 */
	tjs_int64 getDevice() {
		return reinterpret_cast<tjs_int64>((tTVPDrawDevice*)this);
	}

	bool getZoomMode() {
		return zoomMode;
	}
	
	void setZoomMode(bool zoomMode) {
		this->zoomMode = zoomMode;
		Window->NotifySrcResize();
	}

	tjs_int getWidth() {
		return width;
	}
	
	void setWidth(tjs_int width) {
		this->width = width;
		Window->NotifySrcResize();
	}

	tjs_int getHeight() {
		return height;
	}
	
	void setHeight(tjs_int height) {
		this->height = height;
		Window->NotifySrcResize();
	}

	void setSize(tjs_int width, tjs_int height) {
		this->width = width;
		this->height = height;
		Window->NotifySrcResize();
	}

	tjs_int getDestWidth() {
		return destWidth;
	}

	tjs_int getDestHeight() {
		return destHeight;
	}
	
protected:
	/*
	 * �v���C�}�����C���̕W���� visible
	 */
	bool defaultVisible;

public:
	void setDefaultVisible(bool visible) {
		defaultVisible = visible;
	}
	
	bool getDefaultVisible() {
		return defaultVisible;
	}

	/**
	 * �v���C�}�����C���̕\����Ԃ̎w��
	 * @param id �v���C�}�����C���̓o�^ID
	 * @param visible �\�����
	 */
	void setVisible(int id, bool visible);

	/**
	 * �v���C�}�����C���̕\����Ԃ̎w��
	 * @param id �v���C�}�����C���̓o�^ID
	 * @return visible �\�����
	 */
	bool getVisible(int id);
	
};

#endif
