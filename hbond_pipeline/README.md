# Preparing Hydrogen Bond Files for ROPE

This guide explains how to use the `hbondpipe` script to prepare hydrogen bond (`.hbond`) files that are compatible with [ROPE](https://github.com/helenginn/rope/tree/virginia), a torsion-angle-based protein modeling toolkit.

## 📁 Directory Structure

Current working directory:
ROPE/hbond_pipeline

The main script is:
hbondpipe

You can run it directly from this directory or make it accessible system-wide by following the steps below.

---

## ⚙️ Dependencies

Required tools and libraries:

- **Python 3** (with `conda` environment manager)
- **Phenix** (must be installable via terminal; script sources `phenix_env.sh`)
- **PyMOL** (https://anaconda.org/conda-forge/pymol-open-source, used via `pymol_env`)
- **Conda** (must be installed and available in your shell)
- **Python libraries**: Listed in `dependencies.yml` in this directory

Ensure you can activate the `pymol_env` and source the appropriate `phenix_env.sh` file in your shell.

---

## 🐍 Python Scripts Used

`hbondpipe` internally calls the following Python scripts:

- `prepare_hbonds_rope.py`: Extracts and formats hydrogen bond information in the format required by ROPE.
- `convert_cid_to_pdb_gemmi.py`: Converts compound IDs or structure references into full `.pdb` files using the [Gemmi](https://project-gemmi.github.io/) library.

---

## 📥 Input / 📤 Output

**Input:**  
- Protein structure file in `.pdb` or `.mmCIF` format

**Output:**  
- A `[prefix of input file]_hbond.txt` file that can be read by ROPE

---

## 🚀 Usage

After setup (see below), you can run:

hbondpipe my_structure.pdb 

The script will activate the required Conda environment, source Phenix, and generate a .hbond file.

## 🌐 Global Installation

1. Move the script to your local binary folder:

mkdir -p ~/.local/bin
cp hbondpipe ~/.local/bin/

2. Edit your shell configuration file (e.g. ~/.bashrc, ~/.zshrc) and add:

export PATH="$HOME/.local/bin:$PATH"

3. Reload your shell configuration:
source ~/.bashrc  # or source ~/.zshrc

🧪 Configuration Notes

Make sure that:
* phenix_env.sh is located and sourced correctly by hbondpipe
* The Python scripts (prepare_hbonds_rope.py and convert_cid_to_pdb_gemmi.py) are either:
		* in the same directory as hbondpipe, or
		* available in your PYTHONPATH
* You may need to adjust paths depending on your setup.


💻 Operating System Compatibility

This setup has been tested on macOS.
If you're using Linux or Windows, slight modifications may be needed (e.g. different shell syntax or environment file paths).










