Title: layerExBTOA
Author: �킽�Ȃׂ���

������͂ȂɁH

���C���̃��̈�� Province�摜�������郁�\�b�h���W�߂����̂ł�

���g����

�e���\�b�h�ɂ��Ă� manual.tjs �Q��

������p�Ɏg���ꍇ�́AVideoOverlay �N���X���g���ă��C���ɓ���(�E�����Ƀ��摜)
��`�悵�����ƁAonFrameUpdate() �� copyRightBlueToLeftAlpha() ��
�Ăяo���Ă��������B�����͓����摜�f�[�^(imageWidth�̃T�C�Y)�ɑ΂��čs���܂��B

�`��惌�C���� width �� VideoOverlay �N���X�ɂ���ē���̃T�C�Y��
�g������Ă�̂ŁA���̃^�C�~���O�Ŕ����ɍĒ������Ă��������B

��P
class AlphaVideo extends VideoOverlay
{
  function AlphaVideo(window) {
    super.VideoOverlay(window);
    mode = vomLayer;
  }

  function onFrameUpdate(frame) {
    if (layer1) {
      layer1.width = layer1.imageWidth / 2;
      layer1.copyRightBlueToLeftAlpha();
    }
  }
}

��Q
Movie.tjs ���A���t�@���[�r�[�Ή��ɉ��������T���v���ł��B
�ύX�_�� Movie.patch ���Q�Ƃ��Ă��������B

video �^�O�� alphatype �̃I�v�V�������g������܂��B

alphatype=0 �� �A���t�@���g�p���Ȃ��ʏ�̃��[�h
alphatype=1 �� copyRightBlueToLeftAlpha ���g�p�i�E���ɃA���t�@�j
alphatype=2 �� copyBottomBlueToTopAlpha ���g�p�i�����ɃA���t�@�j

�A���t�@���[�r�[�� mode=layer �ł����@�\���Ȃ����Ƃɂ����ӂ��������B
�܂��Aimage �^�O�� mode �����ŁA���炩���ߑΏۂ̃��C���̓��߃��[�h��
�ύX���Ă����Ȃ��ƁA�������A���t�@���o�Ȃ��ꍇ������܂��B


�����C�Z���X

���̃v���O�C���̃��C�Z���X�͋g���g���{�̂ɏ������Ă��������B
