#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
from PySide.QtCore import *
from PySide.QtGui import *

# Create the QApplication object
qt_app = QApplication(sys.argv)

class HelloWorldApp(QWidget):
    ''' A Qt application that displays the text, "Hello, world!" '''
    def __init__(self):
        # Initialize the object as a QLabel
        QLabel.__init__(self, "Hello, world!")

        # Set the size, alignment, and title
        self.setMinimumSize(QSize(600, 400))
        self.setAlignment(Qt.AlignCenter)
        self.setWindowTitle('Hello, world!')

        go_button = QPushButton('Go', self)

    def run(self):
        ''' Show the application window and start the main event loop '''
        self.show()
        qt_app.exec_()

# Create an instance of the application and run it
HelloWorldApp().run()
