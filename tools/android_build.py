"""Ninja rules for the Android build (``ninja android``).

The Android port (port/android/README.md) runs the game as ILP32 AArch64
code - 32-bit pointers, as the game's data formats require - inside an
ordinary 64-bit Android app. This graph builds

- the guest image, build/android/halo_guest.elf: the game sources, the
  platform layer shared with the Linux port (port/linux/src) and the guest
  runtime (port/android/guest) with a subset of musl as its C library, all
  compiled by clang for arm64_32-apple-watchos, converted to ELF assembly
  (tools/android_asm_convert.py), assembled for AArch64 and linked at a fixed
  address below 4 GB;
- the host library, build/android/jniLibs/arm64-v8a/libmain.so, with the
  Android NDK: the loader and the services the guest calls, over SDL3;
- SDL3 itself, with the NDK's CMake toolchain file;

and stages both, with the SDL3 Java sources, for the Gradle project in
port/android/app, which ``ninja android_apk`` then assembles.
"""

import hashlib
import os
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any, Dict, List, Optional

from .linux_build import (LINUX_PROFILE, MINIUPNPC_DEFINES, MINIUPNPC_DIR, MUSL_MATH_DIR, STB_VORBIS_SOURCE, XDK_INCLUDE,
                          compile_launcher, configuration_defines, game_defines_and_includes, game_sources,
                          miniupnpc_sources, musl_math_sources, opus_cflags, opus_sources, pgo_mode, pgo_profile,
                          profile_use_flags, updater_defines, xdk_headers)
from .embed_assets import hud_assets_build, hud_configure_inputs
from .ninja_syntax import Writer

PORT_DIR = Path("port/android")
LINUX_DIR = Path("port/linux")
BUILD = Path("build/android")
THIRD_PARTY = BUILD / "third_party"
# the TOML parser config.toml is read with (port/linux/src/port_config.c)
TOML_DIR = Path("port/third_party/tomlc17")
EXPAT_DIR = Path("port/third_party/expat")
EXPAT_SOURCES = ("xmlparse.c", "xmlrole.c", "xmltok.c")
KCP_DIR = Path("port/third_party/kcp")
# internet play's DTLS with browsers (port/linux/src/posix_dtls.c, in the host)
MBEDTLS_DIR = Path("port/third_party/mbedtls")
MONOCYPHER_DIR = Path("port/third_party/monocypher")
# the port's zlib (port/third_party/zlib/zlib_prefixed.h): what inflates the
# maps, the menus' and the HUD's PNGs and the updates, data from anywhere,
# instead of the game's own 1.1.3 (its inflate only, its names prefixed z_)
ZLIB_DIR = Path("port/third_party/zlib")
ZLIB_SOURCES = ("adler32.c", "crc32.c", "inffast.c", "inflate.c", "inftrees.c", "uncompr.c", "zutil.c")
# (its names prefixed, and the one Z_PREFIX leaves, its error messages, which
# the game's zlib names the same)
ZLIB_DEFINES = ("-DZ_PREFIX", "-Dz_errmsg=z_port_errmsg")
MUSL_VERSION = "1.2.5"
MUSL_DIR = THIRD_PARTY / f"musl-{MUSL_VERSION}"
MUSL_URL = f"https://musl.libc.org/releases/musl-{MUSL_VERSION}.tar.gz"
# (as tools/windows_build.py's and tools/linux_sysroot.py's SDL_VERSION)
# (the download's SHA-256, and the tag's commit, when they were pinned)
MUSL_SHA256 = "a9a118bbe84d8764da0ea0d28b3ab3fae8477fc7e4085d90102b8596fc7c75e4"
SDL_TAG = "release-3.4.16"
SDL_COMMIT = "fa2c02bb6e21974a89ea9824bc53c9932abe5f9c"
SDL_DIR = THIRD_PARTY / "SDL3"
SDL_URL = "https://github.com/libsdl-org/SDL.git"
SDL_ANDROID_MOUSE_PATCH = Path("port/android/patches/sdl-relative-mouse.patch")
SDL_ANDROID_MOUSE_LISTENER = "android-project/app/src/main/java/org/libsdl/app/SDLControllerManager.java"
ANDROID_API = 28

