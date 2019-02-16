#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include "tp_stub.h"

/**
 * ���O�o�͗p
 */
void
message_log(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char msg[1024];
	_vsnprintf(msg, 1024, format, args);
	TVPAddLog(ttstr(msg));
	va_end(args);
}

/**
 * �G���[���O�o�͗p
 */
void
error_log(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char msg[1024];
	_vsnprintf(msg, 1024, format, args);
	TVPAddImportantLog(ttstr(msg));
	va_end(args);
}

#include "LayerExSWF.hpp"
#include "SWFMovie.hpp"

////////////////////////////////////////////////////////////////////////////////
/// ncBind �p�}�N��

#include "ncbind/ncbind.hpp"

/**
 * �t�@�C�����ϊ��p proxy
 */
void swfload(SWFMovie *swf, const char *name)
{
	ttstr path(name);
	TVPGetLocalName(path);
	int len = path.GetNarrowStrLen() + 1;
	char *filename = new char[len];
	path.ToNarrowStr(filename, len);
	swf->load(filename);
	delete filename;
}

NCB_REGISTER_CLASS(SWFMovie) {
	NCB_CONSTRUCTOR(());
	NCB_METHOD_PROXY(load, swfload);
	NCB_METHOD(update);
	NCB_METHOD(notifyMouse);
	NCB_METHOD(play);
	NCB_METHOD(stop);
	NCB_METHOD(restart);
	NCB_METHOD(back);
	NCB_METHOD(next);
	NCB_METHOD(gotoFrame);
}

NCB_GET_INSTANCE_HOOK(layerExSWF)
{
	// �C���X�^���X�Q�b�^
	NCB_INSTANCE_GETTER(objthis) { // objthis �� iTJSDispatch2* �^�̈����Ƃ���
		
		ClassT* obj = GetNativeInstance(objthis);	// �l�C�e�B�u�C���X�^���X�|�C���^�擾
		if (!obj) {
			obj = new ClassT(objthis);				// �Ȃ��ꍇ�͐�������
			SetNativeInstance(objthis, obj);		// objthis �� obj ���l�C�e�B�u�C���X�^���X�Ƃ��ēo�^����
		}
		if (obj) obj->reset();						// ���\�b�h���ĂԑO�ɕK���Ă΂��
		return (_obj = obj);						//< �f�X�g���N�^�Ŏg�p�������ꍇ�̓v���C�x�[�g�ϐ��ɕۑ�
	}

	// �f�X�g���N�^�i���ۂ̃��\�b�h���Ă΂ꂽ��ɌĂ΂��j
	~NCB_GET_INSTANCE_HOOK_CLASS () {
		if (_obj) _obj->redraw();					// ���\�b�h���Ă񂾌�ɕK���Ă΂��
	}

private:
	ClassT *_obj;
}; // ���̂� class ��`�Ȃ̂� ; ��Y��Ȃ��ł�

// �t�b�N���A�^�b�`
NCB_ATTACH_CLASS_WITH_HOOK(layerExSWF, Layer) {
	NCB_METHOD(drawSWF);
}

extern void initSWFMovie();
extern void destroySWFMovie();

NCB_POST_REGIST_CALLBACK(initSWFMovie);
NCB_PRE_UNREGIST_CALLBACK(destroySWFMovie);
