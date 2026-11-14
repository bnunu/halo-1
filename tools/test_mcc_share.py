"""MCC package sharing: staging, the host's list, admission, and a host and a joiner exchanging
the real messages over real files (no game, no sockets, no game data)."""
from pathlib import Path
import shutil
import subprocess
import sys

import pytest
from mcc_test_support import compile_tool

ROOT = Path(__file__).resolve().parents[1]
GAME = ROOT / "port/linux/game"
SOURCES = [ROOT / "tools/tests/mcc_share_session.c", GAME / "mcc_share_stage.c", GAME / "mcc_share_session.c",
           GAME / "mcc_share_list.c", GAME / "mcc_share_rules.c", GAME / "mcc_package.c", GAME / "mcc_sha256.c",
           ROOT / "port/linux/src/posix_mcc_share.c"]
CASES = ["list", "admit", "manifest", "redirect", "stage_cancel", "stage_hash_and_size", "stage_exists",
         "stage_install", "stage_race", "stage_move_failure", "stage_names", "transfer", "rate", "cancel",
         "bad_hash", "changed_size", "identity", "disk_space", "taken", "exists", "refusals", "busy", "host_requests",
         "hostile_manifest", "hostile_chunk", "hashed_once", "names", "timeouts", "aggregate_rate", "offer_accept",
         "offer_expiry", "offer_cancel", "backpressure", "request_fields", "response_fields", "manifest_pages", "clock_zero", "clock_wrap",
         "manifest_rate", "offer_delivery_deadline", "progress_expiry", "terminal_cancel", "terminal_success",
         "terminal_failure", "terminal_disconnect", "terminal_expiry", "terminal_new_request",
         "prepare_success", "prepare_failure", "prepare_unverified", "reuse_files", "reuse_absent",
         "reuse_bounded", "reuse_cancel", "reuse_disconnect", "reuse_short_read", "reuse_bad_hash",
         "reuse_missing", "reuse_backpressure", "have_fields", "done_expired", "done_cancelled",
         "verify_success", "verify_recheck", "verify_heartbeat", "verify_cancel", "verify_disconnect", "verify_timeout",
         "verify_refused", "verify_invalid", "verify_metadata_failure", "verify_receipt_failure",
         "verify_install_failure", "verify_bind_failure", "hold_reuse", "hold_limit", "hold_declined", "window", "hashed_elsewhere",
         "hashed_elsewhere_fails"]


@pytest.fixture(scope="module")
def share_tool(tmp_path_factory):
    return compile_tool(tmp_path_factory.mktemp("mcc-share-session"), SOURCES)


def run_case(exe, case, work):
    work.mkdir()
    return subprocess.run([str(exe), case, str(work)], capture_output=True, text=True, timeout=120)


