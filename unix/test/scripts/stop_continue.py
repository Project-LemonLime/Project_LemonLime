import os
import pathlib
import re
import signal
import subprocess  # nosec B404
import tempfile
import time

with tempfile.TemporaryDirectory() as directory:
    output = pathlib.Path(directory) / 'out'
    error = pathlib.Path(directory) / 'err'
    watcher = subprocess.Popen(  # nosec B603
        ['./watcher_unix', './memory_cases', 'stop-continue', '', str(output), str(error),
         '3000', '512', '3000', '512', '', ''],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    pid = None
    try:
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            text = output.read_text() if output.exists() else ''
            match = re.search(r'pid=(\d+)', text)
            if match:
                pid = int(match[1])
                status = pathlib.Path(f'/proc/{pid}/status').read_text()
                if re.search(r'^State:\s+[Tt]\b', status, re.M):
                    break
            assert watcher.poll() is None, (watcher.returncode, text)
            time.sleep(0.01)
        else:
            raise AssertionError('fixture never entered stopped state')
        assert 'resumed' not in text, text
        os.kill(pid, signal.SIGCONT)
        stdout, stderr = watcher.communicate(timeout=12)
        assert watcher.returncode == 0, (stdout, stderr, error.read_text())
        fields = stdout.split()
        assert len(fields) == 2 and all(int(x) >= 0 for x in fields), stdout
        assert 'resumed' in output.read_text()
        print('stop-continue: stopped, SIGCONT resumed, AC', stdout.decode().strip())
    finally:
        if pid is not None:
            try:
                os.killpg(pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
        if watcher.poll() is None:
            watcher.terminate()
            try:
                watcher.communicate(timeout=4)
            except subprocess.TimeoutExpired:
                watcher.kill()
                watcher.communicate(timeout=4)
