#!/usr/bin/env python3

import os
import sys
import json
import yaml
import hashlib

if __name__ == '__main__':
    files = os.listdir('output')
    files.sort()

    if not files:
        print('output dir is empty')
        sys.exit(1)

    strs = []

    for fname in files:
        with open('output/%s' % fname, 'r') as f:
            ext = os.path.splitext(fname)[1]
            if ext != '.yaml':
                continue

            obj = yaml.load(f)
            json_str = json.dumps(obj)
            sha1 = hashlib.sha1((json_str + '\n').encode()).hexdigest()
            crypto = 'OVSDB JSON {} {}'.format(len(json_str) + 1, sha1)

            strs.append(crypto)
            strs.append(json_str)

    with open('output.db', 'w') as f:
        f.write('\n'.join(strs))
        f.write('\n')
