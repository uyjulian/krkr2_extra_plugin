#include "IrrlichtDrawDevice.h"
#include "LayerManagerInfo.h"

extern void message_log(const char* format, ...);
extern void error_log(const char *format, ...);

using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace io;
using namespace gui;

/**
 * �R���X�g���N�^
 */
IrrlichtDrawDevice::IrrlichtDrawDevice(iTJSDispatch2 *objthis, int width, int height)
	: IrrlichtBase(objthis), width(width), height(height), destWidth(0), destHeight(0), zoomMode(true), defaultVisible(true)
{
	// Irrlicht�I��ʃT�C�Y
	screenWidth = width;
	screenHeight = height;
	screenRect = rect<s32>(0,0,screenWidth,screenHeight);
}

/**
 * �f�X�g���N�^
 */
IrrlichtDrawDevice::~IrrlichtDrawDevice()
{
	stop();
	detach();
}

/**
 * �����t�@�N�g��
 */
tjs_error
IrrlichtDrawDevice::Factory(IrrlichtDrawDevice **obj, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	if (numparams < 2) {
		return TJS_E_BADPARAMCOUNT;
	}
	*obj = new IrrlichtDrawDevice(objthis, (tjs_int)*param[0], (tjs_int)*param[1]);
	return TJS_S_OK;
}

// -----------------------------------------------------------------------
// Continuous
// -----------------------------------------------------------------------

/**
 * Continuous �R�[���o�b�N
 * �g���g�����ɂȂƂ��ɏ�ɌĂ΂��
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::OnContinuousCallback(tjs_uint64 tick)
{
	Window->RequestUpdate();
}

void
IrrlichtDrawDevice::onAttach()
{
	if (device) {
		Window->NotifySrcResize(); // ������ĂԂ��Ƃ� GetSrcSize(), SetDestRectangle() �̌ĂѕԂ�������
		// �}�l�[�W���ɑ΂���e�N�X�`���̊��蓖��
		IVideoDriver *driver = device->getVideoDriver();
		if (driver) {
			for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
				iTVPLayerManager *manager = *i;
				LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
				if (info != NULL) {
					info->alloc(manager, driver);
				}
			}
		}
	}
}

void
IrrlichtDrawDevice::onDetach()
{
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
		if (info) {
			info->free();
		}
	}
}

/**
 * Device��Irrlicht�����̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
IrrlichtDrawDevice::transformToIrrlicht(tjs_int &x, tjs_int &y)
{
	x = screenWidth  ? (x * destWidth  / screenWidth) : 0;
	y = screenHeight ? (y * destHeight / screenHeight) : 0;
}

/** Irrlicht��Device�����̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
IrrlichtDrawDevice::transformFromIrrlicht(tjs_int &x, tjs_int &y)
{
	x = destWidth  ? (x * screenWidth  / destWidth) : 0;
	y = destHeight ? (y * screenHeight / destHeight) : 0;
}

/**
 * Device���v���C�}�����C���̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
IrrlichtDrawDevice::transformToManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	// �v���C�}�����C���}�l�[�W���̃v���C�}�����C���̃T�C�Y�𓾂�
	tjs_int pl_w, pl_h;
	manager->GetPrimaryLayerSize(pl_w, pl_h);
	x = destWidth  ? (x * pl_w / destWidth) : 0;
	y = destHeight ? (y * pl_h / destHeight) : 0;
}

/** �v���C�}�����C����Device�����̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
IrrlichtDrawDevice::transformFromManager(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	// �v���C�}�����C���}�l�[�W���̃v���C�}�����C���̃T�C�Y�𓾂�
	tjs_int pl_w, pl_h;
	manager->GetPrimaryLayerSize(pl_w, pl_h);
	x = pl_w ? (x * destWidth  / pl_w) : 0;
	y = pl_h ? (y * destHeight / pl_h) : 0;
}

/**
 * Device���W����ʂ̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� DestRect�� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
IrrlichtDrawDevice::transformTo(tjs_int &x, tjs_int &y)
{
	x = destWidth  ? (x * width / destWidth) : 0;
	y = destHeight ? (y * height / destHeight) : 0;
}

/** �W����ʁ�Device�����̍��W�̕ϊ����s��
 * @param		x		X�ʒu
 * @param		y		Y�ʒu
 * @note		x, y �� ���C���� (0,0) �����_�Ƃ�����W�Ƃ��ēn�����ƌ��Ȃ�
 */
void
IrrlichtDrawDevice::transformFrom(tjs_int &x, tjs_int &y)
{
	// �v���C�}�����C���}�l�[�W���̃v���C�}�����C���̃T�C�Y�𓾂�
	x = width ? (x * destWidth  / width) : 0;
	y = height ? (y * destHeight / height) : 0;
}

