#ifndef __psddata_h__
#define __psddata_h__

#include "psdbase.h"
#include "psddesc.h"

#include <vector>

namespace psd {
  // ���C���^�C�v
  enum LayerType {
    LAYER_TYPE_NORMAL,    // �ʏ탌�C��
    LAYER_TYPE_HIDDEN,    // �t�H���_��؂�
    LAYER_TYPE_FOLDER,    // �t�H���_���C��
    LAYER_TYPE_ADJUST,    // �������C��
    LAYER_TYPE_FILL,      // �h��Ԃ����C��
    LAYER_TYPE_TEXT,      // �e�L�X�g���C��
  };

  // �P��
  enum Unit {
    UNIT_INCH    = 1,    // inch
    UNIT_CM      = 2,    // cm
    UNIT_POINT   = 3,    // points
    UNIT_PICA    = 4,    // pica
    UNIT_COLUMN  = 5,    // columns
  };

  enum ChannelId {
    CH_ID_REAL_UMASK = -3,
    CH_ID_UMASK      = -2,
    CH_ID_TRANSP     = -1,
    CH_ID_GRAY = 0,
    CH_ID_RGB_R = 0,
    CH_ID_RGB_G = 1,
    CH_ID_RGB_B = 2,
    CH_ID_CMYK_C = 0,
    CH_ID_CMYK_M = 1,
    CH_ID_CMYK_Y = 2,
    CH_ID_CMYK_K = 3,
  };

  enum ColorMode {
    COLOR_MODE_BITMAP        = 0,
    COLOR_MODE_GRAYSCALE     = 1,
    COLOR_MODE_INDEXED       = 2,
    COLOR_MODE_RGB           = 3,
    COLOR_MODE_CMYK          = 4,
    COLOR_MODE_MULTICHANNEL  = 7,
    COLOR_MODE_DUOTONE       = 8,
    COLOR_MODE_LAB           = 9,
  };

  enum ColorSpace {
    COLOR_SPACE_DUMMY = -1,
    COLOR_SPACE_RGB,
    COLOR_SPACE_HSB,
    COLOR_SPACE_CMYK,
    COLOR_SPACE_PANTONE,
    COLOR_SPACE_FOCOLTONE,
    COLOR_SPACE_TRUMATCH,
    COLOR_SPACE_TOYO,
    COLOR_SPACE_LAB,
    COLOR_SPACE_GRAY,
    COLOR_SPACE_WIDECMYK,
    COLOR_SPACE_HKS,
    COLOR_SPACE_DIC,
    COLOR_SPACE_TOTALINK,
    COLOR_SPACE_MONITORRGB,
    COLOR_SPACE_DUOTONE,
    COLOR_SPACE_OPACITY,
    COLOR_SPACE_WEB,
    COLOR_SPACE_GRAYFLOAT,
    COLOR_SPACE_RGBFLOAT,
    COLOR_SPACE_OPACITYFLOAT,
  };

  enum BlendMode {
    BLEND_MODE_INVALID = -1,  // invalid(unsupported mode)
    BLEND_MODE_NORMAL,        // normal
    BLEND_MODE_DISSOLVE,      // dissolve
    BLEND_MODE_DARKEN,        // darken
    BLEND_MODE_MULTIPLY,      // multiply
    BLEND_MODE_COLOR_BURN,    // color burn
    BLEND_MODE_LINEAR_BURN,   // linear burn
    BLEND_MODE_LIGHTEN,       // lighten
    BLEND_MODE_SCREEN,        // screen
    BLEND_MODE_COLOR_DODGE,   // color dodge
    BLEND_MODE_LINEAR_DODGE,  // linear dodge
    BLEND_MODE_OVERLAY,       // overlay
    BLEND_MODE_SOFT_LIGHT,    // soft light
    BLEND_MODE_HARD_LIGHT,    // hard light
    BLEND_MODE_VIVID_LIGHT,   // vivid light
    BLEND_MODE_LINEAR_LIGHT,  // linear light
    BLEND_MODE_PIN_LIGHT,     // pin light
    BLEND_MODE_HARD_MIX,      // hard mix
    BLEND_MODE_DIFFERENCE,    // difference
    BLEND_MODE_EXCLUSION,     // exclusion
    BLEND_MODE_HUE,           // hue
    BLEND_MODE_SATURATION,    // saturation
    BLEND_MODE_COLOR,         // color
    BLEND_MODE_LUMINOSITY,    // luminosity
    BLEND_MODE_PASS_THROUGH,  // pass
    // �ȍ~�� libpsd ��݊�
    BLEND_MODE_DARKER_COLOR,  // darker color
    BLEND_MODE_LIGHTER_COLOR, // lighter color
    BLEND_MODE_SUBTRACT,      // subtract
    BLEND_MODE_DIVIDE,        // divide
  };

