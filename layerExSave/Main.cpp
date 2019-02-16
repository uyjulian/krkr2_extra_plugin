#include "ncbind/ncbind.hpp"
#include <vector>
using namespace std;
#include <process.h>

#define WM_SAVE_TLG_PROGRESS (WM_APP+4)
#define WM_SAVE_TLG_DONE     (WM_APP+5)

iTJSDispatch2 *getLayerClass(void)
{
  tTJSVariant var;
  TVPExecuteExpression(TJS_W("Layer"), &var);
  return  var.AsObjectNoAddRef();
}

iTJSDispatch2 *getLayerAssignImages(void)
{
  tTJSVariant var;
  TVPExecuteExpression(TJS_W("Layer.assignImages"), &var);
  return  var.AsObjectNoAddRef();
}

#include "compress.hpp"
#include "savetlg5.hpp"
#include "savepng.hpp"

//---------------------------------------------------------------------------
// �E�C���h�E�g��
//---------------------------------------------------------------------------

class WindowSaveImage;

/**
 * �Z�[�u�����X���b�h�p���
 */
class SaveInfo {

	friend class WindowSaveImage;
	
protected:

	// �������ϐ�
	WindowSaveImage *notify; //< ���ʒm��
	tTJSVariant layer; //< ���C��
	tTJSVariant filename; //< �t�@�C����
	tTJSVariant info;  //< �ۑ��p�^�O���
	bool canceled;        //< �L�����Z���w��
	tTJSVariant handler;  //< �n���h���l
	tTJSVariant progressPercent; //< �i�s�x����
	
protected:
	/**
	 * ���݂̏�Ԃ̒ʒm
	 * @param percent �p�[�Z���g
	 */
	bool progress(int percent);

	/**
	 * �Ăяo���p
	 */
	static bool progressFunc(int percent, void *userData) {
		SaveInfo *self = (SaveInfo*)userData;
		return self->progress(percent);
	}
	
	// �o�߃C�x���g���M
	void eventProgress(iTJSDispatch2 *objthis) {
		tTJSVariant *vars[] = {&handler, &progressPercent, &layer, &filename};
		objthis->FuncCall(0, L"onSaveLayerImageProgress", NULL, NULL, 4, vars, objthis);
	}

	// �I���C�x���g���M
	void eventDone(iTJSDispatch2 *objthis) {
		tTJSVariant result = canceled ? 1 : 0;
		tTJSVariant *vars[] = {&handler, &result, &layer, &filename};
		objthis->FuncCall(0, L"onSaveLayerImageDone", NULL, NULL, 4, vars, objthis);
	}
	
public:
	// �R���X�g���N�^
	SaveInfo(int handler, WindowSaveImage *notify, tTJSVariant layer, const tjs_char *filename, tTJSVariant info)
		: handler(handler), notify(notify), layer(layer), filename(filename), info(info), canceled(false) {}
	
	// �f�X�g���N�^
	~SaveInfo() {}

	// �n���h���擾
	int getHandler() {
		return (int)handler;
	}
	
 	// �����J�n
	void start();

	// �����L�����Z��
	void cancel() {
		canceled = true;
	}

	// �����I��
	void stop() {
		canceled = true;
		notify = NULL;
	}
};

/**
 * �E�C���h�E�Ƀ��C���Z�[�u�@�\���g��
 */
class WindowSaveImage {

protected:
	iTJSDispatch2 *objthis; //< �I�u�W�F�N�g���̎Q��

	vector<SaveInfo*> saveinfos; //< �Z�[�u�����ێ��p

	// ���s�X���b�h
	static void checkThread(void *data) {
		((SaveInfo*)data)->start();
	}

	// �o�ߒʒm
	void eventProgress(SaveInfo *sender) {
		int handler = sender->getHandler();
		if (saveinfos[handler] == sender) {
			sender->eventProgress(objthis);
		}
	}

	// �I���ʒm
	void eventDone(SaveInfo *sender) {
		int handler = sender->getHandler();
		if (saveinfos[handler] == sender) {
			saveinfos[handler] = NULL;
			sender->eventDone(objthis);
		}
		delete sender;
	}

	/*
	 * �E�C���h�E�C�x���g�������V�[�o
	 */
	static bool __stdcall receiver(void *userdata, tTVPWindowMessage *Message) {
		if (Message->Msg == WM_SAVE_TLG_PROGRESS) {
			iTJSDispatch2 *obj = (iTJSDispatch2*)userdata;
			WindowSaveImage *self = ncbInstanceAdaptor<WindowSaveImage>::GetNativeInstance(obj);
			if (self) {
				self->eventProgress((SaveInfo*)Message->WParam);
			}
			return true;
		} else if (Message->Msg == WM_SAVE_TLG_DONE) {
			iTJSDispatch2 *obj = (iTJSDispatch2*)userdata;
			WindowSaveImage *self = ncbInstanceAdaptor<WindowSaveImage>::GetNativeInstance(obj);
			if (self) {
				self->eventDone((SaveInfo*)Message->WParam);
			}
			return true;
		}
		return false;
	}

	// ���[�U���b�Z�[�W���V�[�o�̓o�^/����
	void setReceiver(tTVPWindowMessageReceiver receiver, bool enable) {
		tTJSVariant mode     = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)receiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)objthis;
		tTJSVariant *p[] = {&mode, &proc, &userdata};
		if (objthis->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 4, p, objthis) != TJS_S_OK) {
			TVPThrowExceptionMessage(L"can't regist user message receiver");
		}
	}

