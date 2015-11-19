import healthybeard.repository

import argparse
import os.path
import sys


def exit_error(*args, **kw):
    kw['file'] = sys.stderr
    print(*args, **kw)
    sys.exit(-1)


# ----------------------------------------------------------------------------
def cmd_init_parse(parser, parent):
    init_parse = parent.add_parser('init', help='Initialize a Healthy Beard repository')

    init_parse.add_argument(
        'path',
        default=os.path.abspath('.'),
        help='Path to the repository directory. (Defaults to current working directory)')
    init_parse.add_argument(
        '-f', '--force',
        action='store_true',
        default=False,
        help='Overwrite any existing config files')


def cmd_init(args):
    repo = healthybeard.repository.Repository()

    if not os.path.exists(args.path):
        os.makedirs(args.path)

    if not os.path.isdir(args.path):
        exit_error('The given path is not a directory')

    try:
        repo.create(args.path, args.force)
    except healthybeard.repository.RepositoryAlreadyExistsError:
        exit_error('There already exists a Healthy Beard repository at that location.\nUse --force to overwrite.')
    except:
        exit_error('An unknown error occurred:\n{0}'.format(sys.exc_info()[0]))


# ----------------------------------------------------------------------------
def cmd_run_parse(parser, parent):
    run_parse = parent.add_parser('run', help='Execute the file filters')

    run_parse.add_argument(
        'path',
        default=os.path.abspath('.'),
        help='Path to the repository directory. (Defaults to current working directory)')


def cmd_run(args):
    if not (os.path.exists(args.path) and os.path.isdir(args.path)):
        exit_error('The repository path does not exist or is not a directory')

    repo = healthybeard.repository.Repository()

    try:
        if repo.open(args.path):
            repo.apply_filters()
        else:
            exit_error('Unable to open Healthy Beard repository')
    except:
        exit_error('An unknown error occurred:\n{0}'.format(sys.exc_info()[0]))


# ----------------------------------------------------------------------------
def cmd_monitor_parse(parse, parent):
    monitor_parse = parent.add_parser('monitor', help='Run Healthy Beard as a service that monitors the repository')

    monitor_parse.add_argument(
        'path',
        default=os.path.abspath('.'),
        help='Path to the repository directory. (Defaults to current working directory)')


def cmd_monitor(args):
    exit_error('This feature has not been implemented yet')

    if not (os.path.exists(args.path) and os.path.isdir(args.path)):
        exit_error('The repository path does not exist or is not a directory')

    repo = healthybeard.repository.Repository()

    try:
        if repo.open(args.path):
            repo.monitor()
        else:
            exit_error('Unable to open Healthy Beard repository')
    except:
        exit_error('An unknown error occurred:\n{0}'.format(sys.exc_info()[0]))


# ----------------------------------------------------------------------------
def parse_args():
    parser = argparse.ArgumentParser(prog='hbrd', description='Healthy Beard')
    cmd_subparsers = parser.add_subparsers(dest='command', help='sub-command help')

    cmd_init_parse(parser, cmd_subparsers)
    cmd_run_parse(parser, cmd_subparsers)
    cmd_monitor_parse(parser, cmd_subparsers)

    args = parser.parse_args()
    if not args.command:
        parser.print_help()
        parser.exit(-1)

    return args


def main():
    args = parse_args()

    if args.command == 'init':
        cmd_init(args)
    elif args.command == 'run':
        cmd_run(args)
    elif args.command == 'monitor':
        cmd_monitor(args)
    else:
        print('wtf')


if __name__ == '__main__':
    main()
