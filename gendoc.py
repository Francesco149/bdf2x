#!/usr/bin/env python

import sys
print_lines = False

for line in sys.stdin:
    if line.strip().endswith("*/"):
        sys.exit(0);
    if print_lines:
        print(line[4:-1])
    elif line.strip().startswith("/*"):
        print_lines = True
