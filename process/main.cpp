#include <windows.h>
#include <tchar.h>
#include <process.h>
#include "ncbind/ncbind.hpp"

#define WM_SHELLEXECUTED   (WM_APP+1)
#define WM_SHELLCONSOLEOUT (WM_APP+2)

#define WSO_LOOPTIMEOUT 100

/**
 * �R���\�[���R�}���h�p����
 */

struct CommandExecute
{
	typedef void (*LineCallbackT)(void*, int, LPCWSTR);

	HANDLE hOR, hOW, hIR, hIW, hEW;
	PROCESS_INFORMATION pi;
	DWORD exitcode;
	bool timeouted;
	enum { ERR_NONE, ERR_PIPE, ERR_PROC, ERR_TOUT, ERR_WAIT } error;

	~CommandExecute() {
		if (hOR) ::CloseHandle(hOR);
		if (hOW) ::CloseHandle(hOW);
		if (hIR) ::CloseHandle(hIR);
		if (hIW) ::CloseHandle(hIW);
		if (hEW) ::CloseHandle(hEW);
		if (pi.hThread)  ::CloseHandle(pi.hThread);
		if (pi.hProcess) ::CloseHandle(pi.hProcess);
	}

	CommandExecute()
		: hOR(0), hOW(0), hIR(0), hIW(0), hEW(0), exitcode(~0L), timeouted(false), error(ERR_NONE)
	{
		::ZeroMemory(&pi, sizeof(pi));

		// �Z�L�����e�B����
		SECURITY_ATTRIBUTES sa;
		SECURITY_DESCRIPTOR sd;
		::ZeroMemory(&sa, sizeof(sa));
		sa.nLength= sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;

		// NT�n�̏ꍇ�̓Z�L�����e�B�L�q�q��
		OSVERSIONINFO osv;
		::ZeroMemory(&osv, sizeof(osv));
		osv.dwOSVersionInfoSize = sizeof(osv);
		::GetVersionEx(&osv);
		if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			::ZeroMemory(&sd, sizeof(sd));
			::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
			::SetSecurityDescriptorDacl(&sd, true, NULL, false);
			sa.lpSecurityDescriptor = &sd;
		}

		// �p�C�v���쐬
		HANDLE hOT=0, hIT=0;
		HANDLE hPID = ::GetCurrentProcess();
		if (!(::CreatePipe(&hOT, &hOW, &sa,0) &&
			  ::CreatePipe(&hIR, &hIT, &sa,0) &&
			  ::DuplicateHandle(hPID, hOW, hPID, &hEW, 0,  TRUE, DUPLICATE_SAME_ACCESS) &&
			  ::DuplicateHandle(hPID, hOT, hPID, &hOR, 0, FALSE, DUPLICATE_SAME_ACCESS) &&
			  ::DuplicateHandle(hPID, hIT, hPID, &hIW, 0, FALSE, DUPLICATE_SAME_ACCESS)
			  )) {
			error = ERR_PIPE;
		}
		::CloseHandle(hOT);
		::CloseHandle(hIT);
	}

	bool start(ttstr const &target, const tjs_char *param=0, const tjs_char *folder=0) {
		if (hasError()) return false;

		ttstr cmd(L"\"");
		// �g���g���T�[�`�p�X��ɂ���ꍇ�͂������D��
		if (TVPIsExistentStorage(target)) {
			ttstr tmp = TVPGetPlacedPath(target);
			TVPGetLocalName(tmp);
			/**/cmd += tmp    + L"\"";
		} else  cmd += target + L"\"";

		if (param && wcslen(param) > 0) cmd += L" " + ttstr(param);
		LPWSTR cmdline = (LPWSTR)cmd.c_str();

		// �q�v���Z�X�쐬
		STARTUPINFO si;
		::ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.hStdOutput = hOW;
		si.hStdInput  = hIR;
		si.hStdError  = hEW;
		si.wShowWindow = SW_HIDE;
		if (!::CreateProcessW(0, cmdline, 0, 0, TRUE, CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP, 0, folder, &si, &pi)) {
			error = ERR_PROC;
			return false;
		}
		return true;
	}

