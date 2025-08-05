import subprocess
import shutil
import os

pid = os.getpid()
tmpout = f"_tmpout_{pid}"
tmperr = f"_tmperr_{pid}"

shutil.copy("./hello", "./he llo")

p = subprocess.Popen(["./watcher_unix", "./he llo", "", "", tmpout, tmperr, "1000", "100", "1000", "100", "", ""], shell=False, stdout=subprocess.PIPE)

assert(p.wait() == 0)
assert(os.path.exists(tmpout))
with open(tmpout, 'r') as f:
    assert(f.read() == "Hello World!\n")