/**
 * �ŗL�X�V����
 */
void
IrrlichtDrawDevice::update(irr::video::IVideoDriver *driver)
{
	// �ʃ��C���}�l�[�W���̕`��
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		LayerManagerInfo *info = (LayerManagerInfo*)(*i)->GetDrawDeviceData();
		if (info) {
			info->draw(driver, screenRect);
		}
	}
}


/**
 * ���C���}�l�[�W���̓o�^
 * @param manager ���C���}�l�[�W��
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::AddLayerManager(iTVPLayerManager * manager)
{
	int id = (int)Managers.size();
	tTVPDrawDevice::AddLayerManager(manager);
	LayerManagerInfo *info = new LayerManagerInfo(id, defaultVisible);
	manager->SetDrawDeviceData((void*)info);
}

/**
 * ���C���}�l�[�W���̍폜
 * @param manager ���C���}�l�[�W��
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::RemoveLayerManager(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info != NULL) {
		manager->SetDrawDeviceData(NULL);
		delete info;
	}
	tTVPDrawDevice::RemoveLayerManager(manager);
}

/***
 * �E�C���h�E�̎w��
 * @param wnd �E�C���h�E�n���h��
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::SetTargetWindow(HWND wnd, bool is_main)
{
	stop();
	detach();
	if (wnd != NULL) {
		attach(wnd, screenWidth, screenHeight);
		start();
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::SetDestRectangle(const tTVPRect &dest)
{
	destRect.setLeft(dest.left);
	destRect.setTop(dest.top);
	destRect.setWidth((destWidth = dest.get_width()));
	destRect.setHeight((destHeight = dest.get_height()));
	if (device) {
		IVideoDriver *driver = device->getVideoDriver();
		if (driver) {
			tjs_int w, h;
			if (zoomMode) {
				w = width;
				h = height;
			} else {
				w = destWidth;
				h = destHeight;
			}
			if (screenWidth != w ||	screenHeight != h) {
				screenWidth = w;
				screenHeight = h;
				screenRect = rect<s32>(0,0,screenWidth,screenHeight);
				driver->OnResize(dimension2d<s32>(w, h));
			}
		}
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::GetSrcSize(tjs_int &w, tjs_int &h)
{
	w = width;
	h = height;
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::NotifyLayerResize(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info != NULL) {
		info->free();
		if (device) {
			IVideoDriver *driver = device->getVideoDriver();
			if (driver) {
				info->alloc(manager, driver);
			}
		}
	}
}

// -------------------------------------------------------------------------------------
// ���̓C�x���g�����p
// -------------------------------------------------------------------------------------

void TJS_INTF_METHOD
IrrlichtDrawDevice::OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags)
{
	// Irrlicht �ɑ���
	if (device) {
		tjs_int dx = x;
		tjs_int dy = y;
		transformToIrrlicht(dx, dy);
		SEvent ev;
		ev.EventType = EET_MOUSE_INPUT_EVENT;
		ev.MouseInput.X = dx;
		ev.MouseInput.Y = dy;
		ev.MouseInput.Wheel = 0;
		switch ((mb & 0xff)) {
		case mbLeft:
			ev.MouseInput.Event = EMIE_LMOUSE_PRESSED_DOWN;
			break;
		case mbMiddle:
			ev.MouseInput.Event = EMIE_MMOUSE_PRESSED_DOWN;
			break;
		case mbRight:
			ev.MouseInput.Event = EMIE_RMOUSE_PRESSED_DOWN;
			break;
		}
		postEvent(ev);
	}
	// �g���g���̃v���C�}�����C���ɑ���
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseDown(x, y, mb, flags);
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags)
{
	// Irrlicht �ɑ���
	if (device) {
		tjs_int dx = x;
		tjs_int dy = y;
		transformToIrrlicht(dx, dy);
		SEvent ev;
		ev.EventType = EET_MOUSE_INPUT_EVENT;
		ev.MouseInput.X = dx;
		ev.MouseInput.Y = dy;
		ev.MouseInput.Wheel = 0;
		switch ((mb & 0xff)) {
		case mbLeft:
			ev.MouseInput.Event = EMIE_LMOUSE_LEFT_UP;
			break;
		case mbMiddle:
			ev.MouseInput.Event = EMIE_MMOUSE_LEFT_UP;
			break;
		case mbRight:
			ev.MouseInput.Event = EMIE_RMOUSE_LEFT_UP;
			break;
		}
		postEvent(ev);
	}
	// �g���g���̃v���C�}�����C���ɑ���
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseUp(x, y, mb, flags);
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags)
{
	// Irrlicht �ɑ���
	if (device) {
		tjs_int dx = x;
		tjs_int dy = y;
		transformToIrrlicht(dx, dy);
		SEvent ev;
		ev.EventType = EET_MOUSE_INPUT_EVENT;
		ev.MouseInput.X = dx;
		ev.MouseInput.Y = dy;
		ev.MouseInput.Wheel = 0;
		ev.MouseInput.Event = EMIE_MOUSE_MOVED;
		postEvent(ev);
	}
	// �g���g���̃v���C�}�����C���ɑ���
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseMove(x, y, flags);
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y)
{
	// Irrlicht �ɑ���
	if (device) {
		tjs_int dx = x;
		tjs_int dy = y;
		transformToIrrlicht(dx, dy);
		SEvent ev;
		ev.EventType = EET_MOUSE_INPUT_EVENT;
		ev.MouseInput.X = dx;
		ev.MouseInput.Y = dy;
		ev.MouseInput.Wheel = (f32)delta;
		ev.MouseInput.Event = EMIE_MOUSE_WHEEL;
		postEvent(ev);
	}
	// �g���g���̃v���C�}�����C���ɑ���
	iTVPLayerManager * manager = GetLayerManagerAt(PrimaryLayerManagerIndex);
	if (manager) {
		transformToManager(manager, x, y);
		manager->NotifyMouseWheel(shift, delta, x, y);
	}
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::GetCursorPos(iTVPLayerManager * manager, tjs_int &x, tjs_int &y)
{
	Window->GetCursorPos(x, y);
	transformToManager(manager, x, y);
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::SetCursorPos(iTVPLayerManager * manager, tjs_int x, tjs_int y)
{
	transformFromManager(manager, x, y);
	Window->SetCursorPos(x, y);
}

void TJS_INTF_METHOD
IrrlichtDrawDevice::RequestInvalidation(const tTVPRect & rect)
{
	for (std::vector<iTVPLayerManager *>::iterator i = Managers.begin(); i != Managers.end(); i++) {
		iTVPLayerManager *manager = *i;
		LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
		if (info && info->visible) {
			tjs_int l = rect.left, t = rect.top, r = rect.right, b = rect.bottom;
			transformToManager(manager, l, t);
			transformToManager(manager, r, b);
			r ++; // �덷�̋z��(�{���͂���������ƌ����ɂ��Ȃ��ƂȂ�Ȃ������ꂪ���ɂȂ邱�Ƃ͂Ȃ�)
			b ++;
			manager->RequestInvalidation(tTVPRect(l, t, r, b));
		}
	}
}


// -------------------------------------------------------------------------------------
// �ĕ`�揈���p
// -------------------------------------------------------------------------------------

void
IrrlichtDrawDevice::Show()
{
	show(&destRect);
}

// -------------------------------------------------------------------------------------
// LayerManager����̉摜�����킽��
// -------------------------------------------------------------------------------------

/**
 * �r�b�g�}�b�v�R�s�[�����J�n
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::StartBitmapCompletion(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->lock();
	}
}

/**
 * �r�b�g�}�b�v�R�s�[����
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::NotifyBitmapCompleted(iTVPLayerManager * manager,
	tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
	const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->copy(x, y, bits, bitmapinfo, cliprect, type, opacity);
	}
}

/**
 * �r�b�g�}�b�v�R�s�[�����I��
 */
void TJS_INTF_METHOD
IrrlichtDrawDevice::EndBitmapCompletion(iTVPLayerManager * manager)
{
	LayerManagerInfo *info = (LayerManagerInfo*)manager->GetDrawDeviceData();
	if (info) {
		info->unlock();
	}
}

/**
 * �v���C�}�����C���̕\����Ԃ̎w��
 * @param id �v���C�}�����C���̓o�^ID
 * @param visible �\�����
 */
void
IrrlichtDrawDevice::setVisible(int id, bool visible)
{
	if (id >= 0 && id < (int)Managers.size()) {
		LayerManagerInfo *info = (LayerManagerInfo*)Managers[id]->GetDrawDeviceData();
		if (info) {
			info->visible = visible;
		}
	}
}

/**
 * �v���C�}�����C���̕\����Ԃ̎w��
 * @param id �v���C�}�����C���̓o�^ID
 * @return visible �\�����
 */
bool
IrrlichtDrawDevice::getVisible(int id)
{
	if (id >= 0 && id < (int)Managers.size()) {
		LayerManagerInfo *info = (LayerManagerInfo*)Managers[id]->GetDrawDeviceData();
		if (info) {
			return info->visible;
		}
	}
	return false;
}
