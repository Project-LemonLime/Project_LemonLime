import subprocess
import shutil
import os

shutil.copy("./hello", "./he llo")

cmd = "\"%s\" %s" % ("./he llo", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "_tmpout", "_tmperr", "1000", "100", "1000", "100", "", ""], shell=False, stdout=subprocess.PIPE)

assert(p.wait() == 0)
assert(os.path.exists('_tmpout'))
with open('_tmpout', 'r') as f:
    assert(f.read() == "Hello World!\n")
