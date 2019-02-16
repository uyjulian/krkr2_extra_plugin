#include "stdafx.h"

#include "psddata.h"
#include "psdresource.h"
#include "psddesc.h"

namespace psd {
  // �X���C�X�������[�h
  bool loadResourceSlice(Data &data, ImageResourceInfo &res)
  {
    int version = res.data->getInt32();
    if (version == 6) {
      data.slice.boundingLeft   = res.data->getInt32();
      data.slice.boundingTop    = res.data->getInt32();
      data.slice.boundingRight  = res.data->getInt32();
      data.slice.boundingBottom = res.data->getInt32();
      res.data->getUnicodeString(data.slice.groupName);
      int sliceNum = res.data->getInt32();
      for (int i = 0; i < sliceNum; i++) {
        data.slice.slices.push_back(SliceItem());
        SliceItem &item = data.slice.slices.back();

        item.id = res.data->getInt32();
        item.groupId = res.data->getInt32();
        item.origin = res.data->getInt32();
        if (item.origin == 1) {
          item.associatedLayerId = res.data->getInt32();
        } else {
          item.associatedLayerId = -1; // �����O�ł����͂������ǃh�L�������g�Ȃ��̂Łc
        }
        res.data->getUnicodeString(item.name);
        item.type   = res.data->getInt32();
        item.left   = res.data->getInt32();
        item.top    = res.data->getInt32();
        item.right  = res.data->getInt32();
        item.bottom = res.data->getInt32();
        res.data->getUnicodeString(item.url);
        res.data->getUnicodeString(item.target);
        res.data->getUnicodeString(item.message);
        res.data->getUnicodeString(item.altTag);
        item.isCellTextHtml = (res.data->getCh() != 0);
        res.data->getUnicodeString(item.cellText);
        item.horizontalAlign = res.data->getInt32();
        item.verticalAlign   = res.data->getInt32();
        item.colorA = res.data->getCh();
        item.colorR = res.data->getCh();
        item.colorG = res.data->getCh();
        item.colorB = res.data->getCh();
      }
      data.slice.isEnabled = true;

      // additional descriptor resource
      Descriptor dsc;
      if (!res.data->eoi() &&
          16 == res.data->getInt32() &&
          dsc.load(res.data)) {
        dsc.dump();
        // TODO ��œǂݎ�������̂Ɠ������̂����̂܂�Descriptor�œ����Ă���H
      }
    } else if (version == 7 || version == 8) {
      // v7/v8 �̓f�B�X�N���v�^�`���Ŋi�[����Ă���
      int ver = res.data->getInt32();
      if (ver == 16) {
        Descriptor dsc;
        if (dsc.load(res.data)) {
          dsc.dump();
          // TODO data.slice�ւ̕ϊ��i�[
        }
      }
    }

    return true;
  }

  // �O���b�h/�K�C�h�������[�h
  bool loadResourceGridAndGuide(Data &data, ImageResourceInfo &res)
  {
    // �ǂݎ̂�
    int version = res.data->getInt32();

    data.gridGuide.horizontalGrid = res.data->getInt32();
    data.gridGuide.verticalGrid   = res.data->getInt32();
    int guideNum = res.data->getInt32();
    for (int i = 0; i < guideNum; i++) {
      data.gridGuide.guides.push_back(GuideItem());
      GuideItem &item = data.gridGuide.guides.back();

      item.location  = res.data->getInt32();
      item.direction = (GuideDirection)res.data->getCh();
    }
    data.gridGuide.isEnabled = true;

    return true;
  }

  // �C���f�b�N�X�J���[�e�[�u���J�E���g
  bool loadResourceColorTableCount(Data &data, ImageResourceInfo &res)
  {
    data.colorTable.validCount = res.data->getInt16();
    return true;
  }

  // �����C���f�b�N�X
  bool loadResourceTransparencyIndex(Data &data, ImageResourceInfo &res)
  {
    data.colorTable.transparencyIndex = res.data->getInt16();
    return true;
  }

  // ���C���[�J���v
  bool loadResourceLayerComps(Data &data, ImageResourceInfo &res)
  {
    int ver = res.data->getInt32();
    if (ver == 16) {
      Descriptor dsc;
      if (dsc.load(res.data)) {
        // dsc.dump();
        DescriptorInteger *lastApplied = dsc.item("lastAppliedComp");
        data.lastAppliedCompId = lastApplied ? lastApplied->val : -1;

        DescriptorList *compList = dsc.item("list");
        if (compList) {
          for (int i = 0; i < (int)compList->itemCount(); i++) {
            Descriptor *comp = compList->item(i);
            if (comp) {
              DescriptorInteger *compId = comp->item("compID");
              if (compId) {
                LayerComp lc;
                lc.id = compId->val;

                DescriptorInteger *capturedInfo = comp->item("capturedInfo");
                if (capturedInfo) {
                  lc.isRecordVisibility = ((capturedInfo->val & (1 << 0)) != 0);
                  lc.isRecordPosition   = ((capturedInfo->val & (1 << 1)) != 0);
                  lc.isRecordAppearance = ((capturedInfo->val & (1 << 2)) != 0);
                } else {
                  lc.isRecordVisibility = false;
                  lc.isRecordPosition   = false;
                  lc.isRecordAppearance = false;
                }

                DescriptorString *name = comp->item("Nm  ");
                if (name) {
                  lc.name = name->val;
                }

                DescriptorString *comment = comp->item("comment");
                if (comment) {
                  lc.comment = comment->val;
                }

                data.layerComps.push_back(lc);
              }
            }
          }
        }
        return true;
      }
    }
    // assert(false);
    return false;
  }

} // namespace psd