# The guest ABI: AArch64 code with 32-bit pointers (clang's only such target
# is Apple's arm64_32, whose Mach-O output is converted afterwards). The
# Darwin environment is hidden from the sources; the C library is musl.
GUEST_ABI_FLAGS = [
    "--target=arm64_32-apple-watchos",
    "-U__APPLE__",
    "-U__MACH__",
    "-fno-define-target-os-macros",
    "-D__linux__=1",
    "-D__unix__=1",
    # the ILP32 guest's code paths, the OpenGL ES renderer's, and the app's
    # (in the port's sources, each its own macro)
    "-DHALO_ARM64_GUEST=1",
    "-DHALO_GLES=1",
    "-DHALO_ANDROID=1",
    # ARMv8.0: nothing the emulator's binary translation or an older
    # device could lack (Darwin targets otherwise assume pointer
    # authentication and FP16)
    "-mcpu=cortex-a53",
    "-nostdinc",
    "-fshort-wchar",
    "-fno-stack-protector",
    "-fno-unwind-tables",
    "-fno-asynchronous-unwind-tables",
    "-femulated-tls",
    "-mllvm",
    "-aarch64-neon-syntax=generic",
    # no fused multiply-add: the game was written for x87/SSE arithmetic,
    # and its debug assertions (colours within 0..1, unit vectors) trip on
    # the different rounding of fused operations
    "-ffp-contract=off",
    "-O2",
]

# as the Linux build (tools/linux_build.py), minus what only x86 needs
GUEST_CODE_FLAGS = [
    "-fms-extensions",
    "-fcommon",
    "-fno-strict-aliasing",
    "-fwrapv",
    "-fno-delete-null-pointer-checks",
    "-fno-omit-frame-pointer",
    *(f"-fno-builtin-{name}" for name in (
        "wcslen", "wcsnlen", "wcschr", "wcsrchr", "wcscmp", "wcsncmp", "wcscpy",
        "wcsncpy", "wcscat", "wcsncat", "wmemchr", "wmemcmp", "wmemcpy",
        "wmemmove", "wmemset",
    )),
]

MUSL_DIRECTORIES = [
    "conf", "ctype", "dirent", "env", "errno", "exit", "fcntl", "internal",
    "locale", "malloc", "malloc/mallocng", "math", "mman", "multibyte",
    "prng", "sched", "select", "signal", "stat", "stdio", "stdlib", "string",
    "time", "unistd",
]
MUSL_FILES = [
    "thread/__lock.c", "thread/__wait.c", "thread/__timedwait.c",
    "thread/__syscall_cp.c", "thread/vmlock.c", "thread/pthread_self.c",
    "thread/pthread_equal.c", "thread/pthread_once.c",
    "thread/pthread_setcancelstate.c", "thread/pthread_testcancel.c",
    "thread/default_attr.c", "thread/lock_ptc.c", "misc/getauxval.c",
    "linux/sysinfo.c", "misc/basename.c", "misc/dirname.c",
    "misc/realpath.c", "misc/uname.c", "misc/ioctl.c", "misc/getrlimit.c",
    "misc/syscall.c", "network/htonl.c", "network/htons.c", "network/ntohl.c",
    "network/ntohs.c", "network/inet_addr.c", "network/inet_aton.c",
    "network/inet_ntoa.c", "network/inet_pton.c", "network/inet_ntop.c",
]
MUSL_THREAD_PREFIXES = (
    "pthread_attr_", "pthread_cond", "pthread_mutex", "pthread_rwlock",
    "pthread_spin", "sem_",
)
# replaced by the guest runtime (port/android/guest/runtime)
MUSL_EXCLUDE = {
    "env/__stack_chk.c", "env/__init_tls.c", "env/__libc_start_main.c",
    "env/__reset_tls.c", "malloc/oldmalloc", "thread/pthread_create.c",
    # unused, and its compiler barrier is an inline assembly statement
    "string/explicit_bzero.c",
    # eight bytes at a time (guest_string.c)
    "string/memcmp.c",
}
# game files that call variadic functions without a prototype in scope, which
# only works under x86's calling convention (tools/android_abi_check.py)
VARIADIC_PROTOTYPE_FILES = {
    "source/ai/action_uncover.c", "source/ai/ai.c", "source/ai/ai_debug.c",
    "source/bungie_net/common/public_key_crypt.c", "source/camera/editor_flying_camera.c",
    "source/game/cheats.c", "source/game/game_engine.c", "source/game/players.c",
    "source/hs/hs.c", "source/interface/hud_nav_points.c",
    "source/networking/telnet_console.c", "source/rasterizer/xbox/rasterizer_xbox_errors.c",
    "source/render/render.c",
}

HOST_LIBRARIES = ["SDL3", "GLESv3", "EGL", "log", "android", "m", "dl"]


def _quote(path: Any) -> str:
    text = str(path).replace(os.sep, "/")
    return f'"{text}"' if " " in text else text


