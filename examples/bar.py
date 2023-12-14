from __future__ import print_function

import argparse
import os
import time
import random
from six.moves.urllib.parse import urlparse
import sys

basedir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
os.chdir(basedir)
sys.path.append(basedir)

from LemonGraph.httpc import RESTClient

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument('-u', '--url', help='LemonGraph REST service base url', default='http://localhost:8000')
parser.add_argument('-s', '--socket', help='LemonGraph REST service UNIX domain socket', default=None)
parser.add_argument('-l', '--limit', help='desired number of records per task', type=int, default=200)
parser.add_argument('-p', '--priority', help='job priority', type=int, default=100)
parser.add_argument('-d', '--delay', help='delay in seconds per task', type=float, default=0)
parser.add_argument('count', help='job count', type=int, nargs='?', default=1)

args = parser.parse_args()

server = urlparse(args.url)
if args.socket is not None:
    sockpath = args.socket
    if sockpath[0] == '@':
        sockpath[0] = '\0'
else:
    sockpath = None

rc = RESTClient(sockpath=sockpath, host=server.hostname, port=server.port or 80)

while True:
    # fetch task json - we are just scanning for nodes
    try:
        status, headers, task = rc.get('/lg/adapter/BAR', json={ 'query': 'n(foo)', 'limit': args.limit })
    except rc.error:
        time.sleep(1)
        continue

    if status != 200:
        time.sleep(1)
        continue

    records = iter(task)

    # first item is task metadata
    meta = next(records)
    loc = headers['location'][0]
    print(loc, headers['x-lg-priority'])

    chains = []
    # remaining items are node/edge chains from query
    # our chains for the above query are single nodes
    for n, in records:
        for i in 1,2,3:
            if random.random() < 0.25:
                continue
            chain = [
                { 'ID': n['ID'] },
                { 'type': '%s_%s' % (n['type'], 'bar') },
                { 'type': 'bar', 'value': 'bar%d' % random.randint(0,99999) },
            ]
            # append to updates
            chains.append(chain)

    if args.delay:
        time.sleep(args.delay)

    try:
        status = rc.post(loc, json={'chains': chains})
    except rc.error:
        time.sleep(1)

rc.close()
