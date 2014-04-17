#!/usr/bin/python
import sys
from PySide.QtCore import *
from PySide.QtGui import *
from shape_from_focus import process_stack

__appname__ = "Shape From Focus Refocus"

class refocused_image(QLabel):
    def __init__(self, parent = None):
        super(refocused_image, self).__init__(parent)

        #Set up the image pixmap
        self.refocused_pixmap = QPixmap(400, 300)
        self.setPixmap(self.refocused_pixmap)

    def mousePressEvent(self, QMouseEvent):
        print QMouseEvent.pos()

class Program(QDialog):
    def __init__(self, parent = None):
        super(Program, self).__init__(parent)

        self.setWindowTitle(__appname__)

        #Set up the refocusing sliders
        focus_label = QLabel("Focus depth")
        dof_label = QLabel("Depth of Field")
        self.focus = QSlider()
        self.dof = QSlider()
        self.focus.setMaximum(20)
        self.focus.setMinimum(0)
        self.dof.setMaximum(20)
        self.dof.setMinimum(0)

        #Align the widgets in their containers
        options_layout = QVBoxLayout()
        options_layout.addWidget(focus_label)
        options_layout.addWidget(self.focus)
        options_layout.addWidget(dof_label)
        options_layout.addWidget(self.dof)

        layout = QHBoxLayout()
        layout.addWidget(refocused_image())
        layout.addItem(options_layout)

        self.setLayout(layout)

def main():
    sys.argv[0] = "Shape From Focus"
    img_dir = sys.argv[1]
    stack  = process_stack(img_dir)
    app = QApplication(sys.argv)
    form = Program()
    form.show()
    app.exec_()

if __name__ == '__main__':
    main()
