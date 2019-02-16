// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_LWO_MESH_FILE_LOADER_H_INCLUDED__
#define __C_LWO_MESH_FILE_LOADER_H_INCLUDED__

#include "IMeshLoader.h"
#include "SMeshBuffer.h"
#include "irrString.h"
#include "irrMap.h"
#include "irrList.h"

namespace irr
{
namespace io
{
	class IReadFile;
	class IFileSystem;
} // end namespace io
namespace scene
{

	struct SMesh;
	class ISceneManager;

//! Meshloader capable of loading Lightwave 3D meshes.
class CLWOMeshFileLoader : public IMeshLoader
{
public:

	//! Constructor
	CLWOMeshFileLoader(scene::ISceneManager* smgr, io::IFileSystem* fs);

	//! destructor
	virtual ~CLWOMeshFileLoader();

	//! returns true if the file maybe is able to be loaded by this class
	//! based on the file extension (e.g. ".bsp")
	virtual bool isALoadableFileExtension(const c8* fileName) const;

	//! creates/loads an animated mesh from the file.
	//! \return Pointer to the created mesh. Returns 0 if loading failed.
	//! If you no longer need the mesh, you should call IAnimatedMesh::drop().
	//! See IUnknown::drop() for more information.
	virtual IAnimatedMesh* createMesh(io::IReadFile* file);

private:

	struct tLWOMaterial;

	bool readFileHeader();
	bool readChunks();
	void readObj1(u32 size);
	void readTagMapping(u32 size);
	void readVertexMapping(u32 size);
	void readDiscontinuousVertexMapping(u32 size);
	void readObj2(u32 size);
	void readMat(u32 size);
	u32 readString(core::stringc& name, u32 size=0);
	u32 readVec(core::vector3df& vec);
	u32 readVX(u32& num);
	u32 readColor(video::SColor& color);
	video::ITexture* loadTexture(const core::stringc& file);

  core::stringc getVMapNameByLayerId(u32 layerId, core::stringc& name);

	scene::ISceneManager* SceneManager;
	io::IFileSystem* FileSystem;
	io::IReadFile* File;
	SMesh* Mesh;

// ����: 
 /*
 �EPolyMapping �� �|��ID���烌�C��ID���擾
 �E���C��ID����A���̃��C����UV�ꗗ���擾
 �E���݂̃|���̃}�e���A���̃e�N�X�`����VMapName �ł��̈ꗗ������
 �E�q�b�g���� UV �� Tcoords �Ƃ��ĎQ�Ƃ���
 */
	core::array<core::array<core::vector3df> > Points;
	core::array<core::array<u32> > Indices;   // �|��ID>Point�C���f�N�X��
  core::array<u16> PolyMapping;             // �|��ID>���C��ID�}�b�v 
	core::array<u16> MaterialMapping;         // �|��ID>�}�e���A��ID(tag)�}�b�v
  // map("���C��ID_" + VMAP-name, array(TXUV))
  core::map<core::stringc, core::array<core::vector2df>* > VMAPMap;
	// map("���C��ID_" + VMAP-name, map((polyID<<16 | vtxId), TXUV))
	struct VMADId {
		u32 vtxId;
		u32 polyId;
		VMADId() : vtxId(0), polyId(0) {}
		VMADId(u32 p, u32 v) : vtxId(v), polyId(p) {}
		bool operator == (const VMADId& other) const { 
			return (vtxId==other.vtxId && polyId==other.polyId);
		}
		bool operator < (const VMADId& other) const { 
			if (polyId<other.polyId)
				return true;
			else if(polyId>other.polyId)
				return false;
			else
				return (vtxId<other.vtxId);
		}
	};
	core::map<core::stringc, core::map<VMADId, core::vector2df>* > VMADMap;
	core::array<tLWOMaterial*> Materials;
	core::array<core::stringc> Images;
	u8 FormatVersion;

  // �p�[�T�̏�ԕϐ�
  u16 CurrentLayerId;
  u32 CurrentPolyIndexOffset;  // ���݂̃��C���̃|��ID�I�t�Z�b�g
  u32 CurrentPolyNum;          // ���݂̃��C���̃|����
};

} // end namespace scene
} // end namespace irr

#endif
