#!/usr/bin/env python3
"""Builds one port in one configuration, as the GitHub workflow does
(.github/workflows/build.yml), and collects what it built into dist/:

    python tools/ci_build.py linux debug
    python tools/ci_build.py android release
    python tools/ci_build.py linux profile
    python tools/ci_build.py web release

Builds are portable (any x86-64 processor), so they run on other
computers. Debug builds skip link-time and profile-guided optimisation,
which only make the build slower; release builds use both, as a local
release build does (profile-guided optimisation needs clang 22 or later,
and is skipped with an older one). A profile build is a debug build with
configure.py --profile, so that the profiling build is built too. CI_COMPILER_LAUNCHER (ccache, say) is
passed on as --compiler-launcher. A build of the main branch gets the run's
number (HALO_BUILD_NUMBER), which its release is named after and the
self-updater compares (and the web site's version.json names).

The web build (port/web/README.md) needs Emscripten (emcc on the PATH, as
emsdk_env.sh puts it); dist/halo-web-<config>/ is the whole site, which a
static host serves as it is: the workflow publishes the release build's to
GitHub Pages.

    python tools/ci_build.py sums release/

writes release/SHA256SUMS: a line for each file of the folder (the
release's assets: the workflow zips each build there), "<sha256 hex>  <name>",
as sha256sum writes it and `sha256sum -c` reads it. The workflow signs it
with the release key (tools/release_sign.sh: SHA256SUMS.sig) and publishes
both with the release.
"""

import argparse
import hashlib
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# what each port's build leaves, and what goes into dist/
OUTPUTS = {
    # (and the SDL3 the portable build brings, tools/linux_build.py, with its
    # zlib license, as the release carries the other libraries')
    "linux": ["build/linux/halo", "build/linux/libSDL3.so.0", "build/linux/SDL3-LICENSE.txt"],
    "windows": ["build/windows/halo.exe", "build/windows/SDL3.dll"],
    "android": [],  # the APK, below
    "web": [],  # the site, below
}
APKS = {
    "debug": "port/android/app/build/outputs/apk/debug/app-debug.apk",
    "release": "port/android/app/build/outputs/apk/release/app-release.apk",
}


def run(command, cwd=ROOT):
    print("+", " ".join(str(part) for part in command), flush=True)
    subprocess.run([str(part) for part in command], cwd=cwd, check=True)


def write_sums(folder: Path) -> int:
    """folder/SHA256SUMS: each file's SHA-256 and name, in name order (the
    sums file itself and its signature left out)"""
    lines = []
    for path in sorted(folder.iterdir(), key=lambda item: item.name):
        if not path.is_file() or path.name in ("SHA256SUMS", "SHA256SUMS.sig"):
            continue
        digest = hashlib.sha256()
        with path.open("rb") as file:
            for chunk in iter(lambda: file.read(1 << 20), b""):
                digest.update(chunk)
        lines.append(f"{digest.hexdigest()}  {path.name}\n")
    if not lines:
        print(f"{folder}: no files to sum", file=sys.stderr)
        return 1
    (folder / "SHA256SUMS").write_text("".join(lines), encoding="utf-8", newline="\n")
    print(f"{folder / 'SHA256SUMS'}: {len(lines)} files", flush=True)
    return 0


