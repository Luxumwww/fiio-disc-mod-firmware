"""Reuse only the existing radial renderer's fixed FFT tables, no pixel data."""
from pathlib import Path
import hashlib
import json
import re

HERE = Path(__file__).resolve().parent
SOURCE = HERE.parents[1] / 'audio-visuals/spectrum/geometry.h'
text = SOURCE.read_text(encoding='utf-8')
out = [
    '/* Fixed Hann / twiddle / bit-reversal tables copied from the existing',
    ' * DISC 4096-point spectrum renderer. Regenerate with the adjacent script.',
    ' * No runtime trigonometry or radial rendering tables are included. */',
    '#ifndef DISC_DAC_HOME_SPECTRUM_TABLES_H',
    '#define DISC_DAC_HOME_SPECTRUM_TABLES_H',
]
for name in ('fft_window', 'fft_cos', 'fft_sin', 'fft_reverse'):
    match = re.search(r'static const (?:float|unsigned short) ' + name + r'\[\d+\]=\{[^}]+\};', text)
    if not match:
        raise ValueError('Missing upstream table: ' + name)
    out.append(match[0].replace(name, 'dac_home_' + name))
out.append('#endif')
target = HERE / 'spectrum_tables.h'
target.write_text('\n'.join(out) + '\n', encoding='utf-8')
record = {
    'source': str(SOURCE.relative_to(HERE.parents[3])),
    'source_sha256': hashlib.sha256(SOURCE.read_bytes()).hexdigest(),
    'generated_sha256': hashlib.sha256(target.read_bytes()).hexdigest(),
    'fft_samples': 4096,
    'tables_only': True,
}
(HERE / 'spectrum_tables.json').write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
print(json.dumps(record, indent=2))
