#!/usr/bin/env python3
"""Zet een Markdown-bestand om naar een opgemaakte PDF (via Chrome headless).

Gebruik: md2pdf.py invoer.md uitvoer.pdf
"""
import pathlib
import subprocess
import sys
import tempfile

import markdown

CHROME = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"

CSS = """
@page { size: A4; margin: 18mm 18mm 20mm 18mm; }
body { font-family: -apple-system, "Helvetica Neue", Arial, sans-serif; font-size: 10.5pt;
       line-height: 1.5; color: #1e2226; }
h1 { font-size: 20pt; margin: 0 0 6pt 0; color: #1e2226; border-bottom: 3px solid #3ddc84; padding-bottom: 4pt; }
h2 { font-size: 13pt; margin: 16pt 0 4pt 0; color: #1f7a4a; }
h3 { font-size: 11pt; margin: 10pt 0 2pt 0; color: #1e2226; }
p, li { margin: 3pt 0; }
ul, ol { padding-left: 18pt; margin: 4pt 0; }
code { font-family: "SF Mono", Menlo, monospace; font-size: 9.5pt; background: #eef1f3;
       padding: 1pt 3pt; border-radius: 3px; }
/* Opdrachten nooit afbreken: een afgebroken regel kopieert als kapotte opdracht */
pre { background: #eef1f3; border-left: 3px solid #3ddc84; padding: 6pt 8pt; border-radius: 4px;
      white-space: pre; font-size: 8.3pt; overflow: hidden; }
pre code { background: none; padding: 0; font-size: inherit; }
hr { border: none; border-top: 1px solid #ccd2d7; margin: 18pt 0; page-break-after: always; }
strong { color: #000; }
"""


def main() -> None:
    src, dst = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2])
    body = markdown.markdown(src.read_text(encoding="utf-8"), extensions=["fenced_code"])
    title = src.stem
    html = f"<!doctype html><html><head><meta charset='utf-8'><title>{title}</title>" \
           f"<style>{CSS}</style></head><body>{body}</body></html>"

    with tempfile.TemporaryDirectory() as tmp:
        page = pathlib.Path(tmp) / "page.html"
        page.write_text(html, encoding="utf-8")
        subprocess.run([CHROME, "--headless=new", "--disable-gpu", "--no-pdf-header-footer",
                        f"--user-data-dir={tmp}/profile", f"--print-to-pdf={dst.resolve()}",
                        page.as_uri()],
                       check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


if __name__ == "__main__":
    main()
