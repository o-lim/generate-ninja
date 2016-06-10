#!/usr/bin/env python
import os
import re
from sys import argv

dirname = os.path.dirname(argv[1])
f = open(argv[1], 'r')
head = f.readline()
f.close()
match = re.match('ref: (.*)', head)
if match and os.path.isfile(dirname + "/" + match.group(1)):
  print match.group(1)
elif match:
  print "packed-refs"
else:
  print ""
