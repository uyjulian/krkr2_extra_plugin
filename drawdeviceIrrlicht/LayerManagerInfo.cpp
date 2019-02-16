#include <windows.h>
#include "IrrlichtDrawDevice.h"
#include "LayerManagerInfo.h"

/**
 * �R���X�g���N�^
 */
LayerManagerInfo::LayerManagerInfo(int id, bool visible)
	: id(id), visible(visible), driver(NULL), texture(NULL), destBuffer(NULL)
{
};

/**
 * �f�X�g���N�^
 */
LayerManagerInfo::~LayerManagerInfo()
{
	free();
}

// ���蓖�ď���
void
LayerManagerInfo::alloc(iTVPLayerManager *manager, irr::video::IVideoDriver *driver)
{
	free();
	tjs_int w, h;
	if (manager->GetPrimaryLayerSize(w, h) && w > 0 && h > 0) {
		// �e�N�X�`���̃T�C�Y��2�̊K�� Irrlicht �����̃T�C�Y�ɒ������Ă���e�N�X�`���������Ă�̂ł��킹�Ă���
		tjs_int tw = 1; while(tw < w) tw <<= 1;
		tjs_int th = 1; while(th < h) th <<= 1;
		char name[20];
		snprintf(name, sizeof name-1, "krkr%d", id);
		texture = driver->addTexture(irr::core::dimension2d<irr::s32>(tw, th), name, irr::video::ECF_A8R8G8B8);
		if (texture == NULL) {
			TVPThrowExceptionMessage(L"�e�N�X�`���̊��蓖�ĂɎ��s���܂���");
		} else {
			this->driver = driver;
			manager->RequestInvalidation(tTVPRect(0,0,w,h));
			srcRect = irr::core::rect<irr::s32>(0,0,w,h);
		}
	}
}

void
LayerManagerInfo::free()
{
	if (driver) {
		driver->removeTexture(texture);
		driver = NULL;
		texture = NULL;
	}
}

/**
 * �e�N�X�`�������b�N���ĕ`��̈�����擾����
 */
void
LayerManagerInfo::lock()
{
	if (texture) {
		destBuffer = (unsigned char *)texture->lock();
		irr::core::dimension2d<irr::s32> size = texture->getSize();
		destWidth  = size.Width;
		destHeight = size.Height;
		destPitch  = texture->getPitch();
	} else {
		destBuffer = NULL;
	}
}

/**
 * ���b�N���ꂽ�e�N�X�`���Ƀr�b�g�}�b�v�`����s��
 */
void
LayerManagerInfo::copy(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
					   const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity)
{
	// bits, bitmapinfo �ŕ\�����r�b�g�}�b�v�� cliprect �̗̈���Ax, y �ɕ`�悷��B

	if (destBuffer) {
		int srcPitch = -bitmapinfo->bmiHeader.biWidth * 4; // XXX ���߂���
		unsigned char *srcBuffer = (unsigned char *)bits - srcPitch * (bitmapinfo->bmiHeader.biHeight - 1);
		int srcx   = cliprect.left;
		int srcy   = cliprect.top;
		int width  = cliprect.get_width();
		int height = cliprect.get_height();
		// �N���b�s���O
		if (x < 0) {
			srcx  += x;
			width += x;
			x = 0;
		}
		if (x + width > destWidth) {
			width -= ((x + width) - destWidth);
		}
		if (y < 0) {
			srcy += y;
			height += y;
			y = 0;
		}
		if (y + height > destHeight) {
			height -= ((y + height) - destHeight);
		}
		unsigned char *src  = srcBuffer  + srcy * srcPitch  + srcx * 4;
		unsigned char *dest = destBuffer +    y * destPitch +    x * 4;
		for (int i=0;i<height;i++) {
			memcpy(dest, src, width * 4);
			src  += srcPitch;
			dest += destPitch;
		}
	}
}

/**
 * �e�N�X�`���̃��b�N�̉���
 */
void
LayerManagerInfo::unlock()
{
	if (texture) {
		texture->unlock();
		destBuffer = NULL;
	}
}

/**
 * ��ʂւ̕`��
 */
void
LayerManagerInfo::draw(irr::video::IVideoDriver *driver, irr::core::rect<irr::s32> destRect)
{
	if (visible && texture) {
		driver->draw2DImage(texture, destRect, srcRect, NULL, NULL, true);
	}
}
