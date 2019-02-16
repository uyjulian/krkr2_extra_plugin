#include <ncbind.hpp>
#include "psdclass.h"

#define BMPEXT L".bmp"

// ncb.typeconv: cast: enum->int
NCB_TYPECONV_CAST_INTEGER(psd::LayerType);
NCB_TYPECONV_CAST_INTEGER(psd::BlendMode);

static int convBlendMode(psd::BlendMode mode)
{
	switch (mode) {
	case psd::BLEND_MODE_NORMAL:			// 'norm' = normal
		return ltPsNormal;
	case psd::BLEND_MODE_DARKEN:			// 'dark' = darken
		return ltPsDarken;
	case psd::BLEND_MODE_MULTIPLY:		// 'mul ' = multiply
		return ltPsMultiplicative;
	case psd::BLEND_MODE_COLOR_BURN:		// 'idiv' = color burn
		return ltPsColorBurn;
	case psd::BLEND_MODE_LINEAR_BURN:		// 'lbrn' = linear burn
		return ltPsSubtractive;
	case psd::BLEND_MODE_LIGHTEN:			// 'lite' = lighten
		return ltPsLighten;
	case psd::BLEND_MODE_SCREEN:			// 'scrn' = screen
		return ltPsScreen;
	case psd::BLEND_MODE_COLOR_DODGE:		// 'div ' = color dodge
		return ltPsColorDodge;
	case psd::BLEND_MODE_LINEAR_DODGE:	// 'lddg' = linear dodge
		return ltPsAdditive;
	case psd::BLEND_MODE_OVERLAY:			// 'over' = overlay
		return ltPsOverlay;
	case psd::BLEND_MODE_SOFT_LIGHT:		// 'sLit' = soft light
		return ltPsSoftLight;
	case psd::BLEND_MODE_HARD_LIGHT:		// 'hLit' = hard light
		return ltPsHardLight;
	case psd::BLEND_MODE_DIFFERENCE:		// 'diff' = difference
		return ltPsDifference;
	case psd::BLEND_MODE_EXCLUSION:		// 'smud' = exclusion
		return ltPsExclusion;
	case psd::BLEND_MODE_DISSOLVE:		// 'diss' = dissolve
	case psd::BLEND_MODE_VIVID_LIGHT:		// 'vLit' = vivid light
	case psd::BLEND_MODE_LINEAR_LIGHT:	// 'lLit' = linear light
	case psd::BLEND_MODE_PIN_LIGHT:		// 'pLit' = pin light
	case psd::BLEND_MODE_HARD_MIX:		// 'hMix' = hard mix
  case psd::BLEND_MODE_DARKER_COLOR:
  case psd::BLEND_MODE_LIGHTER_COLOR:
  case psd::BLEND_MODE_SUBTRACT:
  case psd::BLEND_MODE_DIVIDE:
		// not supported;
		break;
	}
	return ltPsNormal;
}

/**
 * C�����񏈗��p
 */
class NarrowString {
private:
	tjs_nchar *_data;
public:
	NarrowString(const ttstr &str) : _data(NULL) {
		tjs_int len = str.GetNarrowStrLen();
		if (len > 0) {
			_data = new tjs_nchar[len+1];
			str.ToNarrowStr(_data, len+1);
		}
	}
	~NarrowString() {
		delete[] _data;
	}

	const tjs_nchar *data() {
		return _data;
	}

	operator const char *() const
	{
		return (const char *)_data;
	}
};

/**
 * �R���X�g���N�^
 */
PSD::PSD(iTJSDispatch2 *objthis) : objthis(objthis)
#ifdef LOAD_MEMORY
, hBuffer(0)
#else
, pStream(0)
, mStreamSize(0)
, mBufferPos(0)
, mBufferSize(0)
#endif
, storageStarted(false)
{
};

/**
 * �f�X�g���N�^
 */
PSD::~PSD() {
	clearData(); // �����ŌĂ΂Ȃ��� delete ���ɂ͐e�̂ق��ł͉��z�֐�����΂�Ȃ�
};

/**
 * �C���X�^���X�����t�@�N�g��
 */
tjs_error
PSD::factory(PSD **result, tjs_int numparams, tTJSVariant **params, iTJSDispatch2 *objthis)
{
	*result = new PSD(objthis);
	return S_OK;
}

