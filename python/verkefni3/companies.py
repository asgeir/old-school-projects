import http.client
import json


def companies(day, month, year):
    conn = http.client.HTTPConnection('apis.is')

    month = '{0:02d}'.format(month)
    year = '{0:02d}'.format(year)[-2:]
    conn.request('GET', '/company?socialnumber={0}{1}{2}'.format(day+40, month, year))
    r1 = conn.getresponse()

    if r1.status != 200:
        return []

    companies = json.loads(r1.read().decode('utf-8'))['results']

    return [c['name'] for c in companies if c.get('active') if c.get('name')]


if __name__ == '__main__':
    print(companies(13, 1, 1998))
