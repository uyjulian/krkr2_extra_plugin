Title: Magick++ Plugin
Author: Miahmie.

���y���Ӂz�ꉞ������Ԃł͂���܂����C�܂��������ł��I 
  ��ImageMagick/Magick++�̂��ׂĂ̋@�\���g�p�ł���킯�ł͂���܂���


�� �Ȃɂ��H

ImageMagick �� Magick++ �� TJS �Ŏg����悤�ɂȂ邩������܂���B
�Ƃ肠�������K�v�ȋ@�\�͂������������Ă��܂����̂ŁC
����C�`��Ȃǂ̋@�\��S���������邩�ǂ��������ȂƂ���ł��B


�� �g�p���@

MagickPP
MagickPP.�N���X�� (MagickPP.Image, MagickPP.Geometry, MagickPP.Color, etc.)
MagickPP.Enum�^.Enum�l (MagickPP.ColorspaceType.RGBColorspace, etc.)

�����肪�g������܂��B���y���Ӂz���O�͏����ύX�����\��������܂��B

���\�b�h����v���p�e�B�͂ق� Magick++ �����ł�������d�l�ɂ��
�ʖ�������U���Ă���ꍇ������܂��B

enum �l�͂����p�̌^�������Ă���킯�ł͂Ȃ��C
int�ɃL���X�g���ꂽ�l��Ԃ��܂��B


����ł͏ڂ����̓\�[�X���Ă���������Ԃł��B�i���݂܂���j


���ƁC���_�Ƃ��āC
�d���������iPSD�̓ǂݍ��ݓ��j�͊��S�ɋg���g���̔������~�܂�܂��B
�V���O���X���b�h�d�l�Ȃ̂Ō���ŉ����@�͂���܂���B

�d���������̃��b�Z�[�W��C�E�B���h�E�Ȃǂ�\���������Ƃ��́C
System.inform �ŏ����O�ɃN���b�N�҂��𑣂����C
AsyncTrigger ���g�p���āC��x���b�Z�[�W�������������Ă���
�g���K�t�@���N�V�������ŏd���������s���Ɨǂ��ł��傤�B

	// ���ꂾ�ƃR���\�[���Ƀ��b�Z�[�W���\�������O��
	// ���[�h�����Ōł܂��Ă��܂��C���b�Z�[�W�������Ȃ�
	Debug.message("���[�h���ł�");
	image.read(filename);

	// �N���b�N�҂��������Ă��ǂ��Ȃ�
	// ���[�_���_�C�A���O�Œ��ӂ𑣂�
	System.inform("���[�h���܂�" + filename);
	image.read(filename);

	// �N���b�N�҂��Ȃ��ŒP�ɕ\���������Ȃ炱��Ȋ���
	Debug.message("���[�h���ł�");
	(new AsyncTrigger(function() {
	  image.read(filename);
	  load_finished(image);
	}, "")).trigger();
	// �����������̍s����������i�K�ł͂܂��ǂݍ��݂�
	// �������ĂȂ��̂ŁC�g���K�t�@���N�V������
	// ���ׂĂ̏������L�q����K�v������
	return;


�� ������

���̃��C�u����������Ȃ̂� ncbind �������Ă��Ă�
���\�b�h�̌�����邾���ł���ρB
�܂���������Ă���@�\��S���e�X�g���Ă���킯�ł͂���܂���B

�EGeometry
�EColor
	�قڊ���
	������ϊ��� string �v���p�e�B��ǂݏ������邱�Ƃōs���B
		��F
			myColor.string = "#102030";
			Debug.message(myGeometry.string);

�EImage
	�قڋ@�\�͑����Ă邪�C�I�[�o�[���[�h����Ă��郁�\�b�h��
	�z���n���悤�ȃ��\�b�h�͎�������Ă��Ȃ��B

	�l�𕡐��n���悤�ȃv���p�e�B�̓��\�b�h�Ƃ��Ď�������Ă���
	�����F
	attribute   �� getAttribute(str), setAttribute(str, str)
	colorMap    �� getColorMap(int),  setColorMap(int, color)
	defineValue �� getDefineValue(str, str), setDefineValue(str, str, str)
	defineSet   �� getDefineValue(str, str), setDefineValue(str, str, bool)

	gamma �� double�̓ǂݏ����v���p�e�B �� setGamma(r,g,b) ���\�b�h

	fontTypeMetrics �� method �Ŏ���
			   TypeMetric imageInstance.fontTypeMetrics(str);

	fillRule �� ncbind �̕s��ŕۗ�

	chroma{Red,Blue,Green}Primary
	chromaWhitePoint
			�˒l�𕡐��Ԃ��̂ŕۗ�

	strokeDashArray		�� double * ��n���̂ŕۗ�
	convolve		�� double * ��n���̂ŕۗ�


	display() ���\�b�h�� display(layer) ���\�b�h�ɕύX
	�\���ł͂Ȃ����C���ɃC���[�W���R�s�[���鏈���ɂ��Ă���


�ECoderInfo
	�����BMagickPP.support �Ŏg�p

�E���̑�
	�C���X�^���X�̃��b�p�͑��݂��邪���\�b�h�̌��͗p�ӂ���Ă��Ȃ�
	enum �l�̓w�b�_�ɗp�ӂ���Ă�����̂͂��ׂēo�^��������
	(Magick++�Ŏg�p����Ȃ�PreviewType������)

�EMagickPP
	STL�֘A�̒P�̃��\�b�h�� enum �Ȃǂ��W�񂷂�\��
	���ׂ� static �ȃ��\�b�h/�v���p�e�B�ŁC�C���X�^���X�͐����s��

	����ł�
		version �v���p�e�B �� �o�[�W�����������Ԃ�
		support �v���p�e�B �� �T�|�[�g�����CoderInfo�̔z���Ԃ�

		readImages(string) �� Magick::readImages
					�ǂݍ��� Image �̔z���Ԃ�

	�����݂���B


�� �R���p�C��

MSYS/MinGW �ŃR���p�C�����m�F���Ă���܂��B
���炩���� static link archive �� ImageMagick ��
�R���p�C���E�C���X�g�[�����Ă�������Ԃ� make ����� OK �ł��B

make test �ɂ��e�X�g���� svn ��̃c���[�`��Ɠ������ł�
���s��O��Ƃ��Ă��܂��B�����B


