#!/usr/bin/env python3
"""Export the checked native-backend grids as an MDN GUI project.

The grids have already undergone carry normalization in the backend. This
script preserves every coordinate and digit; it neither performs arithmetic
nor replaces a grid by its central window. It reads the two depth-20/20 and
depth-20/21 cases emitted by backend_check.py.

Example:
    python3 research/fuzzy/export_project.py --work /tmp/mdn-fuzzy-01a12881 \
        --output /home/d.gaden/GADEN/MDN/fuzzy-family-examples.mdnproj

The output is created exclusively: an existing project is never overwritten.
The binary format saves one global cascade depth, so the original cutoffs are
also retained in tab names. Loading preserves the already-generated grids.
"""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess


CONFIG = {"base": 10, "precision": -1, "sign": 2, "depth": 20, "fraxis": 2}
ACTIVE_TAB = 7  # The first 100F boundary grid.
TAB_SPECS = (
    ("L", "L_N"), ("R", "R_M"), ("F", "F"),
    ("FX", "FX"), ("FY", "FY"), ("F2", "F2"),
    ("F2-alt", "F2_alternate"), ("100F", "b2F"),
    ("1000FX", "b3FX"), ("1000FY", "b3FY"), ("10000F2", "b4F2"),
)


def pack(fmt, *values):
    return struct.pack("<" + fmt, *values)


def encode_string(value):
    encoded = value.encode("utf-8")
    return pack("I", len(encoded)) + encoded


def validated_grid(cells):
    result = {}
    for cell in cells:
        if len(cell) != 3 or any(type(value) is not int for value in cell):
            raise ValueError("Each cell must contain exactly three integers")
        x, y, digit = cell
        if not -(2**31) <= x < 2**31 or not -(2**31) <= y < 2**31:
            raise ValueError("Coordinate does not fit the binary format")
        if not 0 < abs(digit) < CONFIG["base"]:
            raise ValueError("Expected nonzero, already-normalized base-10 digits")
        if (x, y) in result:
            raise ValueError("Duplicate coordinate in backend grid")
        result[x, y] = digit
    return result


def load_tabs(work):
    tabs, sources = [], []
    for depth_y in (20, 21):
        path = work / f"case_10_2_20_{depth_y}_20_{depth_y}.json"
        content = path.read_bytes()
        grids = json.loads(content)
        if validated_grid(grids["L_N"]) != validated_grid(grids["L_P"]):
            raise ValueError("The selected X multiplier must match L")
        if validated_grid(grids["R_M"]) != validated_grid(grids["R_Q"]):
            raise ValueError("The selected Y multiplier must match R")
        for label, key in TAB_SPECS:
            tabs.append({"name": f"20x{depth_y} {label}",
                         "source_key": key, "grid": validated_grid(grids[key])})
        sources.append({"path": str(path.resolve()),
                        "sha256": hashlib.sha256(content).hexdigest()})
    return tabs, sources


def encode_project(tabs):
    result = bytearray(b"MDNPRJ\0\0" + pack("I", 1))
    result += encode_string("Fuzzy family examples")
    result += pack("iiiii", *CONFIG.values())
    result += pack("iI", ACTIVE_TAB, len(tabs))
    for index, tab in enumerate(tabs):
        name, grid = tab["name"], tab["grid"]
        result += pack("i", index) + encode_string(name)
        result += b"MDN2D\0" + pack("H", 1) + encode_string(name)
        result += pack("iiB", CONFIG["base"], CONFIG["precision"], CONFIG["sign"])
        if grid:
            x0, x1 = min(x for x, _ in grid), max(x for x, _ in grid)
            y0, y1 = min(y for _, y in grid), max(y for _, y in grid)
        else:
            x0, y0, x1, y1 = 0, 0, -1, -1
        height, width = y1 - y0 + 1, x1 - x0 + 1
        result += pack("iiiiii", x0, y0, x1, y1, height, width)
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                result += pack("b", grid.get((x, y), 0))
    return bytes(result)


