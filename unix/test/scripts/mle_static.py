import subprocess
import os
import platform

pid = os.getpid()
tmperr = f"_tmperr_{pid}"

p = subprocess.Popen(["./watcher_unix", "./mle_static", "", "", "", tmperr, "1000", "380", "1000", "380", "", ""], shell=False, stdout=subprocess.PIPE)

stdout, _ = p.communicate()

assert(p.wait() == (2 if platform.system() == 'Linux' else 0))
assert(len(stdout.split()) == 2)
