Title: layerExSave plugin
Author: �킽�Ȃׂ���

������͂ȂɁH

Layer/Window �N���X��TGL5/PNG �`���ł̕ۑ����\�b�h��ǉ�����v���O�C���ł��B

���g���g��Z�ł͕W����TLG5/TLG6/PNG/JPEG�̕ۑ��@�\������̂�
�@���̃v���O�C���ł͂Ȃ��A������𗘗p���邱�Ƃ𐄏����܂��B
���ڍׂ�Layer.saveLayerImage/Bitmap.save�̃}�j���A�����Q�Ƃ��Ă�������

��PNG�ۑ��̐���

PNG�ۑ���libpng���g�p�����C���L���\�b�h���Ƃɕۑ��̎������قȂ�܂��B

	Window.startSaveLayerImage   : �Ǝ������ɂ��ۑ�
	Layer.saveLayerImagePng      : LodePNG�ɂ��ۑ�
	Layer.saveLayerImagePngOctet : �V

�Ǝ������̓v���O���X�����̓s���ɂ��ȈՏ����̂��߁A
�t�H�[�}�b�g�ɂ́A���L�̐���������܂��B

�E32bitRGBA�Œ�i�����x����j
�E�t�B���^/�C���^�[���[�X�����Ȃ�

�t�B���^�������Ȃ��̂ŁA���k���� libpng/LodePNG �̎����̂��̂����܂��B
�����k�����d�v�ɂȂ�悤�ȉ摜�́ALodePNG�ł̕ۑ��𐄏����܂��B

LodePNG ( http://lodev.org/lodepng/ )�̓|�[�^�u����PNG�̃��[�h/�Z�[�u�̎����ł��B
./LodePNG/* ��2�t�@�C�����Y�����܂��B(version 20161127���g�p)


�^�O���ioffs_*, reso_*, vpag_*�j���T�|�[�g����܂�������m�F���s�\���ł��B

�^�O��񎫏��� comp_lv ��n���ƈ��k����ύX�ł��܂��B
�i0�`9�܂ŁF���̏���PNG�̃`�����N�Ƃ��Ă͕ۑ�����܂���j
��LodePNG����comp_lv���w�肷���zlib��deflate�������g�p���܂��B
�@���w��̏ꍇ��LodePNG�g�ݍ��݂�deflate�������g�p���܂��B


���g����

manual.tjs �Q��


���R���p�C��

premake4�ɂăv���W�F�N�g���쐬���Ă��������B(vs20xx�t�H���_�쐬�ς݁j
�R���p�C���ɂ�
../tp_stub.*
../ncbind/*
../zlib/*
../../../tools/win32/krdevui/tpc/tlg5/slide.*
�̃t�H���_�E�t�@�C�����K�v�ł��B

LAYEREXSAVE_DISABLE_LODEPNG ���w�肵�ăR���p�C�������LodePNG���g�p����
���ׂēƎ������ɂ��ۑ������ƂȂ�܂��B�i���łƓ����d�l�ł��j


�����C�Z���X

���̃v���O�C���̃��C�Z���X�͋g���g���{�̂ɏ������Ă��������B


LodePNG �̃��C�Z���X�� zlib license �ɂȂ�܂��B

LodePNG version 20151208

Copyright (c) 2005-2015 Lode Vandevenne

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.


