import mastermind.preferences_dialog
from PyQt5 import QtCore, QtGui, QtWidgets
import random


random.seed()


class GameSettings(object):
    def __init__(self):
        super().__init__()

        self._settings = QtCore.QSettings('Asgeir Bjarni Ingvarsson', 'Mastermind')

        self.colors = [QtGui.QColor(x) for x in ['red', 'green', 'blue', 'cyan', 'magenta', 'yellow']]
        self.response_colors = [QtGui.QColor(x) for x in ['white', 'red']]

    def save(self):
        for i in range(len(self.colors)):
            self._settings.setValue('play/pin_{0}'.format(i), self.colors[i])

        for i in range(len(self.response_colors)):
            self._settings.setValue('response/pin_{0}'.format(i), self.response_colors[i])

    def load(self):
        for i in range(len(self.colors)):
            self.colors[i] = self._settings.value('play/pin_{0}'.format(i), self.colors[i])

        for i in range(len(self.response_colors)):
            self.response_colors[i] = self._settings.value('response/pin_{0}'.format(i), self.response_colors[i])


class Pin(QtWidgets.QWidget):
    TRANSPARENT = -2
    BLACK = -1

    def __init__(self, settings, is_response_pin, parent):
        super().__init__(parent)

        if is_response_pin:
            self.setMinimumSize(20, 20)
            self.setMaximumSize(20, 20)
        else:
            self.setMinimumSize(50, 50)
            self.setMaximumSize(50, 50)

        self._imageLabel = QtWidgets.QLabel(self)
        self._color_menu = QtWidgets.QMenu(self)
        self._menu_enabled = True

        layout = QtWidgets.QHBoxLayout()
        layout.setContentsMargins(1, 1, 1, 1)
        layout.setSpacing(1)
        layout.addWidget(self._imageLabel)
        self.setLayout(layout)

        self._settings = settings
        self.is_response_pin = is_response_pin
        self.color = -2

        self.set_color(self.color)

    def resizeEvent(self, event):
        super().resizeEvent(event)

        self.set_color(self.color)

    def mousePressEvent(self, event):
        super().mousePressEvent(event)

        if self._menu_enabled and not self.is_response_pin:
            self._populate_color_menu()
            self._color_menu.exec_(QtGui.QCursor.pos())

    def enable_menu(self):
        self._menu_enabled = True

    def disable_menu(self):
        self._menu_enabled = False

    def set_color(self, color):
        image = QtGui.QImage(self.width(), self.height(), QtGui.QImage.Format_ARGB32)
        image.fill(QtGui.QColor('transparent'))

        self.color = color
        if color >= 0 and self.is_response_pin:
            image.fill(self._settings.response_colors[color])
        elif color >= 0:
            image.fill(self._settings.colors[color])
        elif color == Pin.BLACK:
            image.fill(QtGui.QColor('black'))
        elif color == Pin.TRANSPARENT:
            image.fill(QtGui.QColor('transparent'))

        self._imageLabel.setPixmap(QtGui.QPixmap.fromImage(image))

    def refresh(self):
        if self.color >= 0:
            self.set_color(self.color)

    def _populate_color_menu(self):
        old_actions = self._color_menu.actions()[:]
        for action in old_actions:
            self._color_menu.removeAction(action)
            action.deleteLater()

        for i, color in enumerate(self._settings.colors):
            image = QtGui.QImage(20, 20, QtGui.QImage.Format_ARGB32)
            image.fill(color)

            def closure(color):
                def inner():
                    self.set_color(color)
                return inner

            action = QtWidgets.QAction(QtGui.QIcon(QtGui.QPixmap.fromImage(image)), '', self)
            action.triggered.connect(closure(i))

            self._color_menu.addAction(action)


class ResponsePins(QtWidgets.QWidget):
    def __init__(self, settings, parent):
        super().__init__(parent)

        self.setMinimumSize(50, 50)
        self.setMaximumSize(50, 50)

        self.pins = []

        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(1, 1, 1, 1)
        layout.setSpacing(1)

        for i in range(2):
            row = QtWidgets.QHBoxLayout()
            row.setContentsMargins(1, 1, 1, 1)
            row.setSpacing(1)

            for j in range(2):
                pin = Pin(settings, True, self)
                row.addWidget(pin)
                self.pins.append(pin)

            layout.addLayout(row)

        self.setLayout(layout)

    def make_ready(self):
        for pin in self.pins:
            pin.set_color(Pin.BLACK)

    def reset(self):
        for pin in self.pins:
            pin.set_color(Pin.TRANSPARENT)

    def refresh(self):
        for pin in self.pins:
            pin.refresh()

    def set_score(self, score):
        for i, color in enumerate(reversed(sorted(score))):
            self.pins[i].set_color(color)


