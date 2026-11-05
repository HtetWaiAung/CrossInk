#!/usr/bin/env python3
"""Static regression check for X4 sleep-screen waveform selection.

Not a substitute for PlatformIO build or actual panel testing.
"""
from pathlib import Path
import re

src = Path(__file__).resolve().parents[1] / 'src/activities/boot_sleep/SleepActivity.cpp'
s = src.read_text(encoding='utf-8')
checks = []
for match in re.finditer(r'renderer\.displayBuffer\(([^;]*)\);', s, re.S):
    args = match.group(1)
    if 'TURN_OFF_SCREEN_AFTER_SLEEP_REFRESH' in args:
        checks.append(('displayBuffer', 'HalDisplay::HALF_REFRESH' in args, s.count('\n', 0, match.start()) + 1))
for match in re.finditer(r'renderer\.displayGrayscaleBase\(([^;]*)\);', s, re.S):
    args = match.group(1)
    if 'HalDisplay::HALF_REFRESH' in args:
        checks.append(('grayscaleBase', True, s.count('\n', 0, match.start()) + 1))
assert checks, 'No sleep-screen update calls found'
for kind, ok, line in checks:
    print(f'{"PASS" if ok else "FAIL"} {kind} line {line}')
assert all(ok for _, ok, _ in checks), 'Non-HALF sleep-screen refresh call found'
print(f'PASS: {len(checks)} X4 sleep refresh call sites meet the static policy')
