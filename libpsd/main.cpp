#include "ncbind/ncbind.hpp"
#include <libpsd.h>

// ncb.typeconv: cast: enum->int
NCB_TYPECONV_CAST_INTEGER(psd_layer_type);
NCB_TYPECONV_CAST_INTEGER(psd_blend_mode);

static int convBlendMode(psd_blend_mode mode)
{
	switch (mode) {
	case psd_blend_mode_normal:			// 'norm' = normal
		return ltPsNormal;
	case psd_blend_mode_darken:			// 'dark' = darken
		return ltPsDarken;
	case psd_blend_mode_multiply:		// 'mul ' = multiply
		return ltPsMultiplicative;
	case psd_blend_mode_color_burn:		// 'idiv' = color burn
		return ltPsColorBurn;
	case psd_blend_mode_linear_burn:		// 'lbrn' = linear burn
		return ltPsSubtractive;
	case psd_blend_mode_lighten:			// 'lite' = lighten
		return ltPsLighten;
	case psd_blend_mode_screen:			// 'scrn' = screen
		return ltPsScreen;
	case psd_blend_mode_color_dodge:		// 'div ' = color dodge
		return ltPsColorDodge;
	case psd_blend_mode_linear_dodge:	// 'lddg' = linear dodge
		return ltPsAdditive;
	case psd_blend_mode_overlay:			// 'over' = overlay
		return ltPsOverlay;
	case psd_blend_mode_soft_light:		// 'sLit' = soft light
		return ltPsSoftLight;
	case psd_blend_mode_hard_light:		// 'hLit' = hard light
		return ltPsHardLight;
	case psd_blend_mode_difference:		// 'diff' = difference
		return ltPsDifference;
	case psd_blend_mode_exclusion:		// 'smud' = exclusion
		return ltPsExclusion;
	case psd_blend_mode_dissolve:		// 'diss' = dissolve
	case psd_blend_mode_vivid_light:		// 'vLit' = vivid light
	case psd_blend_mode_linear_light:	// 'lLit' = linear light
	case psd_blend_mode_pin_light:		// 'pLit' = pin light
	case psd_blend_mode_hard_mix:		// 'hMix' = hard mix
		// not supported;
		break;
	}
	return ltPsNormal;
}

class PSD {

protected:
	psd_context *context; //< �����p�R���e�L�X�g
	psd_status status;    //< �Ō�̏����̃X�e�[�^�X

public:
	/**
	 * �R���X�g���N�^
	 */
	PSD() : context(NULL) {};

	/**
	 * �f�X�g���N�^
	 */
	~PSD() {
		if (context) {
			psd_image_free(context);
		}
	};

	/**
	 * PSD�摜�̃��[�h
	 * @param filename �t�@�C����
	 * @return ���[�h�ɐ��������� true
	 */
	bool load(const char *filename) {
		status = psd_image_load(&context, (char*)filename);
		return status == psd_status_done;
	}

#define INTGETTER(tag) int get_ ## tag(){ return context ? context->tag : -1; }

	INTGETTER(width);
	INTGETTER(height);
	INTGETTER(channels);
	INTGETTER(depth);
	INTGETTER(color_mode);
	INTGETTER(layer_count);

protected:

	/**
	 * ���C���ԍ����K�؂��ǂ�������
	 * @param no ���C���ԍ�
	 */
	void checkLayerNo(int no) {
		if (!context) {
			TVPThrowExceptionMessage(L"no data");
		}
		if (no < 0 || no > context->layer_count) {
			TVPThrowExceptionMessage(L"not such layer");
		}
	}

	/**
	 * ���O�̎擾
	 * @param name ���O������i���j�R�[�h)
	 * @len ����
	 */
	ttstr layname(psd_layer_record *lay) {
		ttstr ret;
		if (lay->unicode_name_length > 0) {
			psd_ushort *name = lay->unicode_name;
			for (int i=0;i<lay->unicode_name_length;i++) {
				ret += (tjs_char)_byteswap_ushort(*name++);
			}
		} else {
			ret = ttstr((char*)lay->layer_name);
		}
		return ret;
	}
	
public:
	/**
	 * ���C����ʂ̎擾
	 * @param no ���C���ԍ�
	 * @return ���C�����
	 */
	int getLayerType(int no) {
		checkLayerNo(no);
		psd_layer_record *lay = context->layer_records + no;
		return lay->layer_type;
	}

	/**
	 * ���C�����̂̎擾
	 * @param no ���C���ԍ�
	 * @return ���C�����
	 */
	ttstr getLayerName(int no) {
		checkLayerNo(no);
		psd_layer_record *lay = context->layer_records + no;
		return layname(lay);
	}
	
