import os
import platform
import pathlib
import signal
import time
# subprocess is required to run the local regression-test fixtures.
import subprocess  # nosec B404
import tempfile

with tempfile.TemporaryDirectory() as directory:
    def child_state(pid):
        try:
            status = pathlib.Path(f'/proc/{pid}/status').read_text()
        except FileNotFoundError:
            return None
        fields = dict(line.split(':', 1) for line in status.splitlines() if ':' in line)
        return fields['State'].split()[0], int(fields['TracerPid'])

    def run(mode, limit=64):
        output = os.path.join(directory, 'out')
        error = os.path.join(directory, 'err')
        # Do not read a previous case's output if startup or execution fails.
        pathlib.Path(output).write_text('')
        pathlib.Path(error).write_text('')
        # Fixed local binaries and test-controlled arguments; no shell is involved.
        p = subprocess.Popen(['./watcher_unix', './memory_cases', mode, '', output, error,  # nosec B603
                              '1000', str(limit), '1000', str(limit), '', ''],
                             stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             start_new_session=True)
        try:
            stdout, stderr = p.communicate(timeout=8)
            fields = stdout.decode().split()
            assert len(fields) == 2, (mode, stdout, stderr)
            assert int(fields[0]) >= 0, (mode, stdout, stderr)
            assert int(fields[1]) >= 0, (mode, stdout, stderr)
            text = pathlib.Path(output).read_text()
            if mode == 'background-child':
                assert p.returncode == 0, (mode, p.returncode, stderr)
                assert b'reaping deadline' not in stderr.lower(), stderr
                assert 'reaping deadline' not in pathlib.Path(error).read_text().lower()
                labels = dict(field.split('=', 1) for field in text.split())
                pid = int(labels['child-pid'])
                assert pid > 1, text
                deadline = time.monotonic() + 1
                state = child_state(pid)
                while state is not None and state != ('Z', 0) and time.monotonic() < deadline:
                    time.sleep(0.01)
                    state = child_state(pid)
                print(f'case={mode} child-pid={pid} state={state} '
                      f'statistics={stdout.decode().strip()}', flush=True)
                assert state is None or state == ('Z', 0), (pid, state)
            elif mode == 'syscall-burst':
                assert p.returncode == 0, (mode, p.returncode, stderr)
                labels = dict(field.split('=', 1) for field in text.split())
                assert labels['syscall-burst-ok'] == '1', text
                print(f'case={mode} syscall-burst-ok=1 '
                      f'statistics={stdout.decode().strip()}', flush=True)
            return p.returncode, text, int(fields[1])
        finally:
            if p.poll() is None:
                try:
                    os.killpg(p.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                p.communicate(timeout=3)
            if mode == 'background-child':
                # Even a timeout or malformed watcher report must not leave the
                # fixture's detached, paused child alive.
                for field in pathlib.Path(output).read_text().split():
                    if field.startswith('child-pid='):
                        pid = int(field.split('=', 1)[1])
                        if pid > 1:
                            state = child_state(pid)
                            if state is not None and state[0] != 'Z':
                                try:
                                    os.kill(pid, signal.SIGKILL)
                                except ProcessLookupError:
                                    pass
                                deadline = time.monotonic() + 2
                                while time.monotonic() < deadline:
                                    state = child_state(pid)
                                    if state is None or state == ('Z', 0):
                                        break
                                    time.sleep(0.01)
                                assert state is None or state == ('Z', 0), (pid, state)

    assert run('abort')[0] == 2
    if platform.system() == 'Linux':
        assert run('caught')[0:2] == (0, 'caught\n')
        assert run('uncaught')[0] == 2
        assert run('reserve')[0:2] == (0, 'denied\n')
        # Prelimit refuses the allocation; a nonzero exit is RE, not MLE.
        assert run('resident')[0] == 2
        assert run('background-child', 512)[0] == 0
        assert run('syscall-burst', 512)[0] == 0
    else:
        assert run('reserve')[0:2] == (0, 'reserved\n')
        assert run('resident')[0] == 4
    assert run('sleep')[0] == 3
