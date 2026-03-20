#!/usr/bin/env pwsh
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$SCRPTDIR = Split-Path -Parent $MyInvocation.MyCommand.Path
$SRCDIR   = Split-Path -Parent $SCRPTDIR

$SRCBASENAME  = Split-Path -Leaf $SRCDIR
$SCRPTBASENAME = Split-Path -Leaf $SCRPTDIR

# -- COLOURS --
$RED = "`e[0;31m"
$GREEN = "`e[0;32m"
$YELLOW = "`e[1;33m"
$BLUE = "`e[0;34m"
$CYAN = "`e[0;36m"
$BOLD = "`e[1m"
$DIM = "`e[2m"
$RESET = "`e[0m"

# -- BANNER --

[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$b = [char]0x2588

Write-Host ""
Write-Host "`e[1m`e[38;5;57m  $b$b$b`e[1m`e[38;5;93m$b$b$b `e[1m`e[38;5;129m  $b$b$b`e[1m`e[38;5;165m$b$b$b `e[1m`e[38;5;201m $b$b$b`e[1m`e[38;5;200m$b$b$b  `e[1m`e[38;5;197m$b$b$b$b`e[1m`e[38;5;196m$b$b$b `e[0m"
Write-Host "`e[1m`e[38;5;57m  $b$b `e[1m`e[38;5;93m  $b$b`e[1m`e[38;5;129m $b$b  `e[1m`e[38;5;165m  $b$b`e[1m`e[38;5;201m $b$b `e[1m`e[38;5;200m  $b$b `e[1m`e[38;5;197m$b$b  `e[1m`e[38;5;196m    `e[0m"
Write-Host "`e[1m`e[38;5;57m  $b$b$b`e[1m`e[38;5;93m$b$b$b `e[1m`e[38;5;129m $b$b  `e[1m`e[38;5;165m  $b$b`e[1m`e[38;5;201m $b$b$b`e[1m`e[38;5;200m$b$b$b  `e[1m`e[38;5;197m$b$b$b$b`e[1m`e[38;5;196m$b   `e[0m"
Write-Host "`e[1m`e[38;5;57m  $b$b `e[1m`e[38;5;93m  $b$b`e[1m`e[38;5;129m $b$b  `e[1m`e[38;5;165m  $b$b`e[1m`e[38;5;201m $b$b `e[1m`e[38;5;200m     `e[1m`e[38;5;197m$b$b  `e[1m`e[38;5;196m    `e[0m"
Write-Host "`e[1m`e[38;5;57m  $b$b `e[1m`e[38;5;93m  $b$b`e[1m`e[38;5;129m  $b$b$b`e[1m`e[38;5;165m$b$b$b `e[1m`e[38;5;201m $b$b `e[1m`e[38;5;200m     `e[1m`e[38;5;197m$b$b$b$b`e[1m`e[38;5;196m$b$b$b `e[0m"
Write-Host ""
Write-Host "  ${BOLD}Representation of Protein Entities${RESET}"
Write-Host "  ${DIM}Build configuration script${RESET}"
Write-Host ""

# -- HELPERS --

function Info   { param($msg) Write-Host "  " -NoNewline; Write-Host "●" -ForegroundColor Blue -NoNewline; Write-Host " $msg" }
function Ok     { param($msg) Write-Host "  " -NoNewline; Write-Host "✓" -ForegroundColor Green -NoNewline; Write-Host " $msg" }
function Warn   { param($msg) Write-Host "  " -NoNewline; Write-Host "⚠" -ForegroundColor Yellow -NoNewline; Write-Host " $msg" }
function Err    { param($msg) Write-Host "  " -NoNewline; Write-Host "✘" -ForegroundColor Red -NoNewline; Write-Host " $msg" }
function Print  { param($msg) Write-Host "    $msg" }
function Section {
    param($msg)
    $line = "─" * 50
    Write-Host ""
    Write-Host "── $msg $line" -ForegroundColor Cyan
}
function Die {
    param($msg)
    Err $msg
    Write-Host ""
    exit 1
}
 
function Ask-YN {
    param(
        [string]$Prompt,
        [string]$Default = "Y"
    )
    $hint = if ($Default -eq "Y") { "[Y/n]" } else { "[y/N]" }
    while ($true) {
        Write-Host "  " -NoNewline
        Write-Host $Prompt -ForegroundColor White -NoNewline
        Write-Host " $hint " -ForegroundColor DarkGray -NoNewline
        $answer = Read-Host
        if ([string]::IsNullOrWhiteSpace($answer)) { $answer = $Default }
        switch ($answer.ToLower()) {
            "y"   { return $true }
            "yes" { return $true }
            "n"   { return $false }
            "no"  { return $false }
            default { Write-Host "  Please answer y or n." -ForegroundColor Yellow }
        }
    }
}

function Invoke-Conan {
    if ($script:CONAN_COMMAND.Count -gt 1) {
        $pre = [array]($script:CONAN_COMMAND[1..($script:CONAN_COMMAND.Count - 1)])
    } else {
        $pre = @()
    }
    & $script:CONAN_COMMAND[0] @pre @args
}

# -- SCRIPT START --

Info "Running script from $SRCBASENAME\$SCRPTBASENAME"

section "System Checks"

if (-not [System.Runtime.InteropServices.RuntimeInformation]::IsOSPlatform([System.Runtime.InteropServices.OSPlatform]::Windows)) {
    Die "This setup script only supports Windows. Use setup.sh on Linux/macOS."
}

$OS   = "Windows"
$ARCH = $env:PROCESSOR_ARCHITECTURE   # AMD64, ARM64, x86
Ok "Platform: $OS / $ARCH"

$CXX = $null
foreach ($candidate in @("cl", "clang-cl", "g++")) {
    if (Get-Command $candidate -ErrorAction SilentlyContinue) {
        $CXX = $candidate
        break
      }
  }
if ($null -eq $CXX) {
    Die "No C++ compiler found. Install Visual Studio Build Tools (cl) or LLVM (clang-cl). Make sure compiler is in Path (e.g., via Developer Shell)."
  }
$cxxVer = & $CXX --version 2>&1 | Select-Object -First 1
Ok "C++ compiler: $CXX $cxxVer"

if (Get-Command meson -ErrorAction SilentlyContinue) {
    $mesonVer = & meson --version 2>&1
    Ok "meson: $mesonVer"
  } else {
    Die "meson not found. Install via pip or winget."
  }

if (Get-Command ninja -ErrorAction SilentlyContinue) {
    $ninjaVer = & ninja --version 2>&1
    Ok "ninja: $ninjaVer"
  } else {
    Die "ninja not found. Install via pip or winget."
  }

$CONAN_COMMAND = $null

if (Get-Command conan -ErrorAction SilentlyContinue) {
    $conanVer = & conan --version 2>&1 | Select-Object -First 1
    Ok "conan: $conanVer"
    $CONAN_COMMAND = @("conan")
  } else {
    Warn "conan not found; attempting ${BOLD}${YELLOW}uvx conan${RESET}"
    if (Get-Command uvx -ErrorAction SilentlyContinue) {
        $uvxVer = & uvx --version 2>&1
        Ok "uvx: $uvxVer"
        $CONAN_COMMAND = @("uvx", "conan")
        } else {
          Warn "uvx not found; attempting ${BOLD}${YELLOW}pipx run conan${RESET}"
          if (Get-Command pipx -ErrorAction SilentlyContinue) {
              $pipxVer = & pipx --version 2>&1
              Ok "pipx: $pipxVer"
              $CONAN_COMMAND = @("pipx", "run", "conan")
      } else {
          Die "conan is required on Windows but could not be found. Install conan, pipx or uvx."
        }
      }
  }
# -- CONFIGURE BUILD --
Section "Configure Build"
Info "Using conan for dependency management (required on Windows)"
$PKG_MODE="conan"

if (Ask-YN "Compile in release mode? (debug otherwise)" "Y") {
    $BUILD_TYPE="release"
  } else {
    $BUILD_TYPE="debugoptimized"
    if (Ask-YN "Enable extra debug symbols? (full debug, slower build)" "N") {
        $BUILD_TYPE="debug"
      }
  }

if (Ask-YN "Set up .clangd file for LSP?" "Y") {
    $USE_CLANGD=$true
  } else {
    $USE_CLANGD=$false
  }

# -- CONFIRM --
Section "Confirm"
$BUILDDIR="build\${ARCH}_${OS}_${PKG_MODE}_${BUILD_TYPE}"
Info "Planning to build in ${BUILDDIR}"
Info "Using conan for dependency package management"
if ($USE_CLANGD) {Info "Set up .clangd for LSP"}

if (Ask-YN "Proceed?" "Y") {
    Print ""
  } else {
      Die "Aborted manually"
    }

# -- BUILD --
Invoke-Conan create .\recipes\gemmi -b="missing"
if ($LASTEXITCODE -ne 0) {Die "conan create gemmi failed"}
Invoke-Conan install . "-of=${BUILDDIR}" -b=missing
if ($LASTEXITCODE -ne 0) {Die "conan install failed"}

meson setup $BUILDDIR "--native-file=${BUILDDIR}\conan_meson_native.ini" "--buildtype=${BUILD_TYPE}" --reconfigure
if ($LASTEXITCODE -ne 0) {Die "meson setup failed"}
meson compile -C $BUILDDIR
if ($LASTEXITCODE -ne 0) {Die "meson compile failed"}

# -- CLANGD --
if ($USE_CLANGD) {
    Section "Setup .clangd"
    $clangdContent = @"
CompileFlags:
  CompilationDatabase: "$BUILDDIR"
"@
    Set-Content -Path ".clangd" -Value $clangdContent -Encoding UTF8
    Ok "Compilation database points at $BUILDDIR"
  }

Section "Done!"
Ok "RoPE succesfully built in $BUILDDIR"
Info "Run RoPE locally by starting"
Print "${BOLD}${YELLOW}$BUILDDIR/rope.gui${RESET} from the terminal"
Info "Install RoPE globally by running"
Print "${BOLD}${YELLOW}meson install -C $BUILDDIR${RESET}"
Print ""

