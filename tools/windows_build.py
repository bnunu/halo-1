"""Ninja rules for the native Windows build (``ninja windows``).

Like the Linux build (tools/linux_build.py), it compiles the game sources
with clang for 32-bit x86 Windows (i686-pc-windows-msvc), adds the platform layer shared with
Linux (``port/linux/src``) and the Windows parts in ``port/windows``, and
links ``build/windows/halo.exe`` with lld. It is generated when
configure.py runs on Windows, or with configure.py --windows-cross, which
cross-compiles it from Linux (tools/windows_cross_build.py) against the
MSVC toolchain of --windows-vctools-dir and --windows-sdk-dir. See
port/windows/README.md for the design.
"""

import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import urllib.request
import zipfile
from pathlib import Path
from typing import Any, Dict, List, Optional

from .linux_build import (LINUX_PROFILE, MINIUPNPC_DIR, OPTIMISATION, STB_VORBIS_SOURCE, WINDOWS_PROFILE, XDK_INCLUDE,
                          build_repository,
                          lto_mode, march_flag, miniupnpc_sources, pgo_mode, compile_launcher, configuration_defines,
                          game_defines_and_includes, game_sources, musl_math_cflags, musl_math_sources, opus_cflags,
                          opus_sources, pgo_profile, profile_use_flags, xdk_headers)
from .embed_assets import hud_assets_build, hud_configure_inputs, server_ui_build
from .ninja_syntax import Writer
from .windows_cross_build import cross_toolchain_prefix

LINUX_DIR = Path("port/linux")
PORT_DIR = Path("port/windows")
PORT_CONFIG = PORT_DIR / "port.json"
BUILD = Path("build/windows")

# (as tools/android_build.py's SDL_TAG and tools/linux_sysroot.py's SDL_VERSION)
SDL_VERSION = "3.4.16"
SDL_URL = (
    f"https://github.com/libsdl-org/SDL/releases/download/release-{SDL_VERSION}/"
    f"SDL3-devel-{SDL_VERSION}-VC.zip"
)
# (its SHA-256 when it was pinned: the DLL ships in the builds)
SDL_SHA256 = "1a784cb2a5c64d56fe7a62090fe9d242d9865f235e4ea9678f1a6ba4e693e7de"
THIRD_PARTY = BUILD / "third_party"
SDL_DIR = THIRD_PARTY / f"SDL3-{SDL_VERSION}"

# Flags shared by every unit. The Microsoft target gives the game the ABI it
# was written against natively: 16-bit wchar_t, MSVC structure layout,
# __declspec, calling conventions and COMDAT inline functions.
#  - 32-bit time_t, as in the Xbox's C runtime (the same on both sides of the
#    platform layer, which share struct timespec),
#  - tentative definitions shared between units (-fcommon),
#  - no optimisations that assume the absence of MSVC-tolerated UB,
#  - EBP frames (MSVC /Oy-): get_return_eip and the stack walker follow the
#    frame chain.
# the TOML parser the platform layer reads config.toml with (port_config.c)
TOML_DIR = Path("port/third_party/tomlc17")
EXPAT_DIR = Path("port/third_party/expat")
EXPAT_SOURCES = ("xmlparse.c", "xmlrole.c", "xmltok.c", "random_rand_s.c")
KCP_DIR = Path("port/third_party/kcp")
MONOCYPHER_DIR = Path("port/third_party/monocypher")
# internet play's DTLS with browsers (port/linux/src/posix_dtls.c)
MBEDTLS_DIR = Path("port/third_party/mbedtls")
# the port's zlib (port/third_party/zlib/zlib_prefixed.h), which inflates
# the maps, the menus' and the HUD's PNGs and the updates
ZLIB_DIR = Path("port/third_party/zlib")
ZLIB_SOURCES = ("adler32.c", "crc32.c", "inffast.c", "inflate.c", "inftrees.c", "uncompr.c", "zutil.c")
# (its names prefixed, and the one Z_PREFIX leaves, its error messages, which
# the game's zlib names the same)
ZLIB_DEFINES = ("-DZ_PREFIX", "-Dz_errmsg=z_port_errmsg")


