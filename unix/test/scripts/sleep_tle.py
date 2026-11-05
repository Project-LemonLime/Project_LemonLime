import subprocess
import os

pid = os.getpid()
tmperr = f"_tmperr_{pid}"

# `sleep` does not consume CPU time, so it can bypass RLIMIT_CPU;
# the watcher must time it out by wall clock.
p = subprocess.Popen(["./watcher_unix", "./sleep_tle", "", "", "", tmperr,
                      "1000", "100", "1000", "100", "", "", "0"],
                     shell=False)

try:
    returncode = p.wait(timeout=10)
except subprocess.TimeoutExpired:
    p.kill()
    p.wait()
    raise

assert(returncode == 3)
