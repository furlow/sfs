#!/usr/bin/python
import sys
from PySide.QtCore import *
from PySide.QtGui import *
from shape_from_focus import process_stack
import cv2
import numpy2qimage

__appname__ = "Shape From Focus Refocus"

class refocused_image(QLabel):
    def __init__(self, stack, parent = None):
        QLabel.__init__(self)

        self.stack = stack
        print self.stack.width
        print self.stack.height

        #Set up the image pixmap
        self.refocused_pixmap = numpy2qpixmap( self.stack.focused_image )
        self.setPixmap(self.refocused_pixmap.scaled(200, 200, Qt.KeepAspectRatio))
        self.setScaledContents(False)
        self.setMinimumSize(100, 100)

    def mousePressEvent(self, QMouseEvent):
        pos = QMouseEvent.pos()
        print pos
        x_point = self.x_scale * pos.x()
        y_point = self.y_scale * pos.y()
        self.stack.refocus_by_point(x_point, y_point)
        self.refocused_pixmap = numpy2qpixmap( self.stack.refocused_image )
        self.setPixmap(self.refocused_pixmap.scaled(self.width(), self.height(), Qt.KeepAspectRatio))

    def resizeEvent(self, QResizeEvent):
        self.setPixmap(self.refocused_pixmap.scaled(self.width(), self.height(), Qt.KeepAspectRatio))
        self.x_scale = self.stack.width / QResizeEvent.size().width()
        self.y_scale = self.stack.height / QResizeEvent.size().height()

class Program(QDialog):
    def __init__(self, stack, parent = None):
        super(Program, self).__init__(parent)

        self.stack = stack
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
        layout.addWidget(refocused_image(stack))
        layout.addItem(options_layout)

        self.setLayout(layout)

def numpy2qpixmap(cvBGRImg):
    #convert from BGR to RGB
    #cvBGRImg = cv2.cvtColor(cvBGRImg, cv2.cv.CV_BGR2RGB)

    #convert numpy to pixmap image
    qimg = numpy2qimage.toQImage(cvBGRImg)
    Qpixmap_Img = QPixmap.fromImage(qimg)

    return Qpixmap_Img

def main():
    sys.argv[0] = "Shape From Focus"
    img_dir = sys.argv[1]
    stack  = process_stack(img_dir)
    app = QApplication(sys.argv)
    form = Program(stack)
    form.show()
    app.exec_()

if __name__ == '__main__':
    main()
