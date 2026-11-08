import pathlib
import sys
# Only local test executables and fixed arguments are launched below.
import subprocess  # nosec B404
import tempfile

metric = "RSS" if sys.platform == "darwin" else "VSS"

with tempfile.TemporaryDirectory() as directory:
    for mib in (16, 64):
        output = pathlib.Path(directory) / f"accuracy-{mib}.out"
        error = pathlib.Path(directory) / f"accuracy-{mib}.err"
        result = subprocess.run(  # nosec B603
            ["./watcher_unix", "./memory_cases", f"accuracy {mib}", "", str(output), str(error),
             "2000", "512", "2000", "512", "", ""],
            stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=10,
        )
        assert result.returncode == 0, (mib, result.returncode, result.stderr,
                                       error.read_text() if error.exists() else "")
        fields = result.stdout.decode().split()
        assert len(fields) == 2, result.stdout
        usage = int(fields[1])
        measurements = dict(field.split("=") for field in output.read_text().split())
        reference = int(measurements["reference-bytes"])
        allocation = mib * 1024 * 1024
        assert int(measurements["allocation-bytes"]) == allocation
        assert reference >= allocation, (mib, reference)
        # Kernel snapshots and lifetime peaks can differ slightly during final I/O.
        tolerance = max(1024 * 1024, reference // 100)
        print(f"metric={metric} allocation={allocation} reference={reference} usage={usage} tolerance={tolerance}")
        assert abs(usage - reference) <= tolerance, (mib, reference, usage, tolerance)
