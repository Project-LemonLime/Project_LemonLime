import subprocess
import time
import os

pid = os.getpid()
tmpout = f"_tmpout_{pid}"
tmperr = f"_tmperr_{pid}"

p = subprocess.Popen(["./watcher_unix", "./tle", "", "", "", tmperr, "1000", "100", "1000", "100", "", "", ".", "5000"], shell=False)

p.wait()

assert(p.returncode == 3)