/**
 * �������̎��ȃI�u�W�F�N�g���擾
 */
tTJSVariant
PSD::getSelf()
{
	return tTJSVariant(objthis, objthis);
}

/**
 * PSD�摜�̃��[�h
 * @param filename �t�@�C����
 * @return ���[�h�ɐ��������� true
 */
bool
PSD::load(ttstr filename)
{
	ttstr file = TVPGetPlacedPath(filename);
	if (!file.length()) {
		// ������Ȃ������̂Ń��[�J���p�X�Ƃ݂Ȃ��ēǂݍ���
		psd::PSDFile::load(NarrowString(filename));
	} else {
#ifdef LOAD_MEMORY
		if (!wcschr(file.c_str(), '>')) {
			// ���[�J���t�@�C���Ȃ̂Œ��ړǂݍ���
			TVPGetLocalName(file);
			psd::PSDFile::load(NarrowString(file));
		} else {
			// �������ɓǂݍ���Ń��[�h
			loadMemory(file);
		}
#else
		// �X�g���[���Ƃ��ă��[�h
		loadStream(file);
#endif
	}
	if (isLoaded) {
		addToStorage(filename);
	}
	return isLoaded;
}

void
PSD::clearData()
{
	removeFromStorage();
	layerIdIdxMap.clear();
	pathMap.clear();
	storageStarted = false;

	psd::PSDFile::clearData();
#ifdef LOAD_MEMORY
	clearMemory();
#else
	clearStream();
#endif
}
	
/**
 * ���C���ԍ����K�؂��ǂ�������
 * @param no ���C���ԍ�
 */
void
PSD::checkLayerNo(int no)
{
	if (!isLoaded) {
		TVPThrowExceptionMessage(L"no data");
	}
	if (no < 0 || no >= get_layer_count()) {
		TVPThrowExceptionMessage(L"not such layer");
	}
}

/**
 * ���O�̎擾
 * @param lay���C�����
 */
ttstr
PSD::layname(psd::LayerInfo &lay)
{
	ttstr ret;
	if (!lay.layerNameUnicode.empty()) {
		ret = ttstr(lay.layerNameUnicode.c_str()); 
	} else {
		ret = ttstr(lay.layerName.c_str());
	}
	return ret;
}

/**
 * ���C����ʂ̎擾
 * @param no ���C���ԍ�
 * @return ���C�����
 */
int
PSD::getLayerType(int no)
{
	checkLayerNo(no);
	return (int)layerList[no].layerType;
}

/**
 * ���C�����̂̎擾
 * @param no ���C���ԍ�
 * @return ���C�����
 */
ttstr
PSD::getLayerName(int no)
{
	checkLayerNo(no);
	return layname(layerList[no]);
}

/**
 * ���C�����̎擾
 * @param no ���C���ԍ�
 * @return ���C����񂪊i�[���ꂽ����
 */
