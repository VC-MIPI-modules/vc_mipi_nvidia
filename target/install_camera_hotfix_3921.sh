#!/bin/bash
# NVIDIA camera hotfix (Jetson Customer IQ Migration Tools, L4T R39.2.1) on the Jetson:
# install it with a backup, and choose how nvargus-daemon gets its ISP tuning.
#
#   sudo ./install_camera_hotfix_3921.sh [-t TARBALL] [--mode MODE] [options]
#
# Steps (each only if needed):
#   1. Install the hotfix from TARBALL (default ~/camera_hotfix.tbz2), backup of all replaced
#      files. Skipped if already installed.
#   2. Configure the tuning mode (--mode), can be re-run any time:
#      legacy    text config: NVCAMERA_NITO_PATH=CONFIG, uses camera_overrides.isp if present,
#                otherwise NVIDIA defaults. Recommended by NVIDIA for Argus.
#      nito      NITO (NVIDIA default): no drop-in, the daemon picks the .nito in
#                /var/nvidia/nvcam/settings whose metadata match the camera module.
#      generate  create a NITO for each camera from the text config (camera_overrides.isp or
#                defaults), install it to /var/nvidia/nvcam/settings and switch to 'nito'.
#
# Options:
#   -t, --tarball FILE   hotfix tarball (camera_hotfix.tbz2)
#   -m, --mode MODE      legacy | nito | generate (without: only install, mode unchanged)
#   -i, --isp FILE       install FILE as camera_overrides.isp (legacy, generate)
#   -n, --nito FILE      install a .nito file (repeatable; for nito)
#   -s, --sensors LIST   sensor ids for generate, e.g. 0 or 0,1 (default 0)
#   -M, --modes LIST     sensor modes for generate, e.g. 0 or 0,1 (default 0)
#   --restore            undo everything (backup back, new files and drop-in removed)
#   --force              install even if the L4T version does not match
#
# Examples:
#   sudo ./install_camera_hotfix_3921.sh --mode legacy -i camera_overrides.isp
#   sudo ./install_camera_hotfix_3921.sh --mode generate -s 0 -M 0
#   sudo ./install_camera_hotfix_3921.sh --mode nito -n jakku_front_vc_mipi.nito
set -euo pipefail

L4T_EXPECTED="R39 (release), REVISION: 2.1"
LIBNVSCF_MD5="753175e084270f8d4a6fde6620ff450a"     # libnvscf.so from the r39.2.1 package
SETTINGS="/var/nvidia/nvcam/settings"
STATE_DIR="/var/lib/camera_hotfix"                  # persistent (not /tmp)
BACKUP_DIR="$STATE_DIR/backup"
MANIFEST="$STATE_DIR/manifest"                      # "BACKUP <file>" or "NEW <file>"
INSTALLED_FILES="$STATE_DIR/installed_files"        # .isp/.nito files installed by this script
GEN_DIR="$STATE_DIR/nitogen"                        # output directory for generated NITO files
DROPIN="/etc/systemd/system/nvargus-daemon.service.d/camera_hotfix.conf"

TARBALL=""
MODE=""
ISP=""
NITOS=()
SENSORS="0"
MODES="0"
RESTORE=0
FORCE=0

usage() { sed -n '2,/^set /p' "$0" | grep '^#' | sed 's/^# \{0,1\}//'; exit "${1:-0}"; }

while [ $# -gt 0 ]; do
	case "$1" in
		-t|--tarball) TARBALL="$2"; shift 2 ;;
		-m|--mode)    MODE="$2"; shift 2 ;;
		--legacy)     MODE="legacy"; shift ;;          # compatible with the first version
		-i|--isp)     ISP="$2"; shift 2 ;;
		-n|--nito)    NITOS+=("$2"); shift 2 ;;
		-s|--sensors) SENSORS="$2"; shift 2 ;;
		-M|--modes)   MODES="$2"; shift 2 ;;
		--restore)    RESTORE=1; shift ;;
		--force)      FORCE=1; shift ;;
		-h|--help)    usage 0 ;;
		*) echo "Unknown option: $1" >&2; usage 1 ;;
	esac
