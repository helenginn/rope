#!/usr/bin/env bash
set -euo pipefail

YESMAN=false
FORCE_REBUILD_DEP=false
BUILD_OVERRIDE=""
PREFIX_OVERRIDE=""
DATADIR_OVERRIDE=""

for arg in "$@"; do
  case "$arg" in
  --yesman | -y) YESMAN=true ;;
  --force-rebuild-dependencies | -f) FORCE_REBUILD_DEP=true ;;
  --build=*) BUILD_OVERRIDE="${arg#--build=}" ;;
  --prefix=*) PREFIX_OVERRIDE="${arg#--prefix=}" ;;
  --datadir=*) DATADIR_OVERRIDE="${arg#--datadir=}" ;;
  *) echo "Unknown argument: $arg" ;;
  esac
done

SCRPTDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SRCDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

SRCBASENAME=$(basename "$SRCDIR")
SCRPTBASENAME=$(basename "$SCRPTDIR")

EXTRA_MESON_ARGS=""
EXTRA_MESON_ARGS="${PREFIX_OVERRIDE:+--prefix "$PREFIX_OVERRIDE"} ${DATADIR_OVERRIDE:+--datadir "$DATADIR_OVERRIDE"}"

# -- COLOURS --

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
BOLD='\033[1m'
DIM='\033[2m'
RESET='\033[0m'

# -- BANNER --

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

# -- HELPERS --

info() { echo -e "${BLUE}  ●${RESET} $*"; }
ok() { echo -e "${GREEN}  ✓${RESET} $*"; }
warn() { echo -e "${YELLOW}  ⚠${RESET} $*"; }
error() { echo -e "${RED}  ✘${RESET} $*" >&2; }
section() { echo -e "\n${BOLD}${CYAN}── $* $(printf '─%.0s' {1..50})${RESET}"; }
die() {
  error "$*"
  echo
  exit 1
}
print() { echo -e "    $*"; }

ask_yn() {
  local prompt="$1"
  local default="${2:-Y}"
  local hint
  if [ "$default" = "Y" ]; then hint="[Y/n]"; else hint="[y/N]"; fi
  if $YESMAN; then
    echo -e "  ${BOLD}${prompt}${RESET} ${DIM}${hint}${RESET} ${DIM}y (--yesman)${RESET}"
    return 0
  fi
  while true; do
    echo -ne "  ${BOLD}${prompt}${RESET} ${DIM}${hint}${RESET} "
    read -r answer
    answer="${answer:-$default}"
    case "$(echo "$answer" | tr '[:upper:]' '[:lower:]')" in
    y | yes) return 0 ;;
    n | no) return 1 ;;
    *) echo -e "  ${YELLOW}Please answer y or n.${RESET}" ;;
    esac
  done
}

# -- SCRIPT START --

info "Running script from $SRCBASENAME/$SCRPTBASENAME"

section "System Checks"
OS=$(uname -s)
ARCH=$(uname -m)
ok "Platform: $OS / $ARCH"

USE_CONAN=false
if command -v conan &>/dev/null; then
  ok "conan: $(conan --version)"
  CONAN_COMMAND=(conan)
  CONAN_AVAILABLE=true
else
  warn "conan not found; attempting ${BOLD}${YELLOW}uvx conan${RESET}"
  if command -v uvx &>/dev/null; then
    ok "uvx: $(uvx --version)"
    CONAN_COMMAND=(uvx conan)
    CONAN_AVAILABLE=true
  else
    warn "uvx not found; attempting ${BOLD}${YELLOW}pipx run conan${RESET}"
    if command -v pipx &>/dev/null; then
      ok "pipx: $(pipx --version)"
      CONAN_COMMAND=(pipx run conan)
      CONAN_AVAILABLE=true
    else
      warn "pipx not found"
      error "conan not available"
      CONAN_AVAILABLE=false
      USE_CONAN=false
    fi
  fi
fi

if command -v c++ &>/dev/null; then
  ok "C++ compiler: $(c++ --version | head -1)"
else
  die "No C++ compiler found. Install build-essential."
fi

if command -v pkg-config &>/dev/null; then
  ok "pkg-config: $(pkg-config --version)"
else
  warn "pkg-config not found — dependency detection may fail"
fi

MUST_USE_CONAN=false
if command -v meson &>/dev/null; then
  ok "meson: $(meson --version)"
else
  warn "meson not on path. Conan build mandatory"
  MUST_USE_CONAN=true
