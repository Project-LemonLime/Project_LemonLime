import subprocess
import os

pid = os.getpid()
tmpout = f"_tmpout_{pid}"
tmperr = f"_tmperr_{pid}"

if os.path.exists("hello_s_abs"):
    os.remove("hello_s_abs")

os.symlink(os.path.join(os.getcwd(), "hello"), "hello_s_abs")

p = subprocess.Popen(["./watcher_unix", "./hello_s_abs", "", "", tmpout, tmperr, "1000", "100", "1000", "100", "", ""], shell=False, stdout=subprocess.PIPE)

assert(p.wait() == 0)
assert(os.path.exists(tmpout))
with open(tmpout, 'r') as f:
    assert(f.read() == "Hello World!\n")
