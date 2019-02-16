/*
 * XMLHttpRequest
 *
 * http://www.w3.org/TR/XMLHttpRequest/
 *
 * Written by Kouhei Yanagita
 *
 */

/*

�� �g�p��

var xhr = new XMLHttpRequest();
xhr.open('GET', 'http://example.com/', true);
xhr.onreadystatechange = function(xhr) {
    if (xhr.readyState != 4) return;

    // responseText �̓I�N�e�b�g���Ԃ��̂ŁA
    // ������ɂ��邽�߂ɂ́A�ϊ����K�v�B
    // encode �v���O�C���Ȃǂ��Q�ƁB
    Debug.message(decodeUTF8(xhr.responseText));
};
System.addContinuousHandler(xhr.executeCallback);
xhr.send();


�� ����

�u���E�U�Ɏ�������Ă��� XMLHttpRequest �ł�
readyState ���ω������ onreadystatechange ���Ă΂�܂����A
���̃v���O�C���ł� readyState ���ω����Ă��A�����̃L���[��
�ω����~�ς���邾���ŁA�R�[���o�b�N�͎����I�ɂ͌Ă΂�܂���B
�����̃L���[�� readyState �̕ω����~�ς���Ă����Ԃ�
executeCallback() ���ĂԂƁA���̒i�K�ŃR�[���o�b�N�����s����܂��B

���̂悤�ɂȂ��Ă���̂́A�}���`�X���b�h���ɕʃX���b�h����̃R�[���o�b�N��
�s����N����\�������邽�߂ł��B


�� ���t�@�����X

�R���X�g���N�^
- XMLHttpRequest()

���\�b�h
- open(String method, String url, [bool async, String username, String password])
���N�G�X�g�𓊂���Ώۂ�ݒ肵�܂��B

- setRequestHeader(String headerName, String headerValue)
���N�G�X�g�w�b�_��ݒ肵�܂��B

- send([Octet data])
�I�N�e�b�g�� data ���G���e�B�e�B�{�f�B�Ƃ��āA�T�[�o�Ƀ��N�G�X�g�𑗐M���܂��B

- abort()
�ʐM���L�����Z�����܂��B

- executeCallback()
onreadystatechange �̃R�[���o�b�N�����s���܂��B


�v���p�e�B
- int readyState
- Octet responseText
- int status
- String statusText

 */
//---------------------------------------------------------------------------
#include <winsock2.h>
#include <windows.h>
#include <process.h>
#include "tp_stub.h"
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
//---------------------------------------------------------------------------



class NI_XMLHttpRequest : public tTJSNativeInstance // �l�C�e�B�u�C���X�^���X
{
public:
    NI_XMLHttpRequest()
    {

    }

    tjs_error TJS_INTF_METHOD
        Construct(tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj)
    {
        // TJS2 �I�u�W�F�N�g���쐬�����Ƃ��ɌĂ΂��

        Initialize();
        _target = tjs_obj;

        if (++objcount == 1) {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
                // error
            }
        }

        _hThread = NULL;
        InitializeCriticalSection(&_criticalSection);