tTJSVariant
PSD::getLayerInfo(int no)
{
	checkLayerNo(no);
	psd::LayerInfo &lay = layerList[no];
	tTJSVariant result;	
	ncbDictionaryAccessor dict;
	if (dict.IsValid()) {
#define SETPROP(dict, obj, prop) dict.SetValue(L ## #prop, obj.prop)
		SETPROP(dict, lay, top);
		SETPROP(dict, lay, left);
		SETPROP(dict, lay, bottom);
		SETPROP(dict, lay, right);
		SETPROP(dict, lay, width);
		SETPROP(dict, lay, height);
		SETPROP(dict, lay, opacity);
		bool mask = false;
		for (std::vector<psd::ChannelInfo>::iterator i = lay.channels.begin();
				 i != lay.channels.end();
				 i++) {
			if (i->isMaskChannel()) {
				mask = true;
				break;
			}
		}
		dict.SetValue(L"mask", mask);
		dict.SetValue(L"type",       convBlendMode(lay.blendMode));
		dict.SetValue(L"layer_type", lay.layerType);
		dict.SetValue(L"blend_mode", lay.blendMode);
		dict.SetValue(L"visible",    lay.isVisible());
		dict.SetValue(L"name",       layname(lay));

		// additional information
		SETPROP(dict, lay, clipping);
		dict.SetValue(L"layer_id", lay.layerId);
		dict.SetValue(L"obsolete", lay.isObsolete());
		dict.SetValue(L"transparency_protected", lay.isTransparencyProtected());
		dict.SetValue(L"pixel_data_irrelevant",  lay.isPixelDataIrrelevant());

		// ���C���[�J���v
		if (lay.layerComps.size() > 0) {
			ncbDictionaryAccessor compDict;
			if (compDict.IsValid()) {
				for (std::map<int, psd::LayerCompInfo>::iterator it = lay.layerComps.begin();
						 it != lay.layerComps.end(); it++)	{
					ncbDictionaryAccessor tmp;
					if (tmp.IsValid()) {
						psd::LayerCompInfo &comp = it->second;
						tmp.SetValue(L"id",         comp.id);
						tmp.SetValue(L"offset_x",   comp.offsetX);
						tmp.SetValue(L"offset_y",   comp.offsetY);
						tmp.SetValue(L"enable",     comp.isEnabled);
						compDict.SetValue((tjs_int32)comp.id, tmp.GetDispatch());
					}
				}
				dict.SetValue(L"layer_comp", compDict.GetDispatch());
			}
		}

		// SETPROP(dict, lay, adjustment_valid); // �������C���[���ǂ����H���C���^�C�v�Ŕ��ʉ\
		// SETPROP(dict, lay, fill_opacity);
		// SETPROP(dict, lay, layer_name_id);
		// SETPROP(dict, lay, layer_version);
		// SETPROP(dict, lay, blend_clipped);
		// SETPROP(dict, lay, blend_interior);
		// SETPROP(dict, lay, knockout);
		// SETPROP(dict, lay, transparency); // lspf(protection)�̂���
		// SETPROP(dict, lay, composite);
		// SETPROP(dict, lay, position_respectively);
		// SETPROP(dict, lay, sheet_color);
		// SETPROP(dict, lay, reference_point_x); // �h��Ԃ����C���i�p�^�[���j�̃I�t�Z�b�g
		// SETPROP(dict, lay, reference_point_y); // �h��Ԃ����C���i�p�^�[���j�̃I�t�Z�b�g
		// SETPROP(dict, lay, transparency_shapes_layer);
		// SETPROP(dict, lay, layer_mask_hides_effects);
		// SETPROP(dict, lay, vector_mask_hides_effects);
		// SETPROP(dict, lay, divider_type);
		// SETPROP(dict, lay, divider_blend_mode);

		// group layer �̓X�N���v�g���ł� layer_id �Q�Ƃň����悤�ɂ���
		if (lay.parent != NULL)
			dict.SetValue(L"group_layer_id", lay.parent->layerId);

		result = dict;
	}

	return result;
}

/**
 * ���C���f�[�^�̓ǂݏo��(��������)
 * @param layer �ǂݏo���惌�C��
 * @param no ���C���ԍ�
 * @param imageMode �C���[�W���[�h
 */
void
PSD::_getLayerData(tTJSVariant layer, int no, psd::ImageMode imageMode)
{
	if (!layer.AsObjectNoAddRef()->IsInstanceOf(0, 0, 0, L"Layer", NULL)) {
		TVPThrowExceptionMessage(L"not layer");
	}
	checkLayerNo(no);

	psd::LayerInfo &lay = layerList[no];
	psd::LayerMask &mask  = lay.extraData.layerMask;

	if (lay.layerType != psd::LAYER_TYPE_NORMAL
			&& ! (lay.layerType == psd::LAYER_TYPE_FOLDER
						&& imageMode == psd::IMAGE_MODE_MASK)) {
		TVPThrowExceptionMessage(L"invalid layer type");
	}

	int left, top, width, height, opacity, type;

	bool dummyMask = false;
	if (imageMode == psd::IMAGE_MODE_MASK) {
		left = mask.left;
		top = mask.top;
		width = mask.width;
		height = mask.height;
		opacity = 255;
		type = ltPsNormal;
		if (width == 0 || height == 0) {
			left = top = 0;
			width = height = 1;
			dummyMask = true;
		}
	} else {
		left = lay.left;
		top = lay.top;
		width = lay.width;
		height = lay.height;
		opacity = lay.opacity;
		type = convBlendMode(lay.blendMode);
	}
	if (width <= 0 || height <= 0) {
		// �T�C�Y�O�̃��C���̓��[�h�ł��Ȃ�
		return;
	}

	ncbPropAccessor obj(layer);
	obj.SetValue(L"left", left);
	obj.SetValue(L"top", top);
	obj.SetValue(L"opacity", opacity);
	obj.SetValue(L"width",  width);
	obj.SetValue(L"height", height);
	obj.SetValue(L"type",   type);
	obj.SetValue(L"visible", lay.isVisible());
	obj.SetValue(L"imageLeft",  0);
	obj.SetValue(L"imageTop",   0);
	obj.SetValue(L"imageWidth",  width);
	obj.SetValue(L"imageHeight", height);
	obj.SetValue(L"name", layname(lay));

	if (imageMode == psd::IMAGE_MODE_MASK)
		obj.SetValue(L"defaultMaskColor", mask.defaultColor);

	// �摜�f�[�^�̃R�s�[
	unsigned char *buffer = (unsigned char*)obj.GetValue(L"mainImageBufferForWrite", ncbTypedefs::Tag<tjs_int>());
	int pitch = obj.GetValue(L"mainImageBufferPitch", ncbTypedefs::Tag<tjs_int>());
	if (dummyMask) {
		buffer[0] = buffer[1] = buffer[2] = mask.defaultColor;
		buffer[3] = 255;
	} else {
		getLayerImage(lay, buffer, psd::BGRA_LE, pitch, imageMode);
	}
}


