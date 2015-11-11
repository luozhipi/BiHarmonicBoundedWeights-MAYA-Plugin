__author__ = "Zhiping Luo"
__contact__ = "luozhipi@gmail.com"
__website__ = "luozhipi.github.io"
__buildVersionID__      = '1.0.0'
__ENVIRONMENT_NAME__ = "Bounded_BiHarmonic_Weights"
#----------------------------------------------------------------------

import sys
import os
import maya.cmds as cmds
import maya.mel as mel
import maya.OpenMayaUI as omui
from PySide.QtCore import * 
from PySide.QtGui import *
from shiboken import wrapInstance
import logging
logging.basicConfig()
log = logging.getLogger(__name__)
log.setLevel(logging.INFO)

# Static variables
SCRIPT_DIRECTORY = os.path.dirname(os.path.abspath(__file__)).replace('\\', '/')
BBW_PLUGIN_BASE_NAME = "BBWeightsCmd"
MAIN_WINDOW = "BBWeightsWin"


#----------------------------------------------------------------------
def getMayaWindow():
    ptr = omui.MQtUtil.mainWindow()
    if ptr:
        return wrapInstance(long(ptr), QWidget)

#----------------------------------------------------------------------
def show():
    """"""
    from widget import mainWindow
    reload(mainWindow)
    
    if cmds.window(MAIN_WINDOW, ex=1):
        cmds.deleteUI(MAIN_WINDOW)
    
    parent = getMayaWindow()
    win = mainWindow.MainWindow(parent)
    win.setAttribute(Qt.WA_DeleteOnClose)
    win.show()

#----------------------------------------------------------------------
def getEnviron():
    """"""
    return __ENVIRONMENT_NAME__


#----------------------------------------------------------------------
def loadBBWPlugin():
    """"""    
    os = cmds.about(os=1)
    
    if os == 'win64':
        pluginName = '%s.mll' % (BBW_PLUGIN_BASE_NAME)
    elif os == 'mac':
        pluginName = '%s.bundle' % (BBW_PLUGIN_BASE_NAME)
    elif os == 'linux64':
        pluginName = '%s.so' % (BBW_PLUGIN_BASE_NAME)
    
    if not cmds.pluginInfo(pluginName, q=True, l=True ):
        cmds.loadPlugin(pluginName)
        pluginVers = cmds.pluginInfo(pluginName, q=1, v=1)
        log.info('Plug-in: %s v%s loaded success!' % (pluginName, pluginVers))
    else:
        pluginVers = cmds.pluginInfo(pluginName, q=1, v=1)
        log.info('Plug-in: %s v%s has been loaded!' % (pluginName, pluginVers))

#----------------------------------------------------------------------
def loadPlugin():
    """"""
    loadBBWPlugin()

#----------------------------------------------------------------------
def mayaVersion():
    """
	need to manage this better and use the API version,
    eg: 2013.5 returns 2013
	"""
    return mel.eval('getApplicationVersionAsFloat')

#----------------------------------------------------------------------
def getModulePath():
    '''
    Returns the Main path to the root module folder
    '''
    print os.path.join(os.path.dirname(SCRIPT_DIRECTORY),'').replace('\\', '/')
    return os.path.join(os.path.dirname(SCRIPT_DIRECTORY),'').replace('\\', '/')

#----------------------------------------------------------------------
def getVersion():
    return __buildVersionID__

#----------------------------------------------------------------------
def getAuthor():
    return __author__

#----------------------------------------------------------------------
def getMainWindowName():
    return MAIN_WINDOW
  
# BOOT FUNCTS - Add and Build --------------------------------------------------------------
    
def addScriptsPath(path):
    '''
    Add additional folders to the ScriptPath
    '''
    scriptsPath = os.environ.get('MAYA_SCRIPT_PATH')
    
    if os.path.exists(path):
        if not path in scriptsPath:
            log.info('Adding To Script Paths : %s' % path)
            os.environ['MAYA_SCRIPT_PATH']+='%s%s' % (os.pathsep, path)
        else:
            log.info('BBWeights Script Path already setup : %s' % path)
    else:
        log.debug('Given Script Path is invalid : %s' % path)


#=========================================================================================
# BOOT CALL ------------------------------------------------------------------------------
#=========================================================================================
    
def launch():
    '''
    Main entry point
    '''
    log.info('BBWeights v%s : Author: %s' % (getVersion(), getAuthor()))
    log.info('BBWeights Setup Calls :: Booting from >> %s' % getModulePath())  
    
    # Add module to environment
    os.environ[__ENVIRONMENT_NAME__] = getModulePath()    
    # addScriptsPath(os.environ[__ENVIRONMENT_NAME__])
    
    # Load Plug-in
    loadPlugin()    
    
    # launch UI
    show()
    
    log.info('BBWeights initialize Complete!')
    
def interface():
    show()