        return S_OK;
    }

    void TJS_INTF_METHOD Invalidate()
    {
        // �I�u�W�F�N�g�������������Ƃ��ɌĂ΂��

        if (_hThread) {
            CloseHandle(_hThread);
            _hThread = NULL;
        }

        if (--objcount == 0) {
            WSACleanup();
        }

        DeleteCriticalSection(&_criticalSection);
    }


    void Initialize(void) {
        _responseData.clear();
        _responseHeader.clear();
        _responseBody.clear();
        _responseStatus = 0;
        _requestHeaders.clear();
        _aborted = false;
    }

    tjs_int GetReadyState(void) const { return _readyState; }
    void SetReadyState(tjs_int v) { _readyState = v; OnReadyStateChange(); }

    tjs_int GetResponseStatus(void) const {
        RaiseExceptionIfNotResponsed();
        return _responseStatus;
    }

    ttstr GetResponseStatusText(void) const {
        RaiseExceptionIfNotResponsed();

        switch (_responseStatus) {
        case 200: return ttstr("OK");
        case 201: return ttstr("Created");
        case 202: return ttstr("Accepted");
        case 203: return ttstr("Non-Authoritative Information");
        case 204: return ttstr("No Content");
        case 205: return ttstr("Reset Content");
        case 206: return ttstr("Partial Content");
        case 300: return ttstr("Multiple Choices");
        case 301: return ttstr("Moved Permanently");
        case 302: return ttstr("Found");
        case 303: return ttstr("See Other");
        case 304: return ttstr("Not Modified");
        case 305: return ttstr("Use Proxy");
        case 307: return ttstr("Temporary Redirect");
        case 400: return ttstr("Bad Request");
        case 401: return ttstr("Unauthorized");
        case 402: return ttstr("Payment Required");
        case 403: return ttstr("Forbidden");
        case 404: return ttstr("Not Found");
        case 405: return ttstr("Method Not Allowed");
        case 406: return ttstr("Not Acceptable");
        case 407: return ttstr("Proxy Authentication Required");
        case 408: return ttstr("Request Timeout");
        case 409: return ttstr("Conflict");
        case 410: return ttstr("Gone");
        case 411: return ttstr("Length Required");
        case 412: return ttstr("Precondition Failed");
        case 413: return ttstr("Request Entity Too Large");
        case 414: return ttstr("Request-URI Too Long");
        case 415: return ttstr("Unsupported Media Type");
        case 416: return ttstr("Requested Range Not Satisfiable");
        case 417: return ttstr("Expectation Failed");
        case 500: return ttstr("Internal Server Error");
        case 501: return ttstr("Not Implemented");
        case 502: return ttstr("Bad Gateway");
        case 503: return ttstr("Service Unavailable");
        case 504: return ttstr("Gateway Timeout");
        case 505: return ttstr("HTTP Version Not Supported");
        default: return ttstr("");
        }
    }

    const std::vector<char>* GetResponseText(void) const {
        RaiseExceptionIfNotResponsed();

        return &_responseBody;
    }

    void Open(const ttstr &method, const ttstr &uri, bool async, const ttstr &username, const ttstr &password)
    {
        Initialize();

        if (method == ttstr(TJS_W("GET"))) {
            _method = std::string("GET");
        }
        else if (method == ttstr(TJS_W("POST"))) {
            _method = std::string("POST");
        }
        else {
            TVPThrowExceptionMessage(TJS_W("SYNTAX_ERR (Wrong method)"));
            return;
        }

        _async = async;

        boost::regex re(
            std::string("http://"
                  "("
                  "(?:[a-zA-Z0-9](?:[-a-zA-Z0-9]*[a-zA-Z0-9]|(?:))\\.)*[a-zA-Z](?:[-a-zA-Z0-9]*[a-zA-Z0-9]|(?:))\\.?" // hostname
                  "|"
                  "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+" // IPv4addr
                  ")"
                  "(?::([0-9]+))?" // port
                  "(.*)").c_str());
        boost::cmatch what;

        tjs_int narrow_len = uri.GetNarrowStrLen();
        if(narrow_len == -1) {
            TVPThrowExceptionMessage(TJS_W("string conversion failure"));
        }

        std::vector<char> narrow_str;
        narrow_str.reserve(narrow_len + 1);
        uri.ToNarrowStr(&narrow_str[0], narrow_len);
        std::string curi = std::string(&narrow_str[0]);

        bool matched = boost::regex_search(curi.c_str(), what, re, boost::match_default);

        if (!matched) {
            TVPThrowExceptionMessage(TJS_W("Wrong URL"));
            return;
        }

        _host = "";
        _host.append(what[1].first, what[1].second);

        if (what[2].matched) {
            _port = TJSStringToInteger(ttstr(what[2].first, what[2].second - what[2].first).c_str());
        }
        else {
            _port = 80;
        }
        
        _path = "";
        _path.append(what[3].first, what[3].second);

        if (username.GetLen() > 0) {
            if (IsValidUserInfo(username, password)) {
                std::string authKey = "";
                std::copy(username.c_str(), username.c_str() + username.length(), std::back_inserter(authKey));
                authKey.append(":");
                std::copy(password.c_str(), password.c_str() + password.length(), std::back_inserter(authKey));

                _requestHeaders.insert(std::pair<std::string, std::string>("Authorization", std::string("Basic ") + EncodeBase64(authKey)));
            }
            else {
                TVPThrowExceptionMessage(TJS_W("Wrong UserInfo"));
                return;
            }
        }

        SetReadyState(1);
    }

    void Send(tTJSVariant *data)
    {
        if (_hThread) {
            TVPAddLog(ttstr("���X�|���X���߂�O�� send ���܂���"));
            return;
        }

        if (_readyState != 1) {
            TVPThrowExceptionMessage(TJS_W("INVALID_STATE_ERR"));
            return;
        }

        _aborted = false;

        _sendingData.clear();
        if (data) {
            tTJSVariantOctet *oct = data->AsOctetNoAddRef();
            _sendingData.reserve(oct->GetLength());
            std::copy(oct->GetData(), oct->GetData() + oct->GetLength(), std::back_inserter(_sendingData));
            std::string slen = boost::lexical_cast<std::string>(_sendingData.size());
            _requestHeaders.insert(std::pair<std::string, std::string>("Content-Length", slen));
        }

        std::string hostHeader = _port == 80 ? _host : _host + ":" + boost::lexical_cast<std::string>(_port);
        _requestHeaders.insert(std::pair<std::string, std::string>("Host", hostHeader));

        if (_async) {
            _hThread = (HANDLE)_beginthreadex(NULL, 0, StartProc, this, 0, NULL);
        }
        else {
            _Send();
        }
    }

    void _Send(void)
    {
        _responseData.clear();

        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
            OnErrorOnSending();
            return;
        }

        sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(_port);
        server.sin_addr.S_un.S_addr = inet_addr(_host.c_str());

        if (server.sin_addr.S_un.S_addr == 0xffffffff) {
            hostent *hst;
            hst = gethostbyname(_host.c_str());
            if (!hst) {
                OnErrorOnSending();
                return;
            }

            unsigned int **addrptr;
            addrptr = (unsigned int **)hst->h_addr_list;

            while (*addrptr) {
                server.sin_addr.S_un.S_addr = **addrptr;
                if (!connect(sock, (struct sockaddr *)&server, sizeof(server))) {
                    break;
                }
                ++addrptr;
            }

            if (!*addrptr) {
                OnErrorOnSending();
                return;
            }
        }
        else {
            if (connect(sock, (struct sockaddr *)&server, sizeof(server))) {
                OnErrorOnSending();
                return;
            }
        }

        std::ostringstream req;
        std::vector<char> reqv;
        std::string reqstr;
        char buf[4096];
        int n;

        if (_aborted) {
            goto onaborted;
        }

        req << _method << " " << _path << " HTTP/1.1\r\n";
        for (header_container::const_iterator p = _requestHeaders.begin(); p != _requestHeaders.end(); ++p) {
            req << p->first << ": " << p->second << "\r\n";
        }
        req << "\r\n";

        reqstr = req.str();
        std::copy(reqstr.begin(), reqstr.end(), std::back_inserter(reqv));

        if (!_sendingData.empty()) {
            std::copy(_sendingData.begin(), _sendingData.end(), std::back_inserter(reqv));
        }

        n = send(sock, &reqv[0], reqv.size(), 0);
        SetReadyState(2);
        if (n < 0) {
            OnErrorOnSending();
            return;
        }

        SetReadyState(3);

        fd_set fds, readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 50 * 1000;
        while (!_aborted && n > 0) {
            memcpy(&fds, &readfds, sizeof(fd_set));
            while (select(0, &fds, NULL, NULL, &tv) == 0) {
                if (_aborted) {
                    goto onaborted;
                }
            }

            memset(buf, 0, sizeof(buf));
            n = recv(sock, buf, sizeof(buf), 0);
            _responseData.insert(_responseData.end(), buf, buf + n);
            if (n < 0) {
                OnErrorOnSending();
                return;
            }
        }

    onaborted:
        closesocket(sock);

        if (_hThread) {
            CloseHandle(_hThread);
        }
        _hThread = NULL;

        if (!_aborted) {
            ParseResponse();
            SetReadyState(4);
        }
    }

    void OnReadyStateChange() {
        EnterCriticalSection(&_criticalSection);

        if (_async && _target->IsValid(TJS_IGNOREPROP, L"onreadystatechange", NULL, _target) == TJS_S_TRUE) {
            _readyStateChangeQueue.push_back(_readyState);
        }

        LeaveCriticalSection(&_criticalSection);
    }

    void ExecuteOnReadyStateChangeCallback(void) {
        EnterCriticalSection(&_criticalSection);
        Sleep(1);
        for (std::vector<tjs_int>::const_iterator p = _readyStateChangeQueue.begin(); p != _readyStateChangeQueue.end(); ++p) {
            tTJSVariant val;
            if (_target->PropGet(TJS_IGNOREPROP, L"onreadystatechange", NULL, &val, _target) < 0) {
                ttstr msg = TJS_W("can't get member: onreadystatechange");
                TVPThrowExceptionMessage(msg.c_str());
            }

            try {
                tTJSVariant v = _target;
                tTJSVariant *param[] = { &v };
                tTJSVariantClosure funcval = val.AsObjectClosureNoAddRef();
                funcval.FuncCall(0, NULL, NULL, NULL, 1, param, NULL);
            }
            catch (...) {
                LeaveCriticalSection(&_criticalSection);
                throw;
            }
        }
        _readyStateChangeQueue.clear();

        LeaveCriticalSection(&_criticalSection);
    }


    static unsigned __stdcall StartProc(void *arg)
    {
        ((NI_XMLHttpRequest*)arg)->_Send();
        return 0;
    }

    /*
     * �}�[�W�����ɒP�ɐV�����l�ŏ㏑������
     */
    void SetRequestHeader(const ttstr &header, const ttstr &value)
    {
        if (_readyState != 1) {
            TVPThrowExceptionMessage(TJS_W("INVALID_STATE_ERR"));
        }

        if (!IsValidHeaderName(header)) {
            TVPThrowExceptionMessage(TJS_W("SYNTAX_ERR"));
        }

        if (!IsValidHeaderValue(value)) {
            TVPThrowExceptionMessage(TJS_W("SYNTAX_ERR"));
        }

        std::string sheader;
        std::string svalue;
        std::copy(header.c_str(), header.c_str() + header.length(), std::back_inserter(sheader));
        std::copy(value.c_str(), value.c_str() + value.length(), std::back_inserter(svalue));

        _requestHeaders.erase(sheader);
        _requestHeaders.insert(std::pair<std::string, std::string>(sheader, svalue));
    }

    ttstr GetResponseHeader(const ttstr &header)
    {
        if (!IsValidHeaderName(header)) {
            TVPThrowExceptionMessage(TJS_W("SYNTAX_ERR"));
        }

        if (_readyState < 3) {
            TVPThrowExceptionMessage(TJS_W("INVALID_STATE_ERR"));
        }

        tjs_int narrow_len = header.GetNarrowStrLen();
        if(narrow_len == -1) {
            TVPThrowExceptionMessage(TJS_W("string conversion failure"));
        }

        std::vector<char> narrow_str;
        narrow_str.reserve(narrow_len + 1);
        header.ToNarrowStr(&narrow_str[0], narrow_len);

        std::string s = std::string(&narrow_str[0]) + ":";
        std::vector<char>::iterator p = std::search(_responseHeader.begin(), _responseHeader.end(), s.begin(), s.end());
        if (p == _responseHeader.end()) {
            return TJS_W("");
        }
        else {
            std::string crlf = "\r\n";
            std::vector<char>::iterator q = std::search(p + s.size(), _responseHeader.end(), crlf.begin(), crlf.end());

            std::vector<char>::iterator beg = p + s.size();
            while (*beg == ' ' && beg < q) {
                ++beg;
            }
            std::vector<char>::iterator end = q;
            while (*(end - 1) == ' ' && beg < end) {
                --end;
            }

            std::vector<tjs_char> result;
            result.reserve(end - beg + 1);
            std::copy(beg, end, std::back_inserter(result));
            result.push_back(0);

            return ttstr(&result[0]);
        }
    }


    // for debug
    void PrintRequestHeaders(void)
    {
        for (header_container::const_iterator p = _requestHeaders.begin(); p != _requestHeaders.end(); ++p) {
            TVPAddLog((p->first + ": " + p->second).c_str());
        }
    }

    void Abort(void)
    {
        _aborted = true;

        if (_hThread) {
            WaitForSingleObject(_hThread, INFINITE);
        }

        Initialize();
    }

