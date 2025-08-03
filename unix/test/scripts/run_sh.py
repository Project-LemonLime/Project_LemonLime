import subprocess
import os

pid = os.getpid()
tmpout = f"_tmpout_{pid}"
tmperr = f"_tmperr_{pid}"

cmd = "\"%s\" %s" % ("/bin/sh", "hello.sh")
p = subprocess.Popen(["./watcher_unix", cmd, "", tmpout, tmperr, "1000", "100", "1000", "100", "", ""], shell=False, stdout=subprocess.PIPE)

assert(p.wait() == 0)
assert(os.path.exists(tmpout))
with open(tmpout, 'r') as f:
    assert(f.read() == "Hello World!\n")
