Title: ScriptsEx Plugin
Author: �킽�Ȃׂ���/�����T��

������͂ȂɁH

iTJSDispatch2 �̋@�\������邽�߂̗����v���O�C���ł�

���g�p���@

manual.tjs �Q��


��Scripts.propSet/Get�ɂ���

iTJSDispatch2��PropSet/Get�𐶂ŌĂԊ֐��ƂȂ�܂��B
�t���O�w��ɒ��ӂ��Ă��������B(propSet����pfMemberEnsure�̎w��Y��Ȃ�)

�g�p��F
Plugins.link("ScriptsEx.dll");
with (Scripts) {
	.propSet(Dictionary, /*member*/"testMember", /*value*/1,
		.pfStaticMember|.pfMemberEnsure); // static�w��
}
var test = new Dictionary();
Debug.message(typeof test.testMember); // -> "undefined"
Debug.message(typeof Dictionary.testMember); // -> "Integer"



�����C�Z���X

���C�Z���X�͋g���g���{�̂ɏ������Ă��������B
