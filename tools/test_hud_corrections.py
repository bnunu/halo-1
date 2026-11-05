"""Supplied reticle/zoom artwork fidelity and CE sprite selection."""
import hashlib
import json
from pathlib import Path

import numpy as np
from PIL import Image

from harness import function
from hud_assets import build_asset
import shutil
import subprocess
import sys
import pytest

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / 'port/assets/hud'


def run_c(tmp_path, code):
    compiler = shutil.which('clang')
    if not compiler:
        pytest.skip('clang required')
    source = tmp_path / 'style.c'
    binary = tmp_path / 'style.exe'
    source.write_text(code)
    target = ['--target=i686-pc-windows-msvc'] if sys.platform == 'win32' else ['-m32']
    result = subprocess.run([compiler, *target, '-std=gnu89', '-O1', str(source), '-o', str(binary)],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    subprocess.run([str(binary)], check=True, timeout=10)


def test_supplied_sources_are_unchanged():
    manifest = json.loads((ASSETS / 'corrections/source-sha256.json').read_text())
    for name, record in manifest.items():
        assert hashlib.sha256((ASSETS / 'corrections' / name).read_bytes()).hexdigest() == record['sha256']


def test_packed_corrections_use_supplied_pixels():
    for filename in ['layout.json', 'ce_layout.json']:
        for entry in json.loads((ASSETS / filename).read_text())['assets']:
            if not any('source_png' in cell for cell in entry['cells']):
                continue
            actual = np.asarray(Image.open(ASSETS / (entry['name']+'.png')))
            assert np.array_equal(actual, build_asset(entry, {}))
            # These assets are monochrome premultiplied coverage, not opaque
            # white pictures with black backgrounds or black letter holes.
            assert np.array_equal(actual[..., 0], actual[..., 3])
            assert np.array_equal(actual[..., 1], actual[..., 3])
            assert np.array_equal(actual[..., 2], actual[..., 3])
            for cell in entry['cells']:
                with Image.open(ASSETS / cell['source_png']) as supplied:
                    source = supplied.convert('L').crop(cell['crop'])
                x0,y0,x1,y1 = [round(v*entry['scale']) for v in cell['xbox']]
                ratio = min((x1-x0)/source.width, (y1-y0)/source.height)
                size = tuple(round(n*ratio) for n in source.size)
                x0 += (x1-x0-size[0])//2
                y0 += (y1-y0-size[1])//2
                expected = np.asarray(source.resize(size, Image.Resampling.LANCZOS))
                assert np.array_equal(actual[y0:y0+size[1], x0:x0+size[0], 3], expected)


def test_sniper_reticle_keeps_stock_visible_size_and_position():
    # Measured stock alpha bounds (>=128), not the padded sprite rectangles.
    for name, scale, sprite, bounds in [
        ('sniper__hud_reticles_scope__0', 8, [32, 0, 50, 18], [36, 4, 45, 13]),
        ('ce_sniper_scope', 8, [51, 0, 63, 12], [52, 1, 61, 10]),
    ]:
        alpha = Image.open(ASSETS / (name+'.png')).getchannel('A')
        x0,y0,x1,y1 = [n*scale for n in sprite]
        visible = np.asarray(alpha.crop((x0,y0,x1,y1))) >= 128
        ys,xs = np.where(visible)
        actual = np.array([x0+xs.min(), y0+ys.min(), x0+xs.max()+1, y0+ys.max()+1])
        # Proportional resampling of the supplied image can round one edge
        # by one high-res pixel (1/8 Xbox/CE texel), not one stock texel.
        assert np.all(np.abs(actual - np.array(bounds)*scale) <= 1), name


def test_xbox_zoom_labels_keep_stock_visible_size_and_position():
    # Measured stock alpha bounds (>=128) of the Xbox 2x and 10x, which sit
    # inside their sprites with empty texels around them: filling the
    # sprites drew them a third larger. The supplied 2x is wider for its
    # height than the stock's, so a proportional resize leaves its edges up
    # to three high-res pixels (3/8 texel) from the stock's; 10x, one.
    alpha = Image.open(ASSETS / 'sniper__hud_reticles_scope__0.png').getchannel('A')
    for label, sprite, bounds, tolerance in [
        ('2x', [0, 0, 31, 24], [5, 5, 26, 19], 3),
        ('10x', [0, 24, 43, 48], [6, 29, 38, 43], 1),
    ]:
        x0,y0,x1,y1 = [n*8 for n in sprite]
        visible = np.asarray(alpha.crop((x0,y0,x1,y1))) >= 128
        ys,xs = np.where(visible)
        actual = np.array([x0+xs.min(), y0+ys.min(), x0+xs.max()+1, y0+ys.max()+1])
        assert np.all(np.abs(actual - np.array(bounds)*8) <= tolerance), (label, actual.tolist())


def test_ce_keeps_8x_and_only_selects_verified_scope(tmp_path):
    body = function((ROOT / 'port/linux/game/hud_hires_tags.c').read_text(), 'hud_hires_crosshair_bitmap')
    run_c(tmp_path, r'''
#include <assert.h>
#include <string.h>
struct bitmap_data {int id;};
static struct bitmap_data stock={0},custom={1},replacement={2};
static struct {struct bitmap_data *bitmap;long asset;} hires_sprite_bitmaps[1]={{&stock,0}};
static long hires_sprite_bitmap_count=1;
static int ce=1,enabled=1,verified=1,calls;
static char const *name="ui\\hud\\bitmaps\\sniper\\hud_reticles_scope";
static int custom_edition_cache_tags_loaded(void) {return ce;}
static int config_boolean(char const *s) {(void)s;return enabled;}
static char const *hud_hires_asset_tag(long i) {(void)i;return name;}
static struct bitmap_data const *hud_hires_sprite_bitmap(struct bitmap_data const *b,short s) {
    (void)s;calls++;return verified?&replacement:b;
}
''' + body + r'''
int main(void) {
    assert(hud_hires_crosshair_bitmap(&stock,0,0)==&replacement);
    assert(hud_hires_crosshair_bitmap(&stock,1,0)==&replacement);
    assert(hud_hires_crosshair_bitmap(&stock,1,1)==&stock && calls==2);
    assert(hud_hires_crosshair_bitmap(&custom,0,0)==&custom);
    enabled=0;assert(hud_hires_crosshair_bitmap(&stock,0,0)==&stock);
    enabled=1;ce=0;assert(hud_hires_crosshair_bitmap(&stock,0,0)==&stock);
    ce=1;verified=0;assert(hud_hires_crosshair_bitmap(&stock,0,0)==&stock);
    verified=1;name="custom";assert(hud_hires_crosshair_bitmap(&stock,0,0)==&stock);
    return 0;
}
''')
