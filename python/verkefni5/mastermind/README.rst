Mastermind game implemented in Qt5
========================================

Play a simple code breaking game.

Install
-------

To install you can use pip::

	$ pip install --user /path/to/mastermind-0.1.0-py3-none-any.whl

You will then need to install PyQt5 https://pypi.python.org/pypi/PyQt5
for example by::

	$ sudo aptitude install python3-pyqt5

Usage
-----

To play the game you must then run it from the command line

	$ mastermind

The computer will set up a code for you to break. To make a guess
click on each "hole" with the left mouse button. You will see a
drop-down menu with all the possible colors. Then click one of the
colors to select it. When the row has been filled and no black
holes remain you can click the Guess button. The computer will
evaluate your guess and give you a score in terms of colored pegs.
The default color values are a red peg for placing a pin of the
right color in the right hole and a white peg for placing a pin
of the right color in the wrong hole.
