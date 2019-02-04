#!/usr/bin/env python3

import contextlib
import os
import subprocess
import time
import unittest

DEV_ETH = '06:11:22:33:44:55'
DEV_IP = '10.255.255.102'
GATEWAY_IP = '10.255.255.101'
UNKNOWN_IP = '10.255.255.103'
GOOGLE_DNS_IP = '8.8.8.8'


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


@contextlib.contextmanager
def runNetworkStack():
    process = subprocess.Popen(pathToExample('stack'))
    time.sleep(1)
    err = None
    if process.poll() is not None:
        raise RuntimeError('Network stack could not be started!')
    try:
        yield
    except Exception as ex:
        err = ex
    process.kill()
    process.wait()
    if err is not None:
        raise err


class SmokeTest(unittest.TestCase):
    '''High level smoke tests for the network stack.

    These should be run via ninja smoke from your meson build directory.
    '''

    def testTap(self):
        runAndCheck(pathToExample('tap'))

    def testArpingReply(self):
        with runNetworkStack():
            stdout = runAndCheck(
                ['sudo', 'arping', DEV_IP, '-i', 'tap0', '-c', '1'])
            self.assertIn(DEV_ETH, stdout)

    def testArpingGateway(self):
        stdout = runAndCheck(
            [pathToExample('arping'), '--addr', GATEWAY_IP, '--count', '4'])
        self.assertRegex(
            stdout,
            ('28 bytes from \S{17} \(10.255.255.101\) index=1 time=\d+ ms\n'
             '28 bytes from \S{17} \(10.255.255.101\) index=2 time=\d+ ms\n'
             '28 bytes from \S{17} \(10.255.255.101\) index=3 time=\d+ ms\n'
             '28 bytes from \S{17} \(10.255.255.101\) index=4 time=\d+ ms\n'))

    def testArpingUnknown(self):
        stdout = runAndCheck(
            [pathToExample('arping'), '--addr', UNKNOWN_IP, '--count', '4'])
        self.assertEqual(stdout, ('Timeout\n'
                                  'Timeout\n'
                                  'Timeout\n'
                                  'Timeout\n'))

    def testPingReply(self):
        with runNetworkStack():
            stdout = runAndCheck(['ping', DEV_IP, '-c', '1'])
            self.assertIn(DEV_IP, stdout)

    def testPingGateway(self):
        stdout = runAndCheck(
            [pathToExample('ping'), '--addr', GATEWAY_IP, '--count', '4'])
        self.assertRegex(
            stdout,
            ('64 bytes from 10.255.255.101: icmp_seq=1 ttl=64 time=\d+ ms\n'
             '64 bytes from 10.255.255.101: icmp_seq=2 ttl=64 time=\d+ ms\n'
             '64 bytes from 10.255.255.101: icmp_seq=3 ttl=64 time=\d+ ms\n'
             '64 bytes from 10.255.255.101: icmp_seq=4 ttl=64 time=\d+ ms\n'))

    def testPingGoogleDNS(self):
        stdout = runAndCheck(
            [pathToExample('ping'), '--addr', GOOGLE_DNS_IP, '--count', '4'])
        self.assertRegex(
            stdout,
            ('64 bytes from 8.8.8.8: icmp_seq=1 ttl=\d+ time=\d+ ms\n'
             '64 bytes from 8.8.8.8: icmp_seq=2 ttl=\d+ time=\d+ ms\n'
             '64 bytes from 8.8.8.8: icmp_seq=3 ttl=\d+ time=\d+ ms\n'
             '64 bytes from 8.8.8.8: icmp_seq=4 ttl=\d+ time=\d+ ms\n'))

    def testPingUnknown(self):
        stdout = runAndCheck(
            [pathToExample('ping'), '--addr', UNKNOWN_IP, '--count', '4'])
        self.assertEqual(stdout, ('Timeout\n'
                                  'Timeout\n'
                                  'Timeout\n'
                                  'Timeout\n'))

    def testPingHugePayload(self):
        stdout = runAndCheck([
            pathToExample('ping'), '--addr', GOOGLE_DNS_IP, '--count', '4',
            '--payload', '16384'
        ])
        self.assertEqual(stdout, ('Timeout\n'
                                  'Timeout\n'
                                  'Timeout\n'
                                  'Timeout\n'))


if __name__ == '__main__':
    unittest.main()
