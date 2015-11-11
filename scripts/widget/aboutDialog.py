__author__ = "Zhiping Luo"
__contact__ = "luozhipi@gmail.com"
__website__ = "luozhipi.github.io"

import os

from PySide.QtCore import *
from PySide.QtGui import *

import utils


uifile = os.path.join(utils.SCRIPT_DIRECTORY, "ui/aboutDialog.ui")


########################################################################
class AboutDialog(QDialog):
    """"""
    _TITLE = 'Utility'    
    
    #----------------------------------------------------------------------
    def __init__(self, parent=None):
        """Constructor"""
        super(AboutDialog, self).__init__(parent)
        utils.loadUi(uifile, self)
        self.initWidgets()
    
    #----------------------------------------------------------------------
    def initWidgets(self):
        """"""
        cp = QDesktopWidget().screenGeometry().center()
        self.move(cp)  
        
        
#----------------------------------------------------------------------
def main():
    import sys
    app = QApplication(sys.argv)
    window = AboutDialog()
    window.show()
    app.exec_()


if __name__ == "__main__":
    main()