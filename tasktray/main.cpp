
#include <windows.h>
#include <tchar.h>
#include "ncbind/ncbind.hpp"


#ifndef IDI_TVPWIN32
#define IDI_TVPWIN32 107 // [XXX]
#endif
#define WM_TASKTRAY (WM_APP + 2591)
#define IDI_MY_ICON TJS_W("tasktray icon")

class WindowTasktray {
public:
  static void registerWindowMessage(void) {
    sTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
  }

protected:
    static UINT sTaskbarRestart;

	iTJSDispatch2 *objthis; //< �I�u�W�F�N�g���̎Q��
	bool tasktrayEnable;     //< �^�X�N�g���C���L�����ǂ���
    HWND hwnd;
    ttstr hint;
    tjs_int infoIcon;
    ttstr infoTitle;
    ttstr infoMsg;
    tjs_int infoTimeout;

	typedef bool (__stdcall *NativeReceiver)(iTJSDispatch2 *obj, void *userdata, tTVPWindowMessage *Message);
  
	static bool __stdcall MyReceiver(void *userdata, tTVPWindowMessage *Message) {
		iTJSDispatch2 *obj = (iTJSDispatch2*)userdata; // Window �̃I�u�W�F�N�g
		// �g���g���̓��������̊֌W�ŃC�x���g�������͓o�^�j����ł��Ă΂�邱�Ƃ�����̂�
		// Window �̖{�̃I�u�W�F�N�g����l�C�e�B�u�I�u�W�F�N�g����蒼��
		WindowTasktray *self = ncbInstanceAdaptor<WindowTasktray>::GetNativeInstance(obj);
		if (self == NULL) {
			return false;
		}
		switch (Message->Msg) {
		case TVP_WM_DETACH: // �E�C���h�E���؂藣���ꂽ
			break; 
		case TVP_WM_ATTACH: // �E�C���h�E���ݒ肳�ꂽ
			break;
		case WM_TASKTRAY: // �O������̒ʐM
			{
              tTJSVariant cursorX, cursorY;
              obj->PropGet(0 , L"cursorX", NULL, &cursorX, obj);
              obj->PropGet(0 , L"cursorY", NULL, &cursorY, obj);
              tjs_int shiftValue = 0;
              if (GetKeyState(VK_SHIFT)) shiftValue |= TVP_SS_SHIFT;
              if (GetKeyState(VK_CONTROL)) shiftValue |= TVP_SS_CTRL;
              if (GetKeyState(VK_MENU)) shiftValue |= TVP_SS_ALT;
              tTJSVariant shift = shiftValue;
              tTJSVariant button;
              switch ( Message->LParam ){
              case WM_MOUSEMOVE: {
                /* �}�E�X�ړ��̏��� */
                tTJSVariant *params[] = { &cursorX, &cursorY, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayMouseMove", NULL, NULL, 3, params, obj));
              }
              case WM_LBUTTONUP: {
                /* ���{�^���������ꂽ���� */
                button = mbLeft;
                tTJSVariant *params[] = { &cursorX, &cursorY, &button, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayMouseUp", NULL, NULL, 4, params, obj));
              }
              case WM_RBUTTONUP: {
                /* �E�{�^���������ꂽ���� */
                button = mbRight;
                tTJSVariant *params[] = { &cursorX, &cursorY, &button, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayMouseUp", NULL, NULL, 4, params, obj));
              }
              case WM_MBUTTONUP: {
                /* ���{�^���������ꂽ���� */
                button = mbMiddle;
                tTJSVariant *params[] = { &cursorX, &cursorY, &button, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayMouseUp", NULL, NULL, 4, params, obj));
              }
              case WM_LBUTTONDOWN: {
                /* ���{�^���������ꂽ���� */
                button = mbLeft;
                tTJSVariant *params[] = { &cursorX, &cursorY, &button, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayMouseDown", NULL, NULL, 4, params, obj));
              }
              case WM_RBUTTONDOWN: {
                /* �E�{�^���������ꂽ���� */
                button = mbRight;
                tTJSVariant *params[] = { &cursorX, &cursorY, &button, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayMouseDown", NULL, NULL, 4, params, obj));
              }
              case WM_MBUTTONDOWN: {
                /* ���{�^���������ꂽ���� */
                button = mbMiddle;
                tTJSVariant *params[] = { &cursorX, &cursorY, &button, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayMouseDown", NULL, NULL, 4, params, obj));
              }
              case WM_LBUTTONDBLCLK: {
                /* ���_�u���N���b�N���ꂽ���� */
                button = mbLeft;
                tTJSVariant *params[] = { &cursorX, &cursorY, &button, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayDoubleClick", NULL, NULL, 4, params, obj));
              }
              case WM_RBUTTONDBLCLK: {
                /* �E�_�u���N���b�N���ꂽ���� */
                button = mbRight;
                tTJSVariant *params[] = { &cursorX, &cursorY, &button, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayDoubleClick", NULL, NULL, 4, params, obj));
              }
              case WM_MBUTTONDBLCLK: {
                /* ���_�u���N���b�N���ꂽ���� */
                button = mbMiddle;
                tTJSVariant *params[] = { &cursorX, &cursorY, &button, &shift };
                return ! TJS_SUCCEEDED(Try_iTJSDispatch2_FuncCall(obj, 0, L"onTasktrayDoubleClick", NULL, NULL, 4, params, obj));
              }
              default:
                /* ��L�ȊO�̏��� */
                break;
              }
              return true;
            }
		default:
          // �^�X�N�o�[���č쐬���ꂽ�ۂɓo�^��������
          if (Message->Msg == sTaskbarRestart
              && self->tasktrayEnable) {
            self->registerTasktray(true);
           }
          break;
		}
		return false;
	}

	/**
	 * ���V�[�o�̓o�^
	 */
	void registerReceiver(bool enable) {
		// ���V�[�o�X�V
		tTJSVariant mode    = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)MyReceiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)objthis;
		tTJSVariant *p[3] = {&mode, &proc, &userdata};
		int ret = objthis->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 3, p, objthis);
	}

  /**
   * �^�X�N�g���C�̓o�^
   */
  void registerTasktray(bool enable) {
    if (enable) {
      HINSTANCE hinst = GetModuleHandle(NULL);
      HICON   r = LoadIcon(hinst, L"MAINICON"); // for krkr2
      if (!r) r = LoadIcon(hinst, MAKEINTRESOURCE(IDI_TVPWIN32)); // for krkrz
      if (!r) r = LoadIcon(hinst, IDI_APPLICATION); // for others

      NOTIFYICONDATA nid = { 0 };
      nid.cbSize = sizeof(nid);
      nid.uFlags = NIF_ICON | NIF_MESSAGE |NIF_TIP;
      nid.hWnd = hwnd;
      nid.hIcon = r;
      nid.uCallbackMessage = WM_TASKTRAY;
      wcsncpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(TCHAR), hint.c_str(), hint.length());
      for (;;) {
		if (Shell_NotifyIcon(NIM_ADD, &nid))
          // �o�^�ł�����I���
          break;
        // �^�C���A�E�g���ǂ������ׂ�
        if (::GetLastError() != ERROR_TIMEOUT)
          // �A�C�R���o�^�G���[
          TVPThrowExceptionMessage(L"tasktray register failed.");
        // �ҋ@
        ::Sleep(1000);
        // �o�^�ł��Ă��Ȃ����Ƃ��m�F����
        if (Shell_NotifyIcon(NIM_MODIFY, &nid))
          // �o�^�ł��Ă���
          break;
      }
      DestroyIcon(nid.hIcon);
    } else {
      NOTIFYICONDATA nid = { 0 };
      nid.cbSize = sizeof(nid);
      nid.hWnd = hwnd;
      Shell_NotifyIcon(NIM_DELETE, &nid);
    }      
  }

  void updateTasktray(UINT flags) {
      NOTIFYICONDATA nid = { 0 };
      nid.cbSize = sizeof(nid);
      nid.uFlags = flags;
      nid.hWnd = hwnd;
      if (flags & NIF_TIP)
        wcsncpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(TCHAR), hint.c_str(), hint.length());
      if (flags & NIF_INFO) {
        wcsncpy_s(nid.szInfo, sizeof(nid.szInfo) / sizeof(TCHAR), infoMsg.c_str(), infoMsg.length());
        wcsncpy_s(nid.szInfoTitle, sizeof(nid.szInfoTitle) / sizeof(TCHAR), infoTitle.c_str(), infoTitle.length());
        nid.dwInfoFlags = infoIcon;
        nid.uTimeout = infoTimeout;
      } 
      Shell_NotifyIcon(NIM_MODIFY, &nid);
  }

public:
	// �R���X�g���N�^
	WindowTasktray(iTJSDispatch2 *objthis) : objthis(objthis), tasktrayEnable(false) {
      tTJSVariant val;
      objthis->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
      hwnd = reinterpret_cast<HWND>((tjs_int64)(val));
    }

	// �f�X�g���N�^
	~WindowTasktray() {
		// ���V�[�o�����
      setTasktrayEnable(false);
	}

	/**
	 * �^�X�N�g���C���L�����ǂ�����ݒ�
	 * @param enable true �Ȃ�L��
	 */
	void setTasktrayEnable(bool enable) {
		if (tasktrayEnable != enable) {
			tasktrayEnable = enable;
			registerReceiver(tasktrayEnable);
            registerTasktray(tasktrayEnable);
		} 
	}

	/**
	 * @return �^�X�N�g���C���L�����ǂ������擾
	 */
	bool getTasktrayEnable() {
		return tasktrayEnable;
	}

  /**
   * �^�X�N�g���C�̃q���g��ݒ肷��
   * @param text �e�L�X�g
   */
  void setTasktrayHint(ttstr text) {
    hint = text;
    if (tasktrayEnable)
      updateTasktray(NIF_TIP);
  }

  /**
   * �^�X�N�g���C�̃q���g���擾�ݒ肷��
   * @return �e�L�X�g
   */
  ttstr getTasktrayHint(void) {
    return hint;
  }

  /**
   * �^�X�N�g���C�̃o���[���`�b�v��ݒ肷��
   * @param icon �A�C�R��
   * @param title �^�C�g��
   * @param msg ���b�Z�[�W
   * @param timeout �^�C���A�E�g(ms�B10000�`30000�̊ԂŐݒ肷��)
   */
  void popupTasktrayInfo(tjs_int icon, ttstr title, ttstr msg, tjs_int timeout) {
    infoIcon = icon;
    infoTitle = title;
    infoMsg = msg;
    infoTimeout = timeout;
    if (tasktrayEnable) 
      updateTasktray(NIF_INFO);
  }
};

UINT WindowTasktray::sTaskbarRestart;


void
PreRegistCallback()
{
  WindowTasktray::registerWindowMessage();
  TVPExecuteExpression(L"global.niifNone = 0");
  TVPExecuteExpression(L"global.niifInfo = 1");
  TVPExecuteExpression(L"global.niifWarning = 2");
  TVPExecuteExpression(L"global.niifError = 3");
}

void PostUnregistCallback()
{
  TVPExecuteScript(L"delete global[\"niifNone\"];");
  TVPExecuteScript(L"delete global[\"niifInfo\"];");
  TVPExecuteScript(L"delete global[\"niifWarning\"];");
  TVPExecuteScript(L"delete global[\"niifError\"];");
}

// �C���X�^���X�Q�b�^
NCB_GET_INSTANCE_HOOK(WindowTasktray)
{
	NCB_INSTANCE_GETTER(objthis) { // objthis �� iTJSDispatch2* �^�̈����Ƃ���
		ClassT* obj = GetNativeInstance(objthis);	// �l�C�e�B�u�C���X�^���X�|�C���^�擾
		if (!obj) {
			obj = new ClassT(objthis);				// �Ȃ��ꍇ�͐�������
			SetNativeInstance(objthis, obj);		// objthis �� obj ���l�C�e�B�u�C���X�^���X�Ƃ��ēo�^����
		}
		return obj;
	}
};
    
// �t�b�N���A�^�b�`
NCB_ATTACH_CLASS_WITH_HOOK(WindowTasktray, Window) {
	Property(L"tasktrayEnable", &WindowTasktray::getTasktrayEnable, &WindowTasktray::setTasktrayEnable);
	Property(L"tasktrayHint", &WindowTasktray::getTasktrayHint, &WindowTasktray::setTasktrayHint);
    Method(L"popupTasktrayInfo", &WindowTasktray::popupTasktrayInfo);
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
