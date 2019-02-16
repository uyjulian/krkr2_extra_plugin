#ifndef __PSDCLASS_H__
#define __PSDCLASS_H__

//#define LOAD_MEMORY

#include <tp_stub.h>
#include "psdparse/psdfile.h"

class PSDStorage;
class PSDIterator;

class PSD : public psd::PSDFile
{
	friend class PSDStorage;
	friend class PSDIterator;
	
public:
	/**
	 * �R���X�g���N�^
	 */
	PSD(iTJSDispatch2 *objthis);

	/**
	 * �f�X�g���N�^
	 */
	~PSD();

	/**
	 * ����f�[�^�̏���
	 */
	virtual void clearData();
	
	/**
	 * �C���X�^���X�����t�@�N�g��
	 */
	static tjs_error factory(PSD **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis);

	/**
	 * �������̎��ȃI�u�W�F�N�g���擾
	 */
	tTJSVariant getSelf();
	
	/**
	 * PSD�摜�̃��[�h
	 * @param filename �t�@�C����
	 * @return ���[�h�ɐ��������� true
	 */
	bool load(ttstr filename);

	static void clearStorageCache();
	
#define INTGETTER(tag) int get_ ## tag(){ return isLoaded ? header.tag : -1; }

	INTGETTER(width);
	INTGETTER(height);
	INTGETTER(channels);
	INTGETTER(depth);
  int get_color_mode()  { return isLoaded ? header.mode : -1; }
  int get_layer_count() { return isLoaded ? (int)layerList.size() : -1; }

public:
	/**
	 * ���C����ʂ̎擾
	 * @param no ���C���ԍ�
	 * @return ���C�����
	 */
	int getLayerType(int no);

	/**
	 * ���C�����̂̎擾
	 * @param no ���C���ԍ�
	 * @return ���C�����
	 */
	ttstr getLayerName(int no);

	/**
	 * ���C�����̎擾
	 * @param no ���C���ԍ�
	 * @return ���C����񂪊i�[���ꂽ����
	 */
	tTJSVariant getLayerInfo(int no);

	/**
	 * ���C���f�[�^�̓ǂݏo��(��������)
	 * @param layer �ǂݏo���惌�C��
	 * @param no ���C���ԍ�
     * @param imageMode �C���[�W���[�h
	 */
  void _getLayerData(tTJSVariant layer, int no, psd::ImageMode imageMode);

	/**
	 * ���C���f�[�^�̓ǂݏo��
	 * @param layer �ǂݏo���惌�C��
	 * @param no ���C���ԍ�
	 */
	void getLayerData(tTJSVariant layer, int no);

	/**
	 * ���C���f�[�^�̓ǂݏo��(���C���[�W)
	 * @param layer �ǂݏo���惌�C��
	 * @param no ���C���ԍ�
	 */
	void getLayerDataRaw(tTJSVariant layer, int no);

	/**
	 * ���C���f�[�^�̓ǂݏo��(�}�X�N�̂�)
	 * @param layer �ǂݏo���惌�C��
	 * @param no ���C���ԍ�
	 */
	void getLayerDataMask(tTJSVariant layer, int no);

	/**
	 * �X���C�X�f�[�^�̓ǂݏo��
	 * @return �X���C�X��񎫏� %[ top, left, bottom, right, slices:[ %[ id, group_id, left, top, bottom, right ], ... ] ]
	 *         �X���C�X��񂪂Ȃ��ꍇ�� void ��Ԃ�
	 */
	tTJSVariant getSlices();

	/**
	 * �K�C�h�f�[�^�̓ǂݏo��
	 * @return �K�C�h��񎫏� %[ vertical:[ x1, x2, ... ], horizontal:[ y1, y2, ... ] ]
	 *         �K�C�h��񂪂Ȃ��ꍇ�� void ��Ԃ�
	 */
	tTJSVariant getGuides();

