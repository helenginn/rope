# RoPE (Representation of Protein Entities) Setup Scripts

This directory contains automated build configuration scripts for **RoPE**. They handle system environment checks, dependency verification, and project compilation across different platforms.

---

## What the Scripts Do

* **System & Architecture Discovery:** Detects host OS, CPU architecture, and validates available C++20 compilers (`c++` on Unix; `cl`, `clang-cl`, or `g++` on Windows).
* **Dependency Management:**
  * **macOS/Linux:** Checks for native system libraries via `pkg-config` (`sdl2`, `sdl2_image`, `glew`). Dynamically falls back to **Conan** if local libraries, Meson, or Ninja are missing.
  * **Windows:** Enforces **Conan** as the mandatory package manager to orchestrate all dependencies.
* **Build Configuration:** Interactively prompts for build types (Release, Debug, or DebugOptimized) and gives the option to generate a `.clangd` file for LSP support.
* **Compilation:** Automates environment sourcing, runs `meson setup` with the correct native/conan files, and triggers `meson compile`.
* **Caching:** If `ccache` is detected, it is injected into the compilation pipeline

---

## What the Scripts Do NOT Do (Prerequisites)

The scripts automate configuration but **do not install core system libraries or development toolchains**. Ensure the following are installed manually before running the scripts:

### macOS / Linux Prerequisites

* **System Graphics & UI Libraries (HARD REQUIREMENT):** These **must** be installed via your host package manager (`apt`, `brew`, `pacman`, etc.). Conan will **not** fetch them on macOS/Linux:
  * SDL2 (`libsdl2-dev` / `sdl2`)
  * SDL2_image (`libsdl2-image-dev` / `sdl2_image`)
  * GLEW (`libglew-dev` / `glew`)
  * `pkg-config` (required for dependency detection)
* **Core Toolchain:** A C++ compiler supporting C++20 (GCC, Clang, or Xcode Command Line Tools).
* **Dependency Manager (Optional but Recommended):** `conan` (or `uv`/`pipx`) to automatically fetch secondary dependencies (`gemmi`, `glm`, `fftw`, `libcurl`, `zlib`, `json`, `meson`, `ninja`). If Conan is not available, these must also be installed via your system package manager.

### Windows Prerequisites

* **C++ Toolchain:** A C++ compiler accessible via your environment variable `Path`. If using MSVC (`cl`), you **must run the setup script from a Developer PowerShell** window.
* **Dependency Manager (MANDATORY):** **Conan** must be available (installed natively, or via `uvx`/`pipx`). On Windows, Conan is strictly required and **will** automatically fetch all dependencies, including SDL2, SDL2_image, GLEW, and `pkgconf`.

---

## Usage

### macOS / Linux
```bash
chmod +x setup.sh
./setup.sh [flags]
```

### Windows (PowerShell)
```powershell
.\setup.ps1 [flags]
```

---

## Command-Line Arguments & Flags

| Bash Flag (macOS/Linux) | PowerShell Parameter (Windows) | Description |
| :--- | :--- | :--- |
| `--yesman`, `-y` | `-Yesman`, `-y` | Bypasses all interactive prompts and automatically accepts default choices. |
| `--force-rebuild-dependencies`, `-f` | `-ForceRebuildDependencies`, `-f` | Forces Conan to completely rebuild all cached dependency packages from source. |
| `--build=<path>` | `-Build <path>` | Overrides the default auto-generated build directory path. |
| `--prefix=<path>` | `-Prefix <path>` | Sets a custom installation prefix passed directly to Meson. |
| `--datadir=<path>` | `-Datadir <path>` | Sets a custom data directory path passed directly to Meson. |

---

## Post-Build Execution

Once completed successfully, the application binary (`rope.gui`) will be located inside the generated build folder.

> [!NOTE]
> To use the exact versions of `meson` and `ninja` provided by the Conan toolchain (especially for manual rebuilding), you must source the environment script after setup via:
>
> macOS / Linux
>```bash
>source <your-build-dir>/conanbuild.sh
>```
>
> Windows
>```powershell
>. <your-build-dir>\conanbuild.ps1
>```

* **Run Locally:** Run the executable path shown at the end of the script (e.g., `build/.../rope.gui`).
* **Install Globally:** To install RoPE system-wide, run:
  ```bash
  meson install -C build/<your_build_dir>
  ```