  inline BlendMode blendKeyToMode(int blendModeKey) {
    switch (blendModeKey) {
    case 'norm': return BLEND_MODE_NORMAL;
    case 'diss': return BLEND_MODE_DISSOLVE;
    case 'dark': return BLEND_MODE_DARKEN;
    case 'mul ': return BLEND_MODE_MULTIPLY;
    case 'idiv': return BLEND_MODE_COLOR_BURN;
    case 'lbrn': return BLEND_MODE_LINEAR_BURN;
    case 'dkCl': return BLEND_MODE_DARKER_COLOR;
    case 'lite': return BLEND_MODE_LIGHTEN;
    case 'scrn': return BLEND_MODE_SCREEN;
    case 'div ': return BLEND_MODE_COLOR_DODGE;
    case 'lddg': return BLEND_MODE_LINEAR_DODGE;
    case 'ltCl': return BLEND_MODE_LIGHTER_COLOR;
    case 'over': return BLEND_MODE_OVERLAY;
    case 'sLit': return BLEND_MODE_SOFT_LIGHT;
    case 'hLit': return BLEND_MODE_HARD_LIGHT;
    case 'vLit': return BLEND_MODE_VIVID_LIGHT;
    case 'lLit': return BLEND_MODE_LINEAR_LIGHT;
    case 'pLit': return BLEND_MODE_PIN_LIGHT;
    case 'hMix': return BLEND_MODE_HARD_MIX;
    case 'diff': return BLEND_MODE_DIFFERENCE;
    case 'smud': return BLEND_MODE_EXCLUSION;
    case 'fsub': return BLEND_MODE_SUBTRACT;
    case 'fdiv': return BLEND_MODE_DIVIDE;
    case 'hue ': return BLEND_MODE_HUE;
    case 'sat ': return BLEND_MODE_SATURATION;
    case 'colr': return BLEND_MODE_COLOR;
    case 'lum ': return BLEND_MODE_LUMINOSITY;
    case 'pass': return BLEND_MODE_PASS_THROUGH;
    default:     return BLEND_MODE_INVALID;
    }
  }

  // �K�C�h�f�[�^�̕���
  enum GuideDirection {
    GUIDE_DIR_VERTICAL = 0,
    GGUIDE_DIR_HORIZONTAL = 1
  };

	// �w�b�_���
	struct Header {
		int version;
		int channels;
		int height;
		int width;
		int depth;
		int mode;
	};

  // RGBA�J���[
  struct ColorRgba {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
  };

  // �J���[�e�[�u��
  struct ColorTable {
    ColorTable() : transparencyIndex(-1), validCount(0) {}
    std::vector<ColorRgba> colors; // �Q�Ƒ��̕֋X���Ƀt���T�C�Y(256)�ۏ�
    int16_t transparencyIndex;     // �����F�C���f�b�N�X(���Y�G���g����a=0x0�Z�b�g��)
    int16_t validCount;            // �L���ȃG���g����
  };

  // ���C���[�J���v
  struct LayerComp {
    int id;
    bool isRecordVisibility;
    bool isRecordPosition;
    bool isRecordAppearance;
    std::wstring name;
    std::wstring comment;
  };

  // ���C���[���Ƃ̃��C���[�J���v���
  struct LayerCompInfo {
    int id;
    int offsetX;
    int offsetY;
    bool isEnabled;
  };

