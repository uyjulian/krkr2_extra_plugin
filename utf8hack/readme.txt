TITLE: �g���g��2��UTF8�ǂݍ��݋@�\��ǉ���������v���O�C��
AUTHOR: miahmie

������͂ȂɁH

�g���g���Q�͕W���ł�UTF-8�̃e�L�X�g�ǂݍ��݂ɑΉ����Ă��炸�C
�ꕔ�v���O�C���ilineParser/csvParser�j�Ȃǂ��g���ăX�N���v�g���ŏ�������
UTF-8�̃e�L�X�g��ǂݍ��ޑΉ������Ă��܂����B

���̃v���O�C�����g�p����� Array.load ���̋g���g���̕W����
�e�L�X�g�ǂݍ��ݏ����ɂ��Ă�UTF-8�e�L�X�g�̓ǂݍ��݂ɑΉ����܂��B

�������e�L�X�g�ǂݍ��݂̍����ւ��͋g���g���̃v���O�C���C���^�[�t�F�[�X�ł�
�񋟂���Ă��炸�C�������{�̂Ƀp�b�`�𓖂Ă邱�Ƃŏ�����u��������Ƃ���
�Ή��ɂȂ��Ă��܂��B

���Ȃ킿�C�g���g���Q�̃o�[�W������g�p����Ă���R���p�C���ˑ��ɂȂ邽�߁C
���̃v���O�C�����K�������g�p�ł���^���삷��Ƃ͌���܂���B

��ɋg���g���y�ւ̈ڍs��C�Q�i�J���p�j�Ƃy�i�����[�X�p�j�̕��p��
������e�L�X�g�G���R�[�h��������������Ƃ������g�p��z�肵�Ă��܂��B

�����̃v���O�C�����g�p���ăQ�[���������[�X����̂͂����߂��܂���B


���g����

k2utf8hack.dll �����̃v���O�C���{�̂ł��B
�g���qtpm�Ƀ��l�[�����ċg���g���Q�̋N���O�Ƀ����N�����邩�C
startup.tjs�Ȃǂł��̃v���O�C���������N���Ă��������B
�i���̏ꍇ�C�����N�O�̃e�L�X�g�ǂݍ��݂͏]���ʂ�̋����ɂȂ�܂��j

�EScripts.textEncoding �v���p�e�B���ǉ�����܂����C
�@�g���g��Z�Ǝ኱�d�l���قȂ�܂��F

�@Z: Scripts.{eval|exec}Storage �̕����R�[�h�ɉe�����󂯂�
�@����: ��L�ȊO�ɂ� Array.load �̕����R�[�h���e�����󂯂�

�@���v���p�e�B�̒l�ɂ��Ă� ���L -readencoding �̎w��Ɠ��l�ł��B
�@�@�w��O�̒l��ݒ肷��� ANSI �����̓���ɂȂ�܂��B
�@�@�g���g��Z�ł͔͈͊O�̒l�œǂݍ��ނƖ��T�|�[�g��O����������d�l���ɒ��ӁB

�E�R�}���h���C��/cf/cfu�̋N���I�v�V���� -readencoding �ɑΉ����܂��F
�i�l���Q�Ƃ����̂͂��̃v���O�C���������N���ꂽ���̃^�C�~���O�ł��j
�@UTF-8, Shift_JIS, ANSI, auto �̎w��ɑΉ����܂��B

�@Shift_JIS�͋g���g��Z�ƈႢ�Ccodepage 932 �w��ŕ����R�[�h��ϊ����܂��B
�@ANSI �� CP_ACP ��OS�̌���ݒ�Ɉˑ���������ɂȂ�܂��B����͋g���g��Z��
�@�@Shift_JIS ���w�肵�����̓���Ɠ������̂ɂȂ�܂��B
�@Shift_JIS/ANSI�̎w��ł����Ă�BOM��UTF8�ł���Γǂݍ��ނ��Ƃ��ł��܂��B
�@auto �͂��̃v���O�C���Ǝ��̋@�\�ŁCUTF-8/Shift_JIS�������Ŕ��肵��
�@�@�R�[�h�ϊ��������s���܂��B�i���w�莞�̃f�t�H���g����ł��j


�Ȃ��C�g���g���y�ł��̃v���O�C���������N���Ă������N����܂���B
���p�������ꍇ�͓���2��Z�ŏꍇ��������������K�v�͂Ȃ����Ǝv���܂��B


���R���p�C��

premake4�ɂăv���W�F�N�g�t�@�C�����쐬���ăR���p�C�����Ă��������B
__fastcall�̎����Ɉˑ����邽�ߕK�� VisualC++ �ɂăR���p�C�����Ă��������B

../tp_stub.* ����� ../00_simplebilder �t�H���_���̃t�@�C�����K�v�ł��B


���t�@�C���ɂ���

readme.txt		���̃t�@�C��
Main.cpp		�v���O�C�������N�^�g���g���Q�{�̃p�b�`����
nmh.hpp			�����R�[�h���� https://github.com/shnya/nmh/ �̉�����
premake4.lua		�v���W�F�N�g�����ppremake�t�@�C��
v2link.cpp		simplebinder��v2link�����Łiexporter���K�v�Ȃ��߁j

TextStream.cpp		tTVPTextReadStream�̉�������
TextStreamHack.hpp	TextStream�̃R���p�C����ʂ����߂̃O���[�N���X�^�}�N����
TextStreamHack.cpp	�����R�[�h�ϊ���

data/startup.tjs	�e�X�g�X�N���v�g�i�v bin/k2utf8hack-d.dll�j
data/test.bat		�e�X�g�X�N���v�g�N���p�o�b�`
data/test_*.tjs		�e��G���R�[�h�ς݃X�N���v�g


�����̑����ӎ���

�E�u���삵�Ă���g���g���Q�{�̂Ƀp�b�`�����Ă�v�Ƃ�������̎d�l��C
�@�E�B���X�΍�\�t�g�̃v���O�����ӂ�܂����m�@�\�Ȃǂ���F����
�@��������̌x�����o���\��������܂��̂ł����ӂ��������B
�@�i�{���Ɉ���������̂��ɂ��Ă͌���ł͖��m�F�ł��j

�E�Ǝ��R���p�C�������g���g��2�ł͓��삵�Ȃ��\��������܂��B

�Eauto����ŉ��̂�EUC�G���R�[�h�ɂ��Ή����Ă��܂�������ۏ؂���܂���B


�����C�Z���X

���̃v���O�C���̃��C�Z���X�͋g���g���Q�^�g���g���y�ɏ������Ă��������B
nmh.hpp �� https://github.com/shnya/nmh/ �̃p�u���b�N�h���C�������̃R�[�h����
���p�E�����������̂ɂȂ��Ă��܂��B


���Z�p���

�ETVPCreateTextStreamForRead�Ƀt�b�N�����Ď��O�����ɍ����ւ��邽��
�@function exporter �̃X�^�u�֐���disasm�R�[�h����A�h���X���Z�o
�����̂��ߊ�{�I�ɕW�������[�X�g���g���Q�ł݂̂������삵�Ȃ�

�E�W�������[�X�̋g���g��2�ɂ�����TVPCreateTextStreamForRead��
�@BorlandC++���__fastcall�Ăяo���Ń��W�X�^�����n���ŃR���p�C������Ă���

�E���̃v���O�C���ł�VisualC++��__fastcall�ɒu��������Ή����s�����C
�@BorlandC++��VisualC++�ł�__fastcall�̃��W�X�^�n���̎d�l���قȂ邽��
�@���̕����̋z�����s���K�v������i������ eax->ecx�փR�s�[�j

����̓I�ȏڍׂ� Main.cpp �� DirtyHook �N���X���Q��

