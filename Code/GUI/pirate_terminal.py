import sys
import serial
import threading
import os

from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QLabel, QTextEdit, QLineEdit,
    QVBoxLayout, QHBoxLayout, QMessageBox
)
from PyQt5.QtCore import pyqtSignal, Qt, QObject
from PyQt5.QtGui import QPixmap, QFont, QTextCursor


class SerialReader(QObject):
    line_received = pyqtSignal(str)

    def __init__(self, port, baudrate=115200):
        super().__init__()
        try:
            self.ser = serial.Serial(port, baudrate, timeout=1)
        except serial.SerialException as e:
            QMessageBox.critical(None, "⚓️ Serial Port Error", f"Couldn’t open ye port:\n{e}")
            sys.exit(1)

        self.running = True
        self.thread = threading.Thread(target=self.read_loop)
        self.thread.daemon = True
        self.thread.start()

    def read_loop(self):
        while self.running:
            try:
                line = self.ser.readline().decode(errors='ignore').strip()
                if line:
                    self.line_received.emit(f"☠️ {line}")
            except Exception as e:
                self.line_received.emit(f"[⚓️ ERROR] {e}")
                self.running = False

    def stop(self):
        self.running = False
        self.ser.close()


class PirateGUI(QMainWindow):
    def __init__(self, port='/dev/tty.usbmodem2103', baudrate=115200):
        super().__init__()

        self.setWindowTitle("🏴‍☠️ Pirate's Treasure Terminal")
        self.resize(900, 550)


        # === Background ===
        self.background_widget = QLabel(self)
        self.background_pixmap = QPixmap("background.jpg")
        self.background_widget.setPixmap(self.background_pixmap)
        self.background_widget.setScaledContents(True)
        self.background_widget.setFixedSize(self.size())
        self.setCentralWidget(self.background_widget)

        # === Overlay content ===
        self.overlay_widget = QWidget(self.background_widget)
        self.overlay_widget.setGeometry(0, 0, 900, 550)
        layout = QHBoxLayout(self.overlay_widget)
        layout.setContentsMargins(40, 60, 40, 40)

        # === Pirate image ===
        pirate_label = QLabel()
        pirate_pixmap = QPixmap("pirate.png").scaledToHeight(800, Qt.SmoothTransformation)
        pirate_label.setPixmap(pirate_pixmap)
        pirate_label.setAlignment(Qt.AlignBottom | Qt.AlignLeft)
        layout.addWidget(pirate_label, stretch=2)
        

        # === Text + reply bubble container ===
        bubble_container = QVBoxLayout()

        # Speech bubble
        self.text_area = QTextEdit()
        self.text_area.setReadOnly(True)
        self.text_area.setFont(QFont("Courier New", 20, QFont.Bold))
        self.text_area.setFixedSize(700, 500)
        self.text_area.setStyleSheet("""
            QTextEdit {
                background-color: rgba(0, 0, 0, 180);
                color: #FFD700;
                border: 3px solid #8B0000;
                border-radius: 15px;
                padding: 10px;
            }
        """)
        from PyQt5.QtWidgets import QPushButton

        # Clear Button
        self.clear_button = QPushButton("🧹 Clear")
        self.clear_button.setFixedWidth(80)
        self.clear_button.setStyleSheet("""
            QPushButton {
                background-color: #8B0000;
                color: white;
                border-radius: 8px;
                padding: 4px;
            }
            QPushButton:hover {
                background-color: #a00000;
            }
        """)
        self.clear_button.clicked.connect(self.clear_text_area)


        # Reply input
        self.reply_input = QLineEdit()
        self.reply_input.setPlaceholderText("Type a reply and press Enter...")
        self.reply_input.setFont(QFont("Courier New", 20, QFont.Bold))
        self.reply_input.setStyleSheet("""
            QLineEdit {
                background-color: rgba(255, 255, 255, 220);
                color: #000000;
                border: 2px solid #8B0000;
                border-radius: 10px;
                padding: 6px;
            }
        """)
        self.reply_input.setMaximumWidth(700)
        self.reply_input.returnPressed.connect(self.send_reply)

        # Add both to the container
        bubble_container.addWidget(self.text_area)
        bubble_container.addSpacing(10)
        bubble_container.addWidget(self.clear_button, alignment=Qt.AlignRight)
        bubble_container.addWidget(self.reply_input)
        bubble_container.setAlignment(Qt.AlignTop | Qt.AlignRight)

        # Wrap in QWidget and add to layout
        bubble_widget = QWidget()
        bubble_widget.setLayout(bubble_container)
        layout.addWidget(bubble_widget, alignment=Qt.AlignTop | Qt.AlignRight)

        # Serial reader
        self.serial_reader = SerialReader(port, baudrate)
        self.serial_reader.line_received.connect(self.display_line)

    def display_line(self, line):
        self.text_area.append(line)
        self.text_area.moveCursor(QTextCursor.End)

    def clear_text_area(self):
        self.text_area.clear()


    def send_reply(self):
        text = self.reply_input.text().strip()
        if text or text == "":
            self.serial_reader.ser.write((text + "\n").encode())
            self.reply_input.clear()

    def resizeEvent(self, event):
        self.background_widget.setFixedSize(self.size())
        self.overlay_widget.setGeometry(self.rect())
        super().resizeEvent(event)


    def closeEvent(self, event):
        self.serial_reader.stop()
        event.accept()


if __name__ == '__main__':
    app = QApplication(sys.argv)

    port = '/dev/tty.usbmodem2103'
    baudrate = 115200

    window = PirateGUI(port=port, baudrate=baudrate)
    window.show()

    sys.exit(app.exec_())
