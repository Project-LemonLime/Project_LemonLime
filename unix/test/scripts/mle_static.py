import subprocess

cmd = "\"%s\" %s" % ("./mle_static", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "", "_tmperr", "1000", "380", "1000", "380", "", ""], shell=False, stdout=subprocess.PIPE)

stdout, _ = p.communicate()

assert(p.wait() == 4)
assert(stdout[0] == 48)