#define		BUF_SIZE	1024

	bool wait(LineCallbackT linecb, void *cbdata, int timeout = 0, DWORD cycle = WSO_LOOPTIMEOUT) {
		// �p�C�v����o�͂�ǂݍ���
		ttstr output;
		DWORD cnt, last=::GetTickCount();
		::PeekNamedPipe(hOR, 0, 0, 0, &cnt, NULL);
		char buf[BUF_SIZE], crlf=0;
		char tmp[BUF_SIZE+1];			//�����̏�ʃo�C�g�������f����Ă��܂����ꍇ�ɕ��ג����G���A
		char kind[BUF_SIZE+1];			//�����\���� 0:���p 1:�S�p��ʃo�C�g 2:�S�p���ʃo�C�g
		char halfchar;					//���f���ꂽ��ʃo�C�g
		bool ishalf = false;			//���f����Ă��邩�ǂ���
		tjs_char wbuf[BUF_SIZE+1];
		bool rest = false;
		int line = 0;

		if (!hasError()) while (true) {
			if (cnt > 0) {
				last = GetTickCount();
				::ReadFile(hOR, buf, sizeof(buf)-1, &cnt, NULL);
				buf[cnt] = 0;
				if( ishalf )
				{
					//���f���ꂽ��ʃo�C�g�ɘA��
					ZeroMemory(tmp, sizeof(tmp));
					tmp[0] = halfchar;
					memcpy( &tmp[1], buf, cnt );
					cnt++;
					memcpy( buf, tmp, cnt );
				}
				halfchar = 0;
				ishalf = false;

				//�p�C�v����ǂݍ��񂾃f�[�^�̏I�[�}���`�o�C�g���肨��ѓr���r���ł�
				//���s�R�[�h���肪�����ŕK�v�Ȃ̂Ő�ɐ擪����S���Ȃ߂Ă���
				ZeroMemory(kind, sizeof(kind));
				for (DWORD pos = 0; pos < cnt; pos++) {
					unsigned char cl = buf[pos];
					if( pos )
					{
						//�O�̃o�C�g���S�p��ʃo�C�g�������疳�����ŉ��ʃo�C�g����
						if ( kind[pos-1] == 1 )	{
							kind[pos] = 2;
							continue;
						}
					}
					//�R�}���h���C���̕W�����o�͂Ȃ̂�SJIS�O��ł̌Œ菈���F��ʃo�C�g����
					if ( cl > 0x80 && cl < 0xA0 || cl > 0xDF && cl < 0xFD )
						kind[pos] = 1;
				}

				if ( kind[cnt-1] == 1 ) {
					//�ŏI�o�C�g���}���`�o�C�g�̏�ʃo�C�g���ǂ�������
					ishalf = true;
					halfchar = buf[cnt-1];
					cnt--;
					buf[cnt] = 0;
				}
				DWORD start = 0;
				bool mb = false;
				for (DWORD pos = 0; pos < cnt; pos++) {
					char ch = buf[pos];
					//���s�R�[�h����͔��p(kind=0)�ł��邱�Ƃ��O��
					if ( (ch == '\r' || ch == '\n') && ! kind[pos] ) {
						if (crlf == 0 || crlf == ch) {
							buf[pos] = 0;

							//	�}���`�o�C�g����������C�h������ɕϊ����� ttstr �ɓ���Ȃ��Ɨ�O���o��
							ZeroMemory(wbuf, sizeof(wbuf));
							MultiByteToWideChar(0, 0, buf+start, pos-start, wbuf, sizeof(wbuf)-1);
							ttstr append(wbuf);

							output += append;
							linecb(cbdata, line++, output.c_str());
							output.Clear();
							buf[pos] = ch;
							crlf = 0;
						}
						if (crlf) crlf = 0;
						else crlf = ch;
						start = pos+1;
					} else {
						crlf = 0;
					}
				}
				if ((rest = (start < cnt))) {
					ZeroMemory(wbuf, sizeof(wbuf));
					MultiByteToWideChar(0, 0, buf+start, cnt-start, wbuf, sizeof(wbuf)-1);
					ttstr append(wbuf);

					output += append;
				}
				if ((int)cnt == sizeof(buf)-1) {
					::PeekNamedPipe(hOR, 0, 0, 0, &cnt, NULL);
					if (cnt > 0) continue;
				}
			} else {
				if (timeout > 0 && ::GetTickCount() > last+timeout) {
					::TerminateProcess(pi.hProcess, -1);
					error = ERR_TOUT;
					timeouted = true;
				}
			}
			DWORD wait = ::WaitForSingleObject(pi.hProcess, cycle);
			if (wait == WAIT_FAILED) {
				error = ERR_WAIT;
				break;
			}
			::PeekNamedPipe(hOR, 0, 0, 0, &cnt, NULL);
			if (cnt == 0 && wait == WAIT_OBJECT_0) {
				::GetExitCodeProcess(pi.hProcess, &exitcode);
				break;
			}
		}
		if (rest) linecb(cbdata, line++, output.c_str());

		return hasError();
	}

	bool hasError() const { return (error != ERR_NONE); }
	LPCWSTR getLastError() const {
		switch (error) {
		case ERR_PIPE: return L"can't create/duplicate pipe";
		case ERR_PROC: return L"can't create child process";
		case ERR_TOUT: return L"child process timeout";
		case ERR_WAIT: return L"child process wait failed";
		}
		return L"";
	}

	DWORD getExitCode(bool *err = 0, bool *tout = 0, LPCWSTR *errmes = 0) const {
		if (err )   *err    = hasError();
		if (tout)   *tout   = timeouted;
		if (errmes) *errmes = getLastError();
		return exitcode;
	}

	HANDLE getProcessHandle() const { return pi.hProcess;    }
	DWORD  getProcessId()     const { return pi.dwProcessId; }
};



