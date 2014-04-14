#!/usr/bin/python
import sys

from PySide.QtCore import *
from PySide.QtGui import *


__appname__ = "Shape From Focus Refocus"


class Program(QDialog):
    def __init__(self, parent=None):
        super(Program, self).__init__(parent)

        self.setWindowTitle(__appname__)

        btn1 = QPushButton("Open Dialog")
        btn2 = QPushButton("Close Dialog")

        refocused_pixmap = QPixmap(400,300)
        refocused_image = QLabel(self)
        refocused_image.setPixmap(refocused_pixmap)

        options_layout = QVBoxLayout()
        options_layout.addWidget(btn1)
        options_layout.addWidget(btn2)

        layout = QHBoxLayout()
        layout.addWidget(refocused_image)
        layout.addItem(options_layout)

        self.setLayout(layout)

app = QApplication(sys.argv)
form = Program()
form.show()
app.exec_()
