import mpsort.sorter
import argparse
import os.path
import sys


def main():
    parser = argparse.ArgumentParser(description='A utility to sort and rename mp3 files using id3 tag data.')

    parser.add_argument('source', help='The directory containing the original files')
    parser.add_argument('destination', help='The directory that will contain the organized files')

    args = parser.parse_args()

    if not os.path.exists(args.source):
        print('Source directory does not exist', file=sys.stderr)
        sys.exit(-1)

    if not os.path.isdir(args.source):
        print('Source is not a directory', file=sys.stderr)
        sys.exit(-1)

    if not os.path.exists(args.destination):
        os.makedirs(args.destination)

    if not os.path.isdir(args.destination):
        print('Destination is not a directory', file=sys.stderr)
        sys.exit(-1)

    sorter = mpsort.sorter.Sorter()
    sorter.sort(os.path.abspath(args.source), os.path.abspath(args.destination))


if __name__ == '__main__':
    main()
