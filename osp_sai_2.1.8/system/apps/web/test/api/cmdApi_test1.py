#!/usr/bin/python
#-*- coding: utf-8 -*-

import sys
import getopt
import time
import json
import yaml
import requests
from datetime import datetime

"""
TODO:
    this two line used to eliminate the annoying warnnings of not
    verifying https certificate, I have no time figuring out how
    to verify the certificate

    Do verifying certificate and remove this two lines in the future
"""
from requests.packages.urllib3.exceptions import InsecureRequestWarning
#requests.packages.urllib3.disable_warnings(InsecureRequestWarning)
requests.packages.urllib3.disable_warnings()


class CTCError(Exception):
    """
    base class for all error types
    """
    pass

class CTCTestSkip(Exception):
    """
    Raised when test case passed

    Attributes:
        case, test case name
        reason, the reason why this case skipped
    """
    def __init__(self, case, reason):
        self.case = case
        self.reason = reason

class CTCTestPass(Exception):
    """
    Raised when test case passed

    Attributes:
        case, test case name
    """
    def __init__(self, case):
        self.case = case

class CTCTestFail(Exception):
    """
    Raised when test case failed

    Attributes:
        case, test case name
        reason, failed reason
    """

    def __init__(self, case, reason):
        self.case = case
        self.reason = reason

def usage():
    print
    print '%s [options] <switch ip1> [switch ip2 [switch ip3 ...]]' % sys.argv[0]
    print
    print 'options'
    print
    print '     -t <recipe>, where the test cases read from, defaults to tests.yaml'
    print '     -o <logfile>, where the test report write to'
    print '     -h | --help, show this help info'
    print '     -v | --verbose, output verbose'
    print

def verify_body(c, e, r):
    """
    c, test case
    e, expect body
    r, responsed body
    """

    if type(e) is dict and type(r) is dict:
        for k, v in e.items():
            if k not in r:
                raise CTCTestFail(c['case'], 'Body dismatch')
            else:
                verify_body(c, v, r[k])
    elif type(e) is list and type(r) is list:
        for v in e:
            if type(v) is dict or type(v) is list:
                raise CTCTestSkip(c['case'], 'Body too complex')
            elif v not in r:
                raise CTCTestFail(c['case'], 'Body dismatch')
    elif type(e) is str and type(r) is str or type(r) is unicode:
        if e != r:
            raise CTCTestFail(c['case'], 'Body dismatch')
    else:
        raise CTCTestFail(c['case'], 'Body dismatch')

def do_tests(switch_ip, tests):
    """
    """

    cookies = None
    stats = dict(succ = 0, fail = 0, skip = 0)
    cert = ('./server.crt', './server-private.key')

    start_time = datetime.now()
    report('\n[Switch: %s] Test start on (%s)\n'
            % (switch_ip, start_time.strftime('%Y.%m.%d %H:%M:%S')))

    for c in tests:
        try:
            if 'skip' in c:
                raise CTCTestSkip(c['case'], 'User skip')

            if 'url' not in c:
                raise CTCTestSkip(c['case'], 'Url not specified')

            url = 'http://' + switch_ip + c['url']

            # if not specify http method, defaults to 'GET'
            if 'method' not in c or c['method'] == 'GET':
                r = requests.get(url, cookies = cookies, cert = cert, verify = False)
            elif c['method'] == 'POST':
                if 'body' in c:
                    payload = c['body']

                if c['url'] != '/rest/v1/login' and c['url'] != '/login':
                    payload = json.dumps(payload)

                r = requests.post(url, data = payload, cookies = cookies, cert = cert, verify = False)
                print r.status_code
                z = r.json()
                print z['error']
                print z['sourceDetails']
            elif c['method'] == 'PUT':
                if 'body' in c:
                    payload = c['body']

                r = requests.put(url, json = payload, cookies = cookies, cert = cert, verify = False)

                print r.status_code
                print r.json()
            elif c['method'] == 'DELETE':
                r = requests.delete(url, data = payload, cookies = cookies, cert = cert, verify = False)
            else:
                raise CTCTestSkip(c['case'], 'Method not supported')

            if 'expect' not in c:
                raise CTCTestPass(c['case'])

            t = c['expect']

            if 'status' in t and r.status_code != t['status']:
                raise CTCTestFail(c['case'], 'Status code dismatch, expect %s, get %s' % (t['status'], r.status_code))

            if 'headers' in t:
                for h in t['headers']:
                    if h not in r.headers:
                        raise CTCTestFail(c['case'], 'Header(%s) responsed absent' % h)
                    elif h == 'Set-Cookie':
                        cookies = r.cookies

            if 'body' not in t or t['body'] is None:
                pass
            else:
                verify_body(c, t['body'], r.json())

            raise CTCTestPass(c['case'])
        except ValueError as e:
            stats['fail'] += 1
            report('[Failed] %s, reason: No response body' % e.case)
        except CTCTestFail as e:
            stats['fail'] += 1
            report('[Failed] %s, reason: %s' % (e.case, e.reason))
        except CTCTestSkip as e:
            stats['skip'] += 1
            report('[Skiped] %s, reason: %s' % (e.case, e.reason))
        except CTCTestPass as e:
            stats['succ'] += 1
            report('[Passed] %s' % (e.case))
        except requests.exceptions.ConnectionError as e:
            report('[Switch: %s] Connection error, skip test' % switch_ip)
            break

        time.sleep(0.5)

    end_time = datetime.now()
    spent_time = end_time - start_time
    report('')
    report('[Switch: %s] Test end on (%s), spent %ss, test results:'
            % (switch_ip, end_time.strftime('%Y.%m.%d %H:%M:%S'), spent_time.seconds))
    report('-------------------------------------------------------------------------------')
    report('Passed: %d, Failed: %d, Skipped: %d, Total: %d'
            % (stats['succ'], stats['fail'], stats['skip'], stats['succ'] + stats['fail'] + stats['skip']))
    report('')

rf = None
def report(msg):
    print msg

    global rf
    if rf is not None:
        rf.write(msg)
        rf.write('\n')

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "ho:t:v", ["help", "verbose"])
    except getopt.GetoptError as err:
        # will print something like "option -a not recognized"
        print str(err)
        usage()
        sys.exit(2)

    reportfile = None
    casefile = None
    verbose = False
    for o, a in opts:
        if o == "-v":
            verbose = True
        elif o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o == "-o":
            reportfile = a
        elif o == "-t":
            casefile = a
        else:
            assert False, "unhandled option"

    if args is None or len(args) == 0:
        print 'No switch ip!'
        usage()
        sys.exit()

    if casefile is None:
        casefile = 'cases.yaml'

    cf = open(casefile, 'r')
    tests = yaml.load(cf)

    global rf
    if reportfile is not None:
        rf = open(reportfile, 'a+')

    for switch in args:
        do_tests(switch, tests)

    cf.close()
    rf.close()

if __name__ == '__main__':
    main()


