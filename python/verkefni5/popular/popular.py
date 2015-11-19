#!/usr/bin/env python3
import argparse
import datetime
from html.parser import HTMLParser
import http.client
import sys


class TopListHTMLParser(HTMLParser):
    def __init__(self):
        super().__init__()

        self.state = 'ready'
        self.tmp_url = None
        self.tmp_title = None
        self.tmp_date = None

        self.titles = []

    def handle_starttag(self, tag, attrs):
        if self.state == 'ready' and tag == 'td':
            attrs = dict(attrs)
            if 'titleColumn' in attrs.get('class', []):
                self.state = 'title-found'
        elif self.state == 'title-found' and tag == 'a':
            attrs = dict(attrs)
            self.tmp_url = attrs.get('href', '<<INVALID_ANCHOR_LINK>>')

            idx = self.tmp_url.rfind('/?')
            if idx > -1:
                self.tmp_url = self.tmp_url[:idx]

            self.state = 'link-found'
        elif self.state in ['title-found', 'link-found'] and tag == 'span':
            attrs = dict(attrs)

            if attrs.get('name', 'not-interested') == 'rd':
                self.tmp_date = attrs.get('data-value', '?')

                if self.tmp_date != '?':
                    try:
                        self.tmp_date = datetime.date(*[int(x) for x in self.tmp_date.split('-')])
                    except ValueError:
                        print('Invalid date value: {0}'.format(self.tmp_date))

    def handle_endtag(self, tag):
        if self.state == 'link-found' and tag == 'a':
            self.state = 'title-found'
        elif self.state == 'title-found' and tag == 'td':
            if self.tmp_url and self.tmp_title and self.tmp_title:
                self.titles.append((self.tmp_title, self.tmp_date, self.tmp_url))
                self.tmp_url = None
                self.tmp_title = None
                self.tmp_date = None
            else:
                print('malformed entry {0} {1} {2}'.format(self.tmp_title, self.tmp_url, self.tmp_date))

            self.state = 'ready'

    def handle_data(self, data):
        if self.state == 'link-found':
            self.tmp_title = data


class CastHTMLParser(HTMLParser):
    def __init__(self):
        super().__init__()

        self.state = 'ready'
        self.tmp_name = None

        self.actors = []

    def handle_starttag(self, tag, attrs):
        if self.state == 'ready' and tag == 'table':
            attrs = dict(attrs)
            if 'cast_list' in attrs.get('class', []):
                self.state = 'list-found'
        elif self.state == 'list-found' and tag == 'td':
            attrs = dict(attrs)
            if 'itemprop' in attrs.get('class', []) and 'actor' == attrs.get('itemprop', None):
                self.state = 'actor-found'
        elif self.state == 'actor-found' and tag == 'span':
            attrs = dict(attrs)
            if 'name' == attrs.get('itemprop', None):
                self.state = 'actor-name-found'

    def handle_endtag(self, tag):
        if self.state == 'actor-name-found' and tag == 'span':
            if self.tmp_name:
                self.actors.append(self.tmp_name)
                self.tmp_name = None

            self.state = 'ready'

    def handle_data(self, data):
        if self.state == 'actor-name-found':
            self.tmp_name = data


def fetch_cast(title):
    conn = http.client.HTTPConnection('www.imdb.com')

    conn.request('GET', title + '/fullcredits')
    r1 = conn.getresponse()

    if r1.status != 200:
        return []

    parser = CastHTMLParser()
    parser.feed(r1.read().decode('utf-8'))

    return parser.actors


def fetch_top_actors(start_year=None, end_year=None):
    conn = http.client.HTTPConnection('www.imdb.com')

    conn.request('GET', '/chart/top')
    r1 = conn.getresponse()

    if r1.status != 200:
        return []

    parser = TopListHTMLParser()
    parser.feed(r1.read().decode('utf-8'))

    titles = parser.titles
    if start_year:
        start_year = datetime.date(start_year, 1, 1)

        if end_year:
            end_year = datetime.date(end_year + 1, 1, 1)
        else:
            end_year = datetime.date(datetime.MAXYEAR, 1, 1)

        titles = [(title, date, url) for (title, date, url) in titles if date >= start_year and date < end_year]

    actors = {}

    print('Working', end='', file=sys.stderr, flush=True)
    for title, _, url in titles:
        cast = fetch_cast(url)
        for actor in cast:
            actors[actor] = actors.setdefault(actor, 0) + 1

        print('.', end='', file=sys.stderr, flush=True)

    print('', file=sys.stderr, flush=True)
    return [(count, name) for (count, name) in reversed(sorted([(count, name) for (name, count) in actors.items()]))]


def main():
    parser = argparse.ArgumentParser(description='Find top actors.')
    parser.add_argument('-b', '--start', dest='start', type=int, default=0, help='start year')
    parser.add_argument('-e', '--end', dest='end', type=int, default=0, help='end year (inclusive)')
    parser.add_argument('-n', '--limit', dest='limit', type=int, default=0, help='max number of actors to display')

    args = parser.parse_args()
    actors = fetch_top_actors(args.start, args.end)
    if args.limit:
        actors = actors[:args.limit]

    print('Number of Movies\tName')
    for count, name in actors:
        print('\t{0}\t\t{1}'.format(count, name))


if __name__ == '__main__':
    main()