  // �X���C�X�A�C�e��
  struct SliceItem {
    int id;
    int groupId;
    int origin;
    int associatedLayerId; // Only present if Origin = 1
    std::wstring name;
    int type;
    int left;
    int top;
    int right;
    int bottom;
    std::wstring url;
    std::wstring target;
    std::wstring message;
    std::wstring altTag;
    bool isCellTextHtml;
    std::wstring cellText;
    int horizontalAlign;
    int verticalAlign;
    uint8_t colorA;
    uint8_t colorR;
    uint8_t colorG;
    uint8_t colorB;
  };

  // �X���C�X���\�[�X
  struct SliceResource {
    SliceResource() : isEnabled(false) {}

    bool isEnabled;
    int boundingLeft;
    int boundingTop;
    int boundingRight;
    int boundingBottom;
    std::wstring groupName;
    std::vector<SliceItem> slices;
  };

  // �K�C�h�A�C�e��
  struct GuideItem {
    int location;
    GuideDirection direction;
  };
  
  // �O���b�h�K�C�h���\�[�X
  struct GridGuideResource {
    GridGuideResource() : isEnabled(false) {}
    
    bool isEnabled;
    int horizontalGrid;
    int verticalGrid;
    std::vector<GuideItem> guides;
  };

	// �C���[�W���\�[�X���
	struct ImageResourceInfo {
		ImageResourceInfo(uint16_t id, std::string &name, int size, IteratorBase *data) : id(id), name(name), size(size), data(data) {};
		~ImageResourceInfo() {
			delete data;
		}
		ImageResourceInfo(const ImageResourceInfo &self) {
			this->id   = self.id;
			this->name = self.name;
			this->size = self.size;
			this->data = self.data == 0 ? 0 : self.data->clone();
		}
		ImageResourceInfo & operator = (const ImageResourceInfo &self) {
			delete data;
			this->id   = self.id;
			this->name = self.name;
			this->size = self.size;
			this->data = self.data == 0 ? 0 : self.data->clone();
			return *this;
		}
    uint16_t id;         // ����ID
		std::string name;    // ���O
		int size;            // �T�C�Y
		IteratorBase *data;  // �Q��
	};
	
	struct GlobalLayerMaskInfo {
		int overlayColorSpace;
		int color1;
		int color2;
		int color3;
		int color4;
		int opacity;
		int kind;
	};

	struct LayerMask {
    int width;
    int height;
		int top;
		int left;
		int bottom;
		int right;
		int defaultColor;
		int flags;
		int realFlags;
		int realUserMaskBackground;
		int enclosingTop;
		int enclosingLeft;
		int enclosingBottom;
		int enclosingRight;
	};

	struct LayerBlendingChannel {
		int source;
		int dest;
	};

	struct LayerBlendingRange {
		int grayBlendSource;
		int grayBlendDest;
		std::vector<LayerBlendingChannel> channels;
	};

	// �ǉ����C�����
	struct AdditionalLayerInfo {
    AdditionalLayerInfo(int sigType, int key, int size, IteratorBase *data) : sigType(sigType), key(key), size(size), data(data) {
		}
		~AdditionalLayerInfo() {
			delete data;
		}
		AdditionalLayerInfo(const AdditionalLayerInfo &self) {
			this->sigType = self.sigType;
			this->key = self.key;
			this->size = self.size;
			this->data = self.data == 0 ? 0 : self.data->clone();
		}
		AdditionalLayerInfo & operator = (const AdditionalLayerInfo &self) {
			delete data;
			this->sigType = self.sigType;
			this->key = self.key;
			this->size = self.size;
			this->data = self.data == 0 ? 0 : self.data->clone();
			return *this;
		}
		int sigType;
		int key;
		int size;
		IteratorBase *data;
	};

	struct LayerExtraData {
		LayerMask layerMask;
		LayerBlendingRange layerBlendingRange;
		std::string layerName;
    std::vector<AdditionalLayerInfo> additionalLayers;
	};

