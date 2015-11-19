A utility to sort and rename mp3 files using id3 tag data
=========================================================

This is a tool to rename and sort mp3 files using the embedded ID3 tags.

Install
-------

To install you can use pip::

	$ pip install --user -r /path/to/requirements.txt
	$ pip install --user /path/to/mpsort-0.1.0-py3-none-any.whl

You should either use a virtualenv or check whether requirements.txt
will clobber anything you have installed already.

Usage
-----

mpsort takes two arguments. The source directory and the destination
directory. If the destination directory exists the new files will be
merged into it.::

	$ mpsort /path/to/unsorted/files /destination/path

It will copy the files over, renaming them according to their ID3 tag
data. If a tag is missing it will be replaced with a placeholder value
e.g. 'Unknown Artist', 'Unknown Album'. If there is no title tag the
old file name will be used. The files will be sorted into a structure
like the following.::

	{destination_path}/{artist_name}/{album_name}/{track_number} - {title}.mp3
