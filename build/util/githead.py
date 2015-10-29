import re
from sys import argv

f = open(argv[1], 'r')
head = f.readline()
f.close()
match = re.match('ref: (.*)', head)
if match:
  print match.group(1)
else:
  print ""