def updater_defines(release: bool) -> str:
    """the self-updater's build (port/linux/src/updater.c): its number, from
    HALO_BUILD_NUMBER (tools/ci_build.py gives it for builds of main; none
    elsewhere, which never look for updates), and its configuration"""
    number = os.environ.get("HALO_BUILD_NUMBER", "0")
    if not number.isdigit():
        number = "0"
    flavor = "release" if release else "debug"
    return (f'-DHALO_BUILD_NUMBER={number} -DHALO_BUILD_FLAVOR=\\"{flavor}\\" '
            f'-DHALO_BUILD_REPOSITORY=\\"{build_repository()}\\"')

WINDOWS_ABI_FLAGS = [
    "--target=i686-pc-windows-msvc",
    "-fms-extensions",
    "-fcommon",
    "-fno-strict-aliasing",
    "-fwrapv",
    "-fno-delete-null-pointer-checks",
    "-fno-omit-frame-pointer",
    # the same floating point results on every port (every machine in a
    # system link game simulates it from the same inputs): no
    # fused multiply-adds (port/include/halo_math.h)
    "-ffp-contract=off",
    OPTIMISATION,
    "-g",
    "-gcodeview",
    "-D_USE_32BIT_TIME_T",
    "-D_CRT_SECURE_NO_WARNINGS",
    "-D_CRT_NONSTDC_NO_WARNINGS",
    "-D_WINSOCK_DEPRECATED_NO_WARNINGS",
    # the MSVC 7 signatures the game uses: swprintf without a buffer size,
    # two-argument wcstok
    "-D_CRT_NON_CONFORMING_SWPRINTFS",
    "-D_CRT_NON_CONFORMING_WCSTOK",
]

GAME_FLAGS = [
    "-std=gnu89",
    "-w",
    "-Wno-error=incompatible-pointer-types",
    "-Wno-error=incompatible-function-pointer-types",
    "-Wno-error=int-conversion",
    "-Wno-error=implicit-function-declaration",
    "-Wno-error=implicit-int",
    "-Wno-error=return-type",
]

PLATFORM_FLAGS = [
    "-std=gnu11",
    "-DHALO_LINUX_PLATFORM_LAYER",
    "-Wall",
    "-Wno-unused-function",
    "-Wno-unknown-pragmas",
    "-Wno-microsoft-anon-tag",
    "-Wno-pragma-pack",
    "-Wno-ignored-attributes",
    "-Wno-duplicate-decl-specifier",
    "-Wno-missing-braces",
    "-Wno-unused-variable",
    "-Wno-ignored-pragmas",
    "-Wno-microsoft-enum-forward-reference",
    "-Wno-language-extension-token",
]

# port/windows/src/win32_*.c talk to Windows only: they see the Windows SDK
# instead of the Xbox SDK (the two define the same names differently).
WIN32_FLAGS = [
    "-std=gnu11",
    "-DWIN32_LEAN_AND_MEAN",
    "-DNOMINMAX",
    "-D_WIN32_WINNT=0x0A00",
    "-Wall",
]


def _load_config() -> Dict[str, Any]:
    with open(PORT_CONFIG, "r", encoding="utf-8") as f:
        return json.load(f)


def windows_configure_inputs() -> List[Path]:
    """Files whose change must re-run configure.py."""
    return [
        Path(__file__),
        Path(__file__).with_name("windows_cross_build.py"),
        PORT_CONFIG,
        # (copied into build/windows by windows_resources)
        PORT_DIR / "halo.rc",
        PORT_DIR / "src",
        LINUX_DIR / "src",
        LINUX_DIR / "game",
        *hud_configure_inputs(),
    ]


def _quote(path: Any) -> str:
    text = str(path).replace(os.sep, "/")
    return f'"{text}"' if " " in text else text


def fetch_sdl() -> None:
    """Downloads SDL3's Visual C++ development package (headers, import
    library and DLL) once."""
    if (SDL_DIR / "lib" / "x86" / "SDL3.lib").is_file():
        return
    THIRD_PARTY.mkdir(parents=True, exist_ok=True)
    archive = THIRD_PARTY / f"SDL3-devel-{SDL_VERSION}-VC.zip"
    print(f"Downloading {SDL_URL}")
    with urllib.request.urlopen(SDL_URL) as response, open(archive, "wb") as f:
        shutil.copyfileobj(response, f)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    if digest != SDL_SHA256:
        archive.unlink()
        raise SystemExit(f"{SDL_URL}: SHA-256 {digest}, not the pinned {SDL_SHA256}")
    with zipfile.ZipFile(archive) as z:
        z.extractall(THIRD_PARTY)
    archive.unlink()