/**
 * ���\�b�h�ǉ��p�N���X
 */
class Process {

protected:
	iTJSDispatch2 *objthis; //< �I�u�W�F�N�g���̎Q��
	HWND msgHWND;
	HANDLE process;

	typedef Process SelfClass;
#define MSGWND_CLASSNAME  L"TVP Process Message Window Class"
#define MSGWND_WINDOWNAME L"TVP Process Message"
	static ATOM MessageWindowClass;
	HWND  createMessageWindow() {
		HINSTANCE hinst = ::GetModuleHandle(NULL);
		if (!MessageWindowClass) {
			WNDCLASSEXW wcex = {
				/*size*/sizeof(WNDCLASSEX), /*style*/0, /*proc*/WndProc, /*extra*/0L,0L, /*hinst*/hinst,
				/*icon*/NULL, /*cursor*/NULL, /*brush*/NULL, /*menu*/NULL,
				/*class*/MSGWND_CLASSNAME, /*smicon*/NULL };
			if (!(MessageWindowClass = ::RegisterClassExW(&wcex)))
				TVPThrowExceptionMessage(TJS_W("register window class failed."));
		}
		HWND hwnd = ::CreateWindowExW(0, (LPCWSTR)MAKELONG(MessageWindowClass, 0), MSGWND_WINDOWNAME,
									  0, 0, 0, 1, 1, HWND_MESSAGE, NULL, hinst, NULL);
		if (!hwnd) TVPThrowExceptionMessage(TJS_W("create message window failed."));
		::SetWindowLong(hwnd, GWL_USERDATA, (LONG)this);
		return hwnd;
	}
	HWND destroyMessageWindow(HWND hwnd) {
		if (hwnd) {
			::SetWindowLong(hwnd, GWL_USERDATA, 0);
			::DestroyWindow(hwnd);
		}
		return NULL;
	}
	static LRESULT WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		SelfClass *self = (SelfClass*)(::GetWindowLong(hwnd, GWL_USERDATA));
		if (self) switch (msg) {
		case WM_SHELLEXECUTED:
			self->onShellExecuted(lp);
			return 0;
		case WM_SHELLCONSOLEOUT:
			self->onCommandLineOutput(wp, lp);
			return 0;
		}
		return DefWindowProc(hwnd, msg, wp, lp);
	}
