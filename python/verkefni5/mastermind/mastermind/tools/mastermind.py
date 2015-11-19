import mastermind.game_window
from PyQt5 import QtWidgets
import sys

def main():
    app = QtWidgets.QApplication(sys.argv)

    window = mastermind.game_window.GameWindow()
    window.show()

    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
