#!/usr/bin/env python3
import pathlib
import re
import sys

source_path = pathlib.Path(sys.argv[1])
output_path = pathlib.Path(sys.argv[2])
source_text = source_path.read_text(encoding='utf-8')
match = re.search(
    r"void StatusNotifierItemSource::overlayIcon\(QIcon \*icon, QIcon \*overlay\)\n\{.*?\n\}",
    source_text,
    re.S,
)
if not match:
    raise SystemExit(f"Could not find StatusNotifierItemSource::overlayIcon in {source_path}")

output_text = """#include \"statusnotifieritemsource_overlayicon.h\"\n\n#include <QPainter>\n#include <QPixmap>\n#include <QRect>\n\n""" + match.group(0) + "\n"
output_path.write_text(output_text, encoding='utf-8')