public:
	static void UnregisterMessageWindowClass() {
		if (MessageWindowClass != 0 && ::UnregisterClassW((LPCWSTR)MAKELONG(MessageWindowClass, 0), ::GetModuleHandle(NULL)))
			MessageWindowClass = 0;
	}

protected:

	// �G���[�擾
	static void getLastError(ttstr &message) {
		LPVOID lpMessageBuffer;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					   NULL, GetLastError(),
					   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					   (LPWSTR)&lpMessageBuffer, 0, NULL);
		message = ((tjs_char*)lpMessageBuffer);
		LocalFree(lpMessageBuffer);
	}

	/**
	 * �C�x���g����
	 * �R�}���h���s����
	 */
	void onShellExecuted(LPARAM lp) {
		tTJSVariant endCode = (tjs_int)lp;
		tTJSVariant *p[] = {&endCode};
		objthis->FuncCall(0, L"onExecuted", NULL, NULL, 1, p, objthis);
		process = INVALID_HANDLE_VALUE;
	}

	/**
	 * �C�x���g����
	 * �s�o��
	 */
	void onCommandLineOutput(WPARAM wp, LPARAM lp) {
		tTJSVariant line = (tjs_int)wp;
		tTJSVariant text = ttstr((LPCWSTR)lp);
		tTJSVariant *p[] = {&line, &text};
		objthis->FuncCall(0, L"onOutput", NULL, NULL, 2, p, objthis);
	}

public:
	/**
	 * �C���X�^���X�����t�@�N�g��
	 */
	static tjs_error factory(Process **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis) {
		*result = new Process(objthis);
		return S_OK;
	}

	// �R���X�g���N�^
	Process(iTJSDispatch2 *objthis) : objthis(objthis), process(INVALID_HANDLE_VALUE) {
		msgHWND = createMessageWindow();
	}

	// �f�X�g���N�^
	~Process() {
		msgHWND = destroyMessageWindow(msgHWND);
	}

	// ��ԎQ��
	int getStatus() {
		return process == INVALID_HANDLE_VALUE ? 0 : 1;
	}

