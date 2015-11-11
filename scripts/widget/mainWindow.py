__author__ = "Zhiping Luo"
__contact__ = "luozhipi@gmail.com"
__website__ = "luozhipi.github.io"

import os
from PySide.QtCore import *
from PySide.QtGui import *

import aboutDialog
import utils
from startup import setup

cfgfile = os.path.join(utils.SCRIPT_DIRECTORY, "config.ini")

########################################################################
class MainWindow(QMainWindow):
    """"""
    _TITLE = 'Bounded BiHarmonic Weights'

    #----------------------------------------------------------------------
    def __init__(self, parent=None):
        """Constructor"""
        super(MainWindow, self).__init__(parent)

        self.setObjectName(setup.getMainWindowName())        
        #self.parent = parent  # this's extremely important! without this line
                                # "Internal C++ object already deleted" happen
        self.initWidgets()

    #----------------------------------------------------------------------
    def initWidgets(self):
        """"""      
        self.setWindowTitle(self._TITLE)
        self.resize(100, 100)

        # Add actions
        openAbout = QAction("&About", self)
        openAbout.triggered.connect(self.showAbout)

        resetSettings = QAction("&Reset", self)
        resetSettings.triggered.connect(self.resetSettings)

        # Add menu bar
        bar = self.menuBar()
        editMenu = bar.addMenu("&Edit")
        editMenu.addAction(resetSettings)  

        helpMenu = bar.addMenu("&Help")
        helpMenu.addAction(openAbout)

        # Add base frame
        frame = QFrame(self)      
        layout = QVBoxLayout(self)
        frame.setLayout(layout)

        tabWidget = QTabWidget(self)
        tabWidget.setObjectName("allFuncs_tab")
        layout.addWidget(tabWidget)

        self.setCentralWidget(frame)

        # Add tabs dynamically
        for cls in utils.findAllTabs():
            tab = cls(self)
            self.addTab(tab, tabWidget)

        tabWidget.setCurrentIndex(0)

    #----------------------------------------------------------------------
    def addTab(self, tab, tabWidget):
        """
        adds tab object to tab UI, creating it's ui and attaching to main window
        """
        tabWidget.addTab(tab, tab.title())

    #----------------------------------------------------------------------
    def showAbout(self):
        """"""
        aboutDialog.AboutDialog(self).show()
            
    #----------------------------------------------------------------------
    def resetSettings(self):
        allFuncs_tab = self.findChild(QTabWidget, "allFuncs_tab")
        for i in xrange(allFuncs_tab.count()):
            allFuncs_tab.widget(i).resetSettings()     

    #----------------------------------------------------------------------
    def closeEvent(self, event):
        """"""
        allFuncs_tab = self.findChild(QTabWidget, "allFuncs_tab")
        for i in xrange(allFuncs_tab.count()):
            allFuncs_tab.widget(i).closeEvent(event)

        event.accept()


#----------------------------------------------------------------------
def main():
    import sys
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    app.exec_()


if __name__ == "__main__":
    main()