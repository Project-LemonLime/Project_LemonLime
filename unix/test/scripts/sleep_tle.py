import subprocess
import time
import os

pid = os.getpid()
tmperr = f"_tmperr_{pid}"

p = subprocess.Popen(["./watcher_unix", "./sleep_tle", "", "", "", tmperr,
                       "1000", "100", "1000", "100", "", "", ".", "0"],
                      shell=False)

time.sleep(3)
p.kill()

assert(p.returncode == 3)