private:
    void OnErrorOnSending()
    {
        _readyState = 4;
    }

    void RaiseExceptionIfNotResponsed(void) const
    {
        if (_readyState != 3 && _readyState != 4) {
            TVPThrowExceptionMessage(TJS_W("INVALID_STATE_ERR"));
        }
    }

    bool IsValidUserInfo(const ttstr &username, const ttstr &password)
    {
        if (username.length() == 0 || password.length() == 0) {
            return true;
        }

        return std::find_if(username.c_str(), username.c_str() + username.length(), NI_XMLHttpRequest::IsInvalidUserInfoCharacter) ==
            username.c_str() + username.length() &&
            std::find_if(password.c_str(), password.c_str() + password.length(), NI_XMLHttpRequest::IsInvalidUserInfoCharacter) ==
            password.c_str() + password.length();
    }

    static bool IsInvalidUserInfoCharacter(tjs_char c)
    {
        return c > 127; // non US-ASCII character
    }

    bool IsValidHeaderName(const ttstr &header)
    {
        return header.length() > 0 &&
            std::find_if(header.c_str(), header.c_str() + header.length(), NI_XMLHttpRequest::IsInvalidHeaderNameCharacter) ==
            header.c_str() + header.length();
    }

    static bool IsInvalidHeaderNameCharacter(tjs_char c)
    {
        if (c > 127) return true; // non US-ASCII character
        if (c <= 31 || c == 127) return true; // CTL
        const std::wstring separators = L"()<>@,;:\\\"/[]?={} \t";
        return std::find(separators.begin(), separators.end(), c) != separators.end();
    }

    bool IsValidHeaderValue(const ttstr &value)
    {
        // �P���̂��� "\r\n" �͋����Ȃ����Ƃɂ���
        if (wcsstr(value.c_str(), L"\r\n")) {
            return false;
        }

        return true;
    }

    void ParseResponse()
    {
        boost::regex re("\\AHTTP/[0-9]+\\.[0-9]+ ([0-9][0-9][0-9])");
        boost::cmatch what;
        bool matched = boost::regex_search(&_responseData[0], what, re, boost::match_default);

        if (matched) {
            std::string s(what[1].first, what[1].second - what[1].first);
            _responseStatus = boost::lexical_cast<int>(s);
        }

        _responseHeader.clear();
        _responseBody.clear();
        std::string sep("\r\n\r\n");
        std::vector<char>::iterator p = std::search(_responseData.begin(), _responseData.end(), sep.begin(), sep.end());
        if (p != _responseData.end()) {
            _responseHeader.reserve(p - _responseData.begin());
            std::copy(_responseData.begin(), p, std::back_inserter(_responseHeader));

            _responseBody.reserve(_responseData.end() - p - sep.size());
            std::copy(p + sep.size(), _responseData.end(), std::back_inserter(_responseBody));
        }
    }

    std::string EncodeBase64(const std::string target)
    {
        std::string result = "";
        std::vector<unsigned char> r;

        int len, restlen;
        len = restlen = target.length();

        while (restlen >= 3) {
            char t1 = target[len - restlen];
            char t2 = target[len - restlen + 1];
            char t3 = target[len - restlen + 2];

            r.push_back(t1 >> 2);
            r.push_back(((t1 & 3) << 4) | (t2 >> 4));
            r.push_back(((t2 & 0x0f) << 2) | (t3 >> 6));
            r.push_back(t3 & 0x3f);

            restlen -= 3;
        }

        if (restlen == 1) {
            char t1 = target[len - restlen];
            char t2 = '\0';
            r.push_back(t1 >> 2);
            r.push_back(((t1 & 3) << 4) | (t2 >> 4));

        }
        else if (restlen == 2) {
            char t1 = target[len - restlen];
            char t2 = target[len - restlen + 1];
            char t3 = '\0';
            r.push_back(t1 >> 2);
            r.push_back(((t1 & 3) << 4) | (t2 >> 4));
            r.push_back(((t2 & 0x0f) << 2) | (t3 >> 6));
        }

        for (std::vector<unsigned char>::const_iterator p = r.begin(); p != r.end(); ++p) {
            result.append(1, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[*p]);
        }

        if (restlen > 0) {
            result.append(3 - restlen, '=');
        }

        return result;
    }
