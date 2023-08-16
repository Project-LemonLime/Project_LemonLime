import subprocess
import time

cmd = "\"%s\" %s" % ("./re", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "", "_tmperr", "1000", "100"], shell=False)

assert(p.wait() == 2)
