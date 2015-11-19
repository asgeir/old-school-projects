A utility to keep track of pirated files
========================================

This is a tool to manage a download directory.

Install
-------

To install you can use pip::

	$ pip install --user /path/to/healthybeard-0.1.0-py3-none-any.whl

Usage
-----

The included tools are:

	* hbrd - This is the main tool. Call it with --help for instructions.

pip should have placed the tools somewhere in your path, if not
you will need to consult the pip documentation.

hbrd
----

This is the main tool. To configure its behaviour you must edit
a file called hbrd.yaml located in your repository. If you have no such
file and want a default file you can use the *init* command.
A more detailed sample can be seen under doc/hbrd.sample.yaml

Commands
~~~~~~~~

init
    This command initializes a new Healthy Beard repository. If no path
    is given it will assume the current working directory. You can use
    *--force* to forece it to overwrite an existing repository.

run
    This command will apply the ruleset given in the hbrd.yaml file
    to the repository.

Config File Format
~~~~~~~~~~~~~~~~~~

The YAML file consists of a single map. The top-level nodes are *settings*
and *rules*. If you need information on the YAML file-format in general
you may refer to http://www.yaml.org/spec/1.2/spec.html

*settings* contains general configuration such as the name of
the directory containing the sorted files *repository*. The next crucial
element it contains is *categories* which allows you to set up general
categories into which you can sort your files. And finally it may
contain *default* settings for the entire repository.

You may use any category names you wish and as many as you wish.
The categories must contain their relative path under the *repository*
directory, but may also contain settings and an optional priority.
Categories may also override global *default* settings.

*rules* contains glob and regex filters for your categories. If you
want to further categorize your files such as TV series into separate
directories you need to create a map. Otherwise a list is fine.

A rule node may contain two keys: Optional *settings* for rule-specific
settings, and required *filters* for a list of glob and/or regex nodes.

glob filters are very simple and consist of a map with a single key
*glob* which has a string value containing the glob pattern.

regex filters consist of a map with a single key *regex* which
contains a sub-map. The sub-map must include a *pattern* key which
holds the regular expression that this filter matches. It may include
an optional *flags* list. The allowed flags are *a* for ASCII-only
matching, *i* for case-insensitive matching, and *x* to allow you
to use verbose expressions. For more information see
https://docs.python.org/3/library/re.html

regex filters may also include a *rename* key. This allows you
to use Python string formatting to rename the files. If you
use named matches in your regex pattern you may use those names
to substitute values in the rename string template.
