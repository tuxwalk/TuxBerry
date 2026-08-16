#!/bin/sh
# SPDX-License-Identifier: MIT
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
LINUX_DIR="${MSM8916_LINUX:-$HOME/msm8916-linux}"

cd "$ROOT"

if [ ! -f build/tuxberry.bin ]; then
	echo "Missing build/tuxberry.bin"
	exit 1
fi

if [ ! -f payload/gt58wifi-zImage ]; then
	echo "Missing payload/gt58wifi-zImage"
	exit 1
fi

if [ ! -f /tmp/t350-working/ramdisk ]; then
	echo "Extracting known-good SM-T350 ramdisk..."

	rm -rf /tmp/t350-working

	/usr/bin/unpack_bootimg \
		--boot_img "$LINUX_DIR/boot-initramfs5.img" \
		--out /tmp/t350-working
fi

python3 - <<'PY'
from pathlib import Path
import struct

tb = Path("build/tuxberry.bin").read_bytes()
src = Path("payload/gt58wifi-zImage").read_bytes()

magic = struct.unpack_from("<I", tb, 0x24)[0]
end = struct.unpack_from("<I", tb, 0x2c)[0]

assert magic == 0x016f2818
assert end == len(tb), f"header end {end:#x} != size {len(tb):#x}"

dtb_off = struct.unpack_from("<I", src, 0x2c)[0]
dtb = src[dtb_off:]

assert dtb[:4] == b"\xd0\x0d\xfe\xed"

Path("build/tuxberry-zImage-dtb").write_bytes(tb + dtb)

print(f"TuxBerry: {len(tb):#x}")
print(f"DTB:      {len(dtb):#x}")
print(f"Final:    {len(tb + dtb):#x}")
PY

/usr/bin/mkbootimg \
	--kernel "$ROOT/build/tuxberry-zImage-dtb" \
	--ramdisk /tmp/t350-working/ramdisk \
	--base 0x80000000 \
	--kernel_offset 0x00008000 \
	--ramdisk_offset 0x02000000 \
	--tags_offset 0x00000100 \
	--pagesize 2048 \
	--header_version 0 \
	--cmdline 'console=tty0 lk2nd.pass-simplefb=autorefresh' \
	--output "$ROOT/build/tuxberry-gt58wifi.img"

echo
echo "Created:"
ls -lh "$ROOT/build/tuxberry-gt58wifi.img"