# The runtime of instrumented builds (-fprofile-generate), which LLVM for
# Windows ships only for x86-64: its C sources, from the LLVM release of the
# compiler that uses them, built for the 32-bit game.
PROFILE_RUNTIME_SOURCES = [
    "InstrProfiling.c", "InstrProfilingBuffer.c", "InstrProfilingFile.c", "InstrProfilingInternal.c",
    "InstrProfilingMerge.c", "InstrProfilingMergeFile.c", "InstrProfilingNameVar.c",
    "InstrProfilingPlatformWindows.c", "InstrProfilingUtil.c", "InstrProfilingValue.c",
    "InstrProfilingVersionVar.c", "InstrProfilingWriter.c", "WindowsMMap.c",
]
PROFILE_RUNTIME_HEADERS = [
    "lib/profile/InstrProfiling.h", "lib/profile/InstrProfilingInternal.h", "lib/profile/InstrProfilingPort.h",
    "lib/profile/InstrProfilingUtil.h", "lib/profile/WindowsMMap.h", "include/profile/InstrProfData.inc",
    "include/profile/instr_prof_interface.h", "include/profile/MIBEntryDef.inc", "include/profile/MemProfData.inc",
]


def windows_resources() -> Path:
    """port/windows/halo.rc with the build's number (HALO_BUILD_NUMBER, as
    updater_defines takes it) in place of @BUILD_NUMBER@, written to
    build/windows (only when it changes, so that nothing is rebuilt for
    nothing): the resource compiler runs with /no-preprocess, which leaves it
    no #define to take the number from"""
    number = os.environ.get("HALO_BUILD_NUMBER", "0")
    if not number.isdigit():
        number = "0"
    text = (PORT_DIR / "halo.rc").read_text(encoding="utf-8").replace("@BUILD_NUMBER@", number)
    path = BUILD / "halo.rc"
    if not path.is_file() or path.read_text(encoding="utf-8") != text:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
    return path


def windows_rc(cc: str) -> str:
    """LLVM's resource compiler of the clang named cc: beside it, where cc is
    a path, else the one on the PATH"""
    path = Path(cc)
    if path.parent == Path("."):
        return "llvm-rc"
    return str(path.with_name("llvm-rc" + path.suffix))


def clang_release(cc: str) -> Optional[str]:
    """the release (22.1.0) of the clang named cc, or None"""
    try:
        output = subprocess.run([cc, "--version"], capture_output=True, text=True, check=False).stdout
    except OSError:
        return None
    match = re.search(r"clang version (\d+\.\d+\.\d+)", output)
    return match.group(1) if match else None


def fetch_profile_runtime(release: str) -> Path:
    """Downloads compiler-rt's profile runtime sources for this release once;
    returns their directory (with lib/profile and include/profile)."""
    root = THIRD_PARTY / f"compiler-rt-profile-{release}"
    files = [f"lib/profile/{name}" for name in PROFILE_RUNTIME_SOURCES] + PROFILE_RUNTIME_HEADERS
    for name in files:
        target = root / name
        if target.is_file():
            continue
        url = f"https://raw.githubusercontent.com/llvm/llvm-project/llvmorg-{release}/compiler-rt/{name}"
        print(f"Downloading {url}")
        target.parent.mkdir(parents=True, exist_ok=True)
        with urllib.request.urlopen(url) as response:
            data = response.read()
        target.write_bytes(data)
    return root


# a C __inline function with external linkage, defined in a .c file
EXPORTED_INLINE = re.compile(
    r"^(?!\s*static\b)[^;{}()\n]*\b(?:__inline|_inline|__forceinline)\b[^;{}()]*?"
    r"\b([A-Za-z_]\w*)\s*\([^;{}]*\)\s*\{",
    re.M,
)
COMMENT = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)