�� ���C�Z���X

ImageMagick �� GPL �݊����C���Z���X�ł��B�����B
�ڂ����� http://www.imagemagick.org/script/license.php ���������������B

------------------------------------------------------------------------------
Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization dedicated to making software imaging solutions freely available.

1. Definitions.

"License" shall mean the terms and conditions for use, reproduction, and distribution as defined by Sections 1 through 9 of this document.
  
"Licensor" shall mean the copyright owner or entity authorized by the copyright owner that is granting the License.
  
"Legal Entity" shall mean the union of the acting entity and all other entities that control, are controlled by, or are under common control with that entity. For the purposes of this definition, "control" means (i) the power, direct or indirect, to cause the direction or management of such entity, whether by contract or otherwise, or (ii) ownership of fifty percent (50%) or more of the outstanding shares, or (iii) beneficial ownership of such entity.
  
"You" (or "Your") shall mean an individual or Legal Entity exercising permissions granted by this License.
  
"Source" form shall mean the preferred form for making modifications, including but not limited to software source code, documentation source, and configuration files.
  
"Object" form shall mean any form resulting from mechanical transformation or translation of a Source form, including but not limited to compiled object code, generated documentation, and conversions to other media types.
  
"Work" shall mean the work of authorship, whether in Source or Object form, made available under the License, as indicated by a copyright notice that is included in or attached to the work (an example is provided in the Appendix below).
  
"Derivative Works" shall mean any work, whether in Source or Object form, that is based on (or derived from) the Work and for which the editorial revisions, annotations, elaborations, or other modifications represent, as a whole, an original work of authorship. For the purposes of this License, Derivative Works shall not include works that remain separable from, or merely link (or bind by name) to the interfaces of, the Work and Derivative Works thereof.
  
"Contribution" shall mean any work of authorship, including the original version of the Work and any modifications or additions to that Work or Derivative Works thereof, that is intentionally submitted to Licensor for inclusion in the Work by the copyright owner or by an individual or Legal Entity authorized to submit on behalf of the copyright owner. For the purposes of this definition, "submitted" means any form of electronic, verbal, or written communication intentionally sent to the Licensor by its copyright holder or its representatives, including but not limited to communication on electronic mailing lists, source code control systems, and issue tracking systems that are managed by, or on behalf of, the Licensor for the purpose of discussing and improving the Work, but excluding communication that is conspicuously marked or otherwise designated in writing by the copyright owner as "Not a Contribution."
  
