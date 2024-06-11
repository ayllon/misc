#!/usr/bin/env python3
import csv
import argparse
import logging
import sys
import json

from subprocess import Popen, PIPE
from shutil import which

log = logging.getLogger(__name__)


class OpWrapper(object):
    CAT_LOGIN = "LOGIN"
    CAT_CREDITCARD = "CREDIT_CARD"
    CAT_NOTE = "SECURE_NOTE"
    CAT_PASSWORD = "PASSWORD"
    CAT_DOCUMENT = "DOCUMENT"

    def __init__(self, op_path, out):
        self.__op = op_path
        self.__out = out

    def __run(self, cmd):
        cmd.insert(0, self.__op)
        log.debug(cmd)
        proc = Popen(
            cmd,
            shell=False,
            bufsize=0,
            close_fds=True,
            stdin=PIPE,
            stdout=PIPE,
            stderr=PIPE,
        )
        out, err = proc.communicate()
        if err:
            log.warning(err.decode("utf8").strip())
        log.debug(out)
        if proc.returncode != 0:
            raise Exception(f"Execution failed! ({proc.returncode})")
        return out

    def get_vaults(self):
        cmd = ["vault", "list", "--format=json"]
        vaults = json.loads(self.__run(cmd))
        return [v["name"] for v in vaults]

    def get_items(self, vault):
        cmd = ["items", "list", f"--vault={vault}", "--format=json"]
        return json.loads(self.__run(cmd))

    def get_login(self, vault, item):
        cmd = ["item", "get", f"--vault={vault}", "--format=json", item]
        user, passwd = None, None
        for f in json.loads(self.__run(cmd))["fields"]:
            designation = f.get("id", "")
            if designation == "username":
                user = f.get("value", "")
            elif designation == "password":
                passwd = f.get("value", "")
        return user, passwd

    def get_password(self, vault, item):
        cmd = ["item", "get", f"--vault={vault}", "--format=json", item]
        for f in json.loads(self.__run(cmd))["fields"]:
            if f.get("id") == "password":
                return f.get("value")
        return None

    def __handle_login(self, vault, item):
        log.info(f'Got login {item["title"]}')
        item["vault"] = vault
        item["user"], item["password"] = self.get_login(vault, item["id"])
        item["url"] = item.get("urls", [{"href": ""}])[0].get("href", "")
        self.__out(item)

    def __handle_password(self, vault, item):
        log.info(f'Got password {item["title"]}')
        item["vault"] = vault
        item["user"], item["password"] = None, self.get_password(vault, item["id"])
        self.__out(item)

    def dump(self):
        vaults = self.get_vaults()
        for v in vaults:
            log.info(f"Got vault {v}")
            items = self.get_items(v)
            for i in items:
                if i["category"] == self.CAT_LOGIN:
                    self.__handle_login(v, i)
                elif i["category"] == self.CAT_PASSWORD:
                    self.__handle_password(v, i)
                else:
                    log.warning(
                        f'Ignoring item {i["title"]} with unknown type {i["category"]}'
                    )


class CSVWriter(object):
    def __init__(self, out):
        self.__out = csv.DictWriter(
            out,
            fieldnames=["vault", "title", "url", "user", "password"],
            extrasaction="ignore",
        )

    def __call__(self, item):
        self.__out.writerow(item)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Backup 1password vaults")
    parser.add_argument(
        "-d", "--debug", action="store_true", default=False, help="Verbose logging"
    )

    opts = parser.parse_args()
    log_handler = logging.StreamHandler(sys.stderr)
    log_handler.setFormatter(
        logging.Formatter("%(lineno)03d %(levelname)s\t%(message)s")
    )
    log.addHandler(log_handler)
    if opts.debug:
        log.setLevel(logging.DEBUG)
    else:
        log.setLevel(logging.DEBUG)

    op_path = which("op")
    if op_path is None:
        log.error("Could not find op in the PATH")
        sys.exit(1)
    log.debug(f"Using {op_path}")

    op_wrapper = OpWrapper(
        op_path,
        CSVWriter(sys.stdout),
    )
    op_wrapper.dump()
