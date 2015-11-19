import glob
import os
import posixpath
import re

from yaml import load as yaml_load, dump as yaml_dump
try:
    from yaml import CLoader as YAMLLoader, CDumper as YAMLDumper
except ImportError:
    from yaml import Loader as YAMLLoader, Loader as YAMLDumper


CONFIG_FILE = 'hbrd.yaml'


class RepositoryAlreadyExistsError(Exception):
    pass


class CategoryMissingDirectory(Exception):
    def __init__(self, category):
        super().__init__()
        self.category = category


class InvalidFilterType(Exception):
    def __init__(self, category, rule, keys):
        super().__init__()
        self.category = category
        self.rule = rule
        self.keys = keys


class InvalidRegexFilter(Exception):
    def __init__(self, category, rule):
        super().__init__()
        self.category = category
        self.rule = rule


class Repository(object):
    def __init__(self):
        super().__init__()

        self._path = None
        self._rules = {}
        self._settings = {
            'repository': 'Sorted',
            'default': {
                'delete': [
                    {'glob': '*.torrent'},
                    {'glob': '*.nfo'}
                ]
            },
            'categories': []
        }

    def create(self, repo_path, override=False):
        config_file_name = posixpath.join(repo_path, CONFIG_FILE)

        if posixpath.exists(config_file_name) and not override:
            raise RepositoryAlreadyExistsError()

        try:
            os.remove(config_file_name)
        except FileNotFoundError:
            # don't care if the file exists or not
            pass

        with open(config_file_name, 'w') as f:
            f.write(yaml_dump({'settings': self._settings, 'rules': self._rules}, Dumper=YAMLDumper))

    def open(self, repo_path):
        config_file_name = posixpath.join(repo_path, CONFIG_FILE)

        try:
            self._load_rules(config_file_name)
        except FileNotFoundError:
            self._path = None
            return False

        self._path = repo_path
        return True

    def apply_filters(self):
        if not self._path:
            return False

        categories = sorted([
            (cat.get('priority', 9999), name, cat)
            for name, cat in self._settings.get('categories', {}).items()])
        for _, cat_name, cat in categories:
            cat_settings = self._settings.get('default', {}).copy()
            cat_settings.update(cat.get('default', {}))

            cat_path = None
            try:
                cat_path = posixpath.join(self._repo_path, cat['directory'])
            except KeyError:
                raise CategoryMissingDirectory(cat_name)

            cat_rules = self._rules.get(cat_name, {})
            if type(cat_rules) is dict:
                for rule_name, rule in cat_rules.items():
                    self._apply_rule(rule_name, rule, cat_name, cat_path, cat_settings, True)
            else:
                self._apply_rule(cat_name, {'filters': cat_rules}, cat_name, cat_path, cat_settings, False)

    def monitor():
        print('monitor')

    @property
    def _repo_path(self):
        return posixpath.abspath(posixpath.join(self._path, self._settings['repository']))

    def _apply_rule(self, rule_name, rule, cat_name, cat_path, cat_settings, named):
        rule_settings = cat_settings.copy()
        rule_settings.update(rule.get('settings', {}))

        rule_path = cat_path
        if named:
            rule_path = posixpath.join(cat_path, rule_name)

        for filter in rule['filters']:
            actions = []

            if 'regex' in filter:
                filter = filter['regex']
                actions = self._apply_regex_rule(rule_name, filter, cat_name, rule_path, rule_settings)
            elif 'glob' in filter:
                filter = filter['glob']
                actions = self._apply_glob_rule(filter, rule_path, rule_settings)
            else:
                raise InvalidFilterType(cat_name, rule_name, filter.keys())

            self._execute_action_list(actions)

    def _apply_regex_rule(self, rule_name, rule, cat_name, cat_path, settings):
        accepted_flags = {
            'a': re.ASCII,
            'i': re.IGNORECASE,
            'l': re.LOCALE,
            'x': re.VERBOSE
        }
        flags = sum([accepted_flags[f] for f in rule.get('flags', [])])

        pattern = None
        try:
            pattern = re.compile(rule['pattern'], flags)
        except KeyError:
            raise InvalidRegexFilter(cat_name, rule_name)

        actions = []

        rename = rule.get('rename', None)
        for root, dirs, files in os.walk(self._path):
            if posixpath.abspath(root) == self._repo_path:
                continue

            for file_name in files:
                file_name = posixpath.relpath(posixpath.join(root, file_name), self._path)

                match = pattern.match(file_name)
                if match:
                    new_name = file_name
                    if rename:
                        new_name = rename.format(**match.groupdict())

                    new_name = posixpath.join(cat_path, new_name)
                    actions.append(('mv', posixpath.join(self._path, file_name), new_name))

        return actions

    def _apply_glob_rule(self, rule, cat_path, settings):
        actions = []

        file_names = glob.glob(posixpath.join(self._path, rule))
        for old_name in file_names:
            new_name = posixpath.join(cat_path, posixpath.basename(old_name))
            actions.append(('mv', old_name, new_name))

        return actions

    def _execute_action_list(self, actions):
        for action, old_name, new_name in actions:
            if action == 'mv':
                os.renames(old_name, new_name)
            elif action == 'rm':
                os.remove(old_name)

    def _load_rules(self, file_path):
        config = {}

        with open(file_path, 'r') as config_stream:
            config = yaml_load(config_stream, Loader=YAMLLoader)

        self._settings = config.get('settings', self._settings)
        self._rules = config.get('rules', self._rules)