	/**
	 * �������ʂ̎擾�B�擾�̈�͉摜�S�̃T�C�Y���ɂ����܂��Ă�K�v������܂�
   * ���ӁFPSD�t�@�C�����̂ɍ����ς݉摜�����݂��Ȃ��ꍇ�͎擾�Ɏ��s���܂�
   *
	 * @param layer �i�[�惌�C��(width,height�T�C�Y�ɒ��������)
	 * @return �擾�ɐ��������� true
	 */
  bool getBlend(tTJSVariant layer);

	/**
	 * ���C���[�J���v
	 */
	tTJSVariant getLayerComp();

protected:
	iTJSDispatch2 *objthis; ///< ���ȃI�u�W�F�N�g���̎Q��
	ttstr dname; ///< �o�^�p�x�[�X��

#ifdef LOAD_MEMORY
	HGLOBAL hBuffer; // �I���������ێ��p�n���h��
	bool loadMemory(const ttstr &filename);
	void clearMemory();
#else
	// �X�g���[������ǂݍ���
	IStream *pStream;
	tTVInteger mStreamSize;
	bool loadStream(const ttstr &filename);
	void clearStream();
	unsigned char &getStreamValue(const tTVInteger &pos);
	void copyToBuffer(uint8_t *buf, tTVInteger pos, int size);

	//< PSD�t�@�C���ǂݍ��݃L���b�V���p�o�b�t�@
	tTVInteger mBufferPos;
	ULONG mBufferSize;
	unsigned char mBuffer[4*1024];
#endif
	
	/**
	 * ���C���ԍ����K�؂��ǂ�������
	 * @param no ���C���ԍ�
	 */
	void checkLayerNo(int no);

	/**
	 * ���O�̎擾
	 * @param lay���C�����
	 */
	static ttstr layname(psd::LayerInfo &lay);
	
	// ------------------------------------------------------------
	// �X�g���[�W���C���Q�Ɨp�C���^�[�t�F�[�X
	// ------------------------------------------------------------
	
protected:

	// �X�g���[�W���o�^
	void addToStorage(const ttstr &filename);
	void removeFromStorage();

	bool storageStarted; //< �X�g���[�W�p�̏�񏉊����ς݃t���O

	// ���C������Ԃ�
	static ttstr path_layname(psd::LayerInfo &lay);

	// ���C���̃p�X����Ԃ�
	static ttstr pathname(psd::LayerInfo &lay);

	// �X�g���[�W�����p�f�[�^�̏�����
	void startStorage();

	/*
	 * �w�肵�����O�̃��C���̑��݃`�F�b�N
	 * @param name �p�X���܂ރ��C����
	 * @param layerIdxRet ���C���C���f�b�N�X�ԍ���Ԃ�
	 */
	bool CheckExistentStorage(const ttstr &filename, int *layerIdxRet=0);

	/*
	 * �w�肵���p�X�ɂ���t�@�C�����ꗗ�̎擾
	 * @param pathname �p�X��
	 * @param lister ���X�g�擾�p�C���^�[�t�F�[�X
	 */
	void GetListAt(const ttstr &pathname, iTVPStorageLister *lister);

	/*
	 * �w�肵�����O�̃��C���̉摜�t�@�C�����X�g���[���ŕԂ�
	 * @param name �p�X���܂ރ��C����
	 * @return �t�@�C���X�g���[��
	 */
	IStream *openLayerImage(const ttstr &name);
	
	// �p�X���L�^�p

	typedef std::map<int,int> LayerIdIdxMap; // layerId �ƃ��C�����C���f�b�N�X�̃}�b�v
	LayerIdIdxMap layerIdIdxMap;

	typedef std::map<ttstr,int> NameIdxMap;     //< ���C������layerId �̃}�b�v
	typedef std::map<ttstr,NameIdxMap> PathMap; //< �p�X�ʂ̃��C�����ꗗ
	PathMap pathMap;
};

#endif
