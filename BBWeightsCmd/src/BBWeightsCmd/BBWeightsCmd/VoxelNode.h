#pragma once

#include "headers.h"


/* ==========================================
Class VoxelSampler

voxelizethe input mesh plugged to its 'mesh' attribute
with a resolution set by 'voxelRes'.

the voxels can be retrieved from the 'outVoxels' attribute as
a point array where each pair of points describes the
min and max points of an axis-aligned voxel.

========================================== */

class VoxelNode : public MPxNode
{
public:
	VoxelNode();
	virtual				~VoxelNode();

	virtual MStatus		compute(const MPlug& plug, MDataBlock& data);

	static  void*		creator();
	static  MStatus		initialize();

public:

	// There needs to be a MObject handle declared for each attribute that
	// the node will have.  These handles are needed for getting and setting
	// the values later.
	//
	static MObject  voxelRes;
	static MObject  mesh;
	static MObject	outVoxels;

	// The typeid is a unique 32bit identifier that describes this node.
	// It is used to save and retrieve nodes of this type from the binary
	// file format.  If it is not unique, it will cause file IO problems.
	//
	static	MTypeId		id;

private:

	static bool Voxelize(const MFnMesh& inMesh, int resX, int resY, int resZ,
		MPointArray& voxels);
};
