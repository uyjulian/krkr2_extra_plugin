#include <windows.h>
#include <tchar.h>
#include "ncbind/ncbind.hpp"
#include <map>
using namespace std;

// �g���g���̃E�C���h�E�N���X
#define KRWINDOWCLASS _T("TTVPWindowForm")
#define KZWINDOWCLASS _T("TVPMainWindow")
#define KEYSIZE 256

// -------------------------------------------------------------------
// �A�g������
// -------------------------------------------------------------------

// �m�ۂ����A�g�����
static map<ttstr,ATOM> *atoms = NULL;

// ���O����V�X�e���O���[�o���A�g���擾
static ATOM getAtom(const TCHAR *str)
{
	ttstr name(str);
	map<ttstr,ATOM>::const_iterator n = atoms->find(name);
	if (n != atoms->end()) {
		return n->second;
	}
	ATOM atom = GlobalAddAtom(str);
	(*atoms)[name] = atom;
	return atom;
}

// �A�g�����疼�O���擾����
static void getKey(tTJSVariant &key, ATOM atom)
{
	TCHAR buf[KEYSIZE+1];
	UINT len = GlobalGetAtomName(atom, buf, KEYSIZE);
	if (len > 0) {
		buf[len] = '\0';
		key = buf;
	}
}

// -------------------------------------------------------------------
// �����{��
// -------------------------------------------------------------------

/**
 * ���\�b�h�ǉ��p�N���X
 */
class WindowMsg {

protected:
	iTJSDispatch2 *objthis; //< �I�u�W�F�N�g���̎Q��
	bool messageEnable;     //< ���b�Z�[�W�������L�����ǂ���

	// --------------------------------------------------------

	typedef bool (__stdcall *NativeReceiver)(iTJSDispatch2 *obj, void *userdata, tTVPWindowMessage *Message);
	
	// ���[�U��`���V�[�o���
	struct ReceiverInfo {
		tTJSVariant userData;
		tTJSVariant receiver;
		// �f�t�H���g�R���X�g���N�^
		ReceiverInfo() {};
		// �R���X�g���N�^
		ReceiverInfo(tTJSVariant &receiver, tTJSVariant &userData) : receiver(receiver), userData(userData) {}
		// �R�s�[�R���X�g���N�^
		ReceiverInfo(const ReceiverInfo &orig) {
			userData = orig.userData;
			receiver = orig.receiver;
		}
		// �f�X�g���N�^
		~ReceiverInfo(){}

		// ���s
		bool exec(iTJSDispatch2 *obj, tTVPWindowMessage *message) {
			switch (receiver.Type()) {
			case tvtObject:
				{
					tTJSVariant result;
					tTJSVariant wparam = (tjs_int)message->WParam;
					tTJSVariant lparam = (tjs_int)message->LParam;
					tTJSVariant *p[] = {&userData, &wparam, &lparam};
					receiver.AsObjectClosureNoAddRef().FuncCall(0, NULL, NULL, &result, 3, p, NULL);
					return (int)result != 0;
				}
				break;
			case tvtString:
				{
					tTJSVariant result;
					tTJSVariant wparam = (tjs_int)message->WParam;
					tTJSVariant lparam = (tjs_int)message->LParam;
					tTJSVariant *p[] = {&userData, &wparam, &lparam};
					obj->FuncCall(0, receiver.GetString(), NULL, &result, 3, p, obj);
					return (int)result != 0;
				}
				break;
			case tvtInteger:
				{
					NativeReceiver receiverNative = (NativeReceiver)(tjs_int)receiver;
					return receiverNative(obj, (void*)(tjs_int)userData, message);
				}
				break;
			}
			return false;
		}
	};
	
	map<unsigned int, ReceiverInfo> receiverMap;

	// ���[�U�K�背�V�[�o�̍폜
	void removeUserReceiver(unsigned int Msg) {
		map<unsigned int, ReceiverInfo>::const_iterator n = receiverMap.find(Msg);
		if (n != receiverMap.end()) {
			receiverMap.erase(Msg);
		}
	}
	
	// ���[�U�K�背�V�[�o�̓o�^
	void addUserReceiver(unsigned int Msg, tTJSVariant &receiver, tTJSVariant &userdata) {
		removeUserReceiver(Msg);
		receiverMap[Msg] = ReceiverInfo(receiver, userdata);
	}

	// --------------------------------------------------------

	ttstr storeKey;         //< HWND �ۑ��w��L�[
	
	/**
	 * ���s�t�@�C��������ꏊ�� HWND ����ۑ�����
	 */
	void storeHWND(HWND hwnd) {
		if (storeKey != "") {
			tTJSVariant varScripts;
			TVPExecuteExpression(TJS_W("System.exeName"), &varScripts);
			ttstr path = varScripts;
			path += ".";
			path += storeKey;
			IStream *stream = TVPCreateIStream(path, TJS_BS_WRITE);
			if (stream != NULL) {
				char buf[100];
				DWORD len;
				_snprintf(buf, sizeof buf, "%d", (int)hwnd);
				stream->Write(buf, strlen(buf), &len);
				stream->Release();
			}
		}
	}
	
