import subprocess
import os

if os.path.exists("hello_s_rel"):
    os.remove("hello_s_rel")

os.symlink("hello", "hello_s_rel")

cmd = "\"%s\" %s" % ("./hello_s_rel", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "", "_tmperr", "1000", "100"], shell=False, stdout=subprocess.PIPE)

stdout, _ = p.communicate()

assert(p.wait() == 0)
out_str = stdout.decode()
assert(out_str.split('\n')[0] == 'Hello World!')