/**
 * ���C���f�[�^�̓ǂݏo��
 * @param layer �ǂݏo���惌�C��
 * @param no ���C���ԍ�
 */
void
PSD::getLayerData(tTJSVariant layer, int no)
{
	_getLayerData(layer, no, psd::IMAGE_MODE_MASKEDIMAGE);
}

/**
 * ���C���f�[�^�̓ǂݏo��(���C���[�W)
 * @param layer �ǂݏo���惌�C��
 * @param no ���C���ԍ�
 */
void
PSD::getLayerDataRaw(tTJSVariant layer, int no)
{
	_getLayerData(layer, no, psd::IMAGE_MODE_IMAGE);
}

/**
 * ���C���f�[�^�̓ǂݏo��(�}�X�N�̂�)
 * @param layer �ǂݏo���惌�C��
 * @param no ���C���ԍ�
 */
void
PSD::getLayerDataMask(tTJSVariant layer, int no)
{
	_getLayerData(layer, no, psd::IMAGE_MODE_MASK);
}

/**
 * �X���C�X�f�[�^�̓ǂݏo��
 * @return �X���C�X��񎫏� %[ top, left, bottom, right, slices:[ %[ id, group_id, left, top, bottom, right ], ... ] ]
 *         �X���C�X��񂪂Ȃ��ꍇ�� void ��Ԃ�
 */
tTJSVariant
PSD::getSlices()
{
	if (!isLoaded) TVPThrowExceptionMessage(L"no data");
	tTJSVariant result;
	ncbDictionaryAccessor dict;
	ncbArrayAccessor arr;
	if (slice.isEnabled) {
		if (dict.IsValid()) {
			psd::SliceResource &sr = slice;
			dict.SetValue(L"top",    sr.boundingTop);
			dict.SetValue(L"left",   sr.boundingLeft);
			dict.SetValue(L"bottom", sr.boundingBottom);
			dict.SetValue(L"right",  sr.boundingRight);
			dict.SetValue(L"name",   ttstr(sr.groupName.c_str()));
			if (arr.IsValid()) {
				for (int i = 0; i < (int)sr.slices.size(); i++) {
					ncbDictionaryAccessor tmp;
					if (tmp.IsValid()) {
						psd::SliceItem &item = sr.slices[i];
						tmp.SetValue(L"id",      	item.id);
						tmp.SetValue(L"group_id", item.groupId);
						tmp.SetValue(L"origin",   item.origin);
						tmp.SetValue(L"type",     item.type);
						tmp.SetValue(L"left",     item.left);
						tmp.SetValue(L"top",      item.top);
						tmp.SetValue(L"right",    item.right);
						tmp.SetValue(L"bottom",   item.bottom);
						tmp.SetValue(L"color",    ((item.colorA<<24) | (item.colorR<<16) | (item.colorG<<8) | item.colorB));
						tmp.SetValue(L"cell_text_is_html",    item.isCellTextHtml);
						tmp.SetValue(L"horizontal_alignment", item.horizontalAlign);
						tmp.SetValue(L"vertical_alignment",   item.verticalAlign);
						tmp.SetValue(L"associated_layer_id",	item.associatedLayerId);
						tmp.SetValue(L"name",      ttstr(item.name.c_str()));
						tmp.SetValue(L"url",       ttstr(item.url.c_str()));
						tmp.SetValue(L"target",    ttstr(item.target.c_str()));
						tmp.SetValue(L"message",   ttstr(item.message.c_str()));
						tmp.SetValue(L"alt_tag",   ttstr(item.altTag.c_str()));
						tmp.SetValue(L"cell_text", ttstr(item.cellText.c_str()));
						arr.SetValue((tjs_int32)i, tmp.GetDispatch());
					}
				}
				dict.SetValue(L"slices", arr.GetDispatch());
			}
			result = dict;
		}
	}
	return result;
}

