#!/usr/bin/env python3
import csv
import argparse
import getpass
import logging
import sys
import json

from subprocess import Popen, PIPE
from distutils.spawn import find_executable

log = logging.getLogger(__name__)


class OpWrapper(object):
    LOGIN_UUID      = '001'
    CREDITCARD_UUID = '002'
    NOTE_UUID       = '003'
    PASSWORD_UUID   = '005'
    DOCUMENT_UUID   = '006'
    KEYVALUE_UUID   = '101'

    def __init__(self, op_path, signinaddress, emailaddress, secretkey, out):
        self.__op = op_path
        self.__session = None
        self.__out = out
        self.__signinaddress = signinaddress
        self.__emailaddress = emailaddress
        self.__secretkey = secretkey

    def __run(self, cmd, skip_session=False, instr=None):
        cmd.insert(0, self.__op)
        log.debug(cmd)
        if not skip_session:
            cmd.append(f'--session={self.__get_session()}')
        proc = Popen(cmd, shell=False, bufsize=0, close_fds=True, stdin=PIPE, stdout=PIPE, stderr=PIPE)
        out, err = proc.communicate(instr.encode('utf8') if instr else None)
        if err:
            log.warning(err.decode('utf8').strip())
        log.debug(out)
        if proc.returncode != 0:
            raise Exception(f'Execution failed! ({proc.returncode})')
        return out

    def __do_signin(self):
        cmd = [
            'signin', '--output=raw',
            self.__signinaddress, self.__emailaddress, self.__secretkey
        ]
        log.info('Signing in')
        passwd = getpass.getpass('1Password master password')
        return self.__run(cmd, skip_session=True, instr=passwd)

    def __get_session(self):
        if not self.__session:
            self.__session = self.__do_signin()
        return self.__session

    def get_vaults(self):
        cmd = ['list', 'vaults']
        vaults = json.loads(self.__run(cmd))
        return [v['name'] for v in vaults]

    def get_items(self, vault):
        cmd = ['list', 'items', f'--vault={vault}']
        return json.loads(self.__run(cmd))

    def get_login(self, vault, item):
        cmd = ['get', 'item', f'--vault={vault}', item]
        user, passwd = None, None
        for f in json.loads(self.__run(cmd))['details']['fields']:
            designation = f.get('designation', '')
            if designation == 'username':
                user = f['value']
            elif designation == 'password':
                passwd = f['value']
        return user, passwd

    def get_password(self, vault, item):
        cmd = ['get', 'item', f'--vault={vault}', item]
        return json.loads(self.__run(cmd))['details']['password']

    def __handle_login(self, vault, item):
        overview = item['overview']
        log.info(f'Got login {overview["title"]}')
        overview['vault'] = vault
        overview['user'], overview['password'] = self.get_login(vault, overview['title'])
        self.__out(overview)

    def __handle_password(self, vault, item):
        overview = item['overview']
        log.info(f'Got password {overview["title"]}')
        overview['vault'] = vault
        overview['user'], overview['password'] = None, self.get_password(vault, overview['title'])
        self.__out(overview)

    def dump(self):
        vaults = self.get_vaults()
        for v in vaults:
            log.info(f'Got vault {v}')
            items = self.get_items(v)
            for i in items:
                if i['templateUuid'] == self.LOGIN_UUID:
                    self.__handle_login(v, i)
                elif i['templateUuid'] == self.PASSWORD_UUID:
                    self.__handle_password(v, i)
                else:
                    log.warning(f'Ignoring item {i["overview"]["title"]} with unknown type {i["templateUuid"]}')


class CSVWriter(object):
    def __init__(self, out):
        self.__out = csv.DictWriter(
            out, fieldnames=['vault', 'title', 'url', 'user', 'password'], extrasaction='ignore'
        )

    def __call__(self, item):
        self.__out.writerow(item)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Backup 1password vaults')
    parser.add_argument(
        '-d', '--debug', action='store_true', default=False,
        help='Verbose logging'
    )
    parser.add_argument(
        'signinaddress', type=str,
        help='Signin address (i.e. my.1password.com)'
    )
    parser.add_argument(
        'emailaddress', type=str,
        help='E-Mail'
    )
    parser.add_argument(
        'secretkey', type=str,
        help='Secret account key'
    )

    opts = parser.parse_args()
    log_handler = logging.StreamHandler(sys.stderr)
    log_handler.setFormatter(logging.Formatter('%(lineno)03d %(levelname)s\t%(message)s'))
    log.addHandler(log_handler)
    if opts.debug:
        log.setLevel(logging.DEBUG)
    else:
        log.setLevel(logging.DEBUG)

    op_path = find_executable('op')
    if op_path is None:
        log.error('Could not find op in the PATH')
        sys.exit(1)
    log.debug(f'Using {op_path}')

    op_wrapper = OpWrapper(
        op_path,
        opts.signinaddress, opts.emailaddress, opts.secretkey,
        CSVWriter(sys.stdout)
    )
    op_wrapper.dump()
