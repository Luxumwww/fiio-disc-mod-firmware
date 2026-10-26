"""Build Host 180 and UI 1.10.180 only, using the existing WSL toolchain.

Run with WSL python3. --check validates inputs without compiling; it also works
on Windows against the SHA-locked stock analysis copies. This script neither
builds DSP/render providers nor packages, installs, or operates the player.
"""
from pathlib import Path
import argparse
import ast
import hashlib
import json
import re
import struct
import subprocess
import sys

HERE = Path(__file__).resolve().parent
FEATURES = HERE.parent
ROOT = FEATURES.parents[1]
METER = FEATURES / 'bluetooth-meter'
SETTINGS = FEATURES / 'settings-debug'
BASE = Path('/root/disc-official-component-base-20261004-r1/base')
VERSION = '1.3.180'
UI_VERSION = '1.10.180'
STOCK_SHA = {
    'usr/bin/mq_ui': 'f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226',
    'usr/bin/mq_player': 'a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc',
}
REQUIRED_UI_GUARDS = {0x4f3f10, 0x52607c, 0x52f4f0}
PROVIDERS = [
    FEATURES / 'hardware-accel/runtime-provider/build/libdisc-hwaccel.so',
    FEATURES / 'audio-visuals/xy/build/libdisc-audio-xy.so',
    FEATURES / 'audio-visuals/polar/build/libdisc-audio-polar.so',
    FEATURES / 'audio-visuals/spectrum/build/libdisc-audio-spectrum.so',
    FEATURES / 'audio-visuals/mandala/build/libdisc-audio-mandala.so',
    FEATURES / 'audio-dsp/build/libdisc-audio-dsp.so',
]


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def addresses_from_builder(path):
    """Read literal guard lists without executing a builder or its rebuilds."""
    addresses = []
    for node in ast.parse(path.read_text(encoding='utf-8')).body:
        if isinstance(node, ast.Assign) and any(
            isinstance(target, ast.Name) and target.id == 'addresses'
            for target in node.targets
        ):
            addresses = list(ast.literal_eval(node.value))
        elif isinstance(node, ast.AugAssign) and isinstance(node.target, ast.Name) and node.target.id == 'addresses':
            assert isinstance(node.op, ast.Add)
            addresses.extend(ast.literal_eval(node.value))
        elif isinstance(node, ast.Expr) and isinstance(node.value, ast.Call):
            call = node.value
            if isinstance(call.func, ast.Attribute) and isinstance(call.func.value, ast.Name) and call.func.value.id == 'addresses' and call.func.attr == 'append':
                addresses.append(ast.literal_eval(call.args[0]))
    assert addresses and REQUIRED_UI_GUARDS.issubset(addresses), path
    return addresses


def stock_data(check_only):
    base = BASE
    if check_only and not (base / 'usr/bin/mq_ui').is_file():
        base = ROOT / 'firmware_analysis/main_os_ota_v257/files'
    data = {}
    for name, expected in STOCK_SHA.items():
        path = base / name
        assert path.is_file(), f'Missing SHA-locked original: {path}'
        assert sha(path) == expected, f'Unexpected original: {path}'
        data[name] = path.read_bytes()
    return data


def guard_line(data, address):
    header = struct.unpack_from('<16sHHIIIIIHHHHHH', data)
    segments = [struct.unpack_from('<IIIIIIII', data, header[5] + i * header[9]) for i in range(header[10])]
    segment = next(p for p in segments if p[0] == 1 and p[2] <= address and address + 16 <= p[2] + p[4])
    offset = segment[1] + address - segment[2]
    return '{0x%x,{%s}},' % (address, ','.join(map(str, data[offset:offset + 16])))


def guard_text(data, addresses):
    lines = ['static const struct {unsigned address;unsigned char bytes[16];} bt_guards[]={']
    lines += [guard_line(data['usr/bin/mq_ui'], address) for address in addresses]
    lines += ['};', 'static const struct {unsigned address;unsigned char bytes[16];} player_guards[]={']
    lines += [guard_line(data['usr/bin/mq_player'], address) for address in (0x4e0e20, 0x4cb3e4)]
    return '\n'.join(lines + ['};']) + '\n'


