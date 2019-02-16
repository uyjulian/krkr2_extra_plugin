#ifndef LAYERMANAGERINFO_H
#define LAYERMANAGERINFO_H

#include <windows.h>
#include <stdio.h>
#include <ddraw.h>
#include <d3d.h>
#include <vfw.h>

/**
 * ���C���}�l�[�W���p�t�����
 */
class LayerManagerInfo {

protected:
	// ���ʗpID
	int id;

	// �����C���T�C�Y
	int srcWidth;
	int srcHeight;

	// ���蓖�ăe�N�X�`��
	IDirectDrawSurface7 *texture;
	tjs_uint textureWidth; //< �e�N�X�`���̉���
	tjs_uint textureHeight; //< �e�N�X�`���̏c��

	bool useDirectTransfer; //< ���������ړ]�����s�����ǂ���
	
	void *textureBuffer; //< �e�N�X�`���̃T�[�t�F�[�X�ւ̃������|�C���^
	long texturePitch; //< �e�N�X�`���̃s�b�`

	HDC offScreenDC; //< DIB�`��p�� HDC
	HDRAWDIB drawDibHandle; //< DIB�`��p�n���h��

	bool lastOK;     //< �O��̏����͐���������
	
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
	
	/**
	 * �e�N�X�`�����蓖�ď���
	 */
	void alloc(iTVPLayerManager *manager, IDirectDraw7 *directDraw, IDirect3DDevice7 *direct3DDevice);

	/*
	 * �e�N�X�`�����
	 */
	void free();
	
	// �e�N�X�`���`�摀��p
	void lock();
	void copy(tjs_int x, tjs_int y, const void * bits, const BITMAPINFO * bitmapinfo,
			  const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity);
	void unlock();

	/**
	 * �`��
	 */
	void draw(IDirect3DDevice7 *direct3DDevice7, int destWidth, int destHeight);
};

#endif
