import subprocess
import time
import os

pid = os.getpid()
tmpout = f"_tmpout_{pid}"
tmperr = f"_tmperr_{pid}"

p = subprocess.Popen(["./watcher_unix", "./tle", "", "", "", tmperr, "1000", "100", "1000", "100", "", ""], shell=False)

time.sleep(5)
p.kill()

assert(p.returncode == 3)