done
case "$MODE" in ""|legacy|nito|generate) ;; *) echo "Unknown mode: $MODE" >&2; exit 1 ;; esac

[ "$(id -u)" -eq 0 ] || { echo "Please run with sudo." >&2; exit 1; }

# Files given relative to the caller's directory stay valid, resolve them before anything else
[ -n "$ISP" ] && ISP=$(readlink -f "$ISP")
for i in "${!NITOS[@]}"; do NITOS[$i]=$(readlink -f "${NITOS[$i]}"); done

if [ -z "$TARBALL" ]; then
	USER_HOME=$(getent passwd "${SUDO_USER:-root}" | cut -d: -f6)
	TARBALL="$USER_HOME/camera_hotfix.tbz2"
fi

# ---------------------------------------------------------------------------- helpers
write_dropin() {   # $@: environment lines
	mkdir -p "$(dirname "$DROPIN")"
	{ echo "[Service]"; for e in "$@"; do echo "Environment=$e"; done; } > "$DROPIN"
	systemctl daemon-reload
}

remove_dropin() {
	if [ -f "$DROPIN" ]; then rm -f "$DROPIN"; systemctl daemon-reload; fi
}

restart_daemon() {
	systemctl daemon-reload
	systemctl restart nvargus-daemon
	sleep 1
	systemctl is-active --quiet nvargus-daemon || { echo "ERROR: nvargus-daemon is not running." >&2; return 1; }
}

install_setting() {   # $1: source file, $2: target name in $SETTINGS
	install -o root -g root -m 644 "$1" "$SETTINGS/$2"
	grep -qx "$SETTINGS/$2" "$INSTALLED_FILES" 2>/dev/null || echo "$SETTINGS/$2" >> "$INSTALLED_FILES"
	echo "installed: $SETTINGS/$2"
}

show_status() {
	echo "--- Status"
	systemctl show nvargus-daemon -p Environment
	if command -v nvargus_nvraw >/dev/null; then
		timeout 30 nvargus_nvraw --lps 2>&1 | tail -n +2 || true
	fi
}

# ---------------------------------------------------------------------------- restore
if [ "$RESTORE" -eq 1 ]; then
	[ -f "$MANIFEST" ] || { echo "No installation found ($MANIFEST missing)." >&2; exit 1; }
	while read -r kind file; do
		case "$kind" in
			BACKUP) cp -a "$BACKUP_DIR$file" "$file" && echo "restored: $file" ;;
			NEW)    rm -f "$file" && echo "removed:  $file" ;;
		esac
	done < "$MANIFEST"
	if [ -f "$INSTALLED_FILES" ]; then
		while read -r f; do rm -f "$f" && echo "removed:  $f"; done < "$INSTALLED_FILES"
	fi
	remove_dropin && echo "removed:  $DROPIN"
	rm -rf "$STATE_DIR"
	ldconfig
	restart_daemon || true
	echo "Hotfix removed."
	exit 0
fi

# ---------------------------------------------------------------------------- 1. install
if [ -f "$MANIFEST" ]; then
	echo "Hotfix already installed, installation skipped."