protected:

	/**
	 * ���s���
	 */
	struct ExecuteInfo {
		HWND   message; // ���b�Z�[�W���M��
		HANDLE process; // �҂��Ώۃv���Z�X
		ExecuteInfo(HWND message, HANDLE process)
			 : message(message), process(process) {}
	};
	
	/**
	 * �I���҂��X���b�h����
	 * @param data ���[�U(ExecuteInfo)
	 */
	static void waitExecute(void *data) {
		// �p�����[�^�����p��
		HWND   message = ((ExecuteInfo*)data)->message;
		HANDLE process = ((ExecuteInfo*)data)->process;
		delete data;

		// �v���Z�X�҂�
		WaitForSingleObject(process, INFINITE);
		DWORD dt;
		GetExitCodeProcess(process, &dt); // ���ʎ擾
		CloseHandle(process);

		// ���M
		PostMessage(message, WM_SHELLEXECUTED, NULL, (LPARAM)dt);
	}

	/**
	 * �v���Z�X�̎��s
	 * @param target �^�[�Q�b�g
	 * @praam param �p�����[�^
	 * @param folder �t�H���_
	 */
	bool _execute(ttstr target, const tjs_char *param, const tjs_char *folder) {
		terminate();

		ttstr cmd(L"\"");
		// �g���g���T�[�`�p�X��ɂ���ꍇ�͂������D��
		if (TVPIsExistentStorage(target)) {
			ttstr tmp = TVPGetPlacedPath(target);
			TVPGetLocalName(tmp);
			/**/cmd += tmp    + L"\"";
		} else  cmd += target + L"\"";

		SHELLEXECUTEINFO si;
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);
		si.lpVerb = _T("open");
		si.lpFile       = cmd.c_str();
		si.lpParameters = param;
		si.lpDirectory  = folder;
		si.nShow = SW_SHOWNORMAL;
		si.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
		if (ShellExecuteEx(&si)) {
			if (_beginthread(waitExecute, 0, new ExecuteInfo(msgHWND, si.hProcess)) != -1L) {
				process = si.hProcess;
				return true;
			}
		}
		return false;
	}

	/**
	 * ���s���
	 */
	struct CommandInfo {
		HWND   message; // ���b�Z�[�W���M��
		CommandExecute *cmd; // �R�}���h���s���
		CommandInfo(HWND message, CommandExecute *cmd = 0)
			 : message(message), cmd(cmd) {}
	};

	/**
	 * �e�L�X�g�o�͗p
	 */
	static void consoleLineOutCallback(void *v, int line, LPCWSTR text) {
		SendMessage((HWND)v, WM_SHELLCONSOLEOUT, (WPARAM)line, (LPARAM)text);
	}
	
	/**
	 * commandExec �I���҂��X���b�h����
	 * @param data ���[�U(CommandInfo)
	 */
	static void waitCommand(void *data) {
		// �p�����[�^�����p��
		HWND        message = ((CommandInfo*)data)->message;
		CommandExecute *cmd = ((CommandInfo*)data)->cmd;
		delete data;

		cmd->wait(consoleLineOutCallback, (void*)message, 0);
		DWORD exit = cmd->getExitCode();
		delete cmd;
		
		// ���M
		PostMessage(message, WM_SHELLEXECUTED, NULL, (LPARAM)exit);
	}

	/**
	 * �R�}���h���C���̎��s
	 * @param target �^�[�Q�b�g
	 * @param param �p�����[�^
	 */
	bool _commandExecute(ttstr target, const tjs_char *param, const tjs_char *folder) {
		terminate();
		CommandExecute *cmd = new CommandExecute();
		if (cmd->start(target, param, folder)) {
			if (_beginthread(waitCommand, 0, new CommandInfo(msgHWND, cmd)) != -1L) {
				process = cmd->getProcessHandle();
				return true;
			}
		}
		delete cmd;
		return false;
	}

	// -------------------------------------------------------------------------------------

