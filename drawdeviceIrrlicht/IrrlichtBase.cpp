#include "IrrlichtBase.h"
#include "ncbind/ncbind.hpp"

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
IrrlichtBase::IrrlichtBase(iTJSDispatch2 *objthis)
	: objthis(objthis), device(NULL), attached(false),
	  eventMask(EMASK_ATTACH | EMASK_DETACH)
{
}

/**
 * �f�X�g���N�^
 */
IrrlichtBase::~IrrlichtBase()
{
	stop();
	detach();
}

/**
 * �h���C�o�̏��\��
 */
void
IrrlichtBase::showDriverInfo()
{
	IVideoDriver *driver = device->getVideoDriver();
	if (driver) {
		dimension2d<s32> size = driver->getScreenSize();
		message_log("�f�o�C�X������̃X�N���[���T�C�Y:%d, %d", size.Width, size.Height);
		size = driver->getCurrentRenderTargetSize();
		message_log("�f�o�C�X�������RenderTarget��:%d, %d", size.Width, size.Height);
	}
}

/**
 * TJS�C�x���g�Ăяo���B���ȃI�u�W�F�N�g�̊Y�����\�b�h���Ăяo���B
 * @param eventName �C�x���g��
 */
void
IrrlichtBase::sendTJSEvent(const tjs_char *eventName)
{
	tTJSVariant method;
	if (TJS_SUCCEEDED(objthis->PropGet(0, eventName, NULL, &method, objthis))) { // �C�x���g���\�b�h���擾
		if (method.Type() == tvtObject) {
			iTJSDispatch2 *m = method.AsObjectNoAddRef();
			if (TJS_SUCCEEDED(m->IsInstanceOf(0, NULL, NULL, L"Function", m))) { // �t�@���N�V�������ǂ���
				tTJSVariant self(objthis, objthis);
				tTJSVariant *params[] = {&self};
				m->FuncCall(0, NULL, NULL, NULL, 1, params, method.AsObjectThisNoAddRef());
			}
		}
	}
}

/**
 * �f�o�C�X�̊��蓖��
 * @param hwnd �e�E�C���h�E�n���h��
 * @param width �o�b�N�o�b�t�@�T�C�Y����
 * @param height �o�b�N�o�b�t�@�T�C�Y�c��
 */
void
IrrlichtBase::attach(HWND hwnd, int width, int height)
{
	if (!attached && hwnd) {
		// �f�o�C�X����
		SIrrlichtCreationParameters params;
		params.WindowId     = reinterpret_cast<void*>(hwnd);
		params.DriverType    = EDT_DIRECT3D9;
		params.Stencilbuffer = true;
		params.Vsync = true;
		params.EventReceiver = this;
		params.AntiAlias = true;
		if (width != 0 && height != 0) {
			params.WindowSize = core::dimension2d<s32>(width, height);
		}
		if ((device = irr::createDeviceEx(params))) {
			TVPAddLog(L"Irrlicht�f�o�C�X������");
			// �e�N�X�`���̃��������ɂ����Z�e�X�g���s���悤�ɁB
			// device->getSceneManager()->getParameters()->setAttribute(scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);
			showDriverInfo();
			onAttach();
			if ((eventMask & EMASK_ATTACH)) {
				sendTJSEvent(L"onAttach");
			}
		} else {
			TVPThrowExceptionMessage(L"Irrlicht �f�o�C�X�̏������Ɏ��s���܂���");
		}
		attached = true;
	}
}

/**
 * �f�o�C�X�̔j��
 */
void
IrrlichtBase::detach()
{
	if (device) {
		if ((eventMask & EMASK_DETACH)) {
			sendTJSEvent(L"onDetach");
		}
		onDetach();
		device->drop();
		device = NULL;
	}
	attached = false;
}

/**
 * Irrlicht�`�揈��
 * @param destRect �`���̈�
 * @param srcRect �`�挳�̈�
 * @param destDC �`���DC
 * @return �`�悳�ꂽ
 */