"Contributor" shall mean Licensor and any individual or Legal Entity on behalf of whom a Contribution has been received by Licensor and subsequently incorporated within the Work.

2. Grant of Copyright License. Subject to the terms and conditions of this License, each Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable copyright license to reproduce, prepare Derivative Works of, publicly display, publicly perform, sublicense, and distribute the Work and such Derivative Works in Source or Object form.

3. Grant of Patent License. Subject to the terms and conditions of this License, each Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable patent license to make, have made, use, offer to sell, sell, import, and otherwise transfer the Work, where such license applies only to those patent claims licensable by such Contributor that are necessarily infringed by their Contribution(s) alone or by combination of their Contribution(s) with the Work to which such Contribution(s) was submitted.

4. Redistribution. You may reproduce and distribute copies of the Work or Derivative Works thereof in any medium, with or without modifications, and in Source or Object form, provided that You meet the following conditions:

     a. You must give any other recipients of the Work or Derivative Works a copy of this License; and

     b. You must cause any modified files to carry prominent notices stating that You changed the files; and

     c. You must retain, in the Source form of any Derivative Works that You distribute, all copyright, patent, trademark, and attribution notices from the Source form of the Work, excluding those notices that do not pertain to any part of the Derivative Works; and

     d. If the Work includes a "NOTICE" text file as part of its distribution, then any Derivative Works that You distribute must include a readable copy of the attribution notices contained within such NOTICE file, excluding those notices that do not pertain to any part of the Derivative Works, in at least one of the following places: within a NOTICE text file distributed as part of the Derivative Works; within the Source form or documentation, if provided along with the Derivative Works; or, within a display generated by the Derivative Works, if and wherever such third-party notices normally appear. The contents of the NOTICE file are for informational purposes only and do not modify the License. You may add Your own attribution notices within Derivative Works that You distribute, alongside or as an addendum to the NOTICE text from the Work, provided that such additional attribution notices cannot be construed as modifying the License.

You may add Your own copyright statement to Your modifications and may provide additional or different license terms and conditions for use, reproduction, or distribution of Your modifications, or for any such Derivative Works as a whole, provided Your use, reproduction, and distribution of the Work otherwise complies with the conditions stated in this License.

5. Submission of Contributions. Unless You explicitly state otherwise, any Contribution intentionally submitted for inclusion in the Work by You to the Licensor shall be under the terms and conditions of this License, without any additional terms or conditions. Notwithstanding the above, nothing herein shall supersede or modify the terms of any separate license agreement you may have executed with Licensor regarding such Contributions.

6. Trademarks. This License does not grant permission to use the trade names, trademarks, service marks, or product names of the Licensor, except as required for reasonable and customary use in describing the origin of the Work and reproducing the content of the NOTICE file.

7. Disclaimer of Warranty. Unless required by applicable law or agreed to in writing, Licensor provides the Work (and each Contributor provides its Contributions) on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, including, without limitation, any warranties or conditions of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A PARTICULAR PURPOSE. You are solely responsible for determining the appropriateness of using or redistributing the Work and assume any risks associated with Your exercise of permissions under this License.

8. Limitation of Liability. In no event and under no legal theory, whether in tort (including negligence), contract, or otherwise, unless required by applicable law (such as deliberate and grossly negligent acts) or agreed to in writing, shall any Contributor be liable to You for damages, including any direct, indirect, special, incidental, or consequential damages of any character arising as a result of this License or out of the use or inability to use the Work (including but not limited to damages for loss of goodwill, work stoppage, computer failure or malfunction, or any and all other commercial damages or losses), even if such Contributor has been advised of the possibility of such damages.

9. Accepting Warranty or Additional Liability. While redistributing the Work or Derivative Works thereof, You may choose to offer, and charge a fee for, acceptance of support, warranty, indemnity, or other liability obligations and/or rights consistent with this License. However, in accepting such obligations, You may act only on Your own behalf and on Your sole responsibility, not on behalf of any other Contributor, and only if You agree to indemnify, defend, and hold each Contributor harmless for any liability incurred by, or claims asserted against, such Contributor by reason of your accepting any such warranty or additional liability.
------------------------------------------------------------------------------
