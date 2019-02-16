/**
 * �g���g���N���X�̎擾
 * @param className �g���g���N���X���w��(������)
 * @param ... �p�����Ă���e�N���X���
 * @return �R���X�g���N�^���\�b�h
 *
 * �g���g���̃N���X�� Javascript �N���X�Ƃ��Ď擾���܂��B
 *
 * ���g���g�����Őe�N���X�����Q�Ɛ����ł��Ȃ����߁A
 * �e�N���X���p�����Ă���N���X�̖��O�����ׂĎ蓮�ŗ񋓂���K�v������܂��B
 */
function createTJSClass(className, ...);

/**
 * �g���g���N���X�� javascript �ɂ�������N���X�\��
 * ���̃C���X�^���X�ɑ΂���g���g��������̃����o�Q�Ƃ́A���̋g���g���C���X�^���X�̂��ꂪ�Ă΂�܂����A
 * ���݂��ĂȂ������o�̏ꍇ�́Amissing �@�\�ɂ�� javascript ���I�u�W�F�N�g�̓��������o���Q�Ƃ���܂��B
 * �g���g������Ă΂����̂����� javascript�C���X�^���X�̂���ɂɍ����ւ���ꍇ��
 * tjsOverride() �ŋ����㏑���������邱�Ƃ��ł��܂��B�C�x���g�̓o�^�Ɏg���܂��B
 */
function TJSObject();

TJSObject.prototype = {
	
  // �S���\�b�h/�v���p�e�B���v���g�^�C�v�Ƃ��ēo�^�ςݏ��

  /**
   * �g���g���I�u�W�F�N�g�̗L�����̊m�F
   * ���C���Ȃǋg���g�����ŋ��� invalidate �����\��������I�u�W�F�N�g�̏󋵊m�F�Ɏg���܂��B
   * @return valid �Ȃ� true
   */
 tjsIsValid : function(),

  /**
   * �g���g���I�u�W�F�N�g�̋����I�[�o���C�h����
   * �g���g���C���X�^���X�̃����o�������I�ɏ㏑�����܂��B
   * �C�x���g�Ȃǂ� javascript ���ł��������ꍇ�Ɏw�肵�܂�
   * �l���ȗ������ꍇ�͎��ȃI�u�W�F�N�g���Q�Ƃ��܂�
   * @param name �����o��
   * @param value �o�^����l(�ȗ���)
   */
  tjsOverride : function(name, value=null),
}

// -----------------------------------------------------------
// �p���L�q��
// -----------------------------------------------------------

// �Ǝ����C���N���X
function MyObject(arg)
{
	// �e�R���X�g���N�^�Ăяo��
	TJSObject.call(this, arg);
	// �e�평�����Ȃ�
	this.XXX();
}

// �Ǝ����C���̃v���g�^�C�v
MyObject.prototype = {
  __proto__: TJSObject.prototype // �e�I�u�W�F�N�g�̃v���g�^�C�v���w��
  ...  // �ȉ����O�̃��\�b�h�ǉ�
};