/**
 * �K�C�h�f�[�^�̓ǂݏo��
 * @return �K�C�h��񎫏� %[ vertical:[ x1, x2, ... ], horizontal:[ y1, y2, ... ] ]
 *         �K�C�h��񂪂Ȃ��ꍇ�� void ��Ԃ�
 */
tTJSVariant
PSD::getGuides()
{
	if (!isLoaded) TVPThrowExceptionMessage(L"no data");
	tTJSVariant result;
	ncbDictionaryAccessor dict;
	ncbArrayAccessor vert, horz;
	if (gridGuide.isEnabled) {
		psd::GridGuideResource gg = gridGuide;
		if (dict.IsValid() && vert.IsValid() && horz.IsValid()) {
			dict.SetValue(L"horz_grid",  gg.horizontalGrid);
			dict.SetValue(L"vert_grid",  gg.verticalGrid);
			dict.SetValue(L"vertical",   vert.GetDispatch());
			dict.SetValue(L"horizontal", horz.GetDispatch());
			for (int i = 0, v = 0, h = 0; i < (int)gg.guides.size(); i++) {
				if (gg.guides[i].direction == 0) {
					vert.SetValue(v++, gg.guides[i].location);
				} else {
					horz.SetValue(h++, gg.guides[i].location);
				}
			}
			result = dict;
		}
	}
	return result;
}

/**
 * �������ʂ̎擾�B�擾�̈�͉摜�S�̃T�C�Y���ɂ����܂��Ă�K�v������܂�
 * ���ӁFPSD�t�@�C�����̂ɍ����ς݉摜�����݂��Ȃ��ꍇ�͎擾�Ɏ��s���܂�
 *
 * @param layer �i�[�惌�C��(width,height�T�C�Y�ɒ��������)
 * @return �擾�ɐ��������� true
 */
bool
PSD::getBlend(tTJSVariant layer) {
	if (!layer.AsObjectNoAddRef()->IsInstanceOf(0, 0, 0, L"Layer", NULL)) {
		TVPThrowExceptionMessage(L"not layer");
	}

	// �������ʂ𐶐�
	if (imageData) {

		// �i�[��𒲐�
		ncbPropAccessor obj(layer);
		obj.SetValue(L"width",  get_width());
		obj.SetValue(L"height", get_height());
		obj.SetValue(L"imageLeft",  0);
		obj.SetValue(L"imageTop",   0);
		obj.SetValue(L"imageWidth",  get_width());
		obj.SetValue(L"imageHeight", get_height());

		// �摜�f�[�^�̃R�s�[
		unsigned char *buffer = (unsigned char*)obj.GetValue(L"mainImageBufferForWrite", ncbTypedefs::Tag<tjs_int>());
		int pitch = obj.GetValue(L"mainImageBufferPitch", ncbTypedefs::Tag<tjs_int>());
		getMergedImage(buffer, psd::BGRA_LE, pitch);

		return true;
	}

	return false;
}

/**
 * ���C���[�J���v
 */
