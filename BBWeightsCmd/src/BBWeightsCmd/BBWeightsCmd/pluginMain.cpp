#include "BBWeightsCmd.h"
#include <maya/MFnPlugin.h>


//#define __AUTHOR__   "Zhiping Luo <luozhipi@gmail.com>"
//#define __VERSION__   "1.0.0"

MStatus initializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj, "Zhiping Luo <luozhipi@gmail.com>",
		"1.0.0", "Any");

	status = plugin.registerCommand("bbwSolver",
		BBWeightsCmd::creator,
		BBWeightsCmd::newSyntax);
	CHECK_MSTATUS_AND_RETURN_IT(status);

	return MS::kSuccess;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus   status;
	MFnPlugin plugin(obj);

	status = plugin.deregisterCommand("bbwSolver");
	CHECK_MSTATUS_AND_RETURN_IT(status);
	return MS::kSuccess;
}