	// �`�����l�����
	struct ChannelInfo {
		ChannelInfo(int id, int length) : id(id), length(length), imageData(0) {};
    ~ChannelInfo() { delete imageData; }
    ChannelInfo(const ChannelInfo &self) {
			this->id        = self.id;
			this->length    = self.length;
			this->imageData = self.imageData == 0 ? 0 : self.imageData->clone();
		}
		ChannelInfo & operator = (const ChannelInfo &self) {
			delete imageData;
			this->id        = self.id;
			this->length    = self.length;
			this->imageData = self.imageData == 0 ? 0 : self.imageData->clone();
			return *this;
		}
    
		int id;
		int length;
    IteratorBase *imageData;

    bool isMaskChannel() { return (id == -3 || id == -2); }
	};

	// ���C�����
  class Data;
	struct LayerInfo {
    // ���C���̏�������psd::Data�C���X�^���X
    Data *owner;
    
    // parsed raw data
    int width;
    int height;
		int top;
		int left;
		int bottom;
		int right;
		std::vector<ChannelInfo> channels;
		int blendModeKey;
    BlendMode blendMode;
		int opacity;
		int clipping;
		int flag;
		LayerExtraData extraData;
    
    // migrate from extra/addtionals
    int layerId;
    LayerType layerType;
		std::string layerName;
    std::wstring layerNameUnicode;
    // int layerNameId;
    // int foreignEffectId;
    // BlendMode folderBlendMode; // blendMode �㏑���ɂ��Ă���B��肠��Ε���

    // ���C���[�J���v���
    std::map<int, LayerCompInfo> layerComps;
    Descriptor layerCompDesc; // �f�B�X�N���v�^�`���őS���^�f�[�^���i�[

    // �e�t�H���_���C��
    LayerInfo *parent;

    bool isTransparencyProtected() { return (flag & (1 << 0)) != 0; }
    bool isVisible()               { return (flag & (1 << 1)) == 0; }
    bool isObsolete()              { return (flag & (1 << 2)) != 0; }
    bool isLaterVer5()             { return (flag & (1 << 3)) != 0; }
    bool isPixelDataIrrelevant()   { return (flag & (1 << 4)) != 0; }
	};
	
	/**
	 * PSD Infomation class
	 */
	class Data {
	public:
		// �R���X�g���N�^
		Data()
			 : colorModeSize(0), colorModeIterator(0),
			   mergedAlpha(false), channelImageData(0),
			   imageData(0)
		{
		}

		// �f�X�g���N�^
		virtual ~Data() {
			clearData();
		}

		// �ێ��f�[�^�̏���
		virtual void clearData() {
			delete colorModeIterator; colorModeIterator = 0;
			delete channelImageData; channelImageData = 0;
			delete imageData; imageData = 0;
		}


		// ���C�������C��ID�Ŏ擾
    LayerInfo *getLayerById(int layerId);

		// ---------------------------------------
		// �C���[�W���\�[�X
		// ---------------------------------------

		// �w�b�_���
		Header header;

		// �J���[���[�h���
		int colorModeSize;
		IteratorBase *colorModeIterator;
		
		// �C���[�W���\�[�X�ꗗ
		std::vector<ImageResourceInfo> imageResourceList;
		
		// ��������񂪂��邩�ǂ���
		bool mergedAlpha;

		// ���C�����ꗗ
		std::vector<LayerInfo> layerList;

		// �`�����l���摜�f�[�^
		IteratorBase *channelImageData;
		
		// global layer mask info
		GlobalLayerMaskInfo globalLayerMaskInfo;
		
		// �����ς݉摜�f�[�^
		IteratorBase *imageData;

    // �W�J�ς݃��\�[�X�f�[�^
    SliceResource      slice;       // �X���C�X
    GridGuideResource  gridGuide;   // �O���b�h/�K�C�h
    ColorTable         colorTable;  // �J���[�e�[�u��(�C���f�b�N�X�J���[�p)
    std::vector<LayerComp> layerComps; // ���C���[�J���v
    int lastAppliedCompId;             // �ŏI�K�p�J���v

  protected:
    bool processParsed();
	};
}
#endif
