#!/usr/bin/env python3

import os
import subprocess
import time
import unittest


def pathToExample(name):
    build = os.getenv('MESON_BUILD_ROOT', os.getcwd())
    example = os.path.join(build, name)
    if not os.path.exists(example):
        raise RuntimeError('{} not found!'.format(name))
    return example


def runAndCheck(command):
    return str(
        subprocess.run(
            command,
            check=True,
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE,
            timeout=10).stdout, 'ascii')


class SmokeTest(unittest.TestCase):
    '''High level smoke tests for the network stack.

    These should be run via ninja smoke from your meson build directory.
    '''

    def testTap(self):
        runAndCheck(pathToExample('tap'))

    def testArping(self):
        runAndCheck(pathToExample('arping'))


if __name__ == '__main__':
    unittest.main()
