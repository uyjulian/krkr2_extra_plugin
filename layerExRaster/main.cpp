#include "ncbind/ncbind.hpp"
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>

/**
 * ���O�o�͗p
 */
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

#include "../layerExDraw/LayerExBase.hpp"

/*
 * �A�E�g���C���x�[�X�̃e�L�X�g�`�惁�\�b�h�̒ǉ�
 */
struct layerExRaster : public layerExBase
{
public:
	// �R���X�g���N�^
	layerExRaster(DispatchT obj) : layerExBase(obj) {}

	/**
	 * ���X�^�[�R�s�[����
	 * @param layer �`�挳���C��
	 * @param maxh  �ő�U��(pixel)
	 * @param lines �P���������胉�C����
	 * @param cycle �����w��(msec)
	 * @param time ���ݎ���
	 */
	void copyRaster(tTJSVariant layer, int maxh, int lines, int cycle, tjs_int64 time) {
		
		// ���C���摜���
		tjs_int width, height, pitch;
		unsigned char* buffer;
		{
			iTJSDispatch2 *layerobj = layer.AsObjectNoAddRef();
			tTJSVariant var;
			layerobj->PropGet(0, L"imageWidth", NULL, &var, layerobj);
			width = (tjs_int)var;
			layerobj->PropGet(0, L"imageHeight", NULL, &var, layerobj);
			height = (tjs_int)var;
			layerobj->PropGet(0, L"mainImageBuffer", NULL, &var, layerobj);
			buffer = (unsigned char*)(tjs_int)var;
			layerobj->PropGet(0, L"mainImageBufferPitch", NULL, &var, layerobj);
			pitch = (tjs_int)var;
		}

		if (_width != width || _height != height) {
			return;
		}

		// �p���x�v�Z
		double omega = 2 * M_PI / lines;
		
		//double tt = sin((3.14159265358979/2.0) * time / cycle);
		//tjs_int CurH = (tjs_int)(tt * maxh);
		tjs_int CurH = (tjs_int)maxh;
		
		// �����p�����[�^���v�Z
		double rad = - omega * time / cycle * (height/2);

		// �N���b�v����
		rad += omega * _clipTop;
		_buffer += _pitch * _clipTop + _clipLeft * 4;
		buffer  +=  pitch * _clipTop + _clipLeft * 4;

		// ���C�����Ƃɏ���
		tjs_int n;
		for (n = 0; n < _clipHeight; n++, rad += omega) {
			tjs_int d = (tjs_int)(sin(rad) * CurH);
			if (d >= 0) {
				int w = _clipWidth - d;
				const tjs_uint32 *src = (const tjs_uint32*)(buffer + n * pitch);
				tjs_uint32 *dest = (tjs_uint32 *)(_buffer + n * _pitch) + d;
				for (tjs_int i=0;i<w;i++) {
					*dest++ = *src++;
				}
			} else {
				int w = _clipWidth + d;
				const tjs_uint32 *src = (const tjs_uint32*)(buffer + n * pitch) - d;
				tjs_uint32 *dest = (tjs_uint32 *)(_buffer + n * _pitch);
				for (tjs_int i=0;i<w;i++) {
					*dest++ = *src++;
				}
			}
		}

		redraw();
	}
};

// ----------------------------------- �N���X�̓o�^

NCB_GET_INSTANCE_HOOK(layerExRaster)
{
	// �C���X�^���X�Q�b�^
	NCB_INSTANCE_GETTER(objthis) { // objthis �� iTJSDispatch2* �^�̈����Ƃ���
		ClassT* obj = GetNativeInstance(objthis);	// �l�C�e�B�u�C���X�^���X�|�C���^�擾
		if (!obj) {
			obj = new ClassT(objthis);				// �Ȃ��ꍇ�͐�������
			SetNativeInstance(objthis, obj);		// objthis �� obj ���l�C�e�B�u�C���X�^���X�Ƃ��ēo�^����
		}
		obj->reset();
		return obj;
	}
	// �f�X�g���N�^�i���ۂ̃��\�b�h���Ă΂ꂽ��ɌĂ΂��j
	~NCB_GET_INSTANCE_HOOK_CLASS () {
	}
};


// �t�b�N���A�^�b�`
NCB_ATTACH_CLASS_WITH_HOOK(layerExRaster, Layer) {
	NCB_METHOD(copyRaster);
}
