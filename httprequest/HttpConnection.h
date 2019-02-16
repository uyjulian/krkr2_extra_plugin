#ifndef __HTTPCONNECTION_H_
#define __HTTPCONNECTION_H_

#include <windows.h>
#include <tchar.h>
#include <WinInet.h>
#include <vector>
#include <string>
#include <map>

#include "Base64.h"

using namespace std;
typedef basic_string<TCHAR> tstring;

/**
 * HTTP�ڑ�����������N���X
 */
class HttpConnection
{

public:
	// �G���[���
	enum Error {
		ERROR_NONE,  // �G���[�Ȃ�
		ERROR_INET,  // �l�b�g���[�N���C�u�����̃G���[
		ERROR_CANCEL // �L�����Z�����ꂽ
	};

	/**
	 * ���N�G�X�g�p�R�[���o�b�N����
	 * @param context �R���e�L�X�g
	 * @param buffer �������ݐ�f�[�^�o�b�t�@
	 * @param size �������ݐ�f�[�^�o�b�t�@�̃T�C�Y�B���ۂɏ������񂾃T�C�Y���i�[���ĕԂ�
	 * @return ���f����ꍇ�� true ��Ԃ�
	 */
	typedef bool (*RequestCallback)(void *context, void *buffer, DWORD &size);


	/**
	 * ���g���C�p�R�[���o�b�N����
	 * @param context �R���e�L�X�g
	 * @return ���f����ꍇ�� true ��Ԃ�
	 */
	typedef void (*RetryCallback)(void *context);
	
	/**
	 * ���X�|���X�p�R�[���o�b�N����
	 * @param context �R���e�L�X�g
	 * @param buffer �ǂݍ��݌��f�[�^�o�b�t�@�B�Ō�� NULL
	 * @param size �f�[�^�o�b�t�@�̃T�C�Y�B�Ō��0
	 * @return ���f����ꍇ�� true ��Ԃ�
	 */
	typedef bool (*ResponseCallback)(void *context, const void *buffer, DWORD size);
	
	/**
	 * �R���X�g���N�^
	 * @param agentName �G�[�W�F���g��
	 * @param checkCert �F�؊m�F���邩�ǂ���
	 */
	HttpConnection(tstring agentName, bool checkCert=false) : agentName(agentName), checkCert(checkCert), contentLength(0), secure(false){
		::InitializeCriticalSection(&cs);
		hInet = NULL;
		hConn = NULL;
		hReq  = NULL;
	}

	// �f�X�g���N�^
	~HttpConnection(void) {
		clearParam();
		::DeleteCriticalSection(&cs);
	}

	// ���M�w�b�_���N���A
	void clearHeader() {
		header.clear();
		requestContentLength = 0;
		requestContentType.erase();
		requestEncoding.erase();
	}

	// �n���h�����N���A
	void closeHandle();

	// ���M�p�����[�^���N���A(���O��ς��������ŁA���̂͑��M�f�[�^�N���A)
	void clearParam() {
		closeHandle();
		clearHeader();
	}

	// ----------------------------------------------------------------------------------------
	
	// HTTP �w�b�_��ǉ�����
	void addHeader(const TCHAR *name, const TCHAR *value);

	// �F�؃w�b�_���Z�b�g����(addHeader �̃��[�e�B���e�B)
	void addBasicAuthHeader(const tstring &user, const tstring &passwd) {
		tstring sendStr = user + _T(":") + passwd;
		tstring value = _T("Basic") + base64encode(sendStr.c_str(), sendStr.length());
		addHeader(_T("Authorization"), value.c_str());
	}
	
	// ----------------------------------------------------------------------------------------------------
	
	/**
	 * ���N�G�X�g�J�n
	 * @param method �A�N�Z�X���\�b�h
	 * @param url URL
	 * @param user �A�N�Z�X���[�U
	 * @param passwd �A�N�Z�X�p�X���[�h
	 * @return ���������� true
	 */
	bool open(const TCHAR *method,
			  const TCHAR *url,
			  const TCHAR *user = NULL,
			  const TCHAR *passwd = NULL);

