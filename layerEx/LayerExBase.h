#ifndef __LayerExBase__
#define __LayerExBase__

#include <windows.h>
#include "tp_stub.h"

/**
 * ���C���g�� ��{���ێ��p�l�C�e�B�u�C���X�^���X�B
 */
class NI_LayerExBase : public tTJSNativeInstance
{
protected:
	// ���C����������擾���邽�߂̃v���p�e�B
	// �����ł����������邽�߃L���b�V�����Ă���
	static iTJSDispatch2 * _leftProp;
	static iTJSDispatch2 * _topProp;
	static iTJSDispatch2 * _widthProp;
	static iTJSDispatch2 * _heightProp;
	static iTJSDispatch2 * _pitchProp;
	static iTJSDispatch2 * _bufferProp;
	static iTJSDispatch2 * _updateProp;

public:
	// ���C������r�ێ��p
	tjs_int _width;
	tjs_int _height;
	tjs_int _pitch;
	unsigned char *_buffer;

public:
	// �N���X�h�c�ێ��p
	static int classId;
	static void init(iTJSDispatch2 *layerobj);
	static void unInit();
	
	/**
	 * �l�C�e�B�u�I�u�W�F�N�g�̎擾
	 * @param layerobj ���C���I�u�W�F�N�g
	 * @return �l�C�e�B�u�I�u�W�F�N�g
	 */
	static NI_LayerExBase *getNative(iTJSDispatch2 *objthis, bool create=true);

	/**
	 * �ĕ`��v��
	 * @param layerobj ���C���I�u�W�F�N�g
	 */
	void redraw(iTJSDispatch2 *layerobj);
	
	/**
	 * �O���t�B�b�N������������
	 * ���C���̃r�b�g�}�b�v��񂪕ύX����Ă���\��������̂Ŗ���`�F�b�N����B
	 * �ύX����Ă���ꍇ�͕`��p�̃R���e�L�X�g��g�݂Ȃ���
	 * @param layerobj ���C���I�u�W�F�N�g
	 * @return ���������s���ꂽ�ꍇ�� true ��Ԃ�
	 */
	void reset(iTJSDispatch2 *layerobj);
	
public:
	/**
	 * �R���X�g���N�^
	 */
	NI_LayerExBase();
};

#endif
