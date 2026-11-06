"""Pack supplied HUD pictures directly: remove black, resize, and place.

Run: python tools/hud_corrections.py
No tracing, redrawing or font substitution. CE's existing 8x is retained.
"""
import json
from PIL import Image
import hud_assets

ASSETS = hud_assets.ASSETS


def cell(rect, name, crop):
    return dict(xbox=rect, source_png='corrections/'+name+'.png', crop=crop, kind='alpha')


def scope_cells(kind):
    # Stock visible coverage is 9x9 texels: Xbox [36,4,45,13],
    # and CE [52,1,61,10].
    # This supplied crop includes black padding, so a 10x10 placement
    # reproduces that footprint within one output pixel. Keep the crop,
    # its antialiasing and the stock centre; do not fill the sprite cell.
    if kind == 'ce':
        return [cell([0, 0, 25, 18], 'scope', [50, 26, 188, 112]),
                cell([51.5, 0.5, 61.5, 10.5], 'scope', [241, 12, 301, 73])]
    # The Xbox zoom labels' stock glyphs (alpha >= 128) do not fill their
    # sprites [0,0,31,24] and [0,24,43,48]: 2x covers [5,5,26,19] (21x14
    # texels) and 10x [6,29,38,43] (32x14), with the rest of each sprite
    # empty. Filling the sprite drew them a third larger than the Xbox's.
    # These placements put the supplied glyphs on the stock bounds: 10x
    # within one output pixel; 2x within three, its supplied glyphs being
    # wider for their height (1.65) than the stock's (1.50), which a
    # proportional resize cannot change.
    rectangles = [[4.125, 4.875, 26.75, 19.0], [5.25, 28.75, 38.5, 43.125],
                  [35.5, 3.5, 45.5, 13.5]]
    return [cell(rect, 'scope', crop) for rect, crop in zip(rectangles,
        [[50, 26, 188, 112], [50, 168, 247, 253], [241, 12, 301, 73]])]


def write_asset(entry):
    Image.fromarray(hud_assets.build_asset(entry, {})).save(ASSETS / (entry['name']+'.png'), optimize=True)


def rebuild():
    layout = json.loads(hud_assets.LAYOUT.read_text())
    for entry in layout['assets']:
        if entry['name'] == 'hud_reticles__7':
            entry['cells'] = [cell([0, 0, 64, 64], 'rocket', [92, 92, 1188, 1188])]
        elif entry['name'] == 'sniper__hud_reticles_scope__0':
            entry['cells'] = scope_cells('xbox')
        else:
            continue
        entry['correction'] = True
        write_asset(entry)
    hud_assets.LAYOUT.write_text(json.dumps(layout, indent=1)+'\n')
    ce = json.loads((ASSETS / 'ce_layout.json').read_text())
    entry = dict(name='ce_sniper_scope', tag=r'ui\hud\bitmaps\sniper\hud_reticles_scope', bitmap=0,
                 width=64, height=64, scale=8, format='a8r8g8b8', ce_only=True,
                 size=16384, crc=3359213591, cells=scope_cells('ce'),
                 sprites=[{'sequence': 0}, {'sequence': 1}])
    ce['assets'] = [e for e in ce['assets'] if e['name'] != entry['name']] + [entry]
    (ASSETS / 'ce_layout.json').write_text(json.dumps(ce, indent=1)+'\n')
    listing = ASSETS / 'custom_edition.json'
    compatible = json.loads(listing.read_text())
    compatible['assets'] = [e for e in compatible['assets'] if e['tag'] != entry['tag']] + [
        {key: entry[key] for key in ('tag', 'bitmap', 'width', 'height', 'size', 'crc')} |
        {'replacement': entry['name']}]
    listing.write_text('{\n' + ',\n'.join(' ' + json.dumps(k) + ': ' +
        ('[\n' + ',\n'.join('  '+json.dumps(e) for e in v) + '\n ]' if isinstance(v, list) else json.dumps(v))
        for k, v in compatible.items()) + '\n}\n')
    write_asset(entry)


if __name__ == '__main__':
    rebuild()