tTJSVariant
PSD::getLayerComp()
{
	if (!isLoaded) TVPThrowExceptionMessage(L"no data");
	tTJSVariant result;
	ncbDictionaryAccessor dict;
	ncbArrayAccessor arr;
	int compNum = layerComps.size();
	if (compNum > 0) {
		if (dict.IsValid()) {
			dict.SetValue(L"last_applied_id", lastAppliedCompId);
			if (arr.IsValid()) {
				for (int i = 0; i < compNum; i++) {
					ncbDictionaryAccessor tmp;
					if (tmp.IsValid()) {
						psd::LayerComp &comp = layerComps[i];
						tmp.SetValue(L"id",      	        comp.id);
						tmp.SetValue(L"record_visibility", comp.isRecordVisibility);
						tmp.SetValue(L"record_position",   comp.isRecordPosition);
						tmp.SetValue(L"record_appearance", comp.isRecordAppearance);
						tmp.SetValue(L"name",             ttstr(comp.name.c_str()));
						tmp.SetValue(L"comment",          ttstr(comp.comment.c_str()));
						arr.SetValue((tjs_int32)i,        tmp.GetDispatch());
					}
				}
				dict.SetValue(L"comps", arr.GetDispatch());
			}
			result = dict;
		}
	}
	return result;
}

// ���C������Ԃ�
ttstr
PSD::path_layname(psd::LayerInfo &lay)
{
	ttstr ret = layname(lay);
	// ���K��
	ttstr from = "/";
	ttstr to   = "_";
	ret.Replace(from, to, true);
	ret.ToLowerCase();
	return ret;
}

// ���C���̃p�X����Ԃ�
ttstr
PSD::pathname(psd::LayerInfo &lay)
{
	ttstr name = "";
	psd::LayerInfo *p = lay.parent;
	while (p) {
		name = path_layname(*p) + "/" + name;
		p = p->parent;
	}
	return ttstr("root/") + name;
}

// �X�g���[�W�����p�f�[�^�̏�����
void
PSD::startStorage()
{
	if (!storageStarted) {
		storageStarted = true;
		// ���C�������p�̏��𐶐�
		int count = (int)layerList.size();
		for (int i=count-1;i>=0;i--) {
			psd::LayerInfo &lay = layerList[i];
			if (lay.layerType == psd::LAYER_TYPE_NORMAL) {
				pathMap[pathname(lay)][path_layname(lay)] = i;
				layerIdIdxMap[lay.layerId] = i;
			}
		}
	}
}

bool
checkAllNum(const tjs_char *p)
{
	while (*p != '\0') {
		if (!(*p >= '0' && *p <= '9')) {
			return false;
		}
		p++;
	}
	return true;
}
	
/*
 * �w�肵�����O�̃��C���̑��݃`�F�b�N
 * @param name �p�X���܂ރ��C����
 * @param layerIdxRet ���C���C���f�b�N�X�ԍ���Ԃ�
 */
bool
PSD::CheckExistentStorage(const ttstr &filename, int *layerIdxRet)
{
	startStorage();

	// ���[�g�����擾
	const tjs_char *p = filename.c_str();

	// id�w��̏ꍇ
	if (wcsncmp(p, L"id/", 3) == 0) {

		p += 3;

		// �g���q���������Ĕ���
		const tjs_char *q;
		if (!(q = wcsrchr(p, '/')) && ((q = wcschr(p, '.')) && (wcscmp(q, BMPEXT) == 0))) {
			ttstr name = ttstr(p, q-p);
			q = name.c_str();
			if (checkAllNum(q)) { // ���������֎~
				int id = _wtoi(q);
				LayerIdIdxMap::const_iterator n = layerIdIdxMap.find(id);
				if (n != layerIdIdxMap.end()) {
					if (layerIdxRet) *layerIdxRet = n->second;
					return true;
				}
			}
		}

	} else {

		// �p�X�𕪗�
		ttstr pname, fname;
		// �Ō��/��T��
		const tjs_char *q;
		if ((q = wcsrchr(p, '/'))) {
			pname = ttstr(p, q-p+1);
			fname = ttstr(q+1);
		} else {
			return false;
		}

		// �g���q����
		ttstr basename;
		p = fname.c_str();
		// �ŏ��� . ��T��
		if ((q = wcschr(p, '.')) && (wcscmp(q, BMPEXT) == 0)) {
			basename = ttstr(p, q-p);
		} else {
			return false;
		}

		// ���O��T��
		PathMap::const_iterator n = pathMap.find(pname);
		if (n != pathMap.end()) {
			const NameIdxMap &names = n->second;
			NameIdxMap::const_iterator m = names.find(basename);
			if (m != names.end()) {
				if (layerIdxRet) *layerIdxRet = m->second;
				return true;
			}
		}
	}

	return false;
}

/*
 * �w�肵���p�X�ɂ���t�@�C�����ꗗ�̎擾
 * @param pathname �p�X��
 * @param lister ���X�g�擾�p�C���^�[�t�F�[�X
 */
