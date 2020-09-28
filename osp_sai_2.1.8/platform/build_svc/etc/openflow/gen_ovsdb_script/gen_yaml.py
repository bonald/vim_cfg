#!/usr/bin/evn python3

import os
import sys
import yaml
import json
import hashlib

def save_to_yaml(json_str, line):
    try:
        obj = json.loads(json_str)

        with open('output/%d.yaml' % line, 'w') as f:
            yaml.dump(obj, f)
    except Exception as e:
        print('pass line: %d' % line)

if __name__ == '__main__':
    db_str = None

    with open('conf.db', 'r') as f:
        db_str = f.read()

    if not os.path.exists('output'):
        os.mkdir('output')

    if not os.path.isdir('output'):
        print('dir output not exists')
        sys.exit(1)

    if not db_str:
        print('str is empty')
        sys.exit(1)

    curr_line = 0
    for line in db_str.split('\n'):
        if not line.strip():
            continue
        curr_line += 1

        save_to_yaml(line, curr_line)