else
	REL=$(head -1 /etc/nv_tegra_release 2>/dev/null || true)
	if [[ "$REL" != *"$L4T_EXPECTED"* ]]; then
		echo "L4T does not match: '$REL' (expected: $L4T_EXPECTED)." >&2
		[ "$FORCE" -eq 1 ] || { echo "Aborted (use --force to install anyway)." >&2; exit 1; }
	fi
	[ -f "$TARBALL" ] || { echo "Tarball not found: $TARBALL" >&2; exit 1; }
	echo "Tarball: $TARBALL"
	mkdir -p "$BACKUP_DIR"
	: > "$MANIFEST"
	tar -tjf "$TARBALL" | grep -v '/$' | while read -r rel; do
		file="/${rel#./}"
		if [ -e "$file" ]; then
			cp -a --parents "$file" "$BACKUP_DIR/"
			echo "BACKUP $file" >> "$MANIFEST"
		else
			echo "NEW $file" >> "$MANIFEST"
		fi
	done
	echo "Backed up: $(grep -c '^BACKUP' "$MANIFEST") files, new: $(grep -c '^NEW' "$MANIFEST")"
	# extract without touching permissions/ownership of existing directories (/usr, /var ...)
	tar -xjf "$TARBALL" -C / --no-overwrite-dir --no-same-owner
	ldconfig
	MD5=$(md5sum /usr/lib/aarch64-linux-gnu/nvidia/libnvscf.so | cut -d' ' -f1)
	[ "$MD5" = "$LIBNVSCF_MD5" ] && echo "libnvscf.so ok" || echo "WARNING: libnvscf.so MD5 $MD5 (expected $LIBNVSCF_MD5)"
	for f in /usr/sbin/nvcfg2nito /usr/lib/aarch64-linux-gnu/nvidia/libnvm_cam_tuning_l4t_cfg2nito.so \
	         "$SETTINGS/template.nito"; do
		[ -e "$f" ] || echo "MISSING: $f"
	done
fi

# ---------------------------------------------------------------------------- 2. mode
[ -n "$ISP" ] && install_setting "$ISP" camera_overrides.isp
for n in "${NITOS[@]}"; do install_setting "$n" "$(basename "$n")"; done

case "$MODE" in
	"")
		echo "No mode given, nvargus-daemon configuration unchanged."
		[ -f "$DROPIN" ] || echo "Note: without --mode the daemon runs in NITO mode and needs a matching .nito."
		[ ${#NITOS[@]} -gt 0 ] || [ -n "$ISP" ] || exit 0
		restart_daemon
		;;
	legacy)
		write_dropin "NVCAMERA_NITO_PATH=CONFIG" "NVCAMERA_NITO_DUMP_PATH=0"
		[ -f "$SETTINGS/camera_overrides.isp" ] \
			&& echo "Legacy mode with $SETTINGS/camera_overrides.isp" \
			|| echo "Legacy mode without camera_overrides.isp (NVIDIA defaults)"
		restart_daemon
		;;
	nito)
		remove_dropin
		echo "NITO mode. .nito files in $SETTINGS:"; ls -1 "$SETTINGS"/*.nito
		restart_daemon
		;;
	generate)
		# CONFIG mode with the dump enabled: on every capture the daemon writes binary_<mode>.cfg
		# and converts it into <badge>.nito in its home directory, here $GEN_DIR.
		rm -rf "$GEN_DIR"; mkdir -p "$GEN_DIR"
		write_dropin "NVCAMERA_NITO_PATH=CONFIG" "HOME=$GEN_DIR"
		restart_daemon
		IFS=, read -ra S_LIST <<< "$SENSORS"
		IFS=, read -ra M_LIST <<< "$MODES"
		for s in "${S_LIST[@]}"; do
			for m in "${M_LIST[@]}"; do
				echo "Generating NITO: sensor $s, mode $m"
				timeout 60 nvargus_nvraw --c "$s" --mode "$m" --file "$GEN_DIR/capture_${s}_${m}.nvraw" >/dev/null 2>&1 \
					|| echo "  WARNING: nvargus_nvraw sensor $s mode $m failed"
			done
		done
		journalctl -u nvargus-daemon --since "-5 min" --no-pager | grep -E "auto-converted|nvcfg2nito failed|tools not found" | tail -5 || true
		mapfile -t GEN < <(find "$GEN_DIR" -maxdepth 1 -name '*.nito')
		if [ ${#GEN[@]} -eq 0 ]; then
			echo "ERROR: no .nito generated (see journalctl -u nvargus-daemon). Daemon stays in legacy mode." >&2
			write_dropin "NVCAMERA_NITO_PATH=CONFIG" "NVCAMERA_NITO_DUMP_PATH=0"
			restart_daemon
			exit 1
		fi
		for n in "${GEN[@]}"; do install_setting "$n" "$(basename "$n")"; done
		remove_dropin
		restart_daemon
		echo "Switched to NITO mode."
		;;
esac

show_status
echo "Undo: sudo $0 --restore"
