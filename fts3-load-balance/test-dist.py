#!/usr/bin/env python
import hashlib
import sys

def hexdump(bytes):
    str = ''
    for b in bytes:
        str += "%02x" % ord(b)
    return str


def hash(id):
    m = hashlib.sha1()
    m.update(str(id))
    full_hash = hexdump(m.digest())
    return full_hash[:4]


def hash_segment(index, total):
    MAX = 0xFFFFFF
    MIN = 0x000000
    segsize = (MAX - MIN) / total
    segmod  = (MAX - MIN) % total;

    start = MIN + segsize * index
    end   = MIN + segsize * (index + 1) - 1
    # The last one take over what is left
    if index == total - 1:
        end += segmod + 1

    limits = ("%06x" % start, "%06x" % end)
    return limits


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
    id_hashes.append(hash(i))


# Simulate for n hosts
print "Simulating with %d hosts" % nhosts

totalCount = 0
for i in range(nhosts):
    host = "fts%03d.cern.ch" % i
    segment = hash_segment(i, nhosts)

    # Count how many fit
    count = len(filter(lambda id: segment[0] <= id and id <= segment[1], id_hashes))
    print "\tHost %s with hash interval [%s, %s]: %d elements" % (host, segment[0], segment[1], count)
    totalCount += count

assert (totalCount == nids)

