#include "BBWeightsCmd.h"
#include "solver.hpp"
#include "utils_maya.hpp"

static const char *kVoxResolution = "-res";
static const char *kVoxResolutionLong = "-voxResolution";

static const char *kTargetMesh = "-tm";
static const char *kTargetMeshLong = "-targetMesh";

static const char *kTargetBone = "-tb";
static const char *kTargetBoneong = "-targetBone";


BBWeightsCmd::BBWeightsCmd()
{
	_isTargetJointProvided = false;
	_isTargetMeshProvided = false;
}

BBWeightsCmd::~BBWeightsCmd()
{
	_boneDagPaths.clear();
}

MSyntax BBWeightsCmd::newSyntax()
{
	MSyntax syntax;
	syntax.addFlag(kVoxResolution, kVoxResolutionLong, MSyntax::kLong);
	syntax.addFlag(kTargetMesh, kTargetMeshLong, MSyntax::kString);
	syntax.addFlag(kTargetBone, kTargetBoneong, MSyntax::kString);

	syntax.enableQuery(false);
	syntax.enableEdit(false);

	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList, 1, 2);
	return syntax;
}

void* BBWeightsCmd::creator()
{
	return new BBWeightsCmd;
}

bool BBWeightsCmd::isUndoable() const
{
	return true;
}

MStatus BBWeightsCmd::parseArgs(const MArgList &args)
{
	MStatus stat;
	MArgDatabase argData(syntax(), args, &stat);
	// Get object from arguments or selection
	MSelectionList selList;
	//MGlobal::getActiveSelectionList(selList);
	//argData.getObjects(selList);
	//if (selList.isEmpty()) return MS::kFailure;

	// Get object's dagPath then store for later use
	MDagPath dagPath;
	//selList.getDagPath(0, dagPath);

	//if (!dagPath.hasFn(MFn::kMesh)) {
		//MGlobal::displayError(dagPath.partialPathName() + " isn't mesh type.");
		//return MS::kFailure;
	//}

	if (argData.isFlagSet(kTargetMesh)) {
		MString targetMesh;
		stat = argData.getFlagArgument(kTargetMesh, 0, targetMesh);

		selList.clear();
		stat = selList.add(targetMesh);
		if (MFAIL(stat)) {
			MGlobal::displayError(targetMesh + " doesn't exist.");
			return MS::kFailure;
		}

		selList.getDagPath(0, dagPath);
		if (!dagPath.hasFn(MFn::kMesh)) {
			MGlobal::displayError(dagPath.partialPathName() + " isn't mesh type.");
			return MS::kFailure;
		}
		//dagPath.extendToShape();
		//dagPath.child(i, &stat);
		//dagPath.childCount();
		//extendToShapeDirectlyBelow(i);
		_fnTargetMesh.setObject(dagPath);
		_numVertices = _fnTargetMesh.numVertices();
		_isTargetMeshProvided = true;
		MString n_str;
		n_str.set(_numVertices);
		MGlobal::displayInfo("vertices: " + n_str + " " + dagPath.partialPathName());
	}
	if (argData.isFlagSet(kTargetBone)) {
		MString targetBone;
		stat = argData.getFlagArgument(kTargetBone, 0, targetBone);

		selList.clear();
		stat = selList.add(targetBone);
		if (MFAIL(stat)) {
			MGlobal::displayError(targetBone + " doesn't exist.");
			return MS::kFailure;
		}

		selList.getDagPath(0, dagPath);
		if (!dagPath.hasFn(MFn::kJoint)) {
			MGlobal::displayError(dagPath.partialPathName() + " isn't joint type.");
			return MS::kFailure;
		}
		_fnTargetJoint.setObject(dagPath);
		_isTargetJointProvided = true;
	}
	if (argData.isFlagSet(kVoxResolution))
	{
		stat = argData.getFlagArgument(kVoxResolution, 0, vox_res);
	}
	return stat;
}

MStatus BBWeightsCmd::doIt(const MArgList &args)
{
	MStatus stat;
	MTimer timer; timer.beginTimer();
	stat = parseArgs(args);
	if (MFAIL(stat)) return stat;

	preprocessing();

	// solve
	compute(*voxGrid, B, boneWise);
	for (int i = 0; i < vertices.size(); i++)
	{
		voxGrid->getInterpolatedBBW(vertices.row(i), weights[i], _numberOfBones);
	}

	postprocessing();

	timer.endTimer();

	printf("BBW Solver: Time Consuming: %fs\n", timer.elapsedTime());
	cout << "BBW Solver - All Right Reserve by Zhiping Luo <luozhipi@gmail.com>" << endl;
	return stat;
}

