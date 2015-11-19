from PyQt5 import QtWidgets, QtGui

class PreferencesDialog(QtWidgets.QDialog):
    PLAY_COLORS = 'play-colors'
    RESPONSE_COLORS = 'response-colors'

    def __init__(self, settings, parent):
        super().__init__(parent)

        self._settings = settings

        layout = QtWidgets.QVBoxLayout()

        color_layout = QtWidgets.QHBoxLayout()
        color_layout.addWidget(QtWidgets.QLabel('Pin colors:', self))
        for i in range(6):
            self._create_color_button(PreferencesDialog.PLAY_COLORS, i, color_layout)

        layout.addLayout(color_layout)

        feedback_layout = QtWidgets.QHBoxLayout()
        feedback_layout.addWidget(QtWidgets.QLabel('Feedback colors:', self))
        for i in range(2):
            self._create_color_button(PreferencesDialog.RESPONSE_COLORS, i, feedback_layout)

        layout.addLayout(feedback_layout)

        button_layout = QtWidgets.QHBoxLayout()
        button_layout.addStretch()

        done_button = QtWidgets.QPushButton('Done', self)
        done_button.clicked.connect(self.accept)
        done_button.setDefault(True)

        button_layout.addWidget(done_button)

        layout.addLayout(button_layout)

        self.setLayout(layout)

    def _create_color_button(self, color_group, color_index, layout):
        button = QtWidgets.QPushButton(self)
        layout.addWidget(button)

        button.setMinimumSize(20, 20)
        button.setMaximumSize(20, 20)
        button.setFlat(True)

        self._set_button_color(color_group, color_index, button)

        def slot(checked=False):
            self._on_show_color_dialog(color_group, color_index, button)

        button.clicked.connect(slot)

    def _on_show_color_dialog(self, color_group, color_index, button):
        initial_color = QtGui.QColor('black')
        if color_group == PreferencesDialog.PLAY_COLORS:
            initial_color = self._settings.colors[color_index]
        elif color_group == PreferencesDialog.RESPONSE_COLORS:
            initial_color = self._settings.response_colors[color_index]

        dialog = QtWidgets.QColorDialog(initial_color, self)
        if dialog.exec_():
            if color_group == PreferencesDialog.PLAY_COLORS:
                self._settings.colors[color_index] = dialog.selectedColor()
            elif color_group == PreferencesDialog.RESPONSE_COLORS:
                self._settings.response_colors[color_index] = dialog.selectedColor()

            self._set_button_color(color_group, color_index, button)

    def _set_button_color(self, color_group, color_index, button):
        image = QtGui.QImage(20, 20, QtGui.QImage.Format_ARGB32)
        if color_group == PreferencesDialog.PLAY_COLORS:
            image.fill(self._settings.colors[color_index])
        elif color_group == PreferencesDialog.RESPONSE_COLORS:
            image.fill(self._settings.response_colors[color_index])

        button.setIcon(QtGui.QIcon(QtGui.QPixmap.fromImage(image)))