	/**
	 * ���C�����̎擾
	 * @param no ���C���ԍ�
	 * @return ���C����񂪊i�[���ꂽ����
	 */
	tTJSVariant getLayerInfo(int no) {
		checkLayerNo(no);
		psd_layer_record *lay = context->layer_records + no;
		tTJSVariant result;	
		ncbDictionaryAccessor dict;
		if (dict.IsValid()) {
#define SETPROP(dict, obj, prop) dict.SetValue(L ## #prop, obj->prop)
			SETPROP(dict, lay, layer_type);
			SETPROP(dict, lay, top);
			SETPROP(dict, lay, left);
			SETPROP(dict, lay, bottom);
			SETPROP(dict, lay, right);
			SETPROP(dict, lay, width);
			SETPROP(dict, lay, height);
			SETPROP(dict, lay, blend_mode);
			SETPROP(dict, lay, opacity);
			SETPROP(dict, lay, visible);
			dict.SetValue(L"name", layname(lay));
			dict.SetValue(L"type", convBlendMode(lay->blend_mode));

			// additional information
			SETPROP(dict, lay, clipping);
			SETPROP(dict, lay, transparency_protected);
			SETPROP(dict, lay, obsolete);
			SETPROP(dict, lay, pixel_data_irrelevant);
			// ...struct: layer_mask_info
			// ...struct: layer_blending_ranges
			// ...struct: vector_mask
			// ...array:  layer_info_type/data (layer_info_count)
			SETPROP(dict, lay, adjustment_valid); // �������C���[���ǂ����H
			SETPROP(dict, lay, fill_opacity);
			SETPROP(dict, lay, layer_name_id);
			SETPROP(dict, lay, layer_id);
			SETPROP(dict, lay, layer_version);
			SETPROP(dict, lay, blend_clipped);
			SETPROP(dict, lay, blend_interior);
			SETPROP(dict, lay, knockout);
			SETPROP(dict, lay, transparency);
			SETPROP(dict, lay, composite);
			SETPROP(dict, lay, position_respectively);
			SETPROP(dict, lay, sheet_color);
			SETPROP(dict, lay, reference_point_x); // �h��Ԃ����C���i�p�^�[���j�̃I�t�Z�b�g
			SETPROP(dict, lay, reference_point_y); // �h��Ԃ����C���i�p�^�[���j�̃I�t�Z�b�g
			SETPROP(dict, lay, transparency_shapes_layer);
			SETPROP(dict, lay, layer_mask_hides_effects);
			SETPROP(dict, lay, vector_mask_hides_effects);
			SETPROP(dict, lay, divider_type);
			SETPROP(dict, lay, divider_blend_mode);

			// group layer �̓X�N���v�g���ł� layer_id �Q�Ƃň����悤�ɂ���
			if (lay->group_layer != NULL)
				dict.SetValue(L"group_layer_id", lay->group_layer->layer_id);

			result = dict;
		}
		return result;
	}
	
	/**
	 * ���C���f�[�^�̓ǂݏo��
	 * @param layer �ǂݏo���惌�C��
	 * @param no ���C���ԍ�
	 */
	void getLayerData(tTJSVariant layer, int no) {
		if (!layer.AsObjectNoAddRef()->IsInstanceOf(0, 0, 0, L"Layer", NULL)) {
			TVPThrowExceptionMessage(L"not layer");
		}
		checkLayerNo(no);

		psd_layer_record *lay = context->layer_records + no;
		if (lay->layer_type != psd_layer_type_normal) {
			TVPThrowExceptionMessage(L"invalid layer type");
		}

		int width  = lay->width;
		int height = lay->height;

		if (width <= 0 || height <= 0) {
			// �T�C�Y�O�̃��C���̓��[�h�ł��Ȃ�
			return;
		}

		ncbPropAccessor obj(layer);
		SETPROP(obj, lay, left);
		SETPROP(obj, lay, top);
		obj.SetValue(L"width",  width);
		obj.SetValue(L"height", height);
		obj.SetValue(L"type",   convBlendMode(lay->blend_mode));
		SETPROP(obj, lay, opacity);
		SETPROP(obj, lay, visible);
		obj.SetValue(L"imageLeft",  0);
		obj.SetValue(L"imageTop",   0);
		obj.SetValue(L"imageWidth",  width);
		obj.SetValue(L"imageHeight", height);
		obj.SetValue(L"name", layname(lay));

		// �摜�f�[�^�̃R�s�[
		psd_argb_color *src = lay->image_data;
		int srclen = width * 4;
		unsigned char *buffer = (unsigned char*)obj.GetValue(L"mainImageBufferForWrite", ncbTypedefs::Tag<tjs_int>());
		int pitch = obj.GetValue(L"mainImageBufferPitch", ncbTypedefs::Tag<tjs_int>());
		for (int y=0;y<height;y++) {
			memcpy(buffer, (unsigned char*)src, srclen);
			src    += width;
			buffer += pitch;
		}
	}

