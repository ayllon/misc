#!/usr/bin/env python
import argparse
import sys
import subprocess
import tempfile
from lxml import etree
from StringIO import StringIO


def run_gccxml(path, includes):
    tempFile = tempfile.NamedTemporaryFile('r')
    cmdArray = ['gccxml', path, '-fxml=' + tempFile.name]
    for i in includes:
        cmdArray += ['-I', i]
    proc = subprocess.Popen(cmdArray, stdin=None, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    code = proc.wait()
    if code != 0:
        print proc.stderr.read()
        raise Exception('Could not parse ' + path)
    return tempFile.read()


def get_file_id(xml, path):
    matching = xml.xpath("File[@name='%s']" % args.path)
    if len(matching) == 0:
        raise Exception('Source file not found in the intermediate xml')
    fileNode = matching[0]
    return fileNode.get('id')


def get_klass_id(xml, file_id, klass):
    matching = xml.xpath("Class[@name='%s' and @file='%s']" % (klass, file_id))
    if len(matching) == 0:
        raise Exception("Can not find the class %s::%s in the intermediate xml" % (file_id, klass))
    klassNode = matching[0]
    return klassNode.get('id')


def get_klass_methods(xml, klass_id):
    methods = xml.xpath("Method[@context='%s']" % klass_id)
    for m in methods:
        yield m.get('name')


def get_functions(xml, file_id):
    functions = xml.xpath("Function[@file='%s']" % file_id)
    for f in functions:
        yield f.get('name')


def get_methods(xml, file_id, klass=None):
    if klass:
        klass_id = get_klass_id(xml, file_id, klass)
        return get_klass_methods(xml, klass_id)
    else:
        return get_functions(xml, file_id)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate method list from a header file')
    parser.add_argument('path', help='file to parse')
    parser.add_argument('-I', dest='includes', action='append', help='Include directories')
    parser.add_argument('-C', dest='klass', help='Limit scope to methods from a class')
    args = parser.parse_args()

    xml_raw = run_gccxml(args.path, args.includes)
    xml = etree.parse(StringIO(xml_raw))

    file_id = get_file_id(xml, args.path)

    methods = get_methods(xml, file_id, args.klass)

    for m in methods:
        print m