	/**
	 * ���b�Z�[�W��M�֐��{��
	 * @param userdata ���[�U�f�[�^(���̏ꍇ�l�C�e�B�u�I�u�W�F�N�g���)
	 * @param Message �E�C���h�E���b�Z�[�W���
	 */
	static bool __stdcall MyReceiver(void *userdata, tTVPWindowMessage *Message) {

		iTJSDispatch2 *obj = (iTJSDispatch2*)userdata; // Window �̃I�u�W�F�N�g
		// �g���g���̓��������̊֌W�ŃC�x���g�������͓o�^�j����ł��Ă΂�邱�Ƃ�����̂�
		// Window �̖{�̃I�u�W�F�N�g����l�C�e�B�u�I�u�W�F�N�g����蒼��
		WindowMsg *self = ncbInstanceAdaptor<WindowMsg>::GetNativeInstance(obj);
		if (self == NULL) {
			return false;
		}
		switch (Message->Msg) {
		case TVP_WM_DETACH: // �E�C���h�E���؂藣���ꂽ
			break; 
		case TVP_WM_ATTACH: // �E�C���h�E���ݒ肳�ꂽ
			self->storeHWND((HWND)Message->LParam);
			break;
		case WM_COPYDATA: // �O������̒ʐM
			{
				COPYDATASTRUCT *copyData = (COPYDATASTRUCT*)Message->LParam;
				tTJSVariant key;
				getKey(key, (ATOM)copyData->dwData);
				tTJSVariant msg((const tjs_char *)copyData->lpData);
				tTJSVariant *p[] = {&key, &msg};
				obj->FuncCall(0, L"onMessageReceived", NULL, NULL, 2, p, obj);
			}
			return true;
		default:
			{
				map<unsigned int, ReceiverInfo>::iterator n = self->receiverMap.find(Message->Msg);
				if (n != self->receiverMap.end()) {
					return n->second.exec(obj, Message);
				}
			}
			break;
		}
		return false;
	}

	void doStoreKey() {
		if (storeKey != "") {
			tTJSVariant val;
			objthis->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
			storeHWND(reinterpret_cast<HWND>((tjs_int)(val)));
		}
	}
	
	/**
	 * ���V�[�o�̓o�^
	 */
	void registReceiver(bool enable) {
		// ���V�[�o�X�V
		tTJSVariant mode    = enable ? (tTVInteger)(tjs_int)wrmRegister : (tTVInteger)(tjs_int)wrmUnregister;
		tTJSVariant proc     = (tTVInteger)(tjs_int)MyReceiver;
		tTJSVariant userdata = (tTVInteger)(tjs_int)objthis;
		tTJSVariant *p[3] = {&mode, &proc, &userdata};
		int ret = objthis->FuncCall(0, L"registerMessageReceiver", NULL, NULL, 3, p, objthis);
	}

public:
	// �R���X�g���N�^
	WindowMsg(iTJSDispatch2 *objthis) : objthis(objthis), messageEnable(false) {}

	// �f�X�g���N�^
	~WindowMsg() {
		receiverMap.clear();
		// ���V�[�o�����
		registReceiver(false);
	}

	/**
	 * ���b�Z�[�W��M���L�����ǂ�����ݒ�
	 * @param enable true �Ȃ�L��
	 */
	void setMessageEnable(bool enable) {
		if (messageEnable != enable) {
			messageEnable = enable;
			registReceiver(messageEnable);
			if (messageEnable) {
				doStoreKey();
			}
		}
	}
	
	/**
	 * @return ���b�Z�[�W��M���L�����ǂ������擾
	 */
	bool getMessageEnable() {
		return messageEnable;
	}

	/**
	 * storeKey ���w��
	 * ���̒l���w�肷��ƁAHWND �̒l�� ���s�t�@�C����.key�� �Ƃ��ĕۑ������悤�ɂȂ�܂�
	 * @param keyName HWND�ۑ��p�L�[
	 */
	void setStoreKey(const tjs_char *keyName) {
		if (storeKey != keyName) {
			storeKey = keyName;
			if (messageEnable) {
				doStoreKey();
			}
		}
	}

	/**
	 * @return storeKey ���擾
	 */
	const tjs_char *getStoreKey() {
		return storeKey.c_str();
	}

	/**
	 * �O���v���O�C������̃��b�Z�[�W�������W�b�N�̓o�^
	 * @param mode �o�^���[�h
	 * @param msg
	 * @param func
	 */
	static tjs_error TJS_INTF_METHOD registerUserMessageReceiver(tTJSVariant *result,
																 tjs_int numparams,
																 tTJSVariant **param,
																 WindowMsg *self) {
		if (numparams < 2) return TJS_E_BADPARAMCOUNT;
		int mode         = (tjs_int)*param[0];
		unsigned int msg;
		if (param[1]->Type() == tvtString) {
			msg = RegisterWindowMessageW(param[1]->GetString());
		} else {
			msg = (unsigned int)(tTVInteger)*param[1];
		}
		if (mode == wrmRegister) {
			if (numparams < 4) return TJS_E_BADPARAMCOUNT;
			self->addUserReceiver(msg, *param[2], *param[3]);
		} else if (mode == wrmUnregister) {
			self->removeUserReceiver(msg);
		}
		if (result) {
			*result = (tjs_int)msg;
		}
		return TJS_S_OK;
	}

