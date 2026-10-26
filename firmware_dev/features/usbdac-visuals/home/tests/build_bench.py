"""Build isolated MIPS executable; QEMU smoke test only, no ADB/device work."""
from pathlib import Path
import hashlib
import json
import subprocess

HERE = Path(__file__).resolve().parent
OUT = HERE / 'build'
OUT.mkdir(exist_ok=True)
BASE = Path('/root/disc-official-component-base-20261004-r1/base')
HEADERS = Path('/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc/usr/include')
TARGET = OUT / 'disc-dac-home-spectrum-bench'
args = ['clang', '--target=mipsel-linux-gnu', '-march=mips32r2', '-mabi=32',
        '-mhard-float', '-mfp64', '-mnan=2008', '-O2', '-Wall', '-Wextra', '-Werror',
        '-Wno-misleading-indentation', '-Wno-unused-function', '-fno-builtin',
        '-nostdlib', '-fuse-ld=lld', '-mno-abicalls', '-fno-pic', '-G0', '-no-pie',
        '-DDISC_DAC_HOME_STANDALONE_MIPS', '-isystem', str(HEADERS), '-L' + str(BASE / 'lib'),
        str(HERE / 'bench_spectrum.c'),
        '-Wl,-e,_start,-dynamic-linker,/lib/ld-linux-mipsn8.so.1,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack',
        '-o', str(TARGET)]
subprocess.run(args, check=True)
abi = subprocess.run(['readelf', '-h', '-A', '-l', '-d', '-W', str(TARGET)], text=True, capture_output=True, check=True).stdout
assert 'EXEC (Executable file)' in abi and 'nan2008' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi
assert 'libm.so.6' in abi and 'libc.so.6' in abi and '/lib/ld-linux-mipsn8.so.1' in abi and 'RWE' not in abi
(OUT / 'bench-mips-abi.txt').write_text(abi)
result = subprocess.run(['qemu-mipsel', '-cpu', 'P5600', '-L', str(BASE), str(TARGET)], text=True, capture_output=True, check=True, timeout=60)
(OUT / 'bench-qemu-smoke.log').write_text(result.stdout + result.stderr)
record = json.loads(result.stdout)
assert record['iterations'] == 100 and record['analyses'] == 116 and record['audio_opened'] is False and record['settings_written'] is False
report = {'executable': TARGET.name, 'sha256': hashlib.sha256(TARGET.read_bytes()).hexdigest(),
          'bytes': TARGET.stat().st_size, 'mips32r2_hard_fpu_nan2008': True,
          'qemu_smoke_test': 'PASS (timings are not device measurements)',
          'target_device_run': 'not performed', 'source_sha256': hashlib.sha256((HERE / 'bench_spectrum.c').read_bytes()).hexdigest(),
          'module_sha256': hashlib.sha256((HERE.parent / 'spectrum.h').read_bytes()).hexdigest(),
          'clock': ['CLOCK_THREAD_CPUTIME_ID', 'CLOCK_MONOTONIC'], 'warmup': 16, 'iterations': 100,
          'timed_scope': 'gate + existing 4096-point packed-stereo FFT + tick + height reads',
          'excluded_scope': 'synthetic PCM generation, ring read/conversion, actual LVGL draw, DPU, DSP audio effects',
          'priority': 'nice 19 self only', 'audio_opened': False, 'settings_written': False,
          'build_command': args}
(OUT / 'bench-build.json').write_text(json.dumps(report, indent=2) + '\n')
print(json.dumps({key: value for key, value in report.items() if key != 'build_command'}, indent=2))