def main() -> int:
    if len(sys.argv) == 3 and sys.argv[1] == "sums":
        return write_sums(Path(sys.argv[2]))
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("platform", choices=sorted(OUTPUTS))
    parser.add_argument("config", choices=["debug", "release", "profile"])
    args = parser.parse_args()

    configure = [sys.executable, "configure.py", "--portable"]
    if args.config == "release":
        configure.append("--release")
    else:
        configure += ["--lto=off", "--pgo=off"]
    if args.config == "profile":
        configure.append("--profile")
    launcher = os.environ.get("CI_COMPILER_LAUNCHER")
    if launcher:
        configure += ["--compiler-launcher", launcher]
    # a build of main knows its number, which names its release (build-<n>),
    # for the self-updater (port/linux/src/updater.c, and the Android app);
    # other builds have none, and never look for updates
    if os.environ.get("GITHUB_REF") == "refs/heads/main" and os.environ.get("GITHUB_RUN_NUMBER", "").isdigit():
        os.environ["HALO_BUILD_NUMBER"] = os.environ["GITHUB_RUN_NUMBER"]
        print(f"build number {os.environ['HALO_BUILD_NUMBER']}", flush=True)
    run(configure)

    if args.platform == "web":
        # the whole site (tools/web_build.py), its licences in licenses.txt
        run(["ninja", "web"])
        dist = ROOT / "dist" / f"halo-web-{args.config}"
        if dist.exists():
            shutil.rmtree(dist)
        shutil.copytree(ROOT / "build/web/site", dist)
        print(f"build/web/site -> {dist.relative_to(ROOT)}", flush=True)
        return 0
    if args.platform == "android":
        # the native part, then the app around it (Gradle's variant of the
        # same name: release is signed with the debug key, not debuggable)
        run(["ninja", "android"])
        gradlew = "gradlew.bat" if os.name == "nt" else "./gradlew"
        variant = "release" if args.config == "release" else "debug"
        run([gradlew, "--console=plain", "-q", f"assemble{variant.capitalize()}"], cwd=ROOT / "port/android")
        outputs = [APKS[variant]]
    else:
        run(["ninja", args.platform])
        outputs = OUTPUTS[args.platform]

    dist = ROOT / "dist" / f"halo-{args.platform}-{args.config}"
    if dist.exists():
        shutil.rmtree(dist)
    dist.mkdir(parents=True)
    for output in outputs:
        shutil.copy2(ROOT / output, dist)
        print(f"{output} -> {dist.relative_to(ROOT)}", flush=True)
    if args.platform == "windows":
        # the symbols of halo.exe and SDL3.dll, apart (players do not need
        # them): the workflow uploads them to Sentry, which turns the crash
        # reports' minidumps into function names and lines
        # (port/windows/src/win32_crash.c), and tools/symbolize_crash.py
        # reads debug.txt's crash lines with them
        symbols = ROOT / "dist" / f"halo-windows-{args.config}-symbols"
        if symbols.exists():
            shutil.rmtree(symbols)
        symbols.mkdir(parents=True)
        for pdb in [ROOT / "build/windows/halo.pdb", *sorted((ROOT / "build/windows/third_party").glob("SDL3-*/lib/x86/SDL3.pdb"))]:
            shutil.copy2(pdb, symbols)
            print(f"{pdb.relative_to(ROOT)} -> {symbols.relative_to(ROOT)}", flush=True)
    # OpenCE's own licence, the AGPL, which asks each copy to carry its text
    shutil.copy2(ROOT / "LICENSE", dist / "LICENSE.txt")
    if args.platform != "web":
        # internet play's DTLS with browsers, and the Linux self-updater's TLS
        # (port/third_party/mbedtls), whose Apache license asks the same
        shutil.copy2(ROOT / "port/third_party/mbedtls/LICENSE", dist / "mbedtls-LICENSE.txt")
    # internet play's UPnP (port/third_party/miniupnpc), in every build,
    # whose BSD license asks binaries to carry its notice
    shutil.copy2(ROOT / "port/third_party/miniupnpc/LICENSE", dist / "miniupnpc-LICENSE.txt")
    # the text's fonts (port/assets/fonts), embedded in every build, whose
    # SIL Open Font License asks each copy to carry it
    shutil.copy2(ROOT / "port/assets/fonts/Overpass-OFL.txt", dist / "Overpass-OFL.txt")
    # the menus' XML parser (port/third_party/expat), in every build, whose
    # MIT license asks copies to carry its notice
    shutil.copy2(ROOT / "port/third_party/expat/COPYING", dist / "expat-COPYING.txt")
    # MCC's texture decoder is distributed under bcdec's MIT option.
    shutil.copy2(ROOT / "port/third_party/bcdec/LICENSE", dist / "bcdec-LICENSE.txt")
    if args.platform == "android":
        # the Steam Workshop screen's Steam client (port/android/host/steam_*.c):
        # Zstandard's decoder, whose BSD license asks binaries to carry its
        # notice (the LZMA SDK's decoder is public domain)
        shutil.copy2(ROOT / "port/third_party/zstd/LICENSE", dist / "zstd-LICENSE.txt")
    # voice chat's codec (port/third_party/opus), in every build, whose BSD
    # license asks binaries to carry its notice
    shutil.copy2(ROOT / "port/third_party/opus/COPYING", dist / "opus-COPYING.txt")
    # the JSON tokenizer (port/third_party/jsmn) of the server browser's
    # lists and the dedicated server's API, in every build, whose MIT
    # license asks copies to carry its notice
    shutil.copy2(ROOT / "port/third_party/jsmn/LICENSE", dist / "jsmn-LICENSE.txt")
    # voice chat's speaker icons and the Mods menu's warning icons
    # (port/assets/icons/lucide, drawn into the menus' bitmaps), whose ISC
    # license asks copies to carry its notice
    shutil.copy2(ROOT / "port/assets/icons/lucide/LICENSE", dist / "lucide-LICENSE.txt")
    # internet play's MQTT brokers, a file beside the game (network.brokers_file;
    # Android's APK has its own copy)
    if args.platform != "android":
        shutil.copy2(ROOT / "port/assets/network/brokers.txt", dist / "brokers.txt")
        # the master servers that list dedicated servers, for the server
        # browser and the dedicated server (network.masters_file and
        # server.masters_file, docs/dedicated_server.md)
        masters = ROOT / "port/assets/network/masters.txt"
        if masters.exists():
            shutil.copy2(masters, dist / "masters.txt")
    if args.platform in ("linux", "windows"):
        # the dedicated server's examples: init.txt's commands and gametype
        # files (docs/dedicated_server.md, "Files")
        shutil.copy2(ROOT / "deploy/init.txt.example", dist / "init.txt.example")
        shutil.copytree(ROOT / "deploy/gametypes", dist / "gametypes")
    if args.platform == "linux":
        # halo --server's script, and the ways to deploy a server
        # (deploy/: systemd, Docker, Pterodactyl, Kubernetes, other panels)
        shutil.copy2(ROOT / "deploy/halo-server", dist / "halo-server")
        shutil.copytree(ROOT / "deploy", dist / "deploy", ignore=shutil.ignore_patterns("*.zip", "__pycache__"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
