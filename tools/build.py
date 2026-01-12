import subprocess
raise SystemExit(subprocess.run(["cmake", "--build", "build"]).returncode)