def decode_project(content):
    """Read the serialized bytes independently and reject trailing data."""
    offset = 0

    def take(size):
        nonlocal offset
        if size < 0 or offset + size > len(content):
            raise ValueError("Truncated project")
        value = content[offset:offset + size]
        offset += size
        return value

    def read(fmt):
        return struct.unpack("<" + fmt, take(struct.calcsize("<" + fmt)))

    def string():
        return take(read("I")[0]).decode("utf-8")

    if take(8) != b"MDNPRJ\0\0" or read("I")[0] != 1:
        raise ValueError("Unsupported project header")
    name = string()
    config = dict(zip(CONFIG, read("iiiii")))
    active, count = read("iI")
    tabs = []
    for _ in range(count):
        index, = read("i")
        tab_name = string()
        if take(6) != b"MDN2D\0" or read("H")[0] != 1:
            raise ValueError("Unsupported number header")
        number_name = string()
        number_config = read("iiB")
        if number_config != (config["base"], config["precision"], config["sign"]):
            raise ValueError("Per-number configuration differs from project")
        x0, y0, x1, y1, height, width = read("iiiiii")
        if min(height, width) < 0 or (height, width) != (y1 - y0 + 1, x1 - x0 + 1):
            raise ValueError("Inconsistent grid dimensions")
        grid = {}
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                digit, = read("b")
                if digit:
                    grid[x, y] = digit
        if number_name != tab_name:
            raise ValueError("Tab and number names differ")
        tabs.append({"index": index, "name": tab_name, "grid": grid})
    if offset != len(content):
        raise ValueError("Trailing bytes after final tab")
    return {"name": name, "config": config, "active": active, "tabs": tabs}


def verify_tabs(expected, actual):
    if len(expected) != len(actual):
        raise ValueError("Tab count changed during roundtrip")
    for index, (source, loaded) in enumerate(zip(expected, actual)):
        if loaded["index"] != index or source["name"] != loaded["name"]:
            raise ValueError("Tab order or name changed during roundtrip")
        if source["grid"] != loaded["grid"]:
            raise ValueError("Grid changed during roundtrip: " + source["name"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--work", type=Path, required=True, help="Backend JSON output directory")
    parser.add_argument("--output", type=Path, required=True, help="New .mdnproj path")
    parser.add_argument("--validation-output", type=Path, help="Optional compact validation JSON")
    parser.add_argument("--native-probe", type=Path,
                        help="Optional load_project_probe executable using the native library")
    args = parser.parse_args()
    tabs, sources = load_tabs(args.work)
    content = encode_project(tabs)
    decoded = decode_project(content)
    if decoded["config"] != CONFIG or decoded["active"] != ACTIVE_TAB:
        raise ValueError("Global configuration changed during roundtrip")
    verify_tabs(tabs, decoded["tabs"])
    with args.output.open("xb") as output:
        output.write(content)

    native_validation = None
    if args.native_probe is not None:
        result = subprocess.run([str(args.native_probe.resolve()), str(args.output.resolve())],
                                capture_output=True, text=True, check=True, timeout=30)
        loaded = json.loads(result.stdout)
        if any(loaded[key] != value for key, value in CONFIG.items()):
            raise ValueError("Native loader changed the global configuration")
        if loaded["active"] != ACTIVE_TAB:
            raise ValueError("Native loader changed the active tab")
        native_tabs = [{"index": tab["index"], "name": tab["name"],
                        "grid": validated_grid(tab["cells"])} for tab in loaded["tabs"]]
        verify_tabs(tabs, native_tabs)
        native_validation = {"probe": str(args.native_probe.resolve()),
                             "all_cells_names_and_config_match": True}

    report = {
        "project": str(args.output.resolve()),
        "project_sha256": hashlib.sha256(content).hexdigest(),
        "bytes": len(content), "config": CONFIG, "active_tab": ACTIVE_TAB,
        "tab_count": len(tabs), "sources": sources,
        "python_roundtrip_all_cells_and_config_match": True,
        "python_roundtrip_strict_eof": True, "native_validation": native_validation,
        "tab_guide": {
            "prefix": "20x20 and 20x21 are zero-based X and Y cascade depths.",
            "L_R": "Finite 0.1X and 0.1Y cascades; all boundary cells are retained.",
            "F": "L*R", "FX": "F*L", "FY": "F*R", "F2": "F*F",
            "F2-alt": "(L*L)*(R*R): carry-equivalent, potentially different displayed digits.",
            "scaled": "100F, 1000FX, 1000FY and 10000F2 reveal the exact boundary expressions.",
            "global_depth": "The file format saves depth only globally; names preserve each input cutoff.",
        },
        "tabs": [{"index": index, "name": tab["name"], "source_key": tab["source_key"],
                  "nonzero_cells": len(tab["grid"])} for index, tab in enumerate(tabs)],
    }
    if args.validation_output is not None:
        args.validation_output.write_text(json.dumps(report, indent=2) + "\n")
    print(f"Created {args.output}: {len(tabs)} tabs, {len(content)} bytes; every cell verified.")
    if native_validation is not None:
        print("The native library loader also preserved every cell, tab name, and setting.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