MStatus BBWeightsCmd::preprocessing()
{
	MStatus stat;
	/*mesh*/
	vertices.resize(_numVertices, 3);
	UnitPacking(_fnTargetMesh, vertices, bmin, bmax, scale, center);
	weights.resize(_numVertices);
	/*voxelize*/
	m_voxArray.init(vox_res, vox_res, vox_res);
	m_voxArray.setAllTo(-1);
	Voxelize(_fnTargetMesh, vox_res, vox_res, vox_res, voxels, m_voxArray);
	/*skeleton*/
	ReadJointHeirarchy(_fnTargetJoint);
	_numberOfBones = B.size();
	if (voxGrid != 0){ delete voxGrid; voxGrid = 0; }
	voxGrid = new BoxGrid();
	voxGrid->initVoxels(vox_res, m_voxArray);
	voxGrid->initStructure();
	cout << "BBW Solver: Initialization Done." << endl;
	return stat;
}

MStatus BBWeightsCmd::postprocessing()
{
	MStatus stat;
	return stat;
}
MStatus BBWeightsCmd::ReadJointHeirarchy(const MFnIkJoint& _fnJoint)
{
	MStatus stat;
	unsigned int nChild = _fnJoint.childCount(&stat);
	MVector pose = _fnJoint.translation(MSpace::kTransform, &stat);
	RowVector3 _pose = scale*(RowVector3(pose.x, pose.y, pose.z) - center) + RowVector3(0.5, 0.5, 0.5);
	B[_fnJoint.partialPathName().asChar()] = _pose;
	for (unsigned int i = 0; i < nChild; i++)
	{
		MFnIkJoint _fnChildJoint = _fnJoint.child(i, &stat);
		MVector jPose = _fnChildJoint.translation(MSpace::kTransform, &stat);
		boneWise[_fnJoint.partialPathName().asChar()] = _fnChildJoint.partialPathName().asChar();
		ReadJointHeirarchy(_fnChildJoint);
	}
	return stat;
}
MStatus BBWeightsCmd::applySkinCluster()
{
	MStatus stat;

	// Construct commands
	MString meshName = _fnTargetMesh.partialPathName();
	stringstream cmdStr;
	cmdStr << "skinCluster -tsb -nw 1 ";
	for (unsigned int i = 0; i < _numberOfBones; ++i)
		cmdStr << _boneDagPaths[i].partialPathName().asChar() << " ";

	cmdStr << " " << meshName.asChar() << ";";

	// Execute commands
	_dagMod.commandToExecute(cmdStr.str().c_str());
	_dagMod.doIt();

	MObject skinNode = getSkinNode(_fnTargetMesh.dagPath(), &stat);
	_fnTargetMesh.setObject(skinNode);

	return stat;
}

MStatus BBWeightsCmd::applySkinWeights()
{
	MStatus stat;

	MSelectionList compList;
	MDagPathArray infObjs;

	// Get component list
	unsigned int count = _fnSKin.influenceObjects(infObjs);
	for (unsigned int i = 0; i < count; ++i) {
		_infIds.append(i);
		MSelectionList selList;
		MDoubleArray tmp;
		stat = _fnSKin.getPointsAffectedByInfluence(infObjs[i], selList, tmp);
		stat = compList.merge(selList);
	}

	// Get component object
	stat = compList.getDagPath(0, _meshDagPath, _meshComps);

	// Store old weights
	stat = _fnSKin.getWeights(_meshDagPath, _meshComps, _infIds, _oldWeightValues);

	// Copy weights from eigen matrix to MDoubleArray
	//W.transposeInPlace();
	//double *weightArray = new double[W.size()];
	//Map<MatrixXd>(weightArray, W.rows(), W.cols()) = W;
	//MDoubleArray weights(weightArray, (unsigned int)W.size());
	//_newWeightValues.copy(weights);
	//delete[] weightArray;

	// Set weights
	stat = _fnSKin.setWeights(_meshDagPath, _meshComps, _infIds,
		_newWeightValues, true, &_oldWeightValues);

	return stat;
}

MStatus BBWeightsCmd::undoIt()
{
	MStatus stat;
	if (_isTargetJointProvided) {
		if (_isTargetMeshProvided) stat = _dagMod.undoIt();
		else stat = _fnSKin.setWeights(_meshDagPath, _meshComps, _infIds,
			_oldWeightValues, true, &_newWeightValues);
	}
	else {
		stat = _dagMod.undoIt();
		stat = _fnSKin.setWeights(_meshDagPath, _meshComps, _infIds,
			_oldWeightValues, true, &_newWeightValues);
	}
	return stat;
}

MStatus BBWeightsCmd::redoIt()
{
	MStatus stat;
	if (_isTargetJointProvided) {
		if (_isTargetMeshProvided) stat = _dagMod.doIt();
		else stat = _fnSKin.setWeights(_meshDagPath, _meshComps, _infIds,
			_newWeightValues, true, &_oldWeightValues);
	}
	else {
		stat = _dagMod.doIt();
		_fnSKin.setWeights(_meshDagPath, _meshComps, _infIds,
			_newWeightValues, true, &_oldWeightValues);
	}
	return stat;
}