@pytest.mark.parametrize("case", CASES)
def test_share(share_tool, tmp_path, case):
    result = run_case(share_tool, case, tmp_path / "work")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after,case", [
    # the downloaded files must match their SHA-256 and size before they are kept
    ("if (memcmp(digest,file->sha256,32)) return", "if (0 && memcmp(digest,file->sha256,32)) return", "bad_hash"),
    ("if (bytes>file->size-stage->written || (bytes && !data)) return", "if (bytes && !data) return", "stage_hash_and_size"),
    # the content identity is checked before anything moves into mcc_maps
    ("if (result!=MCC_SHARE_CHECK_READY) return", "if (0 && result!=MCC_SHARE_CHECK_READY) return", "identity"),
    ("if (free_bytes && share->offer_total+(64u<<20)>free_bytes) {", "if (0 && free_bytes && share->offer_total+(64u<<20)>free_bytes) {", "disk_space"),
    # an existing package folder is never written to
    ("if (!io->path_safe(stage->package_dir) || io->directory_exists(stage->package_dir)) return", "if (!io->path_safe(stage->package_dir)) return", "stage_exists"),
    ("!stage->io.publish_directory(stage->stage_dir,stage->package_dir)", "0", "stage_race"),
    ("if (stage_same(out[j].leaf,name)) return 0;", "if (0 && stage_same(out[j].leaf,name)) return 0;", "manifest"),
    # leaf names the sender picks are plain file names from the allowlists
    ("if (file.size>MCC_SHARE_MAXIMUM_CREDIT_BYTES || !mcc_share_credit_allowed(name,(unsigned)strlen(name))) return 0;", "", "manifest"),
    # the host's opt-in and the rules decide before anything is offered
    ("if (!opt_in) return MCC_SHARE_RESULT_OFF;", "if (0 && !opt_in) return MCC_SHARE_RESULT_OFF;", "admit"),
    ("package.content_known=digest_known;", "package.content_known=1;", "admit"),
    # the joiner takes only what it asked for
    ("head->offset!=share->stage.written ||", "", "hostile_chunk"),
    ("head->session!=share->session ||", "", "hostile_chunk"),
    # a package is hashed once, and again when a file's size changes
    ("if (host_hashed(share,transfer)) host_offer(share,transfer,0);", "if (0 && host_hashed(share,transfer)) host_offer(share,transfer,0);", "hashed_once"),
    ("if (size<=0 || (uint32_t)size!=share->hashed.files[i].size) return 0;", "if (size<=0) return 0;", "hashed_once"),
    # Offer traffic shares the cap, heartbeats cannot reserve slots or extend consent,
    # and reliable-stream backpressure must not lose the terminal notification.
    ("share->budget-=(int32_t)(sizeof(struct mcc_share_response)+count*sizeof(entries[0]));", "", "manifest_rate"),
    ("transfer->sent_bytes || transfer->reused_files ? share_elapsed(now,transfer->advanced_at,MCC_SHARE_ACTIVE_ADVANCED) :",
     "transfer->sent_bytes ? 0 :", "progress_expiry"),
    ("if (!transfer->manifest_pending && !transfer->offer_complete) {",
     "if (!transfer->manifest_pending) {", "offer_delivery_deadline"),
    ("client_retry_terminal(share);\n    client_update(share);", "client_update(share);", "terminal_cancel"),
    ("if (kind!=MCC_SHARE_FILE_BITMAPS && kind!=MCC_SHARE_FILE_SOUNDS) return;", "(void)kind;", "have_fields"),
    ("#define MCC_SHARE_REUSE_SLICE 0x400000u", "#define MCC_SHARE_REUSE_SLICE 0x1000000u", "reuse_bounded"),
    ("if (result==MCC_SHARE_CHECK_PENDING) return", "if (0 && result==MCC_SHARE_CHECK_PENDING) return", "verify_success"),
    ("cancel(stage->check_user);", "(void)cancel;", "verify_refused"),
    ("if (share->env.installed_ready && !share->env.installed_ready(share->env.user)) {",
     "if (0) {", "verify_success"),
    # a resource on its way from elsewhere holds the host's request for it, the host
    # hearing from us, for at most the hold's limit; never more requests in flight
    # than every host queues
    ("share->env.reuse_wait(share->env.user,file,share->host_rate)) {", "0) {", "hold_reuse"),
    ("client_send(share,MCC_SHARE_OP_MANIFEST,0,0,0,0)) share->reported_at=now;\n                return;",
     "0) share->reported_at=now;\n                return;", "hold_reuse"),
    ("#define MCC_SHARE_HOLD_LIMIT 60000u", "#define MCC_SHARE_HOLD_LIMIT 200000u", "hold_limit"),
    ("while (share->outstanding<MCC_SHARE_CLIENT_WINDOW &&", "while (share->outstanding<MCC_SHARE_CLIENT_WINDOW+1 &&",
     "window"),
    # a file hashed elsewhere is waited for, and its digest taken
    ("                if (!status) return;\n                transfer->hash_job_active=0;",
     "                transfer->hash_job_active=0;", "hashed_elsewhere"),
])
def test_negative_controls(tmp_path, before, after, case):
    """Each selected check must detect its intended fault in the real code."""
    changed = {}
    for source in (GAME / "mcc_share_stage.c", GAME / "mcc_share_session.c"):
        text = source.read_text()
        if before in text:
            assert text.count(before) == 1
            changed[source] = text.replace(before, after)
    assert len(changed) == 1, before
    sources = []
    for source in SOURCES:
        if source in changed:
            replaced = tmp_path / ("changed_" + source.name)
            replaced.write_text(changed[source])
            source = replaced
        sources.append(source)
    exe = compile_tool(tmp_path, sources)
    result = run_case(exe, case, tmp_path / "work")
    assert result.returncode == 1, result.stderr


def test_sanitized(tmp_path):
    """Linux CI runs every case with address and undefined-behaviour checks."""
    if not sys.platform.startswith("linux"):
        pytest.skip("sanitizer runtime exercised by Linux CI")
    clang = shutil.which("clang")
    assert clang, "clang is required for the Linux security checks"
    exe = tmp_path / "sanitized"
    result = subprocess.run([clang, "-D_GNU_SOURCE", "-std=gnu89", "-g", "-O1", "-Wall", "-Wextra", "-Werror",
                             "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                             "-I", str(GAME), *map(str, SOURCES), "-o", str(exe)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    for case in CASES:
        run = run_case(exe, case, tmp_path / ("work-" + case))
        assert run.returncode == 0, case + "\n" + run.stderr
