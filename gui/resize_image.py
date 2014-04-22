import sys
from PySide import QtGui, QtCore

class Example(QtGui.QWidget):

    def __init__(self):
        super(Example, self).__init__()

        self.initUI()

    def initUI(self):

        hbox = QtGui.QHBoxLayout(self)
        self.pixmap_orig = QtGui.QPixmap("frog-1.png")

        self.lbl = main_image(self.pixmap_orig)

        hbox.addWidget(self.lbl)
        self.setLayout(hbox)

        self.setWindowTitle('Red Rock')
        self.show()

class main_image(QtGui.QLabel):
    def __init__(self, pixmap_original):
        QtGui.QLabel.__init__(self)
        self.pixmap_original = pixmap_original
        self.setPixmap(self.pixmap_original)
        self.setScaledContents(False)
        self.setMinimumSize(20,20)

    def resizeEvent(self, QResizeEvent):
        self.setPixmap(self.pixmap_original.scaled(self.width(), self.height(), QtCore.Qt.KeepAspectRatio))

def main():

    app = QtGui.QApplication(sys.argv)
    ex = Example()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
