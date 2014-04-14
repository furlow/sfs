#!/usr/bin/python
import sys

from PySide.QtCore import *
from PySide.QtGui import *


__appname__ = "Ninth Video"


class Program(QDialog):
    def __init__(self, parent=None):
        super(Program, self).__init__(parent)

        self.setWindowTitle(__appname__)

        btn = QPushButton("Open Dialog")
        self.label1 = QLabel("Label 1 Result")
        self.label2 = QLabel("Label 2 Result")

        layout = QVBoxLayout()
        layout.addWidget(btn)
        layout.addWidget(self.label1)
        layout.addWidget(self.label2)
        self.setLayout(layout)

        self.connect(btn, SIGNAL("clicked()"), self.dialogopen)

    def dialogopen(self):
        dialog = Dialog()
        if dialog.exec_():
            self.label1.setText("Spinbox value is " + str(dialog.spinbox.value()))
            self.label2.setText("Checkbox is " + str(dialog.checkbox.isChecked()))
        else:
            QMessageBox.warning(self, __appname__, "Dialog canceled.")


class Dialog(QDialog):
    def __init__(self, parent=None):
        super(Dialog, self).__init__(parent)

        self.setWindowTitle("Dialog.")

        self.checkbox = QCheckBox("Check me out!")
        self.spinbox = QSpinBox()
        buttonOk = QPushButton("Ok")
        buttonCancel = QPushButton("Cancel")

        layout = QGridLayout()
        layout.addWidget(self.spinbox, 0, 0)
        layout.addWidget(self.checkbox, 0, 1)
        layout.addWidget(buttonCancel)
        layout.addWidget(buttonOk)
        self.setLayout(layout)

        self.connect(buttonOk, SIGNAL("clicked()"), self, SLOT("accept()"))
        self.connect(buttonCancel, SIGNAL("clicked()"), self, SLOT("reject()"))


app = QApplication(sys.argv)
form = Program()
form.show()
app.exec_()
