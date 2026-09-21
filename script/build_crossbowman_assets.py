"""Compile the ImageGen atlas into Heroes II ICN/BIN assets (requires Pillow).

Run: python script/build_crossbowman_assets.py --resources build/crossbow-assets
The resource directory contains KB.PAL and ARCHRFRM.BIN from the user's game.
This is format conversion and animation sequencing; the artwork is the source PNG.
"""
import argparse
import json
import struct
from pathlib import Path

from PIL import Image


def encode_icn(frames):
    headers, streams = [], []
    offset = 13 * len(frames)
    for image, x, y in frames:
        colors, alpha = image.convert("RGB"), image.getchannel("A")
        indexed = colors.quantize(palette=PALETTE, dither=Image.Dither.NONE)
        data = bytearray()
        for row in range(image.height):
            col = 0
            while col < image.width:
                opaque = alpha.getpixel((col, row)) >= 128
                end = col + 1
                limit = 127 if opaque else 63
                while (end < image.width and end - col < limit
                       and (alpha.getpixel((end, row)) >= 128) == opaque):
                    end += 1
                if opaque:
                    data.append(end - col)
                    data.extend(indexed.getpixel((i, row)) for i in range(col, end))
                else:
                    data.append(128 + end - col)
                col = end
            data.append(0)
        data.append(128)
        headers.append(struct.pack("<hhHHBI", x, y, image.width, image.height, 0, offset))
        streams.append(data)
        offset += len(data)
    return struct.pack("<HI", len(frames), offset) + b"".join(headers) + b"".join(streams)


def head_x(frame):
    image, x, _ = frame
    alpha = image.getchannel("A")
    rows = [r for r in range(image.height) if alpha.crop((0, r, image.width, r + 1)).getbbox()][:8]
    xs = [c for r in rows for c in range(image.width) if alpha.getpixel((c, r))]
    return x + sum(xs) / len(xs)


def sequence(binary, index, frames):
    assert len(frames) <= 16 and all(0 <= frame < 49 for frame in frames)
    binary[243 + index] = len(frames)
    binary[277 + index * 16:277 + (index + 1) * 16] = bytes(frames).ljust(16, b"\0")


