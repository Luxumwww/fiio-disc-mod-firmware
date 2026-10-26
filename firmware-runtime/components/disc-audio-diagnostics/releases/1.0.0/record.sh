#!/bin/sh
# Bounded TF-only evidence, independently disabled through the module manager.
unset LD_PRELOAD
umask 077
root=/usr/data/disc-runtime/components/disc-audio-diagnostics
ram=/tmp/disc-runtime
target=/tmp/sdcard/DISC_AUDIO_LOGS
boot=$(cat /proc/sys/kernel/random/boot_id)
trap 'exit 0' TERM INT
while [ -f "$root/enabled" ] && [ ! -f "$root/disabled" ];do
 if [ -d /tmp/sdcard ] && grep -q ' /tmp/sdcard ' /proc/mounts && [ ! -f /tmp/sdcard/DISC_MODULES_SAFE_MODE ];then
  mkdir -p "$target"
  if [ -f "$target/trace.log" ] && [ "$(wc -c < "$target/trace.log")" -ge 2097152 ];then
   mv -f "$target/trace.2.log" "$target/trace.3.log" 2>/dev/null || :
   mv -f "$target/trace.1.log" "$target/trace.2.log" 2>/dev/null || :
   mv -f "$target/trace.log" "$target/trace.1.log"
  fi
  {
   printf '\n=== boot=%s uptime=' "$boot";cat /proc/uptime
   for f in "$ram/dop-final.state" "$ram/usbdac-dop.state" "$ram/state" /proc/asound/card0/pcm3p/sub0/status /proc/asound/card0/pcm3p/sub0/hw_params /proc/asound/card1/pcm0c/sub0/hw_params;do
    [ ! -f "$f" ] || { printf '[%s]\n' "$f";cat "$f"; }
   done
  } >> "$target/trace.log" 2>/dev/null
  # Packet samples stay bounded too, with a fixed latest pair.
  for name in dop-final-good dop-final-bad;do
   if [ -f "$ram/$name.bin" ] && ! cmp -s "$ram/$name.bin" "$target/$name.bin";then cp "$ram/$name.bin" "$target/$name.bin";fi
  done
  printf 'version=1.0.0\nTF_path=%s\nrecording=1\nboot=%s\n' "$target" "$boot" > "$ram/audio-diagnostics.state"
 else printf 'version=1.0.0\nrecording=0\nreason=TF_unavailable_or_safe_mode\n' > "$ram/audio-diagnostics.state";fi
 sleep 1
done
