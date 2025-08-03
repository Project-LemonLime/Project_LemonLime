import subprocess
import os

pid = os.getpid()
tmperr = f"_tmperr_{pid}"

cmd = "\"%s\" %s" % ("./re", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "", tmperr, "1000", "100", "1000", "100", "", ""], shell=False)

assert(p.wait() == 2)
