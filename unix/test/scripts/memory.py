import os
import platform
import subprocess
import tempfile

with tempfile.TemporaryDirectory() as directory:
    def run(mode, limit=64):
        output = os.path.join(directory, 'out')
        error = os.path.join(directory, 'err')
        p = subprocess.run(['./watcher_unix', './memory_cases', mode, '', output, error,
                            '1000', str(limit), '1000', str(limit), '', ''],
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=8)
        fields = p.stdout.decode().split()
        assert len(fields) == 2, (mode, p.stdout, p.stderr)
        assert int(fields[1]) >= 0, (mode, p.stdout, p.stderr)
        return p.returncode, open(output).read(), int(fields[1])

    assert run('abort')[0] == 2
    if platform.system() == 'Linux':
        assert run('caught')[0:2] == (0, 'caught\n')
        assert run('uncaught')[0] == 2
        assert run('reserve')[0:2] == (0, 'denied\n')
        # Prelimit refuses the allocation; a nonzero exit is RE, not MLE.
        assert run('resident')[0] == 2
    else:
        assert run('reserve')[0:2] == (0, 'reserved\n')
        assert run('resident')[0] == 4
    assert run('sleep')[0] == 3
