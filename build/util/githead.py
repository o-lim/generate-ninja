#!/usr/bin/env python
import os
import re
from sys import argv

parts = argv[1].rpartition("/.git/")
is_submodule = os.path.isfile(parts[0] + "/.git")
if is_submodule:
  f = open(parts[0] + "/.git", 'r')
  line = f.readline()
  f.close()
  gitdir = re.match('gitdir: (.*)', line).group(1)
  if not os.path.isabs(gitdir):
    gitdir = os.path.normpath(os.path.join(parts[0], gitdir))
  headfile = gitdir + "/" + parts[2]
else:
  headfile = argv[1]
  gitdir = os.path.dirname(headfile)

f = open(headfile, 'r')
head = f.readline()
f.close()
match = re.match('ref: (.*)', head)
if match and os.path.isfile(gitdir + "/" + match.group(1)):
  print(match.group(1))
elif match:
  print("packed-refs")
else:
  print("")