	/**
	 * ���N�G�X�g���M
	 * @param callback ���M�p�R�[���o�b�N
	 * @param context �R�[���o�b�N�p�R���e�L�X�g
	 * @return �G���[
	 */
	int request(RequestCallback requestCallback=NULL, RetryCallback retryCalblack = NULL, void *context=NULL);


	/**
	 * ���X�|���X�擾�O�����W
	 */
	void queryInfo();
	
	/**
	 * ���X�|���X��M
	 * @param callback �ۑ��p�R�[���o�b�N
	 * @param context �R�[���o�b�N�p�R���e�L�X�g
	 * @return �G���[
	 */
	int response(ResponseCallback callback=NULL, void *context=NULL);
	
	// ----------------------------------------------------------------------------------------------------
	
	// �G���[���b�Z�[�W�̎擾
	const TCHAR *getErrorMessage() const {
		return errorMessage.c_str();
	}
	
	// �Ō�̃��N�G�X�g���������Ă��邩�ǂ���
	bool isValid() const {
		return hReq != NULL;
	}

	// HTTP�X�e�[�^�X�R�[�h���擾
	int getStatusCode() const {
		return statusCode;
	}

	// HTTP�X�e�[�^�X�e�L�X�g���擾
	const TCHAR *getStatusText() const {
		return statusText.c_str();
	}
	
	// �擾���ꂽ�R���e���c�̒���
	DWORD getContentLength() const {
		return contentLength;
	}

	// �R���e���c�� MIME-TYPE
	const TCHAR *getContentType() const {
		return contentType.c_str();
	}

	// �R���e���c�̃G���R�[�f�B���O���
	const TCHAR *getEncoding() const {
		return encoding.c_str();
	}

	// �R���e���c�̃G���R�[�f�B���O���
	const TCHAR *getRequestEncoding() const {
		return requestEncoding.c_str();
	}
	
	/**
	 * ���X�|���X�̃w�b�_�����擾
	 */
	const TCHAR *getResponseHeader(const TCHAR *name) {
		map<tstring,tstring>::const_iterator it = responseHeaders.find(tstring(name));
		if (it != responseHeaders.end()) {
			return it->second.c_str();
		}
		return NULL;
	}

	// ���X�|���X�w�b�_�S�擾�p:������
	void initRH() {
		rhit = responseHeaders.begin();
	}

	// ���X�|���X�w�b�_�S�擾�p:�擾
	bool getNextRH(tstring &name, tstring &value) {
		if (rhit != responseHeaders.end()) {
			name  = rhit->first;
			value = rhit->second;
			rhit++;
			return true;
		}
		return false;
	}

	bool getCheckCert() const { return checkCert;	}
	void setCheckCert(bool check)	{ checkCert = check;}

private:
	CRITICAL_SECTION cs;

	// ��b���
	tstring agentName; ///< ���[�U�G�[�W�F���g��
	bool checkCert;	   ///< �ؖ����m�F�_�C�A���O���o����
	bool secure;       ///< https �ʐM���ǂ���

	HINTERNET hInet; ///< �C���^�[�l�b�g�ڑ�
	HINTERNET hConn; ///< �R�l�N�V����
	HINTERNET hReq;  ///< HTTP���N�G�X�g
	
	// ���M�p�f�[�^
	vector<tstring> header;	///< HTTP �w�b�_
	DWORD requestContentLength; ///< ���N�G�X�g�� Content-Length:
	tstring requestContentType; ///< ���N�G�X�g�� Content-Type:
	tstring requestEncoding;    ///< ���N�G�X�g�̃G���R�[�h�w��

	// ��M�p�f�[�^
	bool validContentLength;
	DWORD contentLength;     ///< Content-Length:
	tstring contentType;     ///< Content-Type: ��type��
	tstring encoding;        ///< Content-TYpe: �̃G���R�[�f�B���O��

	DWORD statusCode;        ///< HTTP status code
	tstring statusText;      ///< HTTP status text
	map<tstring,tstring> responseHeaders; ///< ���X�|���X�w�b�_
	map<tstring,tstring>::const_iterator rhit; //< ���X�|���X�w�b�_�Q�Ɨp�C�e���[�^

	// �G���[�R�[�h
	tstring errorMessage; ///< �G���[���b�Z�[�W
};

#endif
