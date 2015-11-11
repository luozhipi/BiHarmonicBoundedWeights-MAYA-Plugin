__author__ = "Zhiping Luo"
__contact__ = "luozhipi@gmail.com"
__website__ = "luozhipi.github.io"

from PySide.QtCore import *
from PySide.QtGui import *
from PySide.QtUiTools import QUiLoader

import utils
#reload(utils)


########################################################################
class BaseTab(QWidget):
    _TITLE = 'Untitle'

    #----------------------------------------------------------------------
    def __init__(self, parent=None):
        super(BaseTab, self).__init__(parent)
        #self.parent = parent
        #loader = QUiLoader()
        #f = QFile(uifile)
        #f.open(QFile.ReadOnly)
        #widget = loader.load(f, self)
        #f.close()
    
        #layout = QVBoxLayout()
        #layout.addWidget(widget)
        #self.setLayout(layout)
        #utils.loadUi(uifile)
    
    #----------------------------------------------------------------------
    def title(self):
        return self._TITLE

    #----------------------------------------------------------------------
    def readSettings(self):
        pass
    
    #----------------------------------------------------------------------
    def writeSettings(self):
        pass
    
    #----------------------------------------------------------------------
    def resetSettings(self):
        pass
    
    #----------------------------------------------------------------------
    def closeEvent(self, event):
        """"""
        self.writeSettings()
        event.accept()    


def main():
    import sys
    app = QApplication(sys.argv)
    window = BaseTab()
    window.show()
    app.exec_()


if __name__ == "__main__":
    main()