#!/usr/bin/env python3

import sys
print("Content-Type: text/plain\r\n\r\nHello World\n", file=sys.stdout)
sys.stdout.flush()