class RowWidget(QtWidgets.QWidget):
    guess_made = QtCore.pyqtSignal()

    def __init__(self, settings, parent):
        super().__init__(parent)

        self._is_code = False
        self._settings = settings

        layout = QtWidgets.QHBoxLayout()
        layout.setContentsMargins(1, 1, 1, 1)
        layout.setSpacing(1)

        self._code = []
        self.pins = []
        self._create_pins(layout)

        self.response_pins = ResponsePins(settings, self)
        layout.addWidget(self.response_pins)

        self._button = QtWidgets.QPushButton('Guess', self)
        self._button.setMinimumSize(50, 50)
        self._button.setMaximumSize(50, 50)
        self._button.clicked.connect(self._button_clicked)
        layout.addWidget(self._button)

        self.setLayout(layout)

    @property
    def code(self):
        if self._code:
            return self._code

        return [pin.color for pin in self.pins]

    def set_is_code(self, value):
        self._is_code = value
        self.response_pins.setVisible(not value)

        if value:
            self._button.setText('Ready')
        else:
            self._button.setText('Guess')

    def set_code(self, code):
        if len(code) != 4:
            raise Exception('Wtf! Invalid code')

        self.response_pins.setVisible(False)
        self._button.setVisible(False)
        self._code = code

        for pin in self.pins:
            pin.set_color(Pin.TRANSPARENT)
            pin.disable_menu()

    def make_ready(self):
        for pin in self.pins:
            pin.set_color(Pin.BLACK)
            pin.enable_menu()

        self.response_pins.make_ready()

        self.response_pins.setVisible(False)
        self._button.setVisible(True)

    def reset(self):
        self._code = []
        for pin in self.pins:
            pin.set_color(Pin.TRANSPARENT)
            pin.disable_menu()

        self.response_pins.reset()

        self.response_pins.setVisible(True)
        self._button.setVisible(False)

    def refresh(self):
        for pin in self.pins:
            pin.refresh()

        self.response_pins.refresh()

    def show_code(self):
        if not self._is_code:
            return

        if self._code:
            for i, pin in enumerate(self.pins):
                pin.set_color(self.code[i])

    def _create_pins(self, layout):
        for i in range(4):
            pin = Pin(self._settings, False, self)
            self.pins.append(pin)
            layout.addWidget(pin)

    def _button_clicked(self, checked=False):
        for pin in self.pins:
            if pin.color < 0:
                message = 'You must set all the pins before beginning a game.'
                if not self._is_code:
                    message = 'You must set all the pins before making a guess.'

                dialog = QtWidgets.QMessageBox(self)
                dialog.setWindowTitle('Mastermind')
                dialog.setText(message)
                dialog.exec_()

                return

        self._button.setVisible(False)
        if self._is_code:
            self._code = [pin.color for pin in self.pins]

            for pin in self.pins:
                pin.set_color(Pin.TRANSPARENT)
        else:
            self.response_pins.setVisible(True)

        for pin in self.pins:
            pin.disable_menu()

        self.guess_made.emit()


