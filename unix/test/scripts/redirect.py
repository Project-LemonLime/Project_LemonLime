import subprocess
import time
from pathlib import Path

Path("tmpin.txt").write_text("1 1")

p = subprocess.Popen(["./watcher_unix", "./add", "", "tmpin.txt", "tmpout.txt", "tmperr.txt", "1000", "100", "1000", "100", "", "", "0"], shell=False)

time.sleep(2)
p.kill()

assert(p.returncode == 0)

assert(Path("tmpout.txt").exists())
assert(Path("tmpout.txt").read_text() == "2\n")

for name in ("tmpin.txt", "tmpout.txt", "tmperr.txt"):
    Path(name).unlink(missing_ok=True)