public:

	/**
	 * �R���X�g���N�^
	 */
	WindowSaveImage(iTJSDispatch2 *objthis) : objthis(objthis) {
		setReceiver(receiver, true);
	}

	/**
	 * �f�X�g���N�^
	 */
	~WindowSaveImage() {
		setReceiver(receiver, false);
		for (int i=0;i<(int)saveinfos.size();i++) {
			SaveInfo *saveinfo = saveinfos[i];
			if (saveinfo) {
				saveinfo->stop();
				saveinfos[i] = NULL;
			}
		}
	}

	/**
	 * ���b�Z�[�W���M
	 * @param msg ���b�Z�[�W
	 * @param wparam WPARAM
	 * @param lparam LPARAM
	 */
	void postMessage(UINT msg, WPARAM wparam=NULL, LPARAM lparam=NULL) {
		// �E�B���h�E�n���h�����擾���Ēʒm
		tTJSVariant val;
		objthis->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
		HWND hwnd = reinterpret_cast<HWND>((tjs_int)(val));
		::PostMessage(hwnd, msg, wparam, lparam);
	}

	/**
	 * ���C���Z�[�u�J�n
	 * @param layer ���C��
	 * @param filename �t�@�C����
	 * @param info �^�O���
	 */
	int startSaveLayerImage(tTJSVariant layer, const tjs_char *filename, tTJSVariant info) {
		int handler = saveinfos.size();
		for (int i=0;i<(int)saveinfos.size();i++) {
			if (saveinfos[i] == NULL) {
				handler = i;
				break;
			}
		}
		if (handler >= (int)saveinfos.size()) {
			saveinfos.resize(handler + 1);
		}

		// �ۑ��p�Ƀ��C���𕡐�����
		tTJSVariant newLayer;
		{
			// �V�������C���𐶐�
			tTJSVariant window(objthis, objthis);
			tTJSVariant primaryLayer;
			objthis->PropGet(0, L"primaryLayer", NULL, &primaryLayer, objthis);
			tTJSVariant *vars[] = {&window, &primaryLayer};
			iTJSDispatch2 *obj;
			if (TJS_SUCCEEDED(getLayerClass()->CreateNew(0, NULL, NULL, &obj, 2, vars, objthis))) {

				// ���O�Â�
				tTJSVariant name = "saveLayer:";
				name +=filename;
				obj->PropSet(0, L"name", NULL, &name, obj);

				// �����C���̉摜�𕡐�
				tTJSVariant *param[] = {&layer};
				if (TJS_SUCCEEDED(getLayerAssignImages()->FuncCall(0, NULL, NULL, NULL, 1, param, obj))) {
					newLayer = tTJSVariant(obj, obj);
					obj->Release();
				} else {
					obj->Release();
					TVPThrowExceptionMessage(L"�ۑ������p���C���ւ̉摜�̕����Ɏ��s���܂���");
				}
			} else {
				TVPThrowExceptionMessage(L"�ۑ������p���C���̐����Ɏ��s���܂���");
			}
		}
		SaveInfo *saveInfo = new SaveInfo(handler, this, newLayer, filename, info);
		saveinfos[handler] = saveInfo;
		_beginthread(checkThread, 0, saveInfo);
		return handler;
	}
	
	/**
	 * ���C���Z�[�u�̃L�����Z��
	 */
	void cancelSaveLayerImage(int handler) {
		if (handler < (int)saveinfos.size() && saveinfos[handler] != NULL) {
			saveinfos[handler]->cancel();
		}
	}

	/**
	 * ���C���Z�[�u�̒��~
	 */
	void stopSaveLayerImage(int handler) {
		if (handler < (int)saveinfos.size() && saveinfos[handler] != NULL) {
			saveinfos[handler]->stop();
			saveinfos[handler] = NULL;
		}
	}
};


/**
 * ���݂̏�Ԃ̒ʒm
 * @param percent �p�[�Z���g
 */
bool
SaveInfo::progress(int percent)
{
	if ((int)progressPercent != percent) {
		progressPercent = percent;
		if (notify) {
			notify->postMessage(WM_SAVE_TLG_PROGRESS, (WPARAM)this);
			Sleep(0);
		}
	}
	return canceled;
}

/*
 * �ۑ������J�n
 */
void
SaveInfo::start()
{
	iTJSDispatch2  *lay = layer.AsObjectNoAddRef();
	iTJSDispatch2  *nfo = info.Type() == tvtObject ? info.AsObjectNoAddRef() : NULL;
	const tjs_char *fn  = filename.GetString();
	ttstr ext(TVPExtractStorageExt(ttstr(fn)));
	ext.ToLowerCase();
	// �摜���Z�[�u�i�g���q�ʁj
	if (ext == TJS_W(".png")) {
		CompressAndSave<CompressPNG >::saveLayerImage(lay, fn, nfo, progressFunc, (void*)this);
	} else {
		CompressAndSave<CompressTLG5>::saveLayerImage(lay, fn, nfo, progressFunc, (void*)this);
	}
	// �����ʒm
	if (notify) {
		notify->postMessage(WM_SAVE_TLG_DONE, (WPARAM)this);
		Sleep(0);
	} else {
		delete this;
	}
}

//---------------------------------------------------------------------------

// �C���X�^���X�Q�b�^
NCB_GET_INSTANCE_HOOK(WindowSaveImage)
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

NCB_ATTACH_CLASS_WITH_HOOK(WindowSaveImage, Window) {
	NCB_METHOD(startSaveLayerImage);
	NCB_METHOD(cancelSaveLayerImage);
	NCB_METHOD(stopSaveLayerImage);
};

