import subprocess
import os

if os.path.exists("hello_s_abs"):
    os.remove("hello_s_abs")

os.symlink(os.path.join(os.getcwd(), "hello"), "hello_s_abs")

cmd = "\"%s\" %s" % ("./hello_s_abs", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "", "_tmperr", "1000", "100"], shell=False, stdout=subprocess.PIPE)

stdout, _ = p.communicate()

assert(p.wait() == 0)
out_str = stdout.decode()
assert(out_str.split('\n')[0] == 'Hello World!')