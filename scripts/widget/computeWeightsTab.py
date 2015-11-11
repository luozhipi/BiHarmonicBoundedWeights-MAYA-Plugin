__author__ = "Zhiping Luo"
__contact__ = "luozhipi@gmail.com"
__website__ = "luozhipi.github.io"

import os

from PySide.QtCore import *
from PySide.QtGui import *

import maya.cmds as cmds

import baseTab
import utils

from core import weightsFunc


reload(weightsFunc)
#reload(baseTab)
#reload(utils)


TAB = ''
uifile = os.path.join(utils.SCRIPT_DIRECTORY, "ui/computeWeightsTab.ui")
cfgfile = os.path.join(utils.SCRIPT_DIRECTORY, "config.ini")
ENABLE = True
INDEX = 1


########################################################################
class ComputeWeightsTab(baseTab.BaseTab):
    """"""
    _TITLE = 'weights'    
    
    #----------------------------------------------------------------------
    def __init__(self, parent=None):
        """Constructor"""
        super(ComputeWeightsTab, self).__init__(parent)
        utils.loadUi(uifile, self)
        self.initWidgets()
    
    #----------------------------------------------------------------------
    def initWidgets(self):
        """"""
        self.readSettings()
        
        # Set transformation widgets        
        intVal = QIntValidator(2, 512, self)              
    
    #----------------------------------------------------------------------
    def getRes(self):
        """"""
        res_spn = self.findChild(QSpinBox, "res_spn")
        if res_spn != None:
            return res_spn.value()
        return 1
    
    #----------------------------------------------------------------------
    def setRes(self, val):
        """"""
        res_spn = self.findChild(QSpinBox, "res_spn")
        if res_spn != None:        
            res_spn.setValue(val) 

    #----------------------------------------------------------------------
    def setTargetSkeleton(self, val):
        """"""
        targetSkeleton_edit = self.findChild(QLineEdit, "targetSkeleton_edit")
        if targetSkeleton_edit != None:         
            targetSkeleton_edit.setText(val)
        
    #----------------------------------------------------------------------
    def getTargetSkeleton(self):
        """"""
        targetSkeleton_edit = self.findChild(QLineEdit, "targetSkeleton_edit")
        if targetSkeleton_edit != None:         
            return str(targetSkeleton_edit.text())        
        
    #----------------------------------------------------------------------
    def getTargetMesh(self):
        """"""
        targetMesh_edit = self.findChild(QLineEdit, "targetMesh_edit")
        if targetMesh_edit != None:         
            return str(targetMesh_edit.text())
    
    #----------------------------------------------------------------------
    def setTargetMesh(self, val):
        """"""
        targetMesh_edit = self.findChild(QLineEdit, "targetMesh_edit")
        if targetMesh_edit != None:         
            targetMesh_edit.setText(val)
    
    #----------------------------------------------------------------------
    def readSettings(self):
        settings = QSettings(cfgfile, QSettings.IniFormat)

        settings.beginGroup("Custom")
        self.setRes(int(settings.value("res")))
        settings.endGroup()
    
    #----------------------------------------------------------------------
    def writeSettings(self):
        settings = QSettings(cfgfile, QSettings.IniFormat)

        settings.beginGroup("Custom")
        settings.setValue("res", self.getRes())
        settings.endGroup()
    
    #----------------------------------------------------------------------
    def resetSettings(self):
        settings = QSettings(cfgfile, QSettings.IniFormat)
        
        settings.beginGroup("Default")
        self.setRes(int(settings.value("res")))
        settings.endGroup()   
        
        self.setTargetMesh("")
        self.setTargetSkeleton("")
    
    #----------------------------------------------------------------------
    @Slot()
    def on_loadTargetModel_btn_clicked(self):
        mesh, skeleton = weightsFunc.getModelFromSelection()
        self.setTargetMesh(mesh)
        self.setTargetSkeleton(skeleton)
    
    #----------------------------------------------------------------------
    @Slot()
    def on_compute_btn_clicked(self):
        
        # Read local settings
        vox_res = self.getRes()
        targetMesh = self.getTargetMesh()
        targetSkeleton = self.getTargetSkeleton()
        weightsFunc.compute(vox_res, targetMesh, targetSkeleton)
        
        
#----------------------------------------------------------------------
def getTab():
    """"""
    return ComputeWeightsTab 
        
#----------------------------------------------------------------------
def main():
    import sys
    app = QApplication(sys.argv)
    window = ComputeWeightsTab()
    window.show()
    app.exec_()


if __name__ == "__main__":
    main()