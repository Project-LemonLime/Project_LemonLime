import subprocess
import os
import time

with open('_tmpin', 'w') as f:
    f.writelines(['1 1'])

cmd = "\"%s\" %s" % ("./add", "")
p = subprocess.Popen(["./watcher_unix", cmd, "_tmpin", "_tmpout", "_tmperr", "1000", "100"], shell=False)

time.sleep(2)
p.kill()

assert(p.returncode == 0)

assert(os.path.exists('_tmpout'))
with open('_tmpout', 'r') as f:
    assert(f.read() == "2\n")
