Title: addFont plugin
Author: �킽�Ȃׂ���

������͂ȂɁH

�g���g���Ńv���C�x�[�g�ȃt�H���g�t�@�C��(.ttf/.otf)��
�������߂̃v���O�C���ł��B

�����
  Win2000 �ȍ~
  9X �n�ł͗��p�ł��܂���

���g����

/**
 * @param fontfilename �t�H���g�t�@�C����
 * @param extract �e���|�����W�J���邩�ǂ���
 * @return void:�t�@�C�����J���̂Ɏ��s 
 *  0:�t�H���g�o�^�Ɏ��s ���l:�o�^�����t�H���g�̐�
 */
System.addFont(fontfilename, extract);

�A�[�J�C�u�O�ɂ���t�H���g�̏ꍇ:
 ���̂܂܃V�X�e���t�H���g�Ɠ��l�ɗ��p���邱�Ƃ��ł��܂��B

�A�[�J�C�u���ɂ���t�H���g�̏ꍇ:

 extract == true �̏ꍇ
  ��������e���|�����̈�Ƀt�H���g�t�@�C����W�J���܂��B
 ���̂܂܃V�X�e���t�H���g�Ɠ��l�ɗ��p���邱�Ƃ��ł��܂��B

 extract == false �̏ꍇ
  �t�H���g�̓������W�J����܂��B
�@���̎��A�t�H���g�͗񋓑ΏۂɂȂ�܂���B
  ���̂��߁AFont.doUserSelect �ŎQ�Ƃł��Ȃ����Afont.face ��
  ���O���w�肵�Ă��t�H���g���Q�Ƃ��邱�Ƃ��ł��܂���B

  �ŐV�J���n�g���g���ł́Afont.face = ",DF�S�V�b�N"; 
  �ƁA�J���}���ŏ��ɂ���邱�Ƃŋ����I�Ƀt�H���g�Q��
  ������Ƃ��ł��܂��B

�����C�Z���X

���̃v���O�C���̃��C�Z���X�͋g���g���{�̂ɏ������Ă��������B