fi

if command -v ninja &>/dev/null; then
  ok "ninja: $(ninja --version)"
else
  warn "ninja not on path. Conan build mandatory"
  MUST_USE_CONAN=true
fi

if pkg-config --exists sdl2; then
  ok "SDL2: $(pkg-config --modversion sdl2)"
else
  error "SDL2 not found on system, meson will verify"
  print "Make sure SDL2 is installed via your system package manager"
fi

if pkg-config --exists SDL2_image; then
  ok "SDL2_image: $(pkg-config --modversion SDL2_image)"
else
  error "SDL2_image not found on system, meson will verify"
  print "Make sure SDL2_image is installed via your system package manager"
fi

if pkg-config --exists glew; then
  ok "SDL2_image: $(pkg-config --modversion glew)"
else
  error "glew not found on system, meson will verify"
  print "Make sure glew is installed via your system package manager"
fi

if $MUST_USE_CONAN; then
  if ! $CONAN_AVAILABLE; then
    print ""
    section "ABORTING"
    die "Neither system build tools, nor conan found."
  else
    print ""
    section "WARNING"
    warn "System package dependencies not satisfied. Conan build mandatory."
    print ""
  fi
fi

if ! $CONAN_AVAILABLE && ! $MUST_USE_CONAN; then
  print ""
  section "WARNING"
  warn "Conan not available, using system dependencies."
  print ""
fi


section "Configure Build"

# conan

PKG_MODE="sys"
if $CONAN_AVAILABLE; then
  if $MUST_USE_CONAN; then
    USE_CONAN=true
    PKG_MODE="conan"
    warn "Using system packages not possible"
  else
    if ask_yn "Use conan for dependencies? (uses system libraries otherwise)" "Y"; then
      USE_CONAN=true
      PKG_MODE="conan"
    else
      USE_CONAN=false
    fi
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

if ask_yn "Set up .clangd file for LSP?" "Y"; then
  USE_CLANGD=true
else
  USE_CLANGD=false
fi

# -- CONFIRM --
section "Confirm"
if [ -n "$BUILD_OVERRIDE" ]; then
  warn "Custom build directory"
  BUILDDIR="$BUILD_OVERRIDE"
else
  BUILDDIR="build/${ARCH}_${OS}_${PKG_MODE}_${BUILD_TYPE}"
fi
info "Planning to build in ${BUILDDIR}"

if $USE_CONAN; then
  info "Using conan for dependency package management"
else
  info "Relying on system packages for dependencies"
fi

if $USE_CLANGD; then
  info "Set up .clangd for LSP"
fi

if ask_yn "Proceed?" "Y"; then
  print ""
else
  die "Aborted manually"
fi

# -- BUILD --
CONAN_BUILD_FLAG="missing"
if $FORCE_REBUILD_DEP; then
  CONAN_BUILD_FLAG="*"
  warn "Force-rebuilding all conan dependencies"
fi

if $USE_CONAN; then
  if ! "${CONAN_COMMAND[@]}" profile detect 2>/dev/null; then
    warn "conan profile already exists. Please verify manually."
  fi
  "${CONAN_COMMAND[@]}" create recipes/gemmi -b="$CONAN_BUILD_FLAG"
  "${CONAN_COMMAND[@]}" install . -of="${BUILDDIR}" -b="$CONAN_BUILD_FLAG"
  source "./${BUILDDIR}/conanbuild.sh"
  meson setup "$BUILDDIR" --native-file="${BUILDDIR}"/conan_meson_native.ini --buildtype="$BUILD_TYPE" $EXTRA_MESON_ARGS --reconfigure --clearcache
else
  meson setup "$BUILDDIR" --buildtype="$BUILD_TYPE" $EXTRA_MESON_ARGS --reconfigure --clearcache
fi
meson compile -C "$BUILDDIR"

if $USE_CLANGD; then
  section "Setup .clangd"
  cat >".clangd" <<EOF
CompileFlags:
  CompilationDatabase: "${BUILDDIR}"
EOF
  ok "Compilation database points at $BUILDDIR"
fi

section "Done!"
ok "RoPE succesfully built in $BUILDDIR"
info "Run RoPE locally by starting"
print "${BOLD}${YELLOW}$BUILDDIR/rope.gui${RESET} from the terminal"
info "Install RoPE globally by running"
print "${BOLD}${YELLOW}meson install -C $BUILDDIR${RESET}"
print ""
