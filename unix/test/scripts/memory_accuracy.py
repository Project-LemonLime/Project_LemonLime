import os
import pathlib
import signal
import sys
# Only local test executables and fixed arguments are launched below.
import subprocess  # nosec B404
import tempfile

metric = "RSS" if sys.platform == "darwin" else "VSS"

with tempfile.TemporaryDirectory() as directory:
    def run(mode):
        output = pathlib.Path(directory) / f"{mode.replace(' ', '-')}.out"
        error = output.with_suffix(".err")
        process = subprocess.Popen(  # nosec B603
            ["./watcher_unix", "./memory_cases", mode, "", str(output), str(error),
             "2000", "512", "2000", "512", "", ""],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, start_new_session=True,
        )
        try:
            stdout, stderr = process.communicate(timeout=10)
        finally:
            if process.poll() is None:
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass
                process.communicate(timeout=3)
        assert process.returncode == 0, (
            mode, process.returncode, stderr, error.read_text() if error.exists() else "")
        fields = stdout.decode().split()
        assert len(fields) == 2, (mode, stdout, stderr)
        assert int(fields[0]) >= 0, (mode, stdout)
        usage = int(fields[1])
        assert usage >= 0, (mode, stdout)
        measurements = {}
        for field in output.read_text().split():
            key, value = field.split("=", 1)
            measurements.setdefault(key, []).append(int(value))
        return usage, measurements

    def check(mode, usage, reference):
        # Kernel snapshots and lifetime peaks can differ slightly during final I/O.
        tolerance = max(1024 * 1024, reference // 100)
        print(f"case={mode} metric={metric} reference={reference} usage={usage} tolerance={tolerance}",
              flush=True)
        assert abs(usage - reference) <= tolerance, (mode, reference, usage, tolerance)

    for mib in (16, 64):
        mode = f"accuracy {mib}"
        usage, measurements = run(mode)
        reference = max(measurements["reference-bytes"])
        allocation = mib * 1024 * 1024
        assert measurements["allocation-bytes"] == [allocation]
        assert reference >= allocation, (mib, reference)
        print(f"allocation={allocation}", flush=True)
        check(mode, usage, reference)

    if sys.platform.startswith("linux"):
        for mode in ("exec-chain", "exec-failed", "execveat-peak", "nonleader-exec",
                     "fork-exec"):
            usage, measurements = run(mode)
            assert measurements["exec-ok"] == [1], (mode, measurements)
            if mode == "fork-exec":
                reference = max(measurements["root-reference-bytes"])
                child_reference = max(measurements["child-reference-bytes"])
                assert child_reference > reference + max(1024 * 1024, reference // 100)
                print(f"case={mode} root-reference-bytes={reference} "
                      f"child-reference-bytes={child_reference} exec-ok=1", flush=True)
            else:
                references = measurements["reference-bytes"]
                reference = max(references)
                if mode == "exec-chain":
                    assert len(references) == 2, measurements
                    assert references[0] > references[1] + max(1024 * 1024, reference // 100)
                if mode == "exec-failed":
                    assert measurements["failed-exec-errno"] == [2], measurements
                    assert measurements["failed-exec-handled"] == [1], measurements
                    print("failed-exec-errno=2 failed-exec-handled=1", flush=True)
                print(f"case={mode} reference-bytes={references} exec-ok=1", flush=True)
            check(mode, usage, reference)
