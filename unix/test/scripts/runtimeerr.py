import subprocess
import os

pid = os.getpid()
tmperr = f"_tmperr_{pid}"

p = subprocess.Popen(["./watcher_unix", "./re", "", "", "", tmperr, "1000", "100", "1000", "100", "", ""], shell=False)

assert(p.wait() == 2)
