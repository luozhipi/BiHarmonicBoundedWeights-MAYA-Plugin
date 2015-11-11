__author__ = "Zhiping Luo"
__contact__ = "luozhipi@gmail.com"
__website__ = "luozhipi.github.io"

import maya.cmds as cmds
import maya.OpenMaya as om

import utils
import fnData
reload(utils)


#----------------------------------------------------------------------
def compute(vox_res, targetMesh, targetSkeleton):
    """
    Args:
      res (int)
    """
        
    cmds.bbwSolver(tm=targetMesh, 
                   res=vox_res,
                   tb=targetSkeleton)
#----------------------------------------------------------------------
def getModelFromSelection():
    """"""
    try:
        sel = cmds.ls(sl=1, ap=1)
        if len(sel) < 2:
            om.MGlobal.displayError("Please select joints first, then mesh.")
            return
        else:
            mesh = sel.pop()   
            dagPath1 = utils.getDagPath(mesh) 
            skeleton = sel.pop()
            dagPath2 = utils.getDagPath(skeleton)
            if dagPath1.hasFn(om.MFn.kMesh) and dagPath2.hasFn(om.MFn.kJoint):
                return mesh, skeleton
            else:
                om.MGlobal.displayError("\"%s\" isn't mesh type, \"%s\" isn't skeleton type." % mesh % skeleton)
    except:
        return ""