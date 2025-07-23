import subprocess
import os

if os.path.exists("hello_s_abs"):
    os.remove("hello_s_abs")

os.symlink(os.path.join(os.getcwd(), "hello"), "hello_s_abs")

cmd = "\"%s\" %s" % ("./hello_s_abs", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "_tmpout", "_tmperr", "1000", "100", "1000", "100", "", ""], shell=False, stdout=subprocess.PIPE)

assert(p.wait() == 0)
assert(os.path.exists('_tmpout'))
with open('_tmpout', 'r') as f:
    assert(f.read() == "Hello World!\n")
