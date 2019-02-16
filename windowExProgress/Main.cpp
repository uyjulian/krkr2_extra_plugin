#include <windows.h>
#include <tchar.h>
#include <process.h>
#include <commctrl.h>
#include <vector>
#include "ncbind/ncbind.hpp"

#define CLASSNAME _T("WindowExProgress")
#define KRKRDISPWINDOWCLASS _T("TScrollBox")

#ifndef ID_CANCEL
#define ID_CANCEL 3
#endif

/**
 * �\���摜���
 */
class ImageInfo {

public:
	ImageInfo() : bitmap(0), bmpdc(0), color(0), left(0), top(0), width(0), height(0) {
	}

	~ImageInfo() {
		removeBitmap();
	}

	// �`�揈��
	void show(HDC dc, PAINTSTRUCT &ps) {
		if (bitmap) {
			// �r�b�g�}�b�v�`��
			::BitBlt(dc, left, top, width, height, bmpdc, 0, 0, SRCCOPY);
		} else {
			// ��`�`��
			if (width > 0 && height > 0) {
				SelectObject(dc, CreateSolidBrush(color));
				::Rectangle(dc, left, top, width, height);
				DeleteObject(SelectObject(dc, GetStockObject(WHITE_BRUSH)));
			}
		}
	}
	
	/**
	 * �r�b�g�}�b�v��ݒ�
	 */
	void setColor(int left, int top, int width, int height, int color) {
		removeBitmap();
		this->left   = left;
		this->top    = top;
		this->width  = width;
		this->height = height;
		this->color  = color;
	}
	
	/**
	 * �r�b�g�}�b�v��ݒ�
	 */
	bool setBitmap(int left, int top, iTJSDispatch2 *lay) {
		
		typedef unsigned char PIX;
		if (!lay || !lay->IsInstanceOf(0, 0, 0, TJS_W("Layer"), lay)) return false;
		
		this->left = left;
		this->top  = top;
		ncbPropAccessor obj(lay);
		width  = obj.getIntValue(TJS_W("imageWidth"));
		height = obj.getIntValue(TJS_W("imageHeight"));
		tjs_int ln = obj.getIntValue(TJS_W("mainImageBufferPitch"));
		PIX *pw, *pr = reinterpret_cast<unsigned char *>(obj.getIntValue(TJS_W("mainImageBuffer")));
		
		BITMAPINFO info;
		ZeroMemory(&info, sizeof(info));
		info.bmiHeader.biSize = sizeof(BITMAPINFO);
		info.bmiHeader.biWidth  = width;
		info.bmiHeader.biHeight = height;
		info.bmiHeader.biPlanes = 1;
		info.bmiHeader.biBitCount = 24;
		
		removeBitmap();
		bmpdc  = ::CreateCompatibleDC(NULL);
		bitmap = ::CreateDIBSection(bmpdc, (LPBITMAPINFO)&info, DIB_RGB_COLORS, (LPVOID*)&pw, NULL, 0);
		
		if (!bitmap || !bmpdc) return false;
		for (int y = height-1; y >= 0; y--) {
			PIX *src = pr + (y * ln);
			PIX *dst = pw + ((height-1 - y) * ((width*3+3) & ~3L));
			for (int n = width-1; n >= 0; n--, src+=4, dst+=3) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
			}
		}
		::SelectObject(bmpdc, bitmap);
		return true;
	}

protected:
	void removeBitmap() {
		if (bmpdc) {
			::DeleteDC(bmpdc);
			bmpdc  = NULL;
		}
		if (bitmap) {
			::DeleteObject(bitmap);
			bitmap = NULL;
		}
	}
	
	HBITMAP bitmap;
	HDC bmpdc;
	HBRUSH brush;
	tjs_int left, top, width, height;
	int color;
};


/**
 * �\�����b�Z�[�W���
 */
class MessageInfo {
	
public:

	MessageInfo() : left(0), top(0), size(0), color(0xffffff), useShadow(false), shadowColor(0), shadowDistanceX(1), shadowDistanceY(1) {
	}

	~MessageInfo() {
	}

	bool setText(iTJSDispatch2 *init) {
		ncbPropAccessor info(init);
#define GETINTVALUE(a,def) a = info.getIntValue(L#a, def)
#define GETBOOLVALUE(a,def) a = info.getIntValue(L#a,def?1:0) != 0
#define GETSTRVALUE(a,def) a = info.getStrValue(L#a,def)

		GETINTVALUE(left, 0);
		GETINTVALUE(top, 0);
		GETINTVALUE(size, 12);
		GETINTVALUE(color, 0xffffff);
		GETINTVALUE(shadowColor,-1);
		GETINTVALUE(shadowDistanceX,1);
		GETINTVALUE(shadowDistanceY,1);
		useShadow = shadowColor > 0;
	}

	void show(HDC dc, PAINTSTRUCT &ps) {
		if (useShadow) {
//			OutputText(left+shadowDistanceX, top+shadowDistanceY, text.c_str());
		}
//		OutputText(left, top, text.c_str());
	}

protected:
	int left;
	int top;
	ttstr text;
	int size;
	int color;
	bool useShadow;
	int shadowColor;
	int shadowDistanceX;
	int shadowDistanceY;
	HFONT font;
};

/**
 * �Z�[�u�����X���b�h�p���
 * �v���O���X���������s����E�C���h�E
 */
class ProgressWindow {

public:
	/**
	 * �E�C���h�E�N���X�̓o�^
	 */
	static void registerWindowClass() {
		WNDCLASSEX wcex;
		ZeroMemory(&wcex, sizeof wcex);
		wcex.cbSize		= sizeof(WNDCLASSEX);
		wcex.style		= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= GetModuleHandle(NULL);
		wcex.hIcon		    = NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_WAIT);
		wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
		wcex.lpszMenuName	= 0;
		wcex.lpszClassName	= CLASSNAME;
		wcex.hIconSm		= 0;
		RegisterClassEx(&wcex);
	}

	/**
	 * �E�C���h�E�N���X�̍폜
	 */
	static void unregisterWindowClass() {
		UnregisterClass(CLASSNAME, GetModuleHandle(NULL));
	}

	/**
	 * �R���X�g���N�^
	 */
	ProgressWindow(iTJSDispatch2 *window, iTJSDispatch2 *init) : window(window), hParent(0), hWnd(0), thread(0), doneflag(false), cancelflag(false), percent(0),
	progressBarEnable(true), progressBarHandle(0), progressBarStyle(0),
	progressBarLeft(-1), progressBarTop(-1), progressBarWidth(-1), progressBarHeight(-1),
	progressBarColor(0xff000000), progressBarBackColor(0xff000000),
	cancelButtonEnable(true), cancelButtonHandle(0), cancelButtonCaption(L"Cancel"),
	cancelButtonLeft(-1), cancelButtonTop(-1), cancelButtonWidth(-1), cancelButtonHeight(-1) {
		prepare = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (init) {
			ncbPropAccessor info(init);
			GETINTVALUE(progressBarStyle, 0);
			GETINTVALUE(progressBarTop, -1);
			GETINTVALUE(progressBarLeft, -1);
			GETINTVALUE(progressBarWidth, -1);
			GETINTVALUE(progressBarHeight, -1);
			GETINTVALUE(progressBarColor, 0xff000000);
			GETINTVALUE(progressBarBackColor, 0xff000000);
			GETBOOLVALUE(progressBarEnable, true);
			GETSTRVALUE(cancelButtonCaption, ttstr("Cancel"));
			GETINTVALUE(cancelButtonLeft, -1);
			GETINTVALUE(cancelButtonTop, -1);
			GETINTVALUE(cancelButtonWidth, -1);
			GETINTVALUE(cancelButtonHeight, -1);
			GETBOOLVALUE(cancelButtonEnable, true);
		}
		setReceiver(true);
		start();
	}

	/**
	 * �f�X�g���N�^
	 */
	~ProgressWindow() {
		CloseHandle(prepare);
		setReceiver(false);
		end();
	}
	
	/**
	 * �v���O���X�ʒm
	 * @return �L�����Z������Ă��� true
	 */
	bool doProgress(int percent) {
		if (percent != this->percent) {
			this->percent = percent;
			if (progressBarHandle) {
				SendMessage(progressBarHandle, PBM_SETPOS, (WPARAM)percent, 0 );
			}
		}
		return !hWnd || cancelflag;
	}

	/**
	 * �v���O���X�����̃e�L�X�g�������ւ���
	 * @param name ���ʖ�
	 * @param text �\���e�L�X�g
	 */
	void setProgressMessage(const tjs_char *name, const tjs_char *text) {
	}
	