class BoardWidget(QtWidgets.QWidget):
    def __init__(self, settings, parent):
        super().__init__(parent)

        self._settings = settings
        self._code = None
        self._rows = []

        self._create_layout()
        self.reset_board()

    def reset_board(self, code=None):
        self._current_row = -1
        self.is_game_in_progress = False

        for row in self._rows:
            row.reset()

        self._code.reset()
        if code:
            self._code.set_code(code)
            self._on_begin_game()
        else:
            self._code.make_ready()

    def refresh(self):
        self._code.refresh()

        for row in self._rows:
            row.refresh()

    def _create_layout(self):
        layout = QtWidgets.QVBoxLayout()
        layout.setContentsMargins(1, 1, 1, 1)
        layout.setSpacing(1)

        for i in range(13):
            row = RowWidget(self._settings, self)
            if i == 0:
                row.set_is_code(True)
                row.make_ready()
                row.guess_made.connect(self._on_begin_game)
                self._code = row
            else:
                row.reset()
                row.guess_made.connect(self._on_make_guess)
                self._rows.append(row)

            layout.addWidget(row)

        self.setLayout(layout)

    def _on_begin_game(self, checked=False):
        self._current_row = 0
        self.is_game_in_progress = True

        self._rows[0].make_ready()

    def _on_make_guess(self, checked=False):
        if not self.is_game_in_progress:
            return

        score = self._score_guess()
        self._rows[self._current_row].response_pins.set_score(score)

        if score == [1,1,1,1]:
            self._code.show_code()
            self.is_game_in_progress = False

            dialog = QtWidgets.QMessageBox(self)
            dialog.setWindowTitle('Mastermind')
            dialog.setText('You won!')
            dialog.exec_()

            return

        self._current_row += 1
        if self._current_row == 12:
            self._code.show_code()
            self.is_game_in_progress = False

            dialog = QtWidgets.QMessageBox(self)
            dialog.setWindowTitle('Mastermind')
            dialog.setText('You lost. :(')
            dialog.exec_()

            return

        self._rows[self._current_row].make_ready()

    def _score_guess(self):
        cur_code = self._rows[self._current_row].code[:]
        code = self._code.code[:]

        if cur_code == code:
            return [1] * 4

        score = []
        for i in range(4):
            if cur_code[i] == code[i]:
                score.append(1)
                cur_code[i] = -99
                code[i] = -88

        for i in range(4):
            for j in range(4):
                if j != i and cur_code[i] == code[j]:
                    score.append(0)
                    cur_code[i] = -99
                    code[j] = -88
                    break

        return score


class GameWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle('Mastermind')

        self._settings = GameSettings()
        self._settings.load()
        self._create_menu()
        self._create_board()

        self.setFixedSize(self.sizeHint())

        self.statusBar().showMessage('Welcome to Mastermind')

    def closeEvent(self, event):
        if self._verify_quit():
            event.accept()
        else:
            event.ignore()

    def _create_menu(self):
        game_menu = self.menuBar().addMenu('&Game')

        new_game_act = QtWidgets.QAction('&New', self)
        new_game_act.setShortcuts(QtGui.QKeySequence.New)
        new_game_act.setStatusTip('Start a new game')
        new_game_act.triggered.connect(self._on_new_game)

        preferences_act = QtWidgets.QAction('&Preferences', self)
        preferences_act.setShortcuts(QtGui.QKeySequence.Preferences)
        preferences_act.setStatusTip('Change game settings')
        preferences_act.triggered.connect(self._on_preferences)

        quit_act = QtWidgets.QAction('E&xit', self)
        quit_act.setShortcuts(QtGui.QKeySequence.Quit)
        quit_act.setStatusTip('Quit Mastermind')
        quit_act.triggered.connect(self._on_quit)

        game_menu.addAction(new_game_act)
        game_menu.addAction(preferences_act)
        game_menu.addSeparator()
        game_menu.addAction(quit_act)

    def _create_board(self):
        self._board = BoardWidget(self._settings, self)
        self.setCentralWidget(self._board)

        self._board.reset_board([random.choice(range(6)) for i in range(4)])

    def _on_new_game(self):
        if self._board.is_game_in_progress:
            dialog = QtWidgets.QMessageBox(self)
            dialog.setWindowTitle('Mastermind')
            dialog.setText('You are currently playing a game. Are you sure you want to abandon your current game?')

            dialog.addButton(QtWidgets.QMessageBox.Yes)

            no_button = dialog.addButton(QtWidgets.QMessageBox.No)
            dialog.setDefaultButton(no_button)
            dialog.setEscapeButton(no_button)

            if dialog.exec_() == QtWidgets.QMessageBox.No:
                return

        self._board.reset_board([random.choice(range(6)) for i in range(4)])

    def _on_preferences(self):
        dialog = mastermind.preferences_dialog.PreferencesDialog(self._settings, self)
        if dialog.exec_():
            self._settings.save()
            self._board.refresh()

    def _verify_quit(self):
        if self._board.is_game_in_progress:
            dialog = QtWidgets.QMessageBox(self)
            dialog.setWindowTitle('Mastermind')
            dialog.setText('You are currently playing a game. Are you sure you want to Quit?')

            dialog.addButton(QtWidgets.QMessageBox.Yes)

            no_button = dialog.addButton(QtWidgets.QMessageBox.No)
            dialog.setDefaultButton(no_button)
            dialog.setEscapeButton(no_button)

            if dialog.exec_() == QtWidgets.QMessageBox.No:
                return False

        return True

    def _on_quit(self):
        if not self._verify_quit():
            return

        QtWidgets.qApp.quit()
