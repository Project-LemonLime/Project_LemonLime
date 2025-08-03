import subprocess
import os

pid = os.getpid()
tmperr = f"_tmperr_{pid}"

cmd = "\"%s\" %s" % ("./mle_static", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "", tmperr, "1000", "380", "1000", "380", "", ""], shell=False, stdout=subprocess.PIPE)

stdout, _ = p.communicate()

assert(p.wait() == 4)
assert(stdout[0] == 48)