protected:
	iTJSDispatch2 *window; //< �e�E�C���h�E
	HWND hParent; //< �e�n���h��
	HWND hWnd; //< �����̃n���h��
	HANDLE thread; //< �v���O���X�����̃X���b�h
	HANDLE prepare; //< �����҂��C�x���g
	bool doneflag;   // �I���t���O
	bool cancelflag; // �L�����Z���t���O
	int percent; // �p�[�Z���g�w��

	bool progressBarEnable;
	HWND progressBarHandle; //< �v���O���X�o�[�̃n���h��
	int progressBarLeft;
	int progressBarStyle;
	int progressBarTop;
	int progressBarWidth;
	int progressBarHeight;
	int progressBarColor;
	int progressBarBackColor;

	bool cancelButtonEnable;
	HWND cancelButtonHandle;
	ttstr cancelButtonCaption;
	int cancelButtonLeft;
	int cancelButtonTop;
	int cancelButtonWidth;
	int cancelButtonHeight;

	ImageInfo *backGround;
	
	/**
	 * �E�C���h�E�v���V�[�W��
	 */
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		ProgressWindow *self = (ProgressWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
		if (self) {
			switch (message) {
			case WM_PAINT: // ��ʍX�V
				{
					PAINTSTRUCT ps;
					HDC dc = BeginPaint(hWnd, &ps);
					self->show(dc, ps);
					EndPaint(hWnd, &ps);
				}
				return 0;
			case WM_COMMAND: // �L�����Z���ʒm
				switch (wParam) {
				case ID_CANCEL:
					self->cancel();
					break;
				}
				break;
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	/*
	 * �E�C���h�E�C�x���g�������V�[�o
	 */
	static bool __stdcall receiver(void *userdata, tTVPWindowMessage *Message) {
		ProgressWindow *self = (ProgressWindow*)userdata;
		switch (Message->Msg) {
		case TVP_WM_ATTACH:
			self->start();
			break;
		case TVP_WM_DETACH:
			self->end();
			break;
		default:
			break;
		}
		return false;
	}

	// ���[�U���b�Z�[�W���V�[�o�̓o�^/����
	void setReceiver(bool enable) {
		tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)this;
		tTJSVariant *p[] = {&mode, &proc, &userdata};
		if (window->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, window) != TJS_S_OK) {
			TVPThrowExceptionMessage(L"can't regist user message receiver");
		}
	}
	
	// ���s�X���b�h
	static unsigned __stdcall threadFunc(void *data) {
		((ProgressWindow*)data)->main();
		_endthreadex(0);
		return 0;
	}

	/**
	 * �����J�n
	 */
	void start() {
		end();
		doneflag = false;
		tTJSVariant krkrHwnd;
		if (TJS_SUCCEEDED(window->PropGet(0, TJS_W("HWND"), NULL, &krkrHwnd, window))) {
			hParent = ::FindWindowEx((HWND)(tjs_int)krkrHwnd, NULL, KRKRDISPWINDOWCLASS, NULL);
            // KRKRDISPWINDOWCLASS��������Ȃ��ꍇ�͋g���g��Z�Ȃ̂Ŏ��g��hParent�Ƃ��� 
            if (! hParent)
              hParent = (HWND)(tjs_int)krkrHwnd;
			if (hParent) {
				thread = (HANDLE)_beginthreadex(NULL, 0, threadFunc, this, 0, NULL);
				if (thread) {
					WaitForSingleObject(prepare, 1000 * 3);
				}
			}
		}
	}
	
	/**
	 * �����I��
	 */
	void end() {
		doneflag = true;
		if (thread) {
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
			thread = 0;
		}
		hParent = 0;
	}

	/**
	 * ���s���C������
	 * �E�C���h�E�̐�������j���܂ł�Ɨ������X���b�h�ōs��
	 */
	void main() {
		// �E�C���h�E����
		if (hParent && !hWnd) {
			RECT rect;
			POINT point;
			point.x = 0;
			point.y = 0;
			::GetClientRect(hParent, &rect);
			::ClientToScreen(hParent, &point);
			int left   = point.x;
			int top    = point.y;
			int width  = rect.right  - rect.left;
			int height = rect.bottom - rect.top;
			hWnd = ::CreateWindowEx(0, CLASSNAME, _T(""), WS_POPUP, left, top, width, height, hParent, 0, GetModuleHandle(NULL), NULL);
			if (hWnd && !doneflag) {
				::SetWindowLong(hWnd, GWL_USERDATA, (LONG)this);
				::ShowWindow(hWnd,TRUE);
				create();
				// �҂����킹����
				SetEvent(prepare);
				// ���b�Z�[�W���[�v�̎��s
				MSG msg;
				while (!doneflag) {
					if (::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
						if (GetMessage(&msg, NULL, 0, 0)) {
							::TranslateMessage (&msg);
							::DispatchMessage (&msg);
						} else {
							break;
						}
					} else {
					    Sleep(0);
					}
				}
				// �E�C���h�E�̔j��
				::DestroyWindow(hWnd);
				hWnd = 0;
			}
		}
	}

	// -------------------------------------------------------------

	static int RGBTOBGR(int c) {
		unsigned int a = (unsigned int)c & 0xff000000;
		unsigned int r = (unsigned int)c & 0x00ff0000;
		unsigned int g = (unsigned int)c & 0x0000ff00;
		unsigned int b = (unsigned int)c & 0x000000ff;
		return (int)(a | r>>16 | g | b<<16);
	}
	
	/**
	 * �`����e����
	 */
	void create() {

		RECT rect;
		GetClientRect(hWnd, &rect);
		int swidth  = rect.right  - rect.left;
		int sheight = rect.bottom - rect.top;
		
		if (progressBarEnable) {
			// �v���O���X�o�[�̔z�u����
			if (progressBarWidth < 0) {
				progressBarWidth = swidth / 3;
			}
			if (progressBarHeight < 0) {
				progressBarHeight = sheight/10;
			}
			if (progressBarLeft < 0) {
				progressBarLeft = (swidth - progressBarWidth)/2;
			}
			if (progressBarTop < 0) {
				progressBarTop = (sheight - progressBarHeight)/2;
			}
			// �v���O���X�o�[���쐬
			progressBarHandle = CreateWindowEx(0, PROGRESS_CLASS, _T(""),
											   WS_VISIBLE | WS_CHILD | (progressBarStyle & (PBS_SMOOTH|PBS_VERTICAL)),
											   progressBarLeft, progressBarTop, progressBarWidth, progressBarHeight,
											   hWnd, (HMENU)1, GetModuleHandle(NULL), NULL);
			SendMessage(progressBarHandle, PBM_SETBARCOLOR, 0, RGBTOBGR(progressBarColor));
			SendMessage(progressBarHandle, PBM_SETBKCOLOR, 0, RGBTOBGR(progressBarBackColor));
			SendMessage(progressBarHandle, PBM_SETRANGE , 0, MAKELPARAM(0, 100));
			SendMessage(progressBarHandle, PBM_SETSTEP, 1, 0 );
			SendMessage(progressBarHandle, PBM_SETPOS, percent, 0);
		}

		if (cancelButtonEnable) {
			// �L�����Z���{�^���̔z�u����
			if (cancelButtonWidth < 0) {
				cancelButtonWidth = cancelButtonCaption.length() * 16 + 8;
			}
			if (cancelButtonHeight < 0) {
				cancelButtonHeight = 24;
			}
			if (cancelButtonLeft < 0) {
				cancelButtonLeft = (swidth - cancelButtonWidth)/2;
			}
			if (cancelButtonTop < 0) {
				cancelButtonTop = sheight - cancelButtonHeight * 3;
			}
			// �L�����Z���{�^�����쐬
			cancelButtonHandle = CreateWindow(_T("BUTTON"), cancelButtonCaption.c_str(),
											  WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
											  cancelButtonLeft, cancelButtonTop, cancelButtonWidth, cancelButtonHeight,
											  hWnd, (HMENU)ID_CANCEL, GetModuleHandle(NULL), NULL);
		}
	}
	
	/**
	 * ��ʍX�V����
	 */
	void show(HDC dc, PAINTSTRUCT &ps) {
		// �w�i�œh��Ԃ�

		// �A�j���p�^�[����`��

		// �e�L�X�g��\��
	}

	/**
	 * �L�����Z���ʒm
	 */
	void cancel() {
		cancelflag = true;
	}
};