def _find_ndk() -> Optional[Path]:
    for variable in ("ANDROID_NDK_HOME", "ANDROID_NDK_ROOT", "ANDROID_NDK"):
        if os.environ.get(variable) and Path(os.environ[variable]).is_dir():
            return Path(os.environ[variable])
    for variable in ("ANDROID_HOME", "ANDROID_SDK_ROOT"):
        sdk = os.environ.get(variable)
        if sdk and (Path(sdk) / "ndk").is_dir():
            versions = sorted((Path(sdk) / "ndk").iterdir())
            if versions:
                return versions[-1]
    for sdk in (Path.home() / "Android/Sdk", Path("/opt/android-sdk")):
        if (sdk / "ndk").is_dir():
            versions = sorted((sdk / "ndk").iterdir())
            if versions:
                return versions[-1]
    return None


def check_sdl_commit(directory: Path) -> None:
    """SDL3's clone refused unless its tag is still the pinned commit."""
    commit = subprocess.run(["git", "-C", str(directory), "rev-parse", "HEAD"], capture_output=True, text=True,
                            check=True).stdout.strip()
    if commit != SDL_COMMIT:
        shutil.rmtree(directory)
        raise SystemExit(f"SDL3 {SDL_TAG} is {commit}, not the pinned {SDL_COMMIT}")


