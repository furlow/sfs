#!/usr/bin/python

import sys
from PySide.QtGui import *
from PySide.QtCore import *

class test_win(QWidget):
  def __int__(self, parent = None):
    super(test_win, self).__init__(parent)
    self.setWindowTitle('First GUI')
    self.setMinimumSize(400, 300)

if __name__ == '__main__':
  app = QApplication(sys.argv)
  mainWin = test_win()
  mainWin.show()
  app.exec_()
