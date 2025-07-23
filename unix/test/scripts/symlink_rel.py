import subprocess
import os

if os.path.exists("hello_s_rel"):
    os.remove("hello_s_rel")

os.symlink("hello", "hello_s_rel")

cmd = "\"%s\" %s" % ("./hello_s_rel", "")
p = subprocess.Popen(["./watcher_unix", cmd, "", "_tmpout", "_tmperr", "1000", "100", "1000", "100", "", ""], shell=False, stdout=subprocess.PIPE)

assert(p.wait() == 0)
assert(os.path.exists('_tmpout'))
with open('_tmpout', 'r') as f:
    assert(f.read() == "Hello World!\n")