	// ���M���b�Z�[�W���
	struct UserMsgInfo {
		HWND hWnd;
		unsigned int msg;
		WPARAM wparam;
		LPARAM lparam;
		UserMsgInfo(HWND hWnd, unsigned int msg, WPARAM wparam, LPARAM lparam) : hWnd(hWnd), msg(msg), wparam(wparam), lparam(lparam) {}
	};

	/**
	 * �ʑ��ւ̃��b�Z�[�W���M����
	 * @param hWnd ���M��E�C���h�E�n���h��
	 * @param parent ���M���
	 */
	static BOOL CALLBACK enumWindowsProcUser(HWND hWnd, LPARAM parent) {
		UserMsgInfo *info = (UserMsgInfo*)parent;
		TCHAR buf[100];
		GetClassName(hWnd, buf, sizeof buf);
		if (info->hWnd != hWnd && (_tcscmp(buf, KRWINDOWCLASS) == 0 ||
								   _tcscmp(buf, KZWINDOWCLASS) == 0)) {
			SendMessage(hWnd, info->msg, info->wparam, info->lparam);
		}
		return TRUE;
	}
	
	/**
	 * ���[�U��`���b�Z�[�W���M����
	 * �N�����Ă���g���g�����ׂĂɃ��b�Z�[�W�𑗐M���܂�
	 * @param msg ���b�Z�[�WID
	 * @param wparam WPARAM�l
	 * @param lparam LPARAM�l
	 */
	void sendUserMessage(unsigned int msg, tjs_int wparam, tjs_int lparam) {
		tTJSVariant val;
		objthis->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
		UserMsgInfo info(reinterpret_cast<HWND>((tjs_int)(val)), msg, (WPARAM)wparam, (LPARAM)lparam);
		EnumWindows(enumWindowsProcUser, (LPARAM)&info);
	}
	
	// --------------------------------------------------------
	
	// ���M���b�Z�[�W���
	struct MsgInfo {
		HWND hWnd;
		COPYDATASTRUCT copyData;
		MsgInfo(HWND hWnd, const TCHAR *key, const tjs_char *msg) : hWnd(hWnd) {
			copyData.dwData = getAtom(key);
			copyData.cbData = (TJS_strlen(msg) + 1) * sizeof(tjs_char);
			copyData.lpData = (PVOID)msg;
		}
	};

	/**
	 * �ʑ��ւ̃��b�Z�[�W���M����
	 * @param hWnd ���M��E�C���h�E�n���h��
	 * @param parent ���M���
	 */
	static BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM parent) {
		MsgInfo *info = (MsgInfo*)parent;
		TCHAR buf[100];
		GetClassName(hWnd, buf, sizeof buf);
		if (info->hWnd != hWnd && (_tcscmp(buf, KRWINDOWCLASS) == 0 ||
								   _tcscmp(buf, KZWINDOWCLASS) == 0)) {
			SendMessage(hWnd, WM_COPYDATA, (WPARAM)info->hWnd, (LPARAM)&info->copyData);
		}
		return TRUE;
	}

	/**
	 * ���b�Z�[�W���M����
	 * �N�����Ă���g���g�����ׂĂɃ��b�Z�[�W�𑗐M���܂�
	 * @param key ���ʃL�[
	 * @param msg ���b�Z�[�W
	 */
	void sendMessage(const TCHAR *key, const tjs_char *msg) {
		tTJSVariant val;
		objthis->PropGet(0, TJS_W("HWND"), NULL, &val, objthis);
		MsgInfo info(reinterpret_cast<HWND>((tjs_int)(val)), key, msg);
		EnumWindows(enumWindowsProc, (LPARAM)&info);
	}

};

// �C���X�^���X�Q�b�^
NCB_GET_INSTANCE_HOOK(WindowMsg)
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
NCB_ATTACH_CLASS_WITH_HOOK(WindowMsg, Window) {
	Property(L"messageEnable", &WindowMsg::getMessageEnable, &WindowMsg::setMessageEnable);
	Property(L"storeHWND", &WindowMsg::getStoreKey, &WindowMsg::setStoreKey);
	RawCallback("registerUserMessageReceiver", &WindowMsg::registerUserMessageReceiver, 0);
	Method(L"sendUserMessage", &WindowMsg::sendUserMessage);
	Method(L"sendMessage", &WindowMsg::sendMessage);
}

/**
 * �o�^�����O
 */
void PreRegistCallback()
{
	atoms = new map<ttstr,ATOM>;
}

/**
 * �J��������
 */
void PostUnregistCallback()
{
	map<ttstr,ATOM>::const_iterator i = atoms->begin();
	while (i != atoms->end()) {
		GlobalDeleteAtom(i->second);
		i++;
	}
	delete atoms;
	atoms = NULL;
}

NCB_PRE_REGIST_CALLBACK(PreRegistCallback);
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);
