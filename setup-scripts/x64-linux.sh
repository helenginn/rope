#!/usr/bin/env bash
# -- COLOURS --

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
DIM='\033[2m'
RESET='\033[0m'

SCRPTDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SRCDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

SRCBASENAME=$(basename "$SRCDIR")
SCRPTBASENAME=$(basename "$SCRPTDIR")

# -- Banner --

echo
echo -e "\033[1m\033[38;5;57m  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;93m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;129m  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;165m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;201m \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;200m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \033[1m\033[38;5;197m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;196m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88 \033[0m"
echo -e "\033[1m\033[38;5;57m  \xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;93m  \xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;129m \xe2\x96\x88\xe2\x96\x88  \033[1m\033[38;5;165m  \xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;201m \xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;200m  \xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;197m\xe2\x96\x88\xe2\x96\x88  \033[1m\033[38;5;196m    \033[0m"
echo -e "\033[1m\033[38;5;57m  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;93m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;129m \xe2\x96\x88\xe2\x96\x88  \033[1m\033[38;5;165m  \xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;201m \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;200m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88  \033[1m\033[38;5;197m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;196m\xe2\x96\x88   \033[0m"
echo -e "\033[1m\033[38;5;57m  \xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;93m  \xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;129m \xe2\x96\x88\xe2\x96\x88  \033[1m\033[38;5;165m  \xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;201m \xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;200m     \033[1m\033[38;5;197m\xe2\x96\x88\xe2\x96\x88  \033[1m\033[38;5;196m    \033[0m"
echo -e "\033[1m\033[38;5;57m  \xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;93m  \xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;129m  \xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;165m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;201m \xe2\x96\x88\xe2\x96\x88 \033[1m\033[38;5;200m     \033[1m\033[38;5;197m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88\033[1m\033[38;5;196m\xe2\x96\x88\xe2\x96\x88\xe2\x96\x88 \033[0m"
echo
echo -e "  ${BOLD}Representation of Protein Entities${RESET}"
echo -e "  ${DIM}Build configuration script${RESET}"
echo

# -- Helpers --
info()    { echo -e "${BLUE}  ●${RESET} $*"; }
ok()      { echo -e "${GREEN}  ✓${RESET} $*"; }
warn()    { echo -e "${YELLOW}  ⚠${RESET} $*"; }
error()   { echo -e "${RED}  ✘${RESET} $*" >&2; }
section() { echo -e "\n${BOLD}${CYAN}── $* $(printf '─%.0s' {1..50})${RESET}"; }
die()     { error "$*"; echo; exit 1; }

ask_yn() {
  local prompt="$1"
  local default="${2:-Y}"
  local hint
  if [ "$default" = "Y" ]; then hint="[Y/n]"; else hint="[y/N]"; fi
  while true; do
    echo -ne "  ${BOLD}${prompt}${RESET} ${DIM}${hint}${RESET} "
    read -r answer
    answer="${answer:-$default}"
    case "${answer,,}" in
      y|yes) return 0 ;;
      n|no)  return 1 ;;
      *)     echo -e "  ${YELLOW}Please answer y or n.${RESET}" ;;
    esac
  done
}

# -- SCRIPT START --

info "Running script from $SRCBASENAME/$SCRPTBASENAME"

section "System Checks"
OS=$(uname -s)
ARCH=$(uname -m)
ok "Platform: $OS / $ARCH"
 
if command -v c++ &>/dev/null; then
  ok "C++ compiler: $(c++ --version | head -1)"
else
  die "No C++ compiler found. Install build-essential."
fi
 
if command -v meson &>/dev/null; then
  ok "Meson: $(meson --version)"
else
  die "Meson not found. Install meson"
fi
 
if command -v ninja &>/dev/null; then
  ok "Ninja: $(ninja --version)"
else
  die "Ninja not found. Install ninja-build"
fi
 
if command -v pkg-config &>/dev/null; then
  ok "pkg-config: $(pkg-config --version)"
else
  warn "pkg-config not found — dependency detection may fail"
fi

if command -v conan &>/dev/null; then
  ok "conan: $(conan --version)"
  CONAN_COMMAND=(conan)
  CONAN_AVAILABLE=true
else
  warn "conan not found — attempt pipx run conan"
  if command -v pipx &>/dev/null; then
    ok "pipx: $(pipx --version)"
    CONAN_COMMAND=(pipx run conan)
    CONAN_AVAILABLE=true
  else
    warn "pipx not found — attempt uvx conan"
    if command -v uvx &>/dev/null; then
      ok "uvx: $(uvx --version)"
      CONAN_COMMAND=(uvx conan)
      CONAN_AVAILABLE=true
    else
      warn "uvx not found"
      error "conan not available"
      CONAN_AVAILABLE=false
      USE_CONAN=false
    fi
  fi
fi


section "Configure Build"

# conan 

PKG_MODE="sys"
if $CONAN_AVAILABLE; then
  if ask_yn "Use conan for dependencies? (uses system libraries otherwise)" "Y"; then
    USE_CONAN=true
    PKG_MODE="conan"
  else
    USE_CONAN=false
  fi
fi

# Build type

if ask_yn "Compile in release mode? (debug otherwise)" "Y"; then
  BUILD_TYPE="release"
else
  BUILD_TYPE="debugoptimized"
  if ask_yn "Enable extra debug symbols? (full debug, slower build)" "N"; then
    BUILD_TYPE="debug"
  fi
fi

section Confirm
BUILDDIR="build/${ARCH}_${OS}_${PKG_MODE}_${BUILD_TYPE}"
info "Planning to build in ${BUILDDIR}"
if ask_yn "Proceed?" "Y"; then
  echo 
else 
  die "Aborted manually"
fi

if $USE_CONAN; then 
  "${CONAN_COMMAND[@]}" create recipes/gemmi -b=missing
  "${CONAN_COMMAND[@]}" install . -of="${BUILDDIR}" -b=missing
  meson setup "$BUILDDIR" --native-file="${BUILDDIR}"/conan_meson_native.ini
else
  meson setup "$BUILDDIR"
fi
meson compile -C "$BUILDDIR"

