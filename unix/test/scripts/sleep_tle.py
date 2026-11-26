import subprocess
import os

pid = os.getpid()
tmperr = f"_tmperr_{pid}"

p = subprocess.Popen(["./watcher_unix", "./sleep_tle", "", "", "", tmperr, "1000", "100", "1000", "100", "", "", "0"], shell=False)

try:
    returncode = p.wait(timeout=10)
except subprocess.TimeoutExpired:
    p.kill()
    p.wait()
    raise

assert(returncode == 3)
