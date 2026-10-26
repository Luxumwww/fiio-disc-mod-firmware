"""Build only Host 1.3.189 and UI 1.10.189 using the pinned WSL toolchain.

--check is read-only (also works on Windows). No provider builders, packaging,
ADB, playback or volume operations are performed. Quoted includes are locked
recursively; a source edit during compilation rejects the build.
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
VERSION = '1.3.189'
UI_VERSION = '1.10.189'
STOCK_SHA = {
    'usr/bin/mq_ui': 'f9b40d825f96906146c31b56a420ab90e6f685f6e2c4761bf867989e12b04226',
    'usr/bin/mq_player': 'a5a6740435758bb3f3d008a4306c93a463c6634318957bbf32086bd7b00fabbc',
    'lib/librt.so.1': '4192eaea2420074471cfb2917121892fae607f44fc69c3629bfe6b71040dfb1e',
}
PINNED_PROVIDERS = {
    'hardware-accel/runtime-provider/build/libdisc-hwaccel.so': 'e94e7c99b2f0c16d331f702ae7f78638bc9b55b659357c8614dd1fc6120b71a1',
    'audio-dsp/build/libdisc-audio-dsp.so': '9153b9bb4c07298b57eabc8eac3e66ee94bd36584c36926f51b80c902b9deebe',
    'audio-visuals/xy/build/libdisc-audio-xy.so': 'ae5f8e51fa5f06f0ad0ccf9348f36f3322a5a77342cd6ff4e58cca206339e2e0',
    'audio-visuals/polar/build/libdisc-audio-polar.so': '4bb8ed70560cfc840d617b959b6cc685102ba4d3f7d5e776d6004f1f130cde78',
    'audio-visuals/spectrum/build/libdisc-audio-spectrum.so': 'c7133f41dfe179ea236c82c87cb00eefa3f468ad10561be4ab1ea7380886adc0',
    'audio-visuals/mandala/build/libdisc-audio-mandala.so': 'b9c76466fd1fd668d4e93e31eb376c2691c6bbd4ffb650a1dc42d3ed13875e8e',
}
REQUIRED_UI_GUARDS = {0x4f3f10, 0x52607c, 0x52f4f0}


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def provider_hashes():
    result = {}
    for name, expected in PINNED_PROVIDERS.items():
        path = FEATURES / name
        require(path.is_file() and sha(path) == expected, 'Pinned provider changed: ' + name)
        result[path.relative_to(ROOT).as_posix()] = expected
    return result


def addresses_from_builder(path):
    # Parse literals only. Importing bluetooth-meter/build.py rebuilds providers.
    addresses = []
    for node in ast.parse(path.read_text(encoding='utf-8')).body:
        if isinstance(node, ast.Assign) and any(isinstance(t, ast.Name) and t.id == 'addresses' for t in node.targets):
            addresses = list(ast.literal_eval(node.value))
        elif isinstance(node, ast.AugAssign) and isinstance(node.target, ast.Name) and node.target.id == 'addresses':
            require(isinstance(node.op, ast.Add), 'Unexpected addresses operation')
            addresses.extend(ast.literal_eval(node.value))
        elif isinstance(node, ast.Expr) and isinstance(node.value, ast.Call):
            call = node.value
            if isinstance(call.func, ast.Attribute) and isinstance(call.func.value, ast.Name) and call.func.value.id == 'addresses' and call.func.attr == 'append':
                addresses.append(ast.literal_eval(call.args[0]))
    require(REQUIRED_UI_GUARDS.issubset(addresses), 'Incomplete ABI guards: ' + str(path))
    return addresses


def stock_data(check_only):
    base = BASE
    if check_only and not (base / 'usr/bin/mq_ui').is_file():
        base = ROOT / 'firmware_analysis/main_os_ota_v257/files'
    result = {}
    for name, expected in STOCK_SHA.items():
        path = base / name
        require(path.is_file() and sha(path) == expected, 'Unexpected/missing original: ' + str(path))
        result[name] = path.read_bytes()
    return result


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
        require(path.is_file() and path.is_relative_to(FEATURES), 'Missing/outside quoted source: ' + str(path))
        seen.add(path)
        # Includes in conditional branches also belong to the source lock.
        for name in re.findall(r'^\s*#\s*include\s*"([^"\r\n]+)"', path.read_text(encoding='utf-8'), re.M):
            pending.append((path.parent / name).resolve())
    return {p.relative_to(ROOT).as_posix(): sha(p) for p in sorted(seen)}


def builder_hashes():
    return {p.relative_to(ROOT).as_posix(): sha(p) for p in (Path(__file__).resolve(), SETTINGS / 'build.py', SETTINGS / 'elf_bridge.py', METER / 'build.py', FEATURES / 'player-utilities/generate_params.py', FEATURES / 'mobile-remote/dsp-schema.json')}


def validate_build(report):
    require(report.get('version') == VERSION and report.get('menu_bridge_version') == UI_VERSION, 'Wrong build version')
    require(report.get('source_sha256') == source_hashes(), 'Sources changed after build')
    require(report.get('build_inputs_sha256') == builder_hashes(), 'Build helpers changed after build')
    require(report.get('unchanged_provider_sha256') == provider_hashes(), 'Provider lock changed')
    for relative, expected in report['artifacts_sha256'].items():
        require(sha(ROOT / relative) == expected, 'Build artifact changed: ' + relative)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    data = stock_data(args.check)
    addresses = addresses_from_builder(METER / 'build.py')
    addresses_from_builder(SETTINGS / 'build.py')
    require(f'releases/{VERSION}\\n' in (METER / 'meter.c').read_text(encoding='utf-8'), 'Host 186 source is not ready')
    require(f"MENU_BRIDGE_VERSION='{UI_VERSION}'" in (SETTINGS / 'build.py').read_text(encoding='utf-8'), 'UI 186 source is not ready')
    require((METER / 'abi_guard.h').read_text(encoding='utf-8') == guard_text(data, addresses), 'Meter ABI guards do not match pinned stock')
    providers = provider_hashes()
    helpers = builder_hashes()
    if args.check:
        before = source_hashes()
        print(json.dumps({'ready': True, 'version': VERSION, 'menu_bridge_version': UI_VERSION, 'quoted_source_count': len(before), 'source_sha256': before, 'unchanged_provider_sha256': providers}, indent=2))
        return
    require(BASE.is_dir(), 'Compile with WSL python3 and the pinned base')
    require('--guards-only' in (SETTINGS / 'build.py').read_text(encoding='utf-8'), 'UI builder must implement the guards-only entry first')
    subprocess.run([sys.executable, str(SETTINGS / 'build.py'), '--guards-only'], check=True)
    require(builder_hashes() == helpers and provider_hashes() == providers, 'Build helper/provider changed while refreshing original guards')
    before = source_hashes()
    out = METER / 'build'
    out.mkdir(exist_ok=True)
    subprocess.run([
        'clang', '--target=mipsel-linux-gnu', '-march=mips32r2', '-mabi=32',
        '-mhard-float', '-mfp64', '-mnan=2008', '-O2', '-Wall', '-Wextra',
        '-Werror', '-Wno-misleading-indentation', '-fno-builtin', '-fPIC',
        '-shared', '-nostdlib', '-fuse-ld=lld', '-L' + str(BASE / 'lib'),
        str(METER / 'meter.c'), '-Wl,-l:libm.so.6,-l:libc.so.6,--no-undefined,-z,noexecstack',
        '-o', str(out / 'libdisc-bt-meter.so'),
    ], check=True)
    # The UI builder regenerates stock ABI headers and the host FNV guard;
    # it invokes no provider builder. Never import the Bluetooth builder.
    require(source_hashes() == before and builder_hashes() == helpers, 'Source changed during host compilation')
    subprocess.run([sys.executable, str(SETTINGS / 'build.py')], check=True)
    after = source_hashes()
    generated = 'firmware_dev/features/settings-debug/bt_module_guard.h'
    require(set(after) == set(before), 'Quoted include closure changed during build')
    require(all(after[k] == v for k, v in before.items() if k != generated), 'Source changed during UI compilation')
    require(builder_hashes() == helpers, 'Build helper changed during compilation')
    host = (out / 'libdisc-bt-meter.so').read_bytes()
    fnv = 2166136261
    for b in host:
        fnv = ((fnv ^ b) * 16777619) & 0xffffffff
    expected = f'#define BT_MODULE_SIZE {len(host)}u\n#define BT_MODULE_FNV {fnv}u\n'
    require((ROOT / generated).read_text(encoding='utf-8') == expected, 'UI host guard mismatch')
    sys.path.insert(0, str(SETTINGS))
    from elf_bridge import bridge
    loader = out / 'librt-ui-loader.so.1'
    loader.write_bytes(bridge(data['lib/librt.so.1'], '/usr/data/disc-settings/module-ui.so'))
    for artifact in (out / 'libdisc-bt-meter.so', SETTINGS / 'build/disc-settings.so'):
        abi = subprocess.run(['readelf', '-h', '-A', '-W', '-l', str(artifact)], capture_output=True, text=True, check=True).stdout
        require('nan2008' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi and 'RWE' not in abi, 'Unexpected ELF ABI/stack: ' + str(artifact))
    require(provider_hashes() == providers, 'Provider changed during build')
    artifacts = (out / 'libdisc-bt-meter.so', SETTINGS / 'build/disc-settings.so', loader)
    report = {
        'version': VERSION, 'menu_bridge_version': UI_VERSION,
        'library_sha256': sha(artifacts[0]), 'menu_sha256': sha(artifacts[1]), 'loader_sha256': sha(loader),
        'hardware_provider_version': '1.0.6', 'dsp_version': '1.15.0',
        'source_sha256': after, 'build_inputs_sha256': helpers,
        'artifacts_sha256': {p.relative_to(ROOT).as_posix(): sha(p) for p in artifacts},
        'unchanged_provider_sha256': providers, 'provider_builds': False,
        'constructor': False, 'new_daemon': False, 'ui_calls_on_ui_thread': True,
        'executable_stack': False,
    }
    validate_build(report)
    (out / 'build.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    (HERE / 'build.json').write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
