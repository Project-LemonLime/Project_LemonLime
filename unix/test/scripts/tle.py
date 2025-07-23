import subprocess
import time

cmd = "\"%s\" %s" % ("./tle", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "", "_tmperr", "1000", "100", "1000", "100", "", ""], shell=False)

time.sleep(5)
p.kill()

assert(p.returncode == 3)
