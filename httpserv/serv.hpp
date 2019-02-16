#ifndef _serv_hpp_
#define _serv_hpp_

/*
 *  Poco::Net ���b�p�[�iinclude�����݂���Ƃ�����ƍ��������ƂɂȂ�̂ŕ�������j
 */

#include <string>

// ���X�|���X�����p
struct PwRequestResponse
{
	typedef std::string String;
	typedef unsigned long Size;
	typedef void (*NameValueCallback)(const String&, const String&, void *param);

	// �X���b�h�Ή��p
	virtual void done() = 0;

	// �e��f�[�^���擾
	virtual int  getHeader  (NameValueCallback, void *param) const = 0;
	virtual int  getFormData(NameValueCallback, void *param) const = 0;
	virtual const String& getMethod() const = 0;
	virtual const String& getURI()    const = 0;
	virtual const String& getPath()   const = 0;
	virtual const String& getHost()   const = 0;
	virtual const String& getClient() const = 0;

	// �ϊ����[�e�B���e�B
	virtual const String getCharset(char const *mediatype) = 0;
	virtual const String getReason(char const *status) = 0;

	// ���X�|���X��Ԃ�
	virtual void setStatus(char const *status) = 0;
	virtual void setContentType(char const *type) = 0;
	virtual void setRedirect(char const *type) = 0;
	// �ȉ��͍Ō�ɂǂ��炩�P��݂̂����ĂׂȂ��iContent-length�𑗐M���Ă��܂����ߕ������M�͕s�j
	virtual void sendBuffer(void const*, Size length) = 0;
	virtual void sendFile(char const *path) = 0;
};

// �T�[�o�p
struct PwHTTPServer
{
	typedef void (*RequestCallback)(PwRequestResponse *rr, void *param);

	virtual ~PwHTTPServer() {}
	virtual int  start(int port) = 0;
	virtual void stop()          = 0;

	static PwHTTPServer* Factory(RequestCallback, void *param, int timeoutsec);
};

#endif