public:

	static tjs_error TJS_INTF_METHOD execute(tTJSVariant *r, tjs_int n, tTJSVariant **p, Process *self) {
		if (n < 1) return TJS_E_BADPARAMCOUNT;
		if (!self) return TJS_E_NATIVECLASSCRASH;
		const tjs_char *param  = n > 1 ? p[1]->GetString() : 0;
		const tjs_char *folder = n > 2 ? p[2]->GetString() : 0;
		bool ret = self->_execute(*p[0], param, folder);
		if (r) {
			*r = ret;
		}
		return TJS_S_OK;
	}

	static tjs_error TJS_INTF_METHOD commandExecute(tTJSVariant *r, tjs_int n, tTJSVariant **p, Process *self) {
		if (n < 1) return TJS_E_BADPARAMCOUNT;
		if (!self) return TJS_E_NATIVECLASSCRASH;
		const tjs_char *param  = n > 1 ? p[1]->GetString() : 0;
		const tjs_char *folder = n > 2 ? p[2]->GetString() : 0;
		bool ret = self->_commandExecute(*p[0], param, folder);
		if (r) {
			*r = ret;
		}
		return TJS_S_OK;
	}
	
	/**
	 * �v���Z�X�̒�~
	 * @param endCode �I���R�[�h
	 */
	void terminate(int endCode=-1) {
		if (process != INVALID_HANDLE_VALUE) {
			::TerminateProcess(process, endCode);
			process = INVALID_HANDLE_VALUE;
		}
	}

	/**
	 * �V�O�i�����M
	 */
	bool sendSignal(bool type) {
		if (process != INVALID_HANDLE_VALUE) {
			DWORD id = ::GetProcessId(process);
			DWORD ev = type ? CTRL_BREAK_EVENT : CTRL_C_EVENT;
			BOOL r = ::GenerateConsoleCtrlEvent(ev, id);
			if (!r) {
				if (::AttachConsole(id)) {
					r = ::GenerateConsoleCtrlEvent(ev, id);
					::FreeConsole();
				} else  {
					ttstr err;
					getLastError(err);
					TVPAddLog(err.c_str());
				}
			}
			return !!r;
		}
		return false;
	}
};

ATOM Process::MessageWindowClass = 0;
static void PostUnregistCallback() { Process::UnregisterMessageWindowClass(); }
NCB_POST_UNREGIST_CALLBACK(PostUnregistCallback);

NCB_REGISTER_CLASS(Process) {
	Factory(&ClassT::factory);
	RawCallback("execute", &ClassT::execute, 0);
	RawCallback("commandExecute", &ClassT::commandExecute, 0);
	Method(L"terminate",      &ClassT::terminate);
	Method(L"sendSignal",     &ClassT::sendSignal);
	NCB_PROPERTY_RO(status, getStatus);
}


static void cmdExecLineCallback(void *va, int line, LPCWSTR text) {
	iTJSDispatch2 *array = (iTJSDispatch2*)va;
	array->PropSetByNum(TJS_MEMBERENSURE, line, &tTJSVariant(text), array);
}

/**
 * �R�}���h���C���Ăяo��
 */
tjs_error TJS_INTF_METHOD commandExecute(
	tTJSVariant *result, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	tTJSVariant vStdOut;

	// �p�����[�^�`�F�b�N
	if (numparams == 0) return TJS_E_BADPARAMCOUNT;
	if (param[0]->Type() != tvtString) return TJS_E_INVALIDPARAM;

	// �R�}���h���C��/�^�C���A�E�g�擾
	int timeout = 0;
	ttstr target(param[0]->GetString());
	const tjs_char *cmdprm = 0;

	if (numparams > 1) cmdprm  = param[1]->GetString();
	if (numparams > 2) timeout = (tjs_int)*param[2];

	DWORD exit = ~0L;
	bool haserr = true, timeouted = false;
	LPCWSTR errmes = 0;

	iTJSDispatch2 *array = TJSCreateArrayObject();
	if (array != 0) {
		CommandExecute exec;
		if (exec.start(target, cmdprm)) exec.wait(cmdExecLineCallback, array, timeout);
		exit = exec.getExitCode(&haserr, &timeouted, &errmes);

		vStdOut.SetObject(array, array);
		array->Release();
	}

	ncbDictionaryAccessor ret;
	if (ret.IsValid()) {
		ret.SetValue(L"stdout", vStdOut);
		if (haserr && errmes != 0) {
			ret.SetValue(L"status", ttstr(timeouted ? L"timeout" : L"error"));
			ret.SetValue(L"message", ttstr(errmes));
		} else {
			ret.SetValue(L"status", ttstr(haserr ? L"failed" : L"ok"));
			ret.SetValue(L"exitcode", (tjs_int)exit);
		}
	}
	if (result != NULL) *result = ret;
	return TJS_S_OK;
}

NCB_ATTACH_FUNCTION(commandExecute,    System, commandExecute);

