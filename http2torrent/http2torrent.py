#!/usr/bin/env python
# Generates a .torrent file from an HTTP resource
import gfal2
import logging
import optparse
import os
import sys
import hashlib
from StringIO import StringIO
from urlparse import urlparse

import bencode

log = logging.getLogger(__name__)


def generate_torrent(src, piece_length, tracker):
    """
    Returns a torrent file generated from the HTTP source, with the given piece length
    """
    url_src = urlparse(src)
    filename = os.path.basename(url_src.path)

    ctx = gfal2.creat_context()
    log.debug("Stating %s" % src)
    stat = ctx.stat(src)
    log.info("Size %d" % stat.st_size)

    hash_chain = StringIO()

    # Poor man's approach
    # We could use threads to make this more efficient, but anyway, this is just
    # a prototype
    log.debug("Opening file")
    fd = ctx.open(src, 'r')
    buffer = fd.read(piece_length)
    total_read = 0
    while buffer:
        chunk_read = len(buffer)
        total_read += chunk_read
        log.debug("Read %d (%d)", total_read, chunk_read)
        hash_chain.write(hashlib.sha1(buffer).digest())
        buffer = fd.read(piece_length)

    torrent = {
        'announce': tracker,
        'url-list': src,
        'info': {
            'name': filename,
            'length': stat.st_size,
            'piece length': piece_length,
            'pieces': hash_chain.getvalue(),
        }
    }
    return bencode.bencode(torrent)


if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-d', '--debug', default=False, action='store_true', help='Debug output')
    parser.add_option('-b', '--block-size', type=int, default=2**20, help='Block size')
    parser.add_option('-t', '--tracker', type=str, default='udp://tracker.openbittorrent.com:80/announce')

    options, args = parser.parse_args()
    if len(args) != 1:
        parser.error('Expected an http resource')

    handler = logging.StreamHandler(sys.stderr)
    handler.setFormatter(logging.Formatter("%(levelname)-10s %(message)s"))
    log.addHandler(handler)
    if options.debug:
        log.setLevel(logging.DEBUG)

    print >>sys.stdout, generate_torrent(args[0], options.block_size, options.tracker)
