#include "psdbase.h"
#include "psddata.h"
#include <boost/iostreams/device/mapped_file.hpp>

namespace psd {
  // �C���[�W�擾���[�h
  enum ImageMode {
    IMAGE_MODE_IMAGE,       // �}�X�N�����肱�܂Ȃ��C���[�W�f�[�^
    IMAGE_MODE_MASK,        // �}�X�N���݂̂̃C���[�W�f�[�^(�O���[)
    IMAGE_MODE_MASKEDIMAGE, // �}�X�N���A���t�@�ɌJ�荞�񂾃C���[�W�f�[�^
  };
  
  /**
   * PSD�t�@�C���N���X
   */
  class PSDFile : public Data {
  public:
    PSDFile() : isLoaded(false) {}
    ~PSDFile() {}

    // �ǂݍ��ݍς݃t���O
    bool isLoaded;
    
    // �t�@�C�����[�h�G���g��
    bool load(const char *filename);

		// �摜�f�[�^�擾�C���^�t�F�[�X(�o�b�t�@�s�b�`���O�̏ꍇ��full fill����܂�)
    // �����ς݉摜(PSD�ɕێ�����Ă���ꍇ�̂�)
    bool getMergedImage(void *buf, const ColorFormat &format, int bufPitchByte);
    // ���C���摜
    bool getLayerImage(LayerInfo &layer, void *buf, const ColorFormat &format,
                       int bufPitchByte, ImageMode mode);
    bool getLayerImageById(int layerId, void *buf, const ColorFormat &format,
                           int bufPitchByte, ImageMode mode);

  private:
		// loadFile�Ŏg�p���郁�����}�b�v�h�t�@�C��
    // (�摜�̒x���ǂݍ��݂̊֌W�� Data �������Ԓ��͊J�����ςȂ�)
    boost::iostreams::mapped_file_source in;
  };

} // namespace psd
