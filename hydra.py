#!/usr/bin/env python
import HTMLParser
import math
import os
import pycurl
import socket
import sys
import threading
import time

class Stomach(HTMLParser.HTMLParser):
  """
  Processor
  """

  def __init__(self, master):
    self.master = master
    self.base   = ""

  def handle_starttag(self, tag, attrs):
    if tag == "a":
      for (attr, value) in attrs:
        if attr == "href":
          if value != ".." and value != ".":
            url = self.base + value
            self.master.give(url)

class Head(threading.Thread):
  """
  Downloader
  """

  def __init__(self, id, master):
    threading.Thread.__init__(self)
    self.master  = master
    self.id      = id
    self.parser  = Stomach(master)
    self.html    = False
    # Curl
    self.curl = pycurl.Curl()
    self.curl.setopt(pycurl.FOLLOWLOCATION, 1)
    self.curl.setopt(pycurl.HEADERFUNCTION, self.header_callback)
    self.curl.setopt(pycurl.WRITEFUNCTION, self.write_callback)
    self.curl.setopt(pycurl.SSL_VERIFYPEER, 0)
    if master.uKey:
      self.curl.setopt(pycurl.SSLKEY, master.uKey)
    if master.uCert:
      self.curl.setopt(pycurl.SSLCERT, master.uCert)
    # Idle
    self.idle = True

  def header_callback(self, buf):
    if ':' in buf:
      (key, value) = buf.split(':', 1)
      if key.lower() == "content-type":
        self.html = "text/html" in value.lower()

  def write_callback(self, buf):
    if self.html:
      self.parser.feed(buf)

  def run(self):
    url = self.master.next()

    while url is not None:      
      self.idle = False
      self.curl.setopt(pycurl.URL, url)
      try:
        self.parser.reset()
        self.parser.base = url
        self.curl.perform()
        self.parser.close()
        # Stats
        self.master.done(self.curl.getinfo(pycurl.EFFECTIVE_URL),
                         self.curl.getinfo(pycurl.HTTP_CODE),
                         self.curl.getinfo(pycurl.SIZE_DOWNLOAD),
                         self.curl.getinfo(pycurl.TOTAL_TIME))
      except pycurl.error, e:
        if e[0] == 28:
          self.master.done(url, 28, 0, 0)
        else:
          print >>sys.stderr, str(e)
          self.master.done(url, 0, 0, 0)
      except Exception, e:
        print >>sys.stderr, str(e)
        self.master.done(url, 0, 0, 0)
      self.idle = True
      url = self.master.next()

    return

class Master:
  """
  Keep a list of resources
  """

  def __init__(self, base, cert, key, max):
    self.uCert    = cert
    self.uKey     = key
    self.maxHeads = max
    self.success  = False
    # Queue
    self.urlQueue  = [base]
    self.processed = []
    # Synchronization
    self.whip   = threading.Semaphore(1)
    self.finish = False
    # Heads
    self.heads = []
    # Logging
    self.log    = open("hydra-%s.log" % socket.getfqdn(), "w")
    self.passed = 0
    self.failed = 0
    self.overloaded = 0
    self.totalSize = 0

  def next(self):
    self.whip.acquire()
    if self.finish:
      return None
    url = self.urlQueue.pop(0)
    self.processed.append(url)
    return url

  def give(self, url):
    if url not in self.processed:
      self.urlQueue.append(url)
      self.whip.release()

  def done(self, url, status, size, time):
    print >>self.log, "[%03d:%d:%d] %s" % (status, size, time, url)
    self.log.flush()
    if status >= 200 and status <= 300:
      self.passed    = self.passed + 1
      self.totalSize = self.totalSize + size
    elif status == 503 or status == 28: # Including SSL timeouts!
      self.overloaded = self.overloaded + 1
    else:
      self.failed = self.failed + 1

  def run(self):

    try:
      # Spawn all the heads
      for i in xrange(self.maxHeads):
        h = Head(i, self)
        h.start()
        self.heads.append(h)

      # Monitor activity
      idle  = False
      start = time.time()
      while not idle and not self.finish:
        # Check there is activity
        idle  = True
        nIdle = 0
        for head in self.heads:
          if head.idle: nIdle = nIdle + 1
        idle = (nIdle == self.maxHeads)
        # Info
        elapsed = time.time() - start
        secs = elapsed % 60
        mins = math.floor(elapsed / 60)
        print >>sys.stdout, """\
\r%02d:%02d - %d heads idling, %d URL's int the queue, \
%d passed, %d failed, %d rejected due to overload, \
%.2fMB downloaded\
"""  % (mins, secs, nIdle, len(self.urlQueue),
        self.passed, self.failed, self.overloaded,
        self.totalSize / 1048576),
        sys.stdout.flush()
        # Wait
        if not idle:
          time.sleep(1)

    except KeyboardInterrupt:
      print "Aborting!"

    # Finish
    self.finish = True
    print
    print "Killing heads..."
    for head in self.heads:
      self.whip.release()

    # Wait
    print "Waiting heads to die..."
    for head in self.heads:
      head.join()

    # End
    return


# Usage
def usage():
  print("""\
%s (options) <url>
\t-h, --help  Shows this message.
\t-c, --cert  User certificate.
\t-k, --key   User private key.
\t-m, --max   Maximum number of threads.
  """ % sys.argv[0])

# Entry point
if __name__ == "__main__":
  import getopt

  # Defaults
  if "X509_USER_PROXY" in os.environ:
    uCert = uKey = os.environ["X509_USER_PROXY"]
  elif "X509_USER_CERT" in os.environ and "X509_USER_KEY" in os.environ:
    uCert = os.environ["X509_USER_CERT"]
    uKey  = os.environ["X509_USER_KEY"]
  else:
    uCert = uKey = None

  maxClients = 5

  # Fetch options
  try:
    (opts, args) = getopt.getopt(sys.argv[1:], "hc:l:m:", ["help", "cert=", "key=", "max="])

    for (opt, arg) in opts:
      if opt in ('-h', '--help'):
        usage()
        sys.exit(0)
      elif opt in ('-c', '--cert'):
        uCert = arg
      elif opt in ('-k', '--key'):
        uKey = arg
      elif opt in ('-m', '--max'):
        maxClients = int(arg)

    if len(args) == 0:
      raise getopt.GetoptError("Need a base URL")
    elif len(args) > 1:
      raise getopt.GetoptError("Too many arguments")

    baseUrl = args[0]

  except getopt.GetoptError, error:
    print >>sys.stderr, str(error)
    sys.exit(2)
  except ValueError:
    print >>sys.stderr, "Invalid value passed as parameter"
    sys.exit(2)

  # Got all we need, so instantiate the Master
  master = Master(baseUrl, uCert, uKey, maxClients)
  master.run()

  if master.success == True:
    sys.exit(0)
  else:
    sys.exit(1)