void
PSD::GetListAt(const ttstr &pathname, iTVPStorageLister *lister)
{
	startStorage();

	// ID�ꗗ���疼�O�𐶐�
	if (pathname == "id/") {
		LayerIdIdxMap::const_iterator it = layerIdIdxMap.begin();
		while (it != layerIdIdxMap.end()) {
			ttstr name = ttstr(it->first);
			lister->Add(name + BMPEXT);
			it++;
		}
		return;
	}

	// �p�X�o�^��񂩂疼�O�𐶐�
	PathMap::const_iterator n = pathMap.find(pathname);
	if (n != pathMap.end()) {
		const NameIdxMap &names = n->second;
		NameIdxMap::const_iterator it = names.begin();
		while (it != names.end()) {
			ttstr name = it->first;
			lister->Add(name + BMPEXT);
			it++;
		}
	}
}

/*
 * �w�肵�����O�̃��C���̉摜�t�@�C�����X�g���[���ŕԂ�
 * @param name �p�X���܂ރ��C����
 * @return �t�@�C���X�g���[��
 */
IStream *
PSD::openLayerImage(const ttstr &name)
{
	static int n=0;

	int layerIdx;
	if (CheckExistentStorage(name, &layerIdx)) {
		if (layerIdx < (int)layerList.size()) {
			psd::LayerInfo &lay = layerList[layerIdx];

			if (lay.layerType != psd::LAYER_TYPE_NORMAL || lay.width <= 0 || lay.height <= 0) {
				return 0;
			}
			int width  = lay.width;
			int height = lay.height;
			int pitch  = width*4;

			int hsize = sizeof(BITMAPFILEHEADER);
			int isize = hsize + sizeof(BITMAPINFOHEADER);
			int size  = isize  + pitch * height;

			// �O���[�o���q�[�v��BMP�摜���쐬���ăX�g���[���Ƃ��ĕԂ�
			HGLOBAL handle = ::GlobalAlloc(GMEM_MOVEABLE, size);
			if (handle) {
				unsigned char *p = (unsigned char*)::GlobalLock(handle);
				if (p) {

					BITMAPFILEHEADER bfh;
					bfh.bfType      = 'B' + ('M' << 8);
					bfh.bfSize      = size;
					bfh.bfReserved1 = 0;
					bfh.bfReserved2 = 0;
					bfh.bfOffBits   = isize;
					memcpy(p,        &bfh, sizeof bfh);

					BITMAPINFOHEADER bih;
					bih.biSize = sizeof(bih);
					bih.biWidth = width;
					bih.biHeight = height;
					bih.biPlanes = 1;
					bih.biBitCount = 32;
					bih.biCompression = BI_RGB;
					bih.biSizeImage = 0;
					bih.biXPelsPerMeter = 0;
					bih.biYPelsPerMeter = 0;
					bih.biClrUsed = 0;
					bih.biClrImportant = 0;
					memcpy(p + hsize, &bih, sizeof bih);
					getLayerImage(lay, p + isize + pitch * (height - 1), psd::BGRA_LE, -pitch, psd::IMAGE_MODE_MASKEDIMAGE);
					::GlobalUnlock(handle);

					IStream *pStream = 0;
					if (SUCCEEDED(::CreateStreamOnHGlobal(handle, TRUE, &pStream))) {
						return pStream;
					}
				}
				::GlobalFree(handle);
			}
		}
	}
	return 0;
}
	