def inline_export_wrapper(source: Path) -> Path:
    """MSVC emits a C __inline function with external linkage wherever a
    call to it is not inlined, and other units may call it through a
    prototype; clang can inline every call in the defining unit, leaving no
    copy. A unit that defines such a function is compiled through a
    generated wrapper that includes it and takes the functions' addresses,
    which makes clang emit them (as COMDATs). Returns the file to compile."""
    if source.suffix != ".c" or not str(source).replace(os.sep, "/").startswith("source/"):
        return source
    names = EXPORTED_INLINE.findall(COMMENT.sub("", source.read_text(encoding="utf-8", errors="replace")))
    if not names:
        return source
    wrapper = BUILD / "inline_exports" / source
    text = (
        "/* generated by tools/windows_build.py: see inline_export_wrapper */\n"
        f'#include "{source.resolve().as_posix()}"\n'
        "static void *const halo_windows_inline_exports[] __attribute__((used)) = {\n"
        + "".join(f"\t(void *){name},\n" for name in names)
        + "};\n"
    )
    wrapper.parent.mkdir(parents=True, exist_ok=True)
    if not wrapper.is_file() or wrapper.read_text(encoding="utf-8") != text:
        wrapper.write_text(text, encoding="utf-8")
    return wrapper


def generate_windows_build(n: Writer, sln: Any) -> None:
    if (sys.platform != "win32" and not getattr(sln, "windows_cross", False)) or not PORT_CONFIG.is_file():
        return
    try:
        fetch_sdl()
    except OSError as error:
        print(f"Windows build disabled: cannot fetch SDL3 ({error})", file=sys.stderr)
        return
    linux_config: Dict[str, Any] = json.loads((LINUX_DIR / "port.json").read_text(encoding="utf-8"))
    config = _load_config()

    tags_header = BUILD / "halo_msvc_tags.h"
    obj_dir = BUILD / "obj"
    output = BUILD / "halo.exe"
    sdl_dll = BUILD / "SDL3.dll"
    cc = getattr(sln, "windows_cc", None) or "clang"
    prefix_header = PORT_DIR / "include" / "halo_windows_prefix.h"
    crt_include = PORT_DIR / "include" / "crt"
    posix_include = PORT_DIR / "include" / "posix"

    # setup cross compilation if we're not on Windows
    cross_prefix = cross_toolchain_prefix(sln) if sys.platform != "win32" else ""

    n.comment("Native Windows build (ninja windows)")
    n.variable("windows_cc", cc)
    # MSVC gives struct tags first named in a prototype file scope; clang
    # does not (the Linux build's generator also writes these declarations)
    n.rule(
        name="windows_msvc_tags",
        command="$python tools/linux_msvc_semantics.py --output $out --tags source",
        description="WINDOWS MSVC TAGS $out",
        restat=True,
    )
    game_headers = sorted(p for p in Path("source").rglob("*") if p.suffix in (".c", ".h"))
    n.build(outputs=tags_header, rule="windows_msvc_tags",
            implicit=[Path("tools/linux_msvc_semantics.py"), *game_headers])
    n.rule(
        name="windows_cc",
        command=f"{cross_prefix}{compile_launcher(sln)}$windows_cc -MMD -MF $out.d $cflags -c $in -o $out",
        description="WINDOWS CC $out",
        depfile="$out.d",
        deps="gcc",
    )
    # ninja quotes the response file's paths for the shell that runs the
    # rule: Windows quoting on Windows, POSIX quoting elsewhere
    rsp_quoting = "windows" if sys.platform == "win32" else "posix"
    n.rule(
        name="windows_link",
        command=f"{cross_prefix}$windows_cc $ldflags -o $out --rsp-quoting={rsp_quoting} @$out.rsp $libs",
        description="WINDOWS LINK $out",
        rspfile="$out.rsp",
        rspfile_content="$in_newline",
    )
    # the executable's resources (port/windows/halo.rc: its icon and version
    # information, with the build's number written in: windows_resources),
    # compiled by LLVM's resource compiler, beside the clang that builds the
    # game; the icon is found through /I, as the copy is in build/windows
    n.variable("windows_rc", windows_rc(cc))
    n.rule(
        name="windows_rc",
        command=f"$windows_rc /no-preprocess /I {_quote(PORT_DIR)} /FO $out $in",
        description="WINDOWS RC $out",
    )
    resources = BUILD / "halo.res"
    n.build(outputs=resources, rule="windows_rc", inputs=windows_resources(),
            implicit=[PORT_DIR / "opence-icon.ico"])
    n.rule(
        name="windows_copy",
        command="$python -c \"import shutil,sys; shutil.copyfile(sys.argv[1], sys.argv[2])\" $in $out",
        description="WINDOWS COPY $out",
    )

    # the high-res HUD's textures (port/assets/hud; port/linux/src/hud_hires.c)
    embedded_assets = hud_assets_build(n, "windows", BUILD / "generated" / "hud_hires_assets.c")
    embedded_assets += server_ui_build(n, "windows", BUILD / "generated" / "server_ui_assets.c")

    # (a debug build checks its stack frames (/GS), and stops at the first
    # one overrun, as at the first failed assertion; a release build does
    # not, so that an overrun nobody has met cannot end a game)
    abi = " ".join(WINDOWS_ABI_FLAGS + [march_flag(sln)] + configuration_defines(sln)
                   + ([] if getattr(sln, "port_release", False) else ["-fstack-protector-strong"]))
    sdl_include = SDL_DIR / "include"
    libs = " ".join(
        [_quote(SDL_DIR / "lib" / "x86" / "SDL3.lib")]
        + [f"-l{lib}" for lib in config.get("libraries", [])]
    )
    base_ldflags = [
        "--target=i686-pc-windows-msvc",
        "-fuse-ld=lld",
        "-g",
        # Reserve the MCC tag window before SDL's ASLR placement. Keep the
        # shipped DLL unchanged; the first SDL call loads it after reservation.
        "-Wl,/DELAYLOAD:SDL3.dll",
        "-ldelayimp",
        # the Xbox memory window is at 0x80000000, in the upper half of the
        # 32-bit address space
        "-Wl,/LARGEADDRESSAWARE",
        "-Wl,/STACK:0x800000",
    ]
    if getattr(sln, "port_release", False):
        # no console window (the port's log goes to halo.log instead,
        # win32_posix.c): under Wine (Proton, gamescope) the console window
        # can hide the game's window
        base_ldflags += ["-Wl,/SUBSYSTEM:WINDOWS", "-Wl,/ENTRY:mainCRTStartup"]
    else:
        base_ldflags += ["-Wl,/SUBSYSTEM:CONSOLE"]

    def emit(obj_dir: Path, output: Path, extra_cflags: List[str], extra_ldflags: List[str],
             extra_objects: List[Path], implicit_inputs: List[Path]) -> None:
        """the objects and the executable, with the given extra flags"""
        extra = " ".join(extra_cflags)
        objects: List[Path] = []

        def add_object(source: Path, cflags: str) -> None:
            obj = obj_dir / source.with_suffix(".o")
            objects.append(obj)
            compiled = inline_export_wrapper(source)
            n.build(
                outputs=obj,
                rule="windows_cc",
                inputs=compiled,
                implicit=[*xdk_headers(), prefix_header, tags_header, source, *implicit_inputs],
                variables={"cflags": f"{cflags} {extra}"},
            )

        game_cflags = " ".join([
            abi,
            " ".join(GAME_FLAGS),
            f"-include {prefix_header}",
            f"-include {tags_header}",
            f"-I{crt_include}",
            f"-I{PORT_DIR / 'include'}",
            # the port's headers the game's units include (halo_keyboard.h,
            # halo_menus.h), but not the Linux build's C runtime wrappers
            # next to them, which no game unit includes in quotes
            f"-iquote {LINUX_DIR / 'include'}",
            # the headers of the port's own game units (port/linux/game), for
            # the game sources that call them
            f"-iquote {Path(linux_config['game_sources'])}",
            game_defines_and_includes(linux_config),
            # the Xbox SDK declarations (port/include/xdk) come before the
            # Windows SDK, which has headers of the same names
            f"-I{XDK_INCLUDE}",
        ])
        for source in game_sources(linux_config):
            # The halt screen and version command identify this native build.
            flags = game_cflags
            if source.as_posix() == "source/main/main.c":
                flags += " " + updater_defines(getattr(sln, "port_release", False))
            add_object(source, flags)
        for source in sorted(Path(linux_config["game_sources"]).glob("*.c")):
            add_object(source, game_cflags)
        add_object(STB_VORBIS_SOURCE, game_cflags)

        linux_platform = Path(linux_config["platform_sources"])
        platform_cflags = " ".join([
            abi,
            " ".join(PLATFORM_FLAGS),
            f"-include {prefix_header}",
            f"-I{posix_include}",
            f"-I{crt_include}",
            f"-I{linux_platform}",
            f"-I{PORT_DIR / 'include'}",
            f"-I{TOML_DIR}",
            f"-I{EXPAT_DIR}",
            f"-I{KCP_DIR}",
            f"-I{MONOCYPHER_DIR}",
            f"-I{ZLIB_DIR}",
            # halo_linux_winsock_names.h, but not the Linux build's C runtime
            # wrappers next to it
            f"-iquote {LINUX_DIR / 'include'}",
            "-Isource -Isource/cseries",
            f"-I{_quote(sdl_include)}",
            f"-I{XDK_INCLUDE}",
        ])
        win32_cflags = " ".join([
            abi,
            " ".join(WIN32_FLAGS),
            f"-I{posix_include}",
            f"-I{linux_platform}",
            f"-I{_quote(sdl_include)}",
        ])
        replaced = set(config.get("replaced_platform_sources", []))
        mbedtls_include = f"-I{MBEDTLS_DIR / 'include'}"
        for source in sorted(linux_platform.glob("*.c")):
            if source.name in replaced:
                continue
            if source.name in ("posix_mcc_preview.c", "posix_mcc_share.c"):
                add_object(source, win32_cflags)
            elif source.name == "posix_dtls.c":
                # (Mbed TLS sees the Windows SDK, as the win32_*.c do)
                add_object(source, f"{win32_cflags} {mbedtls_include}")
            elif source.name in ("updater.c", "server_mode.c"):
                add_object(source, f"{platform_cflags} {updater_defines(getattr(sln, 'port_release', False))}")
            else:
                add_object(source, platform_cflags)
        miniupnpc_include = f"-I{MINIUPNPC_DIR / 'include'} -DMINIUPNP_STATICLIB"
        for source in sorted((PORT_DIR / "src").glob("*.c")):
            if source.name == "win32_upnp.c":
                add_object(source, f"{win32_cflags} {miniupnpc_include}")
            elif source.name == "win32_crash.c":
                # the build's number and configuration name its crash reports
                add_object(source, f"{win32_cflags} {updater_defines(getattr(sln, 'port_release', False))}")
            else:
                add_object(source, win32_cflags if source.name.startswith("win32_") else platform_cflags)
        # internet play's UPnP (port/third_party/miniupnpc), on Winsock, as
        # its own build has it
        for source in miniupnpc_sources():
            add_object(source, " ".join([abi, *WIN32_FLAGS, miniupnpc_include, f"-I{MINIUPNPC_DIR / 'src'}",
                                         "-D_CRT_SECURE_NO_WARNINGS", "-D_WINSOCK_DEPRECATED_NO_WARNINGS", "-w"]))
        for source in embedded_assets:
            add_object(source, platform_cflags)
        # the settings file's parser (port/third_party/tomlc17), with the
        # platform layer's ABI and nothing else
        add_object(TOML_DIR / "tomlc17.c", " ".join([abi, "-std=gnu11", "-w"]))
        # the menus' XML parser (port/third_party/expat; menu_files.c), with
        # its hash salt from rand_s
        for name in EXPAT_SOURCES:
            add_object(EXPAT_DIR / name, " ".join([abi, "-std=gnu11", f"-I{EXPAT_DIR}", "-w"]))
        # internet play's reliable streams (port/third_party/kcp; p2p.c)
        add_object(KCP_DIR / "ikcp.c", " ".join([abi, "-std=gnu11", "-w"]))
        # internet play's DTLS with browsers (port/third_party/mbedtls;
        # posix_dtls.c), on the Windows SDK (its entropy is BCryptGenRandom)
        for source in sorted((MBEDTLS_DIR / "library").glob("*.c")):
            add_object(source, " ".join([abi, *WIN32_FLAGS, mbedtls_include, f"-I{MBEDTLS_DIR / 'library'}",
                                         "-D_CRT_SECURE_NO_WARNINGS", "-w"]))
        # voice chat's codec (port/third_party/opus)
        for source in opus_sources():
            add_object(source, opus_cflags(abi))
        # internet play's signatures, for public games' listings
        # (port/third_party/monocypher; p2p_crypto.c)
        for name in ("monocypher.c", "monocypher-ed25519.c"):
            add_object(MONOCYPHER_DIR / name, " ".join([abi, "-std=gnu11", "-w"]))
        # the port's zlib
        for name in ZLIB_SOURCES:
            add_object(ZLIB_DIR / name, " ".join([abi, "-std=gnu11", *ZLIB_DEFINES, "-w"]))
        # the game's sin, pow and the rest, the same on every port
        # (port/include/halo_math.h)
        for source in musl_math_sources():
            add_object(source, musl_math_cflags(abi))

        n.build(
            outputs=output,
            rule="windows_link",
            inputs=objects + extra_objects + [resources],
            variables={"ldflags": " ".join(base_ldflags + extra_ldflags), "libs": libs},
        )

    # Profile-guided optimisation: with the committed Windows profile (or
    # the Linux one, which matches most of the game), or with --pgo=train
    # one that an instrumented build records while playing
    # (tools/pgo_train.py). A profile is trained once: code changed since
    # simply goes without, and deleting it trains a new one.
    profile = pgo_profile(sln, WINDOWS_PROFILE, [LINUX_PROFILE], cc)
    if pgo_mode(sln) == "train" and profile == WINDOWS_PROFILE:
        release = clang_release(cc)
        if not release:
            sys.exit(f"cannot tell the release of {cc}, whose profile runtime the instrumented build needs")
        runtime = fetch_profile_runtime(release)
        runtime_cflags = " ".join([
            "--target=i686-pc-windows-msvc", OPTIMISATION, "-g", "-gcodeview", "-w",
            "-D_CRT_SECURE_NO_WARNINGS", "-DCOMPILER_RT_HAS_ATOMICS=1",
            f"-I{_quote(runtime / 'include')}", f"-I{_quote(runtime / 'lib' / 'profile')}",
        ])
        generate_dir = BUILD / "pgo-generate"
        runtime_objects: List[Path] = []
        for name in PROFILE_RUNTIME_SOURCES:
            obj = generate_dir / "profile_runtime" / name.replace(".c", ".o")
            n.build(outputs=obj, rule="windows_cc", inputs=runtime / "lib" / "profile" / name,
                    variables={"cflags": runtime_cflags})
            runtime_objects.append(obj)
        # the runtime's start-up, which LLVM writes in C++
        obj = generate_dir / "profile_runtime" / "halo_profile_runtime.o"
        n.build(outputs=obj, rule="windows_cc", inputs=PORT_DIR / "pgo" / "halo_profile_runtime.c",
                variables={"cflags": runtime_cflags})
        runtime_objects.append(obj)
        instrumented = generate_dir / "halo.exe"
        instrumented_dll = generate_dir / "SDL3.dll"
        # instrumented objects ask for LLVM's own runtime library, which
        # these objects replace
        emit(generate_dir / "obj", instrumented, ["-fprofile-generate"],
             ["-Wl,/NODEFAULTLIB:clang_rt.profile.lib"], runtime_objects, [])
        n.build(outputs=instrumented_dll, rule="windows_copy", inputs=SDL_DIR / "lib" / "x86" / "SDL3.dll")
        n.rule(
            name="windows_pgo_train",
            command="$python tools/pgo_train.py --binary $binary --work $work --output $out",
            description="WINDOWS PGO TRAINING: playing levels in the instrumented build",
            pool="console",
        )
        n.build(
            outputs=profile,
            rule="windows_pgo_train",
            order_only=[instrumented, instrumented_dll],
            variables={"binary": str(instrumented), "work": str(sln.build_dir / "pgo" / "windows")},
        )

    # link-time optimisation (configure.py --lto)
    lto = lto_mode(sln)
    lto_cflags = [] if lto == "off" else ["-flto=thin" if lto == "thin" else "-flto=full"]
    emit(obj_dir, output, lto_cflags + profile_use_flags(profile),
         lto_cflags + [OPTIMISATION] if lto_cflags else [], [], [profile] if profile else [])
    n.build(outputs=sdl_dll, rule="windows_copy", inputs=SDL_DIR / "lib" / "x86" / "SDL3.dll")
    # internet play's MQTT brokers, a file beside the game (network.brokers_file)
    brokers = BUILD / "brokers.txt"
    n.build(outputs=brokers, rule="windows_copy", inputs=Path("port/assets/network/brokers.txt"))
    # the master servers of dedicated servers, a file beside the game too
    # (network.masters_file, server.masters_file)
    masters = BUILD / "masters.txt"
    n.build(outputs=masters, rule="windows_copy", inputs=Path("port/assets/network/masters.txt"))
    n.build(outputs="windows", rule="phony", inputs=[output, sdl_dll, brokers, masters])
    n.newline()
