############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
#
############################################################################
# Check a <file_name> against <file_name>.new
# If <file_name> does not exist, or is different to <file_name>.new
# then mv <file_name>.new to <file_name>
#
# NOTE <file_name>.new  MUST exist
import sys
import os.path
import filecmp
import shutil

shortname = os.path.basename(sys.argv[0])

if len(sys.argv) != 2:
  print "%s: bad parameters: usage %s <file_name>" % (shortname, shortname)
  sys.exit(1)

filename=sys.argv[1]
new_filename = filename + '.new'

if not os.path.exists(new_filename):
  print "%s: new file %s does not exist" % (shortname, new_filename)
  sys.exit(1)

if os.path.exists(filename):
  if not filecmp.cmp(new_filename, filename):
     shutil.move(new_filename, filename)
else:
  shutil.move(new_filename, filename)

sys.exit(0)