/**
 * �E�C���h�E�Ƀ��C���Z�[�u�@�\���g��
 */
class WindowExProgress {

protected:
	iTJSDispatch2 *objthis; //< �I�u�W�F�N�g���̎Q��
	ProgressWindow *progressWindow; //< �v���O���X�\���p

public:
	/**
	 * �R���X�g���N�^
	 */
	WindowExProgress(iTJSDispatch2 *objthis) : objthis(objthis), progressWindow(NULL) {}

	/**
	 * �f�X�g���N�^
	 */
	~WindowExProgress() {
		delete progressWindow;
	}

	/**
	 * �v���O���X�������J�n����B
	 * �g���g�������s�u���b�N���ł�����ɕ\���p�����܂��B
	 * @param init �������f�[�^(����)
	 */
	void startProgress(iTJSDispatch2 *init) {
		if (progressWindow) {
			TVPThrowExceptionMessage(L"already running progress");
		}
		progressWindow = new ProgressWindow(objthis, init);
	}
	
	/**
	 * �v���O���X�����̌o�ߏ�Ԃ�ʒm����B
	 * @param percent �o�ߏ�Ԃ��p�[�Z���g�w��
	 * @return �L�����Z���v��������� true
	 */
	bool doProgress(int percent) {
		if (!progressWindow) {
			TVPThrowExceptionMessage(L"not running progress");
		}
		return progressWindow->doProgress(percent);
	}

	/**
	 * �v���O���X�������I������B
	 */
	void endProgress() {
		if (!progressWindow) {
			TVPThrowExceptionMessage(L"not running progress");
		}
		delete progressWindow;
		progressWindow = NULL;
	}
};

//---------------------------------------------------------------------------

// �C���X�^���X�Q�b�^
NCB_GET_INSTANCE_HOOK(WindowExProgress)
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

#define ENUM(n) Variant(#n, (int)n)

NCB_ATTACH_CLASS_WITH_HOOK(WindowExProgress, Window) {
	ENUM(PBS_SMOOTH);
	ENUM(PBS_VERTICAL);
	NCB_METHOD(startProgress);
	NCB_METHOD(doProgress);
	NCB_METHOD(endProgress);
};

/**
 * �o�^������
 */
static void PreRegistCallback()
{
	ProgressWindow::registerWindowClass();
}

/**
 * �J�������O
 */
static void PostUnregistCallback()
{
	ProgressWindow::unregisterWindowClass();
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
