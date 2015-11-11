#ifndef _MAYA_H
#define _MAYA_H

/* Maya iterator headers */
#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>
//#include <maya/MItMeshVertex.h>
#include <maya/MItGeometry.h>

/* Maya function set headers */
#include <maya/MFnMesh.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnTransform.h>
#include <maya/MFnIkJoint.h>

/* Maya proxy headers */
#include <maya/MPxCommand.h>

/* Maya data headers */
#include <maya/MFloatVectorArray.h>
#include <maya/MVectorArray.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MMatrixArray.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MFloatPointArray.h>
#include <maya/MBoundingBox.h>
#include <maya/MFloatMatrix.h>

#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MDagModifier.h>
#include <maya/MTimer.h>
#include <maya/MSyntax.h>
#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MGlobal.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MAnimControl.h>
#include <maya/MTime.h>

/* Maya node */
#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MTypeId.h>

/* Intel TBB headers */
#include <tbb/parallel_for.h>

using namespace tbb;

#endif
