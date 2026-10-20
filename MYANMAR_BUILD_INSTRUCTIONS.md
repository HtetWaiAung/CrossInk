# XTEINK X4 PlatformIO build (not yet compiled)

## Result in this environment
- `python -m platformio --version` failed: `No module named platformio`.
- `pip install platformio` failed: package index inaccessible / no distribution found in this environment.
- `freeink-sdk` is empty in source ZIP archives: GitHub source archives do not contain git submodule data. The `assets/tabler-icons` submodule is also empty.
- No `.bin` was produced. No hardware validation was performed.

## Build in a GitHub fork (recommended)
1. Fork `uxjulia/CrossInk` at tag `v1.6.1` preserving `.gitmodules` and original gitlink submodule references. Create a `myanmar-x4` branch.
2. Copy the modified source files from this ZIP onto that branch **without replacing the fork's `.git` metadata or submodule references**. Do not copy the empty `freeink-sdk` and `assets/tabler-icons` directories over checked-out submodules.
3. In the cloned repo: `git submodule update --init --recursive`.
4. Commit and push the modified files, including `.github/workflows/myanmar-x4-build.yml`.
5. On GitHub, open Actions -> Build Myanmar X4 firmware -> Run workflow, or push to `myanmar-x4` branch.
6. Review build log. On success, download `crossink-myanmar-x4-build` artifact; validate the target, flash layout and firmware before deploying to hardware.

## Build locally (with network and PlatformIO)
```bash
# Execute from a real git clone, with submodules initialized
python -m pip install 'https://github.com/pioarduino/platformio-core/archive/refs/tags/v6.1.19.zip'
git submodule update --init --recursive
pio run -j1 -e default
```

The firmware target is `default` (X3/X4). No changes to X4 sleep-screen logic are included as part of this build step. Building may expose unresolved porting errors. Do not flash until the build succeeds and device-specific behavior is validated.