private:
    tjs_int _readyState;
    std::string _method;
    bool _async;
    int _port;
    std::string _host;
    std::string _path;
    std::vector<char> _responseData;
    std::vector<char> _responseHeader;
    std::vector<char> _responseBody;
    int _responseStatus;
    std::vector<tjs_uint8> _sendingData;

    typedef std::map<std::string, std::string> header_container;
    header_container _requestHeaders;
    HANDLE _hThread;
    bool _aborted;
    iTJSDispatch2 *_target;
    std::vector<tjs_int> _readyStateChangeQueue;
    CRITICAL_SECTION _criticalSection;

    static int objcount;
};

int NI_XMLHttpRequest::objcount = 0;

//---------------------------------------------------------------------------
/*
    ����� NI_XMLHttpRequest �̃I�u�W�F�N�g���쐬���ĕԂ������̊֐��ł��B
    ��q�� TJSCreateNativeClassForPlugin �̈����Ƃ��ēn���܂��B
*/
static iTJSNativeInstance * TJS_INTF_METHOD Create_NI_XMLHttpRequest()
{
    return new NI_XMLHttpRequest();
}
//---------------------------------------------------------------------------
/*
    TJS2 �̃l�C�e�B�u�N���X�͈�ӂ� ID �ŋ�ʂ���Ă���K�v������܂��B
    ����͌�q�� TJS_BEGIN_NATIVE_MEMBERS �}�N���Ŏ����I�Ɏ擾����܂����A
    ���� ID ���i�[����ϐ����ƁA���̕ϐ��������Ő錾���܂��B
    �����l�ɂ͖����� ID ��\�� -1 ���w�肵�Ă��������B
*/
#define TJS_NATIVE_CLASSID_NAME ClassID_XMLHttpRequest
static tjs_int32 TJS_NATIVE_CLASSID_NAME = -1;
//---------------------------------------------------------------------------
/*
    TJS2 �p�́u�N���X�v���쐬���ĕԂ��֐��ł��B
*/
static iTJSDispatch2 * Create_NC_XMLHttpRequest()
{
    /*
        �܂��A�N���X�̃x�[�X�ƂȂ�N���X�I�u�W�F�N�g���쐬���܂��B
        ����ɂ� TJSCreateNativeClassForPlugin ��p���܂��B
        TJSCreateNativeClassForPlugin �̑�P�����̓N���X���A��Q������
        �l�C�e�B�u�C���X�^���X��Ԃ��֐����w�肵�܂��B
        �쐬�����I�u�W�F�N�g���ꎞ�I�Ɋi�[���郍�[�J���ϐ��̖��O��
        classobj �ł���K�v������܂��B
    */
    tTJSNativeClassForPlugin * classobj =
        TJSCreateNativeClassForPlugin(TJS_W("XMLHttpRequest"), Create_NI_XMLHttpRequest);


    /*
        TJS_BEGIN_NATIVE_MEMBERS �}�N���ł��B�����ɂ� TJS2 ���Ŏg�p����N���X��
        ���w�肵�܂��B
        ���̃}�N���� TJS_END_NATIVE_MEMBERS �}�N���ŋ��܂ꂽ�ꏊ�ɁA�N���X��
        �����o�ƂȂ�ׂ����\�b�h��v���p�e�B�̋L�q�����܂��B
    */
    TJS_BEGIN_NATIVE_MEMBERS(/*TJS class name*/XMLHttpRequest)

        TJS_DECL_EMPTY_FINALIZE_METHOD


        TJS_BEGIN_NATIVE_CONSTRUCTOR_DECL(
            /*var.name*/_this,
            /*var.type*/NI_XMLHttpRequest,
            /*TJS class name*/XMLHttpRequest)
        {
            // NI_XMLHttpRequest::Construct �ɂ����e���L�q�ł���̂�
            // �����ł͉������Ȃ�
            return TJS_S_OK;
        }
        TJS_END_NATIVE_CONSTRUCTOR_DECL(/*TJS class name*/XMLHttpRequest)


        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/open)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            if (numparams < 2) return TJS_E_BADPARAMCOUNT;

            if (param[0]->Type() != tvtString || param[1]->Type() != tvtString) {
                return TJS_E_INVALIDPARAM;
            }

            bool async;
            if (numparams < 3) {
                async = true;
            }
            else {
                async = (bool)(tjs_int)*param[2];
            }

            ttstr username;
            if (numparams < 4) {
                username = "";
            }
            else {
                if (param[3]->Type() == tvtString) {
                    username = *param[3];
                }
                else {
                    return TJS_E_INVALIDPARAM;
                }
            }

            ttstr password;
            if (numparams < 5) {
                password = "";
            }
            else {
                if (param[4]->Type() == tvtString) {
                    password = *param[4];
                }
                else {
                    return TJS_E_INVALIDPARAM;
                }
            }

            _this->Open(ttstr(*param[0]), ttstr(*param[1]), async, username, password);

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/open)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/send)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            if (numparams == 0 || param[0]->Type() == tvtVoid) {
                _this->Send(NULL);
            }
            else if (param[0]->Type() == tvtOctet) {
                _this->Send(param[0]);
            }
            else {
                return TJS_E_INVALIDPARAM;
            }

            if (result) {
                result->Clear();
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/send)


        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/setRequestHeader)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            if (numparams < 2) return TJS_E_BADPARAMCOUNT;
            if (param[0]->Type() != tvtString || param[1]->Type() != tvtString) {
                return TJS_E_INVALIDPARAM;
            }

            _this->SetRequestHeader(ttstr(*param[0]), ttstr(*param[1]));
            if (result) {
                result->Clear();
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/setRequestHeader)


        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/printRequestHeaders)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            _this->PrintRequestHeaders();
            if (result) {
                result->Clear();
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/printRequestHeaders)


        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/getResponseHeader)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            if (numparams == 0) {
                return TJS_E_BADPARAMCOUNT;
            }

            if (param[0]->Type() != tvtString) {
                return TJS_E_INVALIDPARAM;
            }

            if (result) {
                ttstr v = _this->GetResponseHeader(ttstr(*param[0]));
                if (v == TJS_W("")) {
                    result->Clear();
                }
                else {
                    *result = v;
                }
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/getResponseHeader)


        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/abort)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            _this->Abort();
            if (result) {
                result->Clear();
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/abort)

        TJS_BEGIN_NATIVE_METHOD_DECL(/*func. name*/executeCallback)
        {
            TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                /*var. type*/NI_XMLHttpRequest);

            _this->ExecuteOnReadyStateChangeCallback();
            if (result) {
                result->Clear();
            }

            return TJS_S_OK;
        }
        TJS_END_NATIVE_METHOD_DECL(/*func. name*/executeCallback)

        TJS_BEGIN_NATIVE_PROP_DECL(readyState)
        {
            TJS_BEGIN_NATIVE_PROP_GETTER
            {
                TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                    /*var. type*/NI_XMLHttpRequest);

                if (result) {
                    *result = (tTVInteger)_this->GetReadyState();
                }

                return TJS_S_OK;
            }
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(readyState)


        TJS_BEGIN_NATIVE_PROP_DECL(responseText)
        {
            TJS_BEGIN_NATIVE_PROP_GETTER
            {
                TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                    /*var. type*/NI_XMLHttpRequest);

                if (result) {
                    const std::vector<char>* data = _this->GetResponseText();
                    tjs_uint8 *d = new tjs_uint8[data->size()];
                    std::copy(data->begin(), data->end(), d);
                    *result = TJSAllocVariantOctet(d, data->size());
                    delete[] d;
                }

                return TJS_S_OK;
            }
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(responseText)



        TJS_BEGIN_NATIVE_PROP_DECL(status)
        {
            TJS_BEGIN_NATIVE_PROP_GETTER
            {
                TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                    /*var. type*/NI_XMLHttpRequest);

                if (result) {
                    *result = (tTVInteger)_this->GetResponseStatus();
                }

                return TJS_S_OK;
            }
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(status)

        TJS_BEGIN_NATIVE_PROP_DECL(statusText)
        {
            TJS_BEGIN_NATIVE_PROP_GETTER
            {
                TJS_GET_NATIVE_INSTANCE(/*var. name*/_this,
                    /*var. type*/NI_XMLHttpRequest);

                if (result) {
                    *result = _this->GetResponseStatusText();
                }

                return TJS_S_OK;
            }
            TJS_END_NATIVE_PROP_GETTER

            TJS_DENY_NATIVE_PROP_SETTER
        }
        TJS_END_NATIVE_PROP_DECL(statusText)

    TJS_END_NATIVE_MEMBERS

    /*
        ���̊֐��� classobj ��Ԃ��܂��B
    */
    return classobj;
}
//---------------------------------------------------------------------------
/*
    TJS_NATIVE_CLASSID_NAME �͈ꉞ undef ���Ă������ق����悢�ł��傤
*/
#undef TJS_NATIVE_CLASSID_NAME
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
    void* lpReserved)
{
    return 1;
}
//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export  __declspec(dllexport) V2Link(iTVPFunctionExporter *exporter)
{
    // �X�^�u�̏�����(�K���L�q����)
    TVPInitImportStub(exporter);

    tTJSVariant val;

    // TJS �̃O���[�o���I�u�W�F�N�g���擾����
    iTJSDispatch2 * global = TVPGetScriptDispatch();


    //-----------------------------------------------------------------------
    // 1 �܂��N���X�I�u�W�F�N�g���쐬
    iTJSDispatch2 * tjsclass = Create_NC_XMLHttpRequest();

    // 2 tjsclass �� tTJSVariant �^�ɕϊ�
    val = tTJSVariant(tjsclass);

    // 3 ���ł� val �� tjsclass ��ێ����Ă���̂ŁAtjsclass ��
    //   Release ����
    tjsclass->Release();


    // 4 global �� PropSet ���\�b�h��p���A�I�u�W�F�N�g��o�^����
    global->PropSet(
        TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
        TJS_W("XMLHttpRequest"), // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
        NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
        &val, // �o�^����l
        global // �R���e�L�X�g ( global �ł悢 )
        );
    //-----------------------------------------------------------------------


    // - global �� Release ����
    global->Release();

    // �����A�o�^����֐�����������ꍇ�� 1 �` 4 ���J��Ԃ�


    // val ���N���A����B
    // ����͕K���s���B�������Ȃ��� val ���ێ����Ă���I�u�W�F�N�g
    // �� Release ���ꂸ�A���Ɏg�� TVPPluginGlobalRefCount �����m�ɂȂ�Ȃ��B
    val.Clear();


    // ���̎��_�ł� TVPPluginGlobalRefCount �̒l��
    GlobalRefCountAtInit = TVPPluginGlobalRefCount;
    // �Ƃ��čT���Ă����BTVPPluginGlobalRefCount �͂��̃v���O�C������
    // �Ǘ�����Ă��� tTJSDispatch �h���I�u�W�F�N�g�̎Q�ƃJ�E���^�̑��v�ŁA
    // ������ɂ͂���Ɠ������A����������Ȃ��Ȃ��ĂȂ��ƂȂ�Ȃ��B
    // �����Ȃ��ĂȂ���΁A�ǂ����ʂ̂Ƃ���Ŋ֐��Ȃǂ��Q�Ƃ���Ă��āA
    // �v���O�C���͉���ł��Ȃ��ƌ������ƂɂȂ�B

    return S_OK;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall _export __declspec(dllexport) V2Unlink()
{
    // �g���g��������A�v���O�C����������悤�Ƃ���Ƃ��ɌĂ΂��֐��B

    // �������炩�̏����Ńv���O�C��������ł��Ȃ��ꍇ��
    // ���̎��_�� E_FAIL ��Ԃ��悤�ɂ���B
    // �����ł́ATVPPluginGlobalRefCount �� GlobalRefCountAtInit ����
    // �傫���Ȃ��Ă���Ύ��s�Ƃ������Ƃɂ���B
    if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
        // E_FAIL ���A��ƁAPlugins.unlink ���\�b�h�͋U��Ԃ�


    /*
        �������A�N���X�̏ꍇ�A�����Ɂu�I�u�W�F�N�g���g�p���ł���v�Ƃ������Ƃ�
        �m�邷�ׂ�����܂���B��{�I�ɂ́APlugins.unlink �ɂ��v���O�C���̉����
        �댯�ł���ƍl���Ă������� (�������� Plugins.link �Ń����N������A�Ō��
        �Ńv���O�C������������A�v���O�����I���Ɠ����Ɏ����I�ɉ��������̂��g)�B
    */

    // TJS �̃O���[�o���I�u�W�F�N�g�ɓo�^���� XMLHttpRequest �N���X�Ȃǂ��폜����

    // - �܂��ATJS �̃O���[�o���I�u�W�F�N�g���擾����
    iTJSDispatch2 * global = TVPGetScriptDispatch();

    // - global �� DeleteMember ���\�b�h��p���A�I�u�W�F�N�g���폜����
    if(global)
    {
        // TJS ���̂����ɉ������Ă����Ƃ��Ȃǂ�
        // global �� NULL �ɂȂ蓾��̂� global �� NULL �łȂ�
        // ���Ƃ��`�F�b�N����

        global->DeleteMember(
            0, // �t���O ( 0 �ł悢 )
            TJS_W("XMLHttpRequest"), // �����o��
            NULL, // �q���g
            global // �R���e�L�X�g
            );
    }

    // - global �� Release ����
    if(global) global->Release();

    // �X�^�u�̎g�p�I��(�K���L�q����)
    TVPUninitImportStub();

    return S_OK;
}
//---------------------------------------------------------------------------