bool
IrrlichtBase::show(irr::core::rect<irr::s32> *destRect, irr::core::rect<irr::s32> *srcRect, HDC destDC)
{
	if (device) {
		// ���Ԃ�i�߂� XXX tick ���O������^�����Ȃ����H
		device->getTimer()->tick();
		
		IVideoDriver *driver = device->getVideoDriver();
		// �`��J�n
		if (driver && driver->beginScene(true, true, irr::video::SColor(0,0,0,0))) {
			
			if ((eventMask & EMASK_BEFORE_SCENE)) {
				sendTJSEvent(L"onBeforeScene");
			}
			
			/// �V�[���}�l�[�W���̕`��
			ISceneManager *smgr = device->getSceneManager();
			if (smgr) {
				smgr->drawAll();
			}

			if ((eventMask & EMASK_AFTER_SCENE)) {
				sendTJSEvent(L"onAfterScene");
			}
			
			// �ŗL����
			update(driver);

			if ((eventMask & EMASK_BEFORE_GUI)) {
				sendTJSEvent(L"onBeforeGUI");
			}
			
			// GUI�̕`��
			IGUIEnvironment *gui = device->getGUIEnvironment();
			if (gui) {
				gui->drawAll();
			}

			if ((eventMask & EMASK_AFTER_GUI)) {
				sendTJSEvent(L"onAfterGUI");
			}
			
			// �`�抮��
			driver->endScene(0, srcRect, destRect, destDC);
			return true;
		}
	}
	return false;
};

/**
 * Irrlicht �ւ̃C�x���g���M
 */
bool
IrrlichtBase::postEvent(SEvent &ev)
{
	if (device) {
		if (device->getGUIEnvironment()->postEventFromUser(ev) ||
			device->getSceneManager()->postEventFromUser(ev)) {
			return true;
		}
	}
	return false;
}

/**
 * �C�x���g��
 * HWND ���w�肵�Đ������Ă���֌W�� Irrlicht ���g�̓E�C���h�E����
 * �C�x���g���擾���邱�Ƃ͂Ȃ��B�̂� GUI Environment ����̃C�x���g
 * �����������ɂ��邱�ƂɂȂ�B�����̓K���ȃ��\�b�h���Ăяo���悤�ɗv�C�� XXX
 * @return ���������� true
 */
bool
IrrlichtBase::OnEvent(const irr::SEvent &event)
{
	bool ret = false;
	if ((eventMask & EMASK_EVENT)) {
		tTJSVariant method;
		if (TJS_SUCCEEDED(objthis->PropGet(0, L"onEvent", NULL, &method, objthis))) { // �C�x���g���\�b�h���擾
			if (method.Type() == tvtObject) {
				iTJSDispatch2 *m = method.AsObjectNoAddRef();
				if (TJS_SUCCEEDED(m->IsInstanceOf(0, NULL, NULL, L"Function", m))) { // �t�@���N�V�������ǂ���
					tTJSVariant self(objthis, objthis);
					tTJSVariant ev;

					// SEvent ��ϊ�
					typedef ncbInstanceAdaptor<SEvent> AdaptorT;
					iTJSDispatch2 *adpobj = AdaptorT::CreateAdaptor(new SEvent(event));
					if (adpobj) {
						ev = tTJSVariant(adpobj, adpobj);
						adpobj->Release();
					}

					tTJSVariant *params[] = {&self, &ev};
					tTJSVariant result;
					m->FuncCall(0, NULL, NULL, &result, 2, params, method.AsObjectThisNoAddRef());
					ret = (tjs_int)result != 0;
				}
			}
		}
	}
	return ret;
}

// --------------------------------------------------------------------------------

void
IrrlichtBase::start()
{
	stop();
	TVPAddContinuousEventHook(this);
}

void
IrrlichtBase::stop()
{
	TVPRemoveContinuousEventHook(this);
}
