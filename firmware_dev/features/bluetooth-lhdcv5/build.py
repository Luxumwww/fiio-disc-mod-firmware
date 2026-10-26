"""Build host and stock-ABI MIPS codec adapters without touching a device."""
from pathlib import Path
import hashlib,json,shutil,subprocess,difflib
P=Path(__file__).resolve().parent
O=P/'build';O.mkdir(exist_ok=True)
PORT=O/'decoder';PORT.mkdir(exist_ok=True)
patches=[]
for f in (P/'third_party/decoder/decoder').iterdir():
 if not f.is_file():continue
 before=f.read_text();after=before
 if f.name=='lhdc_dec.c':
  after=after.replace('timespec_get(&ts, TIME_UTC);','clock_gettime(CLOCK_MONOTONIC, &ts);')
  after=after.replace('(int32_t)(lv << 8)','(int32_t)((uint32_t)lv << 8)').replace('(int32_t)(rv << 8)','(int32_t)((uint32_t)rv << 8)')
  # Explicit 32-bit bit assembly/negation; signed overflow is undefined at O3.
  # Normal matched streams retain identical values; INT_MIN is handled on o32
  # as well as x86_64 (where long happened to hide one of the old overflows).
  replacements={
   'unsigned u = (n < 0) ? (unsigned)(-(long)n) : (unsigned)n;':
    'uint32_t u = n < 0 ? 0u - (uint32_t)n : (uint32_t)n;',
   'int s = (pred + 0x40) >> 7;':
    '''int s = (pred + 0x40) >> 7;
        /* Predictor is bounded by calc_bits<=31; fail closed if violated. */
        if ((unsigned)s >= 32u) {
            for (int j = k; j < cnt; j++) out[j] = 0;
            return INT64_MAX;
        }''',
   'int mag = ((int)coeff[k] << s) | mant;':
    'int32_t mag = (int32_t)(((uint32_t)coeff[k] << (unsigned)s) | (uint32_t)mant);',
   'if (neg) val = -mag;':
    'if (neg) val = (int32_t)(0u - (uint32_t)mag);',
   'int64_t a = qs[k] < 0 ? -qs[k] : qs[k];':
    'int64_t a = (int64_t)qs[k]; if (a < 0) a = -a;',
  }
  for old,new in replacements.items():
   assert after.count(old)==1,('Upstream integer guard changed',old)
   after=after.replace(old,new)
  start=after.index('static float lhdc_dec_level_cal(')
  end=after.index('\n/*',start)
  after=after[:start]+'''/* DISC measured broadband/tone round trips: after rate_norm the
 * reconstruction gain is 16/15 at every tested rate and 16/24-bit depth.
 * Replace signal-dependent per-rate ear trims (including +0.79 dB at 192k)
 * with its exact inverse. 0.1 dB headroom accommodates codec reconstruction
 * peaks before integer conversion; no compressor/filter/soft saturation. */
static float lhdc_dec_level_cal(uint32_t sample_rate, int bit_depth)
{
    (void)sample_rate; (void)bit_depth;
    return 0.9375f * 0.988553095f;
}
''' +after[end:]
  needle='int conceal = (frame_peak > 2.0f * clip_lim);'
  assert after.count(needle)==1
  after=after.replace(needle,needle+'''
    disc_core_quality.frames++;
    disc_core_quality.concealed_frames += conceal != 0;
    disc_core_quality.clipped_frames += clip_cnt != 0;
    disc_core_quality.clipped_samples += (unsigned)clip_cnt;
    float peak_q16 = frame_peak / clip_lim * 65536.0f;
    if (isfinite(peak_q16) && peak_q16 > disc_core_quality.max_peak_q16)
        disc_core_quality.max_peak_q16 = peak_q16 < 4294967040.0f ? (uint32_t)peak_q16 : UINT32_MAX;
''')
 if before!=after:patches.extend(difflib.unified_diff(before.splitlines(True),after.splitlines(True),fromfile='upstream/'+f.name,tofile='port/'+f.name))
 (PORT/f.name).write_text(after)
(P/'reports/port.patch').write_text(''.join(patches))
core=[str(f) for f in sorted(PORT.glob('*.c'))]+[str(P/'codec_compat.c')]
common=['-std=c11','-O3','-ffp-contract=off','-fPIC','-fvisibility=hidden','-D_POSIX_C_SOURCE=200809L','-DLHDC_HOST_BUILD','-I'+str(PORT),'-I'+str(P)]
base=Path('/root/disc-official-component-base-20261004-r1/base')
sysroot=Path('/root/disc-accel-research-20261006/toolchain/mips-linux-gnu/libc')
targets={'host':[], 'mips':['--target=mipsel-linux-gnu','--sysroot='+str(sysroot),'-march=mips32r5','-mabi=32','-mhard-float','-mfp64','-mnan=2008']}
artifacts={}
for name,flags in targets.items():
 objs=[]
 for f in core:
  obj=O/(Path(f).stem+'-'+name+'.o')
  subprocess.run(['clang',*flags,*common,'-include',str(P/'runtime_compat.h'),'-c',f,'-o',str(obj)],check=True);objs.append(str(obj))
 obj=O/('disc_lhdcv5-'+name+'.o');subprocess.run(['clang',*flags,*common,'-Wall','-Wextra','-Werror','-c',str(P/'disc_lhdcv5.c'),'-o',str(obj)],check=True);objs.append(str(obj))
 library=O/('libdisc-lhdcv5-host.so' if name=='host' else 'libdisc-lhdcv5.so')
 libs=['-lm'] if name=='host' else ['-nostdlib','-fuse-ld=lld','-L'+str(base/'lib'),'-Wl,-l:libm.so.6,-l:libc.so.6,-l:libgcc_s.so.1,--no-undefined,-z,noexecstack']
 subprocess.run(['clang',*flags,'-shared',*objs,*libs,'-o',str(library)],check=True)
 artifacts[name]={'sha256':hashlib.sha256(library.read_bytes()).hexdigest(),'bytes':library.stat().st_size}
 if name=='mips':
  abi=subprocess.check_output(['readelf','-h','-A',str(library)],text=True);(P/'reports/mips-abi.txt').write_text(abi)
  assert 'nan2008' in abi and 'Hard float (32-bit CPU, 64-bit FPU)' in abi
  dyn=subprocess.check_output(['readelf','-d','-W',str(library)],text=True);(P/'reports/mips-dependencies.txt').write_text(dyn)
  dis=subprocess.check_output(['/root/disc-accel-research-20261006/toolchain/bin/mips-linux-gnu-objdump','-d',str(library)],text=True)
  (P/'reports/fpu-evidence.txt').write_text('\n'.join(line for line in dis.splitlines() if any(op in line for op in ['mul.s','add.s','sub.s','div.s','sqrt.s']))[:16000])
subprocess.run(['clang','-std=c99','-O3','-ffp-contract=off','-fPIC','-shared','-I'+str(P/'third_party/encoder/include'),'-I'+str(P/'third_party/encoder/src'),*[str(f) for f in sorted((P/'third_party/encoder/src').glob('*.c'))],'-lm','-o',str(O/'liblhdcv5-enc-host.so')],check=True)
subprocess.run(['clang','-std=c11','-O2',str(P/'test_adapter.c'),'-L'+str(O),'-Wl,-rpath,$ORIGIN','-l:libdisc-lhdcv5-host.so','-o',str(O/'test_adapter')],check=True)
(P/'reports/build.json').write_text(json.dumps(artifacts,indent=2)+'\n')
print(json.dumps(artifacts,indent=2))
