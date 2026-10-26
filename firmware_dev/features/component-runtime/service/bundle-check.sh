#!/bin/sh
# TF directory package v1. Hashes detect corruption; they are not signatures.
unset LD_PRELOAD
dir=$1
[ -d "$dir" ] && [ ! -L "$dir" ] || exit 2
case "$dir" in *[!a-zA-Z0-9_./-]*) exit 2;;esac
[ -z "$(find "$dir" -type l -o -type b -o -type c -o -type p -o -type s)" ] || exit 2
count=$(find "$dir" -type f | wc -l)
[ "$count" -gt 0 ] && [ "$count" -le 512 ] || exit 2
[ "$(du -sk "$dir" | awk '{print $1}')" -le 65536 ] || exit 2
cd "$dir" || exit 2
# Restrict checksum names before giving them to sha256sum. No absolute/parent paths.
awk 'length($1)!=64 || $1~/[^0-9a-f]/ || NF!=2 || $2~/[^a-zA-Z0-9_.\/-]/ || $2~/^\// || $2~/\.\./ || $2=="SHA256SUMS" {exit 1} {if(seen[$2]++)exit 1;n++} END {if(!n)exit 1}' SHA256SUMS || exit 2
listed=$(awk '{print $2}' SHA256SUMS | sort)
actual=$(find . -type f | sed 's|^./||' | awk '$0!="SHA256SUMS"' | sort)
[ "$listed" = "$actual" ] || exit 2
sha256sum -c SHA256SUMS >/dev/null 2>&1 || exit 4
valid(){ case "$1" in ''|*[!a-zA-Z0-9._-]*|.*|*..*) return 1;;esac;[ "${#1}" -le 64 ]; }
valid "$(cat MODULE_ID)" && valid "$(cat MODULE_VERSION)" || exit 2
entry=$(cat ENTRY)
case "$entry" in ''|/*|*..*|*[!a-zA-Z0-9_./-]*) exit 2;;esac
[ -f "$entry" ] || exit 2
case "$(cat KIND)" in tool|service) :;;*) exit 2;;esac
[ "$(cat ABI)" = mips32r2-o32-fp64-nan2008 ] && [ "$(cat BASE)" = 257 ] || exit 2
case "$(cat STORAGE)" in nand|tf) :;;*) exit 2;;esac
case "$(cat STARTUP)" in 0|1) :;;*) exit 2;;esac
[ "$(cat KIND):$(cat STARTUP)" != tool:1 ] || exit 2
