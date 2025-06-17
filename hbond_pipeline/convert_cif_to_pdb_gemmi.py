import sys
import os
import gemmi

def convert_cif_to_pdb(cif_path):
    if not cif_path.endswith('.cif'):
        print("Error: Input file must have a .cif extension.")
        return


    pdb_path = os.path.splitext(cif_path)[0] + '.pdb'

    # Read CIF and convert
    doc = gemmi.cif.read_file(cif_path)
    structure = gemmi.make_structure_from_block(doc.sole_block())
    structure.write_pdb(pdb_path)

    print(f"Converted: {cif_path} -> {pdb_path}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python convert_cif_to_pdb_gemmi.py your_file.cif")
    else:
        convert_cif_to_pdb(sys.argv[1])