def compile_assets(resource_dir, output):
    global PALETTE
    raw_palette = (resource_dir / "KB.PAL").read_bytes()
    assert len(raw_palette) == 768
    PALETTE = Image.new("P", (1, 1))
    PALETTE.putpalette([v * 4 for v in raw_palette])
    atlas = Image.open(output / "source/blue-crossbowman-atlas.png").convert("RGBA")
    assert atlas.width % 8 == 0 and atlas.height % 6 == 0
    cell_w, cell_h = atlas.width // 8, atlas.height // 6
    frames = [(Image.new("RGBA", (1, 1)), 0, 0)]  # Original engines reserve frame zero.
    scale = 66 / 152
    row_edges = []
    for row in range(6):
        strip = atlas.getchannel("A").crop((0, row * cell_h, atlas.width, (row + 1) * cell_h))
        blank = [strip.crop((x, 0, x + 1, cell_h)).getextrema()[1] < 128 for x in range(atlas.width)]
        edges = [0]
        for col in range(1, 8):
            nominal = col * cell_w
            # Allow a lunging crossbow to cross its nominal cell boundary, stopping in the actual gutter.
            candidates = [x for x in range(nominal - 60, nominal + 60)
                          if all(blank[x - 2:x + 3])]
            assert candidates, f"No transparent gutter in row {row}, column {col}"
            edges.append(min(candidates, key=lambda x: abs(x - nominal)))
        row_edges.append(edges + [atlas.width])
    for index in range(48):
        col, row = index % 8, index // 8
        left, right = row_edges[row][col:col + 2]
        cell = atlas.crop((left, row * cell_h, right, (row + 1) * cell_h))
        cell = cell.resize((round(cell.width * scale), round(cell_h * scale)), Image.Resampling.NEAREST)
        # ICN supports binary transparency; retain the generated silhouette, including black outlines.
        cell.putalpha(cell.getchannel("A").point(lambda a: 255 if a >= 128 else 0))
        bounds = cell.getbbox()
        assert bounds is not None, f"Empty atlas cell {index}"
        cropped = cell.crop(bounds)
        # Quantize now as well, so PNG/GIF previews show the actual in-game palette.
        preview = cropped.convert("RGB").quantize(palette=PALETTE, dither=Image.Dither.NONE).convert("RGBA")
        preview.putalpha(cropped.getchannel("A"))
        anchor = round(((col + 0.5) * cell_w - left) * scale)
        frames.append((preview, bounds[0] - anchor, -preview.height + 1))

    animation = bytearray((resource_dir / "ARCHRFRM.BIN").read_bytes())
    assert len(animation) == 821
    # Battle walking moves the sprite by its own x offset; the BIN offsets only cancel it in dialogs
    # and halve it on diagonal steps. Bake the Archer's per-frame advance into our in-place walk.
    walk = list(range(9, 17))
    in_place = list(frames)  # The contact sheet shows every pose inside its own cell.
    lean = round(head_x(frames[1]) - sum(head_x(frames[f]) for f in walk) / len(walk))
    for f, advance in zip(walk, struct.unpack_from("<8b", animation, 5 + 2 * 16)):
        image, x, y = frames[f]
        frames[f] = (image, x + lean + advance, y)

    (output / "CROSSBOW.ICN").write_bytes(encode_icn(frames))
    portrait = frames[1][0]
    (output / "CROSSBOWH.ICN").write_bytes(encode_icn([(portrait, (80 - portrait.width) // 2, 89 - portrait.height)]))
    # Keep the original eight-step movement interpolation, but use our own frames everywhere.
    sequences = {
        0: [], 1: [], 2: list(range(9, 17)), 3: [], 4: [], 5: list(range(9, 17)), 6: [],
        7: [1], 8: [1, 2, 3, 4, 3, 2], 9: [], 10: [], 11: [], 12: [],
        13: list(range(41, 49)), 14: [5, 6, 7], 15: [8, 1],
        16: [33, 34, 35, 36, 37], 17: [38, 39, 40, 1], 18: [], 19: [],
        20: [33, 34, 35, 36, 37], 21: [38, 39, 40, 1], 22: [], 23: [],
        24: [33, 34, 35, 36, 37], 25: [38, 39, 40, 1], 26: [], 27: [],
        28: [17, 25, 26, 27], 29: [28, 21, 22, 23, 24, 1],
        30: [17, 18, 19], 31: [20, 21, 22, 23, 24, 1],
        32: [29, 30, 31], 33: [32, 21, 22, 23, 24, 1],
    }
    for index, values in sequences.items():
        sequence(animation, index, values)
    animation[117] = 1
    struct.pack_into("<5f", animation, 118, 1, 0, 0, 0, 0)
    struct.pack_into("<hh", animation, 1, 0, -54)
    # BIN offsets include the Archer corrections applied to this replacement in bin_info.cpp.
    struct.pack_into("<6h", animation, 174, 48, -60, 70, -40, 52, 15)
    (output / "XBOWFRM.BIN").write_bytes(animation)
    (output / "source/sequences.json").write_text(json.dumps(sequences, indent=2) + "\n")

    contact = Image.new("RGBA", (8 * 96, 6 * 88), (42, 48, 40, 255))
    for index, (frame, x, y) in enumerate(in_place[1:]):
        contact.alpha_composite(frame, ((index % 8) * 96 + 40 + x, (index // 8) * 88 + 80 + y))
    contact.save(output / "preview.png")
    for name, ids in {"walk": sequences[2], "shoot": sequences[30] + sequences[31], "death": sequences[13]}.items():
        rendered = []
        for frame_id in ids:
            frame, x, y = frames[frame_id]
            canvas = Image.new("RGBA", (140, 88), (42, 48, 40, 255))
            canvas.alpha_composite(frame, (40 + x, 80 + y))
            rendered.append(canvas.resize((420, 264), Image.Resampling.NEAREST).convert("RGB"))
        rendered[0].save(output / f"{name}.gif", save_all=True, append_images=rendered[1:], duration=120, loop=0)
    print(f"Compiled {len(frames)} ICN frames and {len(animation)} BIN bytes; previews saved in {output}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--resources", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=Path("files/crossbowman"))
    args = parser.parse_args()
    compile_assets(args.resources, args.output)
