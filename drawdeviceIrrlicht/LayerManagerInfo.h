#ifndef LAYERMANAGERINFO_H
#define LAYERMANAGERINFO_H

/**
 * ���C���}�l�[�W���p�t�����
 */
class LayerManagerInfo {

protected:
	// ���ʗpID
	int id;

	// �����C���T�C�Y
	irr::core::rect<irr::s32> srcRect;
	
	// �R�s�[�����p�ꎞ�ϐ�
	unsigned char *destBuffer;
	int destWidth;
	int destHeight;
	int destPitch;

	// �e�N�X�`�����蓖�ĂɎg�����h���C�o
	irr::video::IVideoDriver *driver;
	// ���蓖�ăe�N�X�`��
	irr::video::ITexture *texture;
	
public:
	// �\���Ώۂ��ǂ���
	bool visible;

public:
	/**
	 * �R���X�g���N�^
	 * @param id ���C��ID
	 * @param visible �����\�����
	 */
	LayerManagerInfo(int id, bool visible);
	virtual ~LayerManagerInfo();
	
	// �e�N�X�`�����蓖�ď���
	void alloc(iTVPLayerManager *manager, irr::video::IVideoDriver *driver);
	// �e�N�X�`�����
	void free();
	
	// �e�N�X�`���`�摀��p
	void lock();
	void copy(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
			  const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	void unlock();

	// �`��
	void draw(irr::video::IVideoDriver *driver, irr::core::rect<irr::s32> destRect);
};

#endif
