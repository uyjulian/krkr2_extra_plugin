Title: messenger Plugin
Author: �킽�Ȃׂ���

������͂ȂɁH

����}�V����ŋN�����Ă���g���g���Ԃł̑��ݒʐM�@�\��񋟂��܂��B
window message ���o�R���ē��񃁃b�Z�[�W�𑗐M���܂��B
 
���g�p���@

manual.tjs �Q��

���O���A�v���P�[�V��������̐����

storeHWND �� "hwnd" ���w�肵�āA�O���A�v������ HWND �����m���A
���̃E�C���h�E�ɑ΂��� WM_COPYDATA ��ʒm���܂��B

ruby �ł̐����
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

���C�Z���X�͋g���g���{�̂ɏ������Ă��������B