NCB_REGISTER_CLASS(PSD) {

	Factory(&ClassT::factory);

	Variant("color_mode_bitmap",              (int)psd::COLOR_MODE_BITMAP);
  Variant("color_mode_grayscale",           (int)psd::COLOR_MODE_GRAYSCALE);
  Variant("color_mode_indexed",             (int)psd::COLOR_MODE_INDEXED);
  Variant("color_mode_rgb",                 (int)psd::COLOR_MODE_RGB);
  Variant("color_mode_cmyk",                (int)psd::COLOR_MODE_CMYK);
  Variant("color_mode_multichannel",        (int)psd::COLOR_MODE_MULTICHANNEL);
  Variant("color_mode_duotone",             (int)psd::COLOR_MODE_DUOTONE);
  Variant("color_mode_lab",                 (int)psd::COLOR_MODE_LAB);
  
  Variant("blend_mode_normal",              (int)psd::BLEND_MODE_NORMAL);
  Variant("blend_mode_dissolve",            (int)psd::BLEND_MODE_DISSOLVE);
  Variant("blend_mode_darken",              (int)psd::BLEND_MODE_DARKEN);
  Variant("blend_mode_multiply",            (int)psd::BLEND_MODE_MULTIPLY);
  Variant("blend_mode_color_burn",          (int)psd::BLEND_MODE_COLOR_BURN);
  Variant("blend_mode_linear_burn",         (int)psd::BLEND_MODE_LINEAR_BURN);
  Variant("blend_mode_lighten",             (int)psd::BLEND_MODE_LIGHTEN);
  Variant("blend_mode_screen",              (int)psd::BLEND_MODE_SCREEN);
  Variant("blend_mode_color_dodge",         (int)psd::BLEND_MODE_COLOR_DODGE);
  Variant("blend_mode_linear_dodge",        (int)psd::BLEND_MODE_LINEAR_DODGE);
  Variant("blend_mode_overlay",             (int)psd::BLEND_MODE_OVERLAY);
  Variant("blend_mode_soft_light",          (int)psd::BLEND_MODE_SOFT_LIGHT);
  Variant("blend_mode_hard_light",          (int)psd::BLEND_MODE_HARD_LIGHT);
  Variant("blend_mode_vivid_light",         (int)psd::BLEND_MODE_VIVID_LIGHT);
  Variant("blend_mode_linear_light",        (int)psd::BLEND_MODE_LINEAR_LIGHT);
  Variant("blend_mode_pin_light",           (int)psd::BLEND_MODE_PIN_LIGHT);
  Variant("blend_mode_hard_mix",            (int)psd::BLEND_MODE_HARD_MIX);
  Variant("blend_mode_difference",          (int)psd::BLEND_MODE_DIFFERENCE);
  Variant("blend_mode_exclusion",           (int)psd::BLEND_MODE_EXCLUSION);
  Variant("blend_mode_hue",                 (int)psd::BLEND_MODE_HUE);
  Variant("blend_mode_saturation",          (int)psd::BLEND_MODE_SATURATION);
  Variant("blend_mode_color",               (int)psd::BLEND_MODE_COLOR);
  Variant("blend_mode_luminosity",          (int)psd::BLEND_MODE_LUMINOSITY);
  Variant("blend_mode_pass_through",        (int)psd::BLEND_MODE_PASS_THROUGH);

  // NOTE libpsd ��݊����[�h
  Variant("blend_mode_darker_color",        (int)psd::BLEND_MODE_DARKER_COLOR);
  Variant("blend_mode_lighter_color",       (int)psd::BLEND_MODE_LIGHTER_COLOR);
  Variant("blend_mode_subtract",            (int)psd::BLEND_MODE_SUBTRACT);
  Variant("blend_mode_divide",              (int)psd::BLEND_MODE_DIVIDE);
  

  // NOTE ���̒萔��libpsd�݊��ł͂���܂���(folder�܂ł͌݊�)
  Variant("layer_type_normal",              (int)psd::LAYER_TYPE_NORMAL);
  Variant("layer_type_hidden",              (int)psd::LAYER_TYPE_HIDDEN);
  Variant("layer_type_folder",              (int)psd::LAYER_TYPE_FOLDER);
  Variant("layer_type_adjust",              (int)psd::LAYER_TYPE_ADJUST);
  Variant("layer_type_fill",                (int)psd::LAYER_TYPE_FILL);

	NCB_METHOD(load);

#define INTPROP(name) Property(TJS_W(# name), &Class::get_ ## name, NULL)

	INTPROP(width);
	INTPROP(height);
	INTPROP(channels);
	INTPROP(depth);
	INTPROP(color_mode);
	INTPROP(layer_count);

	NCB_METHOD(getLayerType);
	NCB_METHOD(getLayerName);
	NCB_METHOD(getLayerInfo);
	NCB_METHOD(getLayerData);
	NCB_METHOD(getLayerDataRaw);
	NCB_METHOD(getLayerDataMask);

	NCB_METHOD(getSlices);
	NCB_METHOD(getGuides);
	NCB_METHOD(getBlend);
  NCB_METHOD(getLayerComp);

  NCB_METHOD(clearStorageCache);
};

