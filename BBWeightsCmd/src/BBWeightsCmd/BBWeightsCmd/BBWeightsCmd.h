#ifndef BBWEIGHTSCMD_H
#define BBWEIGHTSCMD_H

#include "MAYA_inc.h"
#include "Array3D.h"
#include "STL_inc.h"
#include "BoxGrid.h"
#include "Weights.h"


class BBWeightsCmd : public MPxCommand
{
public:
	BBWeightsCmd();
	virtual ~BBWeightsCmd();
	MStatus doIt(const MArgList &args);
	virtual MStatus redoIt();
	virtual MStatus undoIt();
	virtual bool isUndoable() const;
	static void* creator();
	static MSyntax newSyntax();

private:
	MStatus parseArgs(const MArgList &args);

	MStatus preprocessing();

	MStatus postprocessing();
	MStatus applySkinCluster();
	MStatus applySkinWeights();

	MStatus ReadJointHeirarchy(const MFnIkJoint& _fnJoint);


	unsigned int vox_res;
	bool _isTargetJointProvided;
	bool _isTargetMeshProvided;
	size_t _numVertices, _numberOfBones, _maxInfluences;

	MFnMesh _fnTargetMesh;
	MFnIkJoint _fnTargetJoint;
	MFnSkinCluster _fnSKin;
	MDoubleArray _oldWeightValues, _newWeightValues;
	MDagPathArray _boneDagPaths;
	MDagPath _meshDagPath;
	MObject _meshComps;
	MIntArray _infIds;
	MDagModifier _dagMod;

	MPointArray voxels;
	Array3D<int> m_voxArray;
	map<string, RowVector3> B;
	map<string, string> boneWise;
	RowVector3 bmin, bmax;
	ScalarType scale;
	RowVector3 center;
	PointMatrixType vertices;
	BoxGrid * voxGrid;
	vector<Weights> weights;
};

#endif