def fetch_third_party() -> None:
    """Download musl and SDL3 (configure time, once)."""
    THIRD_PARTY.mkdir(parents=True, exist_ok=True)
    if not MUSL_DIR.is_dir():
        print(f"Downloading {MUSL_URL}")
        archive = THIRD_PARTY / f"musl-{MUSL_VERSION}.tar.gz"
        subprocess.run(["curl", "-sSfL", "-o", str(archive), MUSL_URL], check=True)
        digest = hashlib.sha256(archive.read_bytes()).hexdigest()
        if digest != MUSL_SHA256:
            archive.unlink()
            raise SystemExit(f"{MUSL_URL}: SHA-256 {digest}, not the pinned {MUSL_SHA256}")
        subprocess.run(["tar", "xzf", archive.name], cwd=THIRD_PARTY, check=True)
        archive.unlink()
    if not SDL_DIR.is_dir():
        print(f"Cloning SDL3 {SDL_TAG}")
        subprocess.run(["git", "clone", "-q", "--depth", "1", "--branch", SDL_TAG, SDL_URL, str(SDL_DIR)],
                       check=True)
        check_sdl_commit(SDL_DIR)
    # SDL 3.4.16's generic mouse listener drops captured relative motion and
    # button transitions unless they are forwarded from captured pointer events.
    # A tree patched by another version of the patch (an older checkout, or
    # CI's cached one) is put back as SDL has it before this one is applied.
    reverse = subprocess.run(
        ["git", "-C", str(SDL_DIR), "apply", "--reverse", "--check", str(SDL_ANDROID_MOUSE_PATCH.resolve())],
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    if reverse.returncode != 0:
        subprocess.run(["git", "-C", str(SDL_DIR), "checkout", "--", SDL_ANDROID_MOUSE_LISTENER], check=True)
        subprocess.run(["git", "-C", str(SDL_DIR), "apply", str(SDL_ANDROID_MOUSE_PATCH.resolve())], check=True)


def _musl_sources() -> List[Path]:
    src = MUSL_DIR / "src"
    result = set()
    for directory in MUSL_DIRECTORIES:
        for path in (src / directory).glob("*.c"):
            result.add(path)
    for name in MUSL_FILES:
        result.add(src / name)
    for path in (src / "thread").glob("*.c"):
        if path.name.startswith(MUSL_THREAD_PREFIXES):
            result.add(path)
    sources = []
    for path in sorted(result):
        relative = path.relative_to(src).as_posix()
        if relative in MUSL_EXCLUDE or any(relative.startswith(e + "/") for e in MUSL_EXCLUDE):
            continue
        sources.append(path)
    return sources


def android_configure_inputs() -> List[Path]:
    return [Path(__file__), SDL_ANDROID_MOUSE_PATCH, PORT_DIR / "guest" / "runtime", PORT_DIR / "host",
            LINUX_DIR / "src", *hud_configure_inputs()]


def generate_android_build(n: Writer, sln: Any) -> None:
    config_path = LINUX_DIR / "port.json"
    if not config_path.is_file() or not (PORT_DIR / "host").is_dir():
        return
    ndk = Path(sln.android_ndk) if getattr(sln, "android_ndk", None) else _find_ndk()
    if not ndk or not ndk.is_dir():
        n.comment("Android build: no NDK found (set ANDROID_NDK_HOME or pass --android-ndk)")
        return
    try:
        fetch_third_party()
    except (subprocess.CalledProcessError, OSError) as error:
        print(f"Android build disabled: cannot fetch musl/SDL3 ({error})", file=sys.stderr)
        return
    import json
    config: Dict[str, Any] = json.loads(config_path.read_text(encoding="utf-8"))

    prebuilt = ndk / "toolchains" / "llvm" / "prebuilt"
    _host_tag = os.environ.get("ANDROID_NDK_HOST_TAG", "")
    if not _host_tag:
        if sys.platform == "darwin":
            _host_tag = "darwin-x86_64"
        elif os.name == "nt":
            _host_tag = "windows-x86_64"
        else:
            _host_tag = "linux-x86_64"
    if not (prebuilt / _host_tag).is_dir():
        # an NDK that names its host folder differently: take the one there is
        _tags = sorted(entry.name for entry in prebuilt.iterdir() if entry.is_dir()) if prebuilt.is_dir() else []
        if not _tags:
            n.comment("Android build: the NDK has no LLVM toolchain")
            return
        _host_tag = _tags[0]
    toolchain = prebuilt / _host_tag
    sysroot_include = toolchain / "sysroot" / "usr" / "include"
    host_cc = toolchain / "bin" / f"aarch64-linux-android{ANDROID_API}-clang"
    ndk_bin = toolchain / "bin"
    guest_cc = getattr(sln, "android_guest_cc", None) or "clang"

    guest_dir = BUILD / "guest"
    obj_dir = guest_dir / "obj"
    gen_dir = guest_dir / "gen"
    libc_include = guest_dir / "libc_include"
    libc_internal = guest_dir / "libc_internal"
    gl_include = guest_dir / "gl_include"
    arch = PORT_DIR / "guest" / "libc" / "arch" / "arm64_32"
    semantics_header = Path("build/linux/halo_msvc_semantics.h")
    platform_semantics_header = Path("build/linux/platform_msvc_semantics.h")
    prefix_header = LINUX_DIR / "include" / "halo_linux_prefix.h"
    image = BUILD / "halo_guest.elf"
    sdl_build = BUILD / "sdl3-build"
    libsdl = sdl_build / "libSDL3.so"
    jni_dir = BUILD / "jniLibs" / "arm64-v8a"
    libmain = jni_dir / "libmain.so"
    assets_dir = BUILD / "assets"
    python = "$python"

    n.comment("Android build (ninja android); see port/android/README.md")
    n.variable("android_guest_cc", guest_cc)
    n.variable("android_host_cc", str(host_cc))
    n.variable("android_ndk_bin", str(ndk_bin))

    # ---------- generated headers and sources

    alltypes = libc_include / "bits" / "alltypes.h"
    syscall_h = libc_include / "bits" / "syscall.h"
    version_h = libc_internal / "version.h"
    n.rule(
        name="android_alltypes",
        command=f"sed -f {MUSL_DIR}/tools/mkalltypes.sed $in > $out",
        description="ANDROID MUSL $out",
    )
    n.build(outputs=alltypes, rule="android_alltypes",
            inputs=[arch / "bits" / "alltypes.h.in", MUSL_DIR / "include" / "alltypes.h.in"])
    n.rule(
        name="android_syscall_h",
        command="cp $in $out && sed -n -e s/__NR_/SYS_/p < $in >> $out",
        description="ANDROID MUSL $out",
    )
    n.build(outputs=syscall_h, rule="android_syscall_h", inputs=arch / "bits" / "syscall.h.in")
    n.rule(
        name="android_version_h",
        command=f"echo '#define VERSION \"{MUSL_VERSION}\"' > $out",
        description="ANDROID MUSL $out",
    )
    n.build(outputs=version_h, rule="android_version_h")

    # the NDK's OpenGL ES headers (C declarations only) for the guest
    gl_stamp = gl_include / "stamp"
    n.rule(
        name="android_gl_include",
        command=(f"mkdir -p {gl_include} && ln -sfn {sysroot_include}/GLES2 {gl_include}/GLES2 && "
                 f"ln -sfn {sysroot_include}/GLES3 {gl_include}/GLES3 && "
                 f"ln -sfn {sysroot_include}/KHR {gl_include}/KHR && touch $out"),
        description="ANDROID GL HEADERS",
    )
    n.build(outputs=gl_stamp, rule="android_gl_include")

    guest_gl_c = gen_dir / "guest_gl.c"
    gl_imports = gen_dir / "gl_imports.list"
    n.rule(
        name="android_gl_stubs",
        command=(f"{python} tools/android_gl_stubs.py {LINUX_DIR}/src/gl.h {sysroot_include}/GLES3/gl32.h "
                 f"{sysroot_include}/GLES2/gl2ext.h {guest_gl_c} {gl_imports}"),
        description="ANDROID GL STUBS",
    )
    n.build(outputs=[guest_gl_c, gl_imports], rule="android_gl_stubs",
            implicit=[Path("tools/android_gl_stubs.py"), LINUX_DIR / "src" / "gl.h"])

    guest_posix_c = gen_dir / "guest_posix.c"
    posix_imports = gen_dir / "posix_imports.list"
    n.rule(
        name="android_posix_stubs",
        command=f"{python} tools/android_posix_stubs.py {LINUX_DIR}/src/posix.h {guest_posix_c} {posix_imports}",
        description="ANDROID POSIX STUBS",
    )
    n.build(outputs=[guest_posix_c, posix_imports], rule="android_posix_stubs",
            implicit=[Path("tools/android_posix_stubs.py"), LINUX_DIR / "src" / "posix.h"])

    imports_s = gen_dir / "imports.s"
    host_table_c = BUILD / "host" / "host_import_table.c"
    host_imports_list = PORT_DIR / "host_imports.list"
    n.rule(
        name="android_imports",
        command=f"{python} tools/android_imports.py --host-table {host_table_c} {imports_s} $in",
        description="ANDROID IMPORTS",
    )
    n.build(outputs=[imports_s, host_table_c], rule="android_imports",
            inputs=[host_imports_list, posix_imports, gl_imports],
            implicit=[Path("tools/android_imports.py")])

    generated_headers = [*xdk_headers(), alltypes, syscall_h, version_h, gl_stamp,
                         semantics_header, platform_semantics_header]

    # ---------- guest compilation: C -> Darwin assembly -> ELF assembly -> object

    n.rule(
        name="android_guest_cc",
        command=(f"{compile_launcher(sln)}$android_guest_cc -MMD -MF $out.d $cflags -S $in -o $out.darwin.s && "
                 f"{python} tools/android_asm_convert.py $out.darwin.s $out.s && "
                 f"$android_guest_cc --target=aarch64-linux-android -c $out.s -o $out"),
        description="ANDROID CC $out",
        depfile="$out.d",
        deps="gcc",
    )
    n.rule(
        name="android_guest_as",
        command="$android_guest_cc --target=aarch64-linux-android -c $in -o $out",
        description="ANDROID AS $out",
    )

    libc_includes = [
        f"-isystem {libc_include}", f"-isystem {arch}", f"-isystem {MUSL_DIR}/arch/generic",
        f"-isystem {MUSL_DIR}/include",
    ]
    guest_abi = " ".join(GUEST_ABI_FLAGS + configuration_defines(sln))
    guest_code = " ".join(GUEST_CODE_FLAGS)
    tool_implicit = [Path("tools/android_asm_convert.py"), *generated_headers]
    # profile-guided optimisation with the Linux build's profile (committed,
    # or trained by the Linux build with --pgo=train): the game and platform
    # code are the same, and functions that differ simply go without
    profile = pgo_profile(sln, LINUX_PROFILE if pgo_mode(sln) == "train" else None, [LINUX_PROFILE], guest_cc)
    profile_flags = " ".join(profile_use_flags(profile))
    if profile:
        tool_implicit.append(profile)

    def guest_object(source: Path, cflags: str, prefix: str = "") -> Path:
        obj = obj_dir / prefix / Path(str(source).lstrip("/")).with_suffix(".o")
        if str(source).startswith(str(BUILD)):
            obj = obj_dir / prefix / source.relative_to(BUILD).with_suffix(".o")
        n.build(outputs=obj, rule="android_guest_cc", inputs=source, implicit=tool_implicit,
                variables={"cflags": cflags})
        return obj

    # musl
    musl_cflags = " ".join([
        guest_abi, "-std=c99", "-ffreestanding", "-fno-common", "-D_XOPEN_SOURCE=700", "-w",
        f"-I{arch}", f"-I{MUSL_DIR}/arch/generic", f"-I{libc_internal}",
        f"-I{PORT_DIR}/guest/libc/src_include", f"-I{MUSL_DIR}/src/include",
        f"-I{MUSL_DIR}/src/internal", f"-I{libc_include}", f"-I{MUSL_DIR}/include",
    ])
    musl_objects = [guest_object(source, musl_cflags, "musl") for source in _musl_sources()]
    libguestc = guest_dir / "libguestc.a"
    n.rule(
        name="android_ar",
        command="rm -f $out && $android_ndk_bin/llvm-ar rcs $out @$out.rsp",
        description="ANDROID AR $out",
        rspfile="$out.rsp",
        rspfile_content="$in_newline",
    )
    n.build(outputs=libguestc, rule="android_ar", inputs=musl_objects)

    # the game
    objects: List[Path] = []
    game_flags = [
        "-std=gnu89", "-D__STRICT_ANSI__", "-w",
        "-Wno-error=incompatible-pointer-types",
        "-Wno-error=incompatible-function-pointer-types",
        "-Wno-error=int-conversion",
        "-Wno-error=implicit-function-declaration",
        "-Wno-error=implicit-int",
        "-Wno-error=return-type",
    ]
    game_cflags = " ".join([
        guest_abi, guest_code, " ".join(game_flags), profile_flags,
        f"-include {prefix_header}", f"-include {semantics_header}",
        f"-I{LINUX_DIR}/include",
        # the headers of the port's own game units (port/linux/game), for the
        # game sources that call them
        f"-iquote {Path(config['game_sources'])}",
        game_defines_and_includes(config), *libc_includes, f"-idirafter {XDK_INCLUDE}",
    ])
    for source in game_sources(config):
        cflags = game_cflags
        if source.as_posix() == "source/main/main.c":
            cflags += " " + updater_defines(getattr(sln, "port_release", False))
        if source.as_posix() in VARIADIC_PROTOTYPE_FILES:
            cflags += f" -include {PORT_DIR}/include/halo_android_variadic_prototypes.h"
        objects.append(guest_object(source, cflags))
    for source in sorted(Path(config["game_sources"]).glob("*.c")):
        objects.append(guest_object(source, game_cflags))
    objects.append(guest_object(STB_VORBIS_SOURCE, game_cflags))

    # the platform layer shared with Linux, and the guest runtime
    platform_cflags = " ".join([
        guest_abi, guest_code, "-std=gnu11", "-D_GNU_SOURCE", "-DHALO_LINUX_PLATFORM_LAYER", "-w", profile_flags,
        f"-include {prefix_header}", f"-include {platform_semantics_header}",
        f"-I{LINUX_DIR}/src", f"-I{LINUX_DIR}/include", f"-I{PORT_DIR}/guest/runtime",
        f"-I{PORT_DIR}/include", f"-I{TOML_DIR}", f"-I{EXPAT_DIR}", f"-I{KCP_DIR}", f"-I{MONOCYPHER_DIR}",
        f"-I{ZLIB_DIR}", "-Isource -Isource/cseries",
        f"-I{SDL_DIR}/include", f"-I{gl_include}", *libc_includes, f"-idirafter {XDK_INCLUDE}",
    ])
    guest_host_only = {"memory_watch.c"}  # replaced by guest_memory_watch.c
    for source in sorted((LINUX_DIR / "src").glob("*.c")):
        if source.name.startswith("posix_") or source.name in guest_host_only:
            continue
        objects.append(guest_object(source, platform_cflags))
    # the high-res HUD's textures (port/assets/hud; port/linux/src/hud_hires.c)
    for source in hud_assets_build(n, "android", gen_dir / "hud_hires_assets.c"):
        objects.append(guest_object(source, platform_cflags))
    # the settings file's parser (port/third_party/tomlc17)
    objects.append(guest_object(TOML_DIR / "tomlc17.c", platform_cflags))
    # the menus' XML parser (port/third_party/expat; menu_files.c)
    for name in EXPAT_SOURCES:
        objects.append(guest_object(EXPAT_DIR / name, platform_cflags))
    # internet play's reliable streams (port/third_party/kcp; p2p.c)
    objects.append(guest_object(KCP_DIR / "ikcp.c", platform_cflags))
    # voice chat's codec (port/third_party/opus), with the guest's ABI and C
    # library
    for source in opus_sources():
        objects.append(guest_object(source, " ".join([opus_cflags(guest_abi), *libc_includes])))
    # internet play's signatures, for public games' listings
    # (port/third_party/monocypher; p2p_crypto.c)
    for name in ("monocypher.c", "monocypher-ed25519.c"):
        objects.append(guest_object(MONOCYPHER_DIR / name, platform_cflags))
    # the port's zlib
    for name in ZLIB_SOURCES:
        # (not the CPU's CRC32 instructions, which the guest's assembly step
        # is not told it may use)
        objects.append(guest_object(ZLIB_DIR / name, " ".join([platform_cflags, *ZLIB_DEFINES,
                                                               "-U__ARM_FEATURE_CRC32"])))
    # the game's sin, pow and the rest, the same on every port
    # (port/include/halo_math.h)
    musl_math_cflags = " ".join([
        guest_abi, "-std=gnu11", "-w", profile_flags, *libc_includes, f"-I{MUSL_MATH_DIR}/include",
        f"-include {MUSL_MATH_DIR}/include/libm.h",
    ])
    for source in musl_math_sources():
        objects.append(guest_object(source, musl_math_cflags))
    runtime_internal_cflags = " ".join([
        guest_abi, "-std=c99", "-ffreestanding", "-fno-common", "-D_XOPEN_SOURCE=700", "-D_GNU_SOURCE",
        f"-I{PORT_DIR}/guest/runtime", f"-I{PORT_DIR}/include",
        f"-I{arch}", f"-I{MUSL_DIR}/arch/generic", f"-I{libc_internal}",
        f"-I{PORT_DIR}/guest/libc/src_include", f"-I{MUSL_DIR}/src/include",
        f"-I{MUSL_DIR}/src/internal", f"-I{libc_include}", f"-I{MUSL_DIR}/include",
    ])
    runtime_cflags = " ".join([
        guest_abi, guest_code, "-std=gnu11", "-D_GNU_SOURCE",
        f"-I{PORT_DIR}/guest/runtime", f"-I{PORT_DIR}/include", f"-I{LINUX_DIR}/src",
        f"-I{SDL_DIR}/include", f"-I{gl_include}", *libc_includes,
    ])
    runtime_dir = PORT_DIR / "guest" / "runtime"
    for source in sorted(runtime_dir.glob("*.c")):
        if source.name in ("guest_thread.c", "guest_start.c"):
            objects.append(guest_object(source, runtime_internal_cflags))
        elif source.name == "guest_memory_watch.c":
            objects.append(guest_object(source, platform_cflags))
        else:
            objects.append(guest_object(source, runtime_cflags))
    objects.append(guest_object(guest_gl_c, runtime_cflags))
    objects.append(guest_object(guest_posix_c, runtime_cflags))
    imports_o = obj_dir / "gen" / "imports.o"
    n.build(outputs=imports_o, rule="android_guest_as", inputs=imports_s)
    objects.append(imports_o)

    # ---------- the guest image

    linker_script = PORT_DIR / "guest" / "guest.ld"
    n.rule(
        name="android_guest_link",
        command=(f"$android_ndk_bin/ld.lld -m aarch64linux -static -nostdlib -T {linker_script} "
                 f"-Map $out.map -o $out @$out.rsp {libguestc} "
                 "$$($android_host_cc -print-libgcc-file-name)"),
        description="ANDROID LINK $out",
        rspfile="$out.rsp",
        rspfile_content="$in_newline",
    )
    n.build(outputs=image, rule="android_guest_link", inputs=objects, implicit=[libguestc, linker_script])

    # ---------- SDL3

    n.rule(
        name="android_sdl3",
        command=(f"cmake -S {SDL_DIR} -B {sdl_build} -G Ninja "
                 f"-DCMAKE_TOOLCHAIN_FILE={ndk}/build/cmake/android.toolchain.cmake "
                 f"-DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-{ANDROID_API} -DCMAKE_BUILD_TYPE=Release "
                 f"-DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF "
                 # 16 KB pages (Android 15 and later), as the host library
                 f"-DCMAKE_SHARED_LINKER_FLAGS=-Wl,-z,max-page-size=16384 "
                 f"> {BUILD}/sdl3-configure.log && ninja -C {sdl_build} > {BUILD}/sdl3-build.log"),
        description="ANDROID SDL3",
        pool="console",
    )
    n.build(outputs=libsdl, rule="android_sdl3", implicit=[SDL_DIR / "CMakeLists.txt"])

    # ---------- the host library

    host_objects: List[Path] = []
    host_obj_dir = BUILD / "host" / "obj"
    n.rule(
        name="android_host_cc",
        command="$android_host_cc -MMD -MF $out.d $cflags -c $in -o $out",
        description="ANDROID HOST CC $out",
        depfile="$out.d",
        deps="gcc",
    )
    host_cflags = " ".join([
        "-O2", "-g", "-fPIC", "-Wall", "-Wno-unused-function", "-D_GNU_SOURCE",
        f"-I{PORT_DIR}/include", f"-I{PORT_DIR}/host", f"-I{SDL_DIR}/include", f"-I{LINUX_DIR}/src",
        f"-I{TOML_DIR}", f"-I{ZLIB_DIR}",
    ])
    host_sources = sorted((PORT_DIR / "host").glob("*.c")) + [
        LINUX_DIR / "src" / "posix_files.c", LINUX_DIR / "src" / "posix_net.c",
        LINUX_DIR / "src" / "posix_mcc_preview.c",
        LINUX_DIR / "src" / "posix_mcc_share.c",
        # the app reads debug.sample_seconds from config.toml (host_main.c)
        TOML_DIR / "tomlc17.c",
    ]
    for source in host_sources:
        obj = host_obj_dir / (source.name + ".o")
        # (the Steam client's chunk unpacker: the LZMA SDK's and Zstandard's
        # decoders, whose warnings are not ours)
        n.build(outputs=obj, rule="android_host_cc", inputs=source,
                variables={"cflags": host_cflags + (" -w" if source.name == "steam_unpack.c" else "")})
        host_objects.append(obj)
    # internet play's UPnP (posix_upnp.c, with port/third_party/miniupnpc),
    # as the other posix_*.c in the host
    miniupnpc_cflags = " ".join([host_cflags, f"-I{MINIUPNPC_DIR / 'include'}", f"-I{MINIUPNPC_DIR / 'src'}",
                                 *MINIUPNPC_DEFINES])
    for source in [LINUX_DIR / "src" / "posix_upnp.c", *miniupnpc_sources()]:
        obj = host_obj_dir / ("miniupnpc_" + source.name + ".o" if source.parent.parent == MINIUPNPC_DIR
                              else source.name + ".o")
        n.build(outputs=obj, rule="android_host_cc", inputs=source,
                variables={"cflags": miniupnpc_cflags + (" -w" if source.name != "posix_upnp.c" else "")})
        host_objects.append(obj)
    # internet play's DTLS with browsers (posix_dtls.c, with
    # port/third_party/mbedtls), as the other posix_*.c in the host
    # (with its locks: internet play's thread and the server browser's master
    # servers' both use it, as on Linux)
    mbedtls_cflags = " ".join([host_cflags, f"-I{MBEDTLS_DIR / 'include'}", f"-I{MBEDTLS_DIR / 'library'}",
                               "-DMBEDTLS_THREADING_C", "-DMBEDTLS_THREADING_PTHREAD"])
    # (and the server browser's requests to the master servers: posix_update.c's,
    # host_https.c)
    for source in [LINUX_DIR / "src" / "posix_dtls.c", LINUX_DIR / "src" / "posix_update.c",
                   *sorted((MBEDTLS_DIR / "library").glob("*.c"))]:
        obj = host_obj_dir / ("mbedtls_" + source.name + ".o" if source.parent.parent == MBEDTLS_DIR
                              else source.name + ".o")
        n.build(outputs=obj, rule="android_host_cc", inputs=source,
                variables={"cflags": mbedtls_cflags + (" -w" if source.parent.parent == MBEDTLS_DIR else "")})
        host_objects.append(obj)
    # the port's zlib, its names z_ (zlib_prefixed.h), for the Steam client's zip chunks and checksums
    # (steam_unpack.c, steam_cm.c), as the guest has its own
    for name in ZLIB_SOURCES:
        obj = host_obj_dir / ("zlib_" + name + ".o")
        n.build(outputs=obj, rule="android_host_cc", inputs=ZLIB_DIR / name,
                variables={"cflags": " ".join([host_cflags, *ZLIB_DEFINES, "-w"])})
        host_objects.append(obj)
    table_obj = host_obj_dir / "host_import_table.c.o"
    n.build(outputs=table_obj, rule="android_host_cc", inputs=host_table_c, variables={"cflags": host_cflags})
    host_objects.append(table_obj)
    n.rule(
        name="android_host_link",
        command=(f"$android_host_cc -shared -o $out $in -L{sdl_build} "
                 + " ".join(f"-l{lib}" for lib in HOST_LIBRARIES)
                 + " -Wl,-z,max-page-size=16384 -Wl,--no-undefined"),
        description="ANDROID HOST LINK $out",
    )
    n.build(outputs=libmain, rule="android_host_link", inputs=host_objects, implicit=[libsdl])

    # ---------- staging for Gradle

    staged_sdl = jni_dir / "libSDL3.so"
    staged_image = assets_dir / "halo_guest.elf"
    n.rule(name="android_copy", command="cp $in $out", description="ANDROID STAGE $out")
    n.build(outputs=staged_sdl, rule="android_copy", inputs=libsdl)
    n.build(outputs=staged_image, rule="android_copy", inputs=image)
    # internet play's MQTT brokers, in the APK: the app writes them beside
    # config.toml (port/android/host/host_main.c)
    staged_brokers = assets_dir / "brokers.txt"
    n.build(outputs=staged_brokers, rule="android_copy", inputs=Path("port/assets/network/brokers.txt"))
    # Include MCC's decoder notice in the APK as well as the release archive.
    staged_bcdec_license = assets_dir / "bcdec-LICENSE.txt"
    n.build(outputs=staged_bcdec_license, rule="android_copy", inputs=Path("port/third_party/bcdec/LICENSE"))
    n.build(outputs="android", rule="phony", inputs=[libmain, staged_sdl, staged_image, staged_brokers, staged_bcdec_license])

    apk = PORT_DIR / "app" / "build" / "outputs" / "apk" / "debug" / "app-debug.apk"
    sdl_android_mouse_listener = SDL_DIR / SDL_ANDROID_MOUSE_LISTENER
    n.rule(
        name="android_gradle",
        # Gradle leaves the APK alone when its contents would not change
        command=(f"cd {PORT_DIR} && ./gradlew --console=plain -q assembleDebug && "
                 "touch app/build/outputs/apk/debug/app-debug.apk"),
        description="ANDROID GRADLE $out",
        pool="console",
    )
    n.build(outputs=apk, rule="android_gradle", inputs=[libmain, staged_sdl, staged_image, staged_brokers],
            implicit=[sdl_android_mouse_listener])
    n.build(outputs="android_apk", rule="phony", inputs=apk)
    n.newline()
