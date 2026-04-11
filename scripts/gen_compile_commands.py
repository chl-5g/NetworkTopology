#!/usr/bin/env python3
"""生成 compile_commands.json（绝对 directory/file），供 clangd / C++ 扩展匹配。"""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
INC = [
    "-Iclient",
    "-Iserver",
    "-Ilayer1",
    "-Ilayer2",
    "-Ilayer3",
    "-Ilayer4",
    "-Ilayer5",
    "-Ilayer6",
    "-Ilayer7",
]
BASE = ["clang", "-std=c11", "-Wall", "-Wextra"] + INC + ["-c"]

entries = []
for src in sorted(ROOT.rglob("*.c")):
    if "/." in str(src):
        continue
    rel = src.relative_to(ROOT).as_posix()
    entries.append(
        {
            "directory": str(ROOT),
            "arguments": BASE + [rel],
            "file": str(src),
        }
    )

out = ROOT / "compile_commands.json"
out.write_text(json.dumps(entries, indent=2) + "\n", encoding="utf-8")
print("wrote", out, "entries:", len(entries))