	/**
	 * �������\�[�X�̎擾
	 * @param id ������ID
	 * @return �����񃊃\�[�X
	 */
	ttstr getStringResource(int id) {
		ttstr ret;
		if (!context) TVPThrowExceptionMessage(L"no data");
		if (id < 0 || id >= context->number_of_unicode_strings)
			TVPThrowExceptionMessage(L"no such string resouce");

		psd_unicode_strings * str = context->unicode_strings + id;
		psd_ushort *name = str->name;
		if (str->name_length > 0) {
			for (int i = 0; i < str->name_length; i++) {
				ret += (tjs_char)_byteswap_ushort(*name++);
			}
		}
		return ret;
	}

	/**
	 * �X���C�X�f�[�^�̓ǂݏo��
	 * @return �X���C�X��񎫏� %[ top, left, bottom, right, slices:[ %[ id, group_id, left, top, bottom, right ], ... ] ]
	 *         �X���C�X��񂪂Ȃ��ꍇ�� void ��Ԃ�
	 */
	tTJSVariant getSlices() {
		if (!context) TVPThrowExceptionMessage(L"no data");
		tTJSVariant result;	
		ncbDictionaryAccessor dict;
		ncbArrayAccessor arr;
		if (context->fill_slices_resource) {
			if (dict.IsValid()) {
				psd_slices_resource *sr = &context->slices_resource;
				dict.SetValue(L"top",    sr->bounding_top);
				dict.SetValue(L"left",   sr->bounding_left);
				dict.SetValue(L"bottom", sr->bounding_bottom);
				dict.SetValue(L"right",  sr->bounding_right);
				dict.SetValue(L"name",   getStringResource(sr->name_string_id));
				if (arr.IsValid()) {
					psd_slices_resource_block *block = sr->slices_resource_block;
					if (block) for (int i = 0; i < sr->number_of_slices; i++) {
						ncbDictionaryAccessor tmp;
						if (tmp.IsValid()) {
#define SLICEPROP(tag) 		SETPROP(tmp, (block + i), tag)
							SLICEPROP(id);
							SLICEPROP(group_id);
							SLICEPROP(origin);
							SLICEPROP(associated_layer_id);
							SLICEPROP(type);
							SLICEPROP(left);
							SLICEPROP(top);
							SLICEPROP(right);
							SLICEPROP(bottom);
							SLICEPROP(cell_text_is_html);
							SLICEPROP(horizontal_alignment);
							SLICEPROP(veritcal_alignment);
							SLICEPROP(color);
							tmp.SetValue(L"name",      getStringResource(block[i].name_string_id));
							tmp.SetValue(L"url",       getStringResource(block[i].url_string_id));
							tmp.SetValue(L"target",    getStringResource(block[i].target_string_id));
							tmp.SetValue(L"message",   getStringResource(block[i].message_string_id));
							tmp.SetValue(L"alt_tag",   getStringResource(block[i].alt_tag_string_id));
							tmp.SetValue(L"cell_text", getStringResource(block[i].cell_text_string_id));
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
	tTJSVariant getGuides() {
		if (!context) TVPThrowExceptionMessage(L"no data");
		tTJSVariant result;	
		ncbDictionaryAccessor dict;
		ncbArrayAccessor vert, horz;
		if (context->fill_grid_and_guides_info) {
			psd_grid_guides *gg = &context->grid_guides;
			if (dict.IsValid() && vert.IsValid() && horz.IsValid()) {
				dict.SetValue(L"horz_grid",  gg->horz_grid);
				dict.SetValue(L"vert_grid",  gg->vert_grid);
				dict.SetValue(L"vertical",   vert.GetDispatch());
				dict.SetValue(L"horizontal", horz.GetDispatch());
				for (int i = 0, v = 0, h = 0; i < gg->guide_count; i++) {
					if (gg->guide_direction[i] == 0) {
						vert.SetValue(v++, gg->guide_coordinate[i]);
					} else {
						horz.SetValue(h++, gg->guide_coordinate[i]);
					}
				}
				result = dict;
			}
		}
		return result;
	}

	/**
	 * �������ʂ̎擾�B�擾�̈�͉摜�S�̃T�C�Y���ɂ����܂��Ă�K�v������܂�
	 * @param layer �i�[�惌�C��(width,height�T�C�Y�ɒ��������)
	 * @param left �������ʎ擾�̈�̍�����W
	 * @param top �������ʎ擾�̈�̍�����W
	 * @param width �擾�T�C�Y����
	 * @param height �擾�T�C�Y�c��
	 * @return �擾�ɐ��������� true
	 */
	bool getBlend(tTJSVariant layer, int left, int top, int width, int height) {
		if (!layer.AsObjectNoAddRef()->IsInstanceOf(0, 0, 0, L"Layer", NULL)) {
			TVPThrowExceptionMessage(L"not layer");
		}

		// �������ʂ𐶐�
		if (psd_image_blend(context, left, top, width, height) == psd_status_done) {

			// �i�[��𒲐�
			ncbPropAccessor obj(layer);
			obj.SetValue(L"width",  width);
			obj.SetValue(L"height", height);
			obj.SetValue(L"imageLeft",  0);
			obj.SetValue(L"imageTop",   0);
			obj.SetValue(L"imageWidth",  width);
			obj.SetValue(L"imageHeight", height);
			
			// �������ʉ摜�f�[�^�̃R�s�[
			psd_argb_color *src = context->blending_image_data + top * context->width + left;
			int len = width * 4;
			unsigned char *buffer = (unsigned char*)obj.GetValue(L"mainImageBufferForWrite", ncbTypedefs::Tag<tjs_int>());
			int pitch = obj.GetValue(L"mainImageBufferPitch", ncbTypedefs::Tag<tjs_int>());
			for (int y=0;y<height;y++) {
				memcpy(buffer, (unsigned char*)src, len);
				src    += context->width;
				buffer += pitch;
			}

			// �������ʏ��̔j��
			psd_image_blend_free(context);

			return true;
		}
		return false;
	}
};

#define ENUM(n) Variant(#n + 4, (int)n)

NCB_REGISTER_CLASS(PSD) {

	Constructor();

	ENUM(psd_color_mode_bitmap);
	ENUM(psd_color_mode_grayscale);
	ENUM(psd_color_mode_indexed);
	ENUM(psd_color_mode_rgb);
	ENUM(psd_color_mode_cmyk);
	ENUM(psd_color_mode_multichannel);
	ENUM(psd_color_mode_duotone);
	ENUM(psd_color_mode_lab);

	ENUM(psd_layer_type_normal);
	ENUM(psd_layer_type_hidden);
	ENUM(psd_layer_type_folder);
	ENUM(psd_layer_type_solid_color);
	ENUM(psd_layer_type_gradient_fill);
	ENUM(psd_layer_type_pattern_fill);
	ENUM(psd_layer_type_levels);
	ENUM(psd_layer_type_curves);
	ENUM(psd_layer_type_brightness_contrast);
	ENUM(psd_layer_type_color_balance);
	ENUM(psd_layer_type_hue_saturation);
	ENUM(psd_layer_type_selective_color);
	ENUM(psd_layer_type_threshold);
	ENUM(psd_layer_type_invert);
	ENUM(psd_layer_type_posterize);
	ENUM(psd_layer_type_channel_mixer);
	ENUM(psd_layer_type_gradient_map);
	ENUM(psd_layer_type_photo_filter);

	ENUM(psd_blend_mode_normal);
	ENUM(psd_blend_mode_dissolve);
	ENUM(psd_blend_mode_darken);
	ENUM(psd_blend_mode_multiply);
	ENUM(psd_blend_mode_color_burn);
	ENUM(psd_blend_mode_linear_burn);
	ENUM(psd_blend_mode_lighten);
	ENUM(psd_blend_mode_screen);
	ENUM(psd_blend_mode_color_dodge);
	ENUM(psd_blend_mode_linear_dodge);
	ENUM(psd_blend_mode_overlay);
	ENUM(psd_blend_mode_soft_light);
	ENUM(psd_blend_mode_hard_light);
	ENUM(psd_blend_mode_vivid_light);
	ENUM(psd_blend_mode_linear_light);
	ENUM(psd_blend_mode_pin_light);
	ENUM(psd_blend_mode_hard_mix);
	ENUM(psd_blend_mode_difference);
	ENUM(psd_blend_mode_exclusion);
	ENUM(psd_blend_mode_hue);
	ENUM(psd_blend_mode_saturation);
	ENUM(psd_blend_mode_color);
	ENUM(psd_blend_mode_luminosity);
	ENUM(psd_blend_mode_pass_through);
	
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

	NCB_METHOD(getSlices);
	NCB_METHOD(getGuides);

	NCB_METHOD(getBlend);
};
