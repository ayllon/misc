#!/usr/bin/env python
import hashlib
import sys
from datetime import datetime


def hexdump(bytes):
    str = ''
    for b in bytes:
        str += "%02x" % ord(b)
    return str


def hash(id):
    m = hashlib.md5()
    m.update(str(id))
    full_hash = hexdump(m.digest())
    return full_hash[:8]


def hash_segment(index, total):
    MAX = 0xFFFFFFFF
    MIN = 0x00000000
    segsize = (MAX - MIN) / total
    segmod = (MAX - MIN) % total

    start = MIN + segsize * index
    end = MIN + segsize * (index + 1) - 1
    # The last one take over what is left
    if index == total - 1:
        end += segmod + 1

    limits = ("%08x" % start, "%08x" % end)
    return limits


def mock_hash(index):
    source = 'http://something.com/path'
    dest   = 'http://something.com/dest/' + str(index % 50)
    now = datetime.utcnow()

    hashable = ''.join([source, dest, str(now)])
    return  hash(hashable)


# From args
if len(sys.argv) < 3:
    print "Missing parameters"
    print "Usage: %s <number of ids> <number of hosts>" % sys.argv[0]
    sys.exit(1)


nids = int(sys.argv[1])
nhosts = int(sys.argv[2])

# Generate a bunch of hashes and see how does it distribute
print "Generating hashes"
id_hashes = []
for i in range(nids):
    id_hashes.append(mock_hash(i))


# Simulate for n hosts
print "Simulating with %d hosts" % nhosts

totalCount = 0
for i in range(nhosts):
    host = "fts%03d.cern.ch" % i
    segment = hash_segment(i, nhosts)

    # Count how many fit
    count = len(filter(lambda id: segment[0] <= id <= segment[1], id_hashes))
    print "\tHost %s with hash interval [%s, %s]: %d elements"\
        % (host, segment[0], segment[1], count)
    totalCount += count

assert (totalCount == nids)