def source_hashes():
    pending = [METER / 'meter.c', SETTINGS / 'settings.c']
    seen = set()
    while pending:
        path = pending.pop().resolve()
        if path in seen:
            continue
        seen.add(path)
        for name in re.findall(r'^\s*#\s*include\s*"([^"]+)"', path.read_text(encoding='utf-8'), re.M):
            include = (path.parent / name).resolve()
            assert include.is_file(), f'Missing quoted include: {include}'
            pending.append(include)
    return {path.relative_to(ROOT).as_posix(): sha(path) for path in sorted(seen)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='Read-only readiness and guard verification.')
    parser.add_argument('--refresh-guards', action='store_true', help='Regenerate the pinned meter ABI header without compiling.')
    args = parser.parse_args()
    assert not (args.check and args.refresh_guards), 'Choose either --check or --refresh-guards.'
    data = stock_data(args.check or args.refresh_guards)
    addresses = addresses_from_builder(METER / 'build.py')
    addresses_from_builder(SETTINGS / 'build.py')
    expected_guard = guard_text(data, addresses)
    if args.refresh_guards:
        (METER / 'abi_guard.h').write_text(expected_guard, encoding='utf-8', newline='\n')
        print('PASS: SHA-locked meter ABI guard regenerated; 0x4f3f10/0x52607c/0x52f4f0 covered.')
        return
    assert f'releases/{VERSION}\\n' in (METER / 'meter.c').read_text(encoding='utf-8'), 'Host 180 source integration is not ready.'
    assert f"MENU_BRIDGE_VERSION='{UI_VERSION}'" in (SETTINGS / 'build.py').read_text(encoding='utf-8')
    assert (METER / 'abi_guard.h').read_text(encoding='utf-8') == expected_guard, 'Regenerate meter ABI guards first.'
    before = {path.relative_to(ROOT).as_posix(): sha(path) for path in PROVIDERS}
    if args.check:
        print(json.dumps({'ready': True, 'version': VERSION, 'menu_bridge_version': UI_VERSION, 'ui_guards': len(addresses), 'unchanged_provider_sha256': before}, indent=2))
        return
    assert BASE.is_dir(), 'Compile with WSL python3 and the existing pinned base.'
    out = METER / 'build'
    out.mkdir(exist_ok=True)
    subprocess.run([
        'clang', '--target=mipsel-linux-gnu', '-march=mips32r2', '-mabi=32',
        '-mhard-float', '-mfp64', '-mnan=2008', '-O2', '-Wall', '-Wextra',
        '-Werror', '-Wno-misleading-indentation', '-fno-builtin', '-fPIC',
        '-shared', '-nostdlib', '-fuse-ld=lld', '-L' + str(BASE / 'lib'),
        str(METER / 'meter.c'),
        '-Wl,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack',
        '-o', str(out / 'libdisc-bt-meter.so'),
    ], check=True)
    subprocess.run([sys.executable, str(SETTINGS / 'build.py')], check=True)
    sys.path.insert(0, str(SETTINGS))
    from elf_bridge import bridge
    (out / 'librt-ui-loader.so.1').write_bytes(bridge((BASE / 'lib/librt.so.1').read_bytes(), '/usr/data/disc-settings/module-ui.so'))
    abi = subprocess.run(['readelf', '-h', '-A', '-W', '-l', str(out / 'libdisc-bt-meter.so')], capture_output=True, text=True, check=True).stdout
    (out / 'abi.txt').write_text(abi, encoding='utf-8')
    assert 'nan2008' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi and 'RWE' not in abi
    dynamic = subprocess.run(['readelf', '-W', '-d', str(out / 'libdisc-bt-meter.so')], capture_output=True, text=True, check=True).stdout
    assert 'libm.so.6' in dynamic and 'libc.so.6' in dynamic
    assert before == {path.relative_to(ROOT).as_posix(): sha(path) for path in PROVIDERS}, 'A DSP/render provider unexpectedly changed.'
    report = {
        'version': VERSION, 'menu_bridge_version': UI_VERSION,
        'library_bytes': (out / 'libdisc-bt-meter.so').stat().st_size,
        'library_sha256': sha(out / 'libdisc-bt-meter.so'),
        'loader_sha256': sha(out / 'librt-ui-loader.so.1'),
        'constructor': False, 'new_daemon': False,
        'ui_calls_on_ui_thread': True, 'executable_stack': False,
        'source_sha256': source_hashes(), 'unchanged_provider_sha256': before,
        'build_script_sha256': sha(Path(__file__)),
    }
    (out / 'build.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
