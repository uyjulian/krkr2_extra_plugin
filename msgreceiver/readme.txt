Title: msgreceiver plugin
Author: �킽�Ȃׂ���

������͂ȂɁH

��obsoluted plugin
  �����ȏ�̋@�\������ messenger plugin �����݂��܂�
  ������̗��p���������Ă�������

�g���g���ɊO������̌���ǉ����܂�

Win32 API �� SendMessage ���g���� WM_COPYDATA ��
�e�L�X�g�𑗂邱�Ƃ��ł��܂��B

���g����

�@(1) �v���O�C�������[�h

  (2) ���b�Z�[�W���V�[�o��o�^

�@�@wmrStart(win);

�@�@�w�肵���E�C���h�E�̃E�C���h�E���b�Z�[�W�����Ɋ��荞�݂��āA
    WM_COPYDATA ������ꂽ�ꍇ�ɁAwin.onCopyData(msg) {} ��
�@�@�Ăяo�����悤�ɂȂ�܂��B
�@�@
�@�@����p�̃n���h�����́A���s�t�@�C����.hwnd �Ƃ����t�@�C����
�@�@��������Ă��ꂪ�����X�V�����̂ŁA���M����v���O������
�@�@���̃t�@�C�����Q�Ƃ��� SendMessage �p�̃n���h�����擾���Ă��������B

  (3) ���b�Z�[�W���V�[�o�̉��� 

    wmrStop(win)

    �Ŏ�M�������I�����܂��B

�@���ӓ_�F�����̑����������Ƃ͔z������Ă܂���

���T���v���̐����iruby ���琧��j

--------------------------------------------------------------------
exename = "krkr"
hwnd = open(exename + ".exe.hwnd").gets.to_i;

require 'dl/import'
require 'dl/struct'

module CopyData
	extend DL::Importable
	dlload 'user32'
	typealias "WPARAM", "UINT"
  	typealias "LPARAM", "UINT"
	WM_COPYDATA = 0x004A
	CopyData = struct [ 
		'ULONG dwData',
		'DWORD cbData',
		'PVOID lpData', 
	] 
	extern 'UINT SendMessage(HWND, UINT, WPARAM, LPARAM)'
end

msg = ARGV[0];

cd = CopyData::CopyData.malloc
cd.dwData = 0
cd.cbData = msg.size
cd.lpData = msg
CopyData::sendMessage(hwnd, CopyData::WM_COPYDATA, 0, cd.to_ptr.to_i)
--------------------------------------------------------------------

�����C�Z���X

���̃v���O�C���̃��C�Z���X�͋g���g���{�̂ɏ������Ă��������B
