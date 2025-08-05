import subprocess
import os
import time

pid = os.getpid()
tmpin = f"_tmpin_{pid}"
tmpout = f"_tmpout_{pid}"
tmperr = f"_tmperr_{pid}"

with open(tmpin, 'w') as f:
    f.writelines(['1 1'])

p = subprocess.Popen(["./watcher_unix", "./add", "", tmpin, tmpout, tmperr, "1000", "100", "1000", "100", "", ""], shell=False)

time.sleep(2)
p.kill()

assert(p.returncode == 0)

assert(os.path.exists(tmpout))
with open(tmpout, 'r') as f:
    assert(f.read() == "2\n")
