import subprocess

cmd = "\"%s\" %s" % ("./tle", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "", "_tmperr", "1000", "100"], shell=False)

p.wait(5)
