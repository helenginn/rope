import pymol
import pandas as pd
import argparse
import shlex
import re

parser = argparse.ArgumentParser()
parser.add_argument("-hb", "--hbonds", help = "path to hbonds.txt data")
parser.add_argument("-p", "--pdb", help = "path to PDB data")
parser.add_argument("-o", "--output_pdb", help = "path to output_PDB data")
parser.add_argument("--openpymol", choices= ["N", "Y"], default="N", help= "choose 'N' to open Pymol in silent mode and choose 'Y' to open Pymol in interactive mode (default:N)")

args = parser.parse_args()

if args.openpymol == "N":
    pymol.finish_launching(['pymol', '-cq'])
elif args.openpymol == "Y":
    pymol.finish_launching(['pymol', '-q'])

data = []

with open(args.hbonds, "r", encoding="utf-8") as f:
    for line in f:
        line = line.replace('"', '')
        line = re.sub(r'(<>)', r'\1 ', line)
        if "symop" in line:
            category = re.split(r'\s+', line.strip())
            if len(category) < 12:
                print(f"Fail: Not enough columns -> {category}")
                continue
        
            h_rang = category[0]
            acc_rang = category[1]
            h_atom = category[2]
            Donor_residue = category[3]
            chain = category[4]
            Dresidue_number = category[5]
            Acceptor = category[7]
            Acceptor_residue = category[8]
            chain2 = category[9]
            Aresidue_number = category[10]

            # Pull key=value parameters safely from the line
            param_string = " ".join(category)
            matches = dict(re.findall(r'(\w+)=([\d\.\-]+)', param_string))

            try:
                d_HA = matches["d_HA"]
                d_DA = matches["d_AD"]
                d_DHA = matches["a_DHA"]
            except KeyError as e:
                print(f"Skipping line due to missing field {e}: {line.strip()}")
                continue

            # save Data 
            data.append([h_rang, acc_rang, h_atom, Donor_residue, chain, Dresidue_number, Acceptor, Acceptor_residue,
                             chain2, Aresidue_number, d_HA, d_DA, d_DHA])   
            

# create Data Frame
df = pd.DataFrame(data, columns=['h_Rang', 'Acc-Rang', 'H_Atom', 'Donor residue', 'Chain1', 'Donor residue Number', 
                                 'Acceptor_Atom', 'Acceptor_Residue', 'Chain2', 
                                 'Acceptor residue number', 'Distance_HA', 'Distance_DA', 'Angle_DHA'])

# makes sure, that the first column is a number ( h_rang should contain a number)
df = df[df.iloc[:, 0].apply(lambda x: str(x).isdigit())]
print(df.tail(10))
print(len(df))
print(f"Amount H-Bonds: {len(df)}")

pymol.cmd.load(args.pdb)

hbond_data = []
for i in range(len(df)):
   
    HAtom = df.iloc[i]['H_Atom']
    Donor_Residue = df.iloc[i]['Donor residue']
    ChainAB1 = df.iloc[i]['Chain1']
    Donor_residue_number = df.iloc[i]['Donor residue Number']
    AcceptorAtom = df.iloc[i]['Acceptor_Atom']
    AcceptorResidue = df.iloc[i]['Acceptor_Residue']
    ChainAB2 = df.iloc[i]['Chain2']
    Acceptor_residue_number = df.iloc[i]['Acceptor residue number']
    print(Donor_Residue)
    DistanceDA = df.iloc[i]['Distance_DA']
    AngleDHA = df.iloc[i]['Angle_DHA']


    selection_name = f"H_{i}" #all Hydogen atoms
    selection_acceptor = f"Acceptor_{i}"
    distance_DA = f"distance_{i}"

    pymol.cmd.select(selection_name, f"resi {Donor_residue_number} and chain {ChainAB1} and name {HAtom}")
    pymol.cmd.select(f"{selection_name}_around", f"{selection_name} around 1.2")
    pymol.cmd.select(selection_acceptor, f"resi {Acceptor_residue_number} and chain {ChainAB2} and name {AcceptorAtom}")
    selection_donor = f"donor_{i}"
    pymol.cmd.select(selection_donor, f"{selection_name}_around and not hydro")
   

    length = pymol.cmd.distance(distance_DA, f"donor_{i}", selection_acceptor)
    model = pymol.cmd.get_model(selection_donor)
    Donor_Atom = None
    for atom in model.atom:
        Donor_Atom = atom.name
        break
    
    if Donor_Atom is None:
        Donor_Atom = None


    pymol.cmd.show("lines", f"{selection_name}_around")
    pymol.cmd.color("red", selection_name)
    pymol.cmd.set("dash_color", "blue", distance_DA)
    pymol.cmd.set("dash_gap", 0.3)
    pymol.cmd.set("dash_length", 0.2)
    pymol.cmd.show("dashes", distance_DA)

   
    donor_atom_ID = pymol.cmd.identify(f"resi {Donor_residue_number} and chain {ChainAB1} and name {Donor_Atom}", mode=0)
    acceptor_atom_ID = pymol.cmd.identify(f"resi {Acceptor_residue_number} and chain {ChainAB2} and name {AcceptorAtom}", mode=0)


    donor_atom_ID = int(donor_atom_ID[0]) if donor_atom_ID else " "
    acceptor_atom_ID = int(acceptor_atom_ID[0]) if acceptor_atom_ID else " "
    hbond_id = i + 1

    hbond_data.append([hbond_id, ChainAB1, Donor_residue_number, Donor_Residue, Donor_Atom, donor_atom_ID, HAtom,
                       ChainAB2, Acceptor_residue_number, AcceptorResidue, AcceptorAtom, acceptor_atom_ID,
                       round(length, 3), AngleDHA])

df_pymol = pd.DataFrame(hbond_data, columns=[
    "H-bond_ID", "H-chain", "H-resi", "H-resn", "Donor-Atom", "Donor-ID", "H-atomn",
    "Acc-chain", "Acc-resi", "Acc-resn", "Acc-atomn", "Acc-ID",
    "length", "angle_D_H_A"])   

#     hbond_data.append([ChainAB1, Donor_residue_number, Donor_Residue, Donor_Atom, donor_atom_ID, HAtom,
#                        ChainAB2, Acceptor_residue_number, AcceptorResidue, AcceptorAtom, acceptor_atom_ID,
#                        round(length, 3), AngleDHA])
    
# df_pymol = pd.DataFrame(hbond_data, columns=[
#     "H-chain", "H-resi", "H-resn", "Donor-Atom", "Donor-ID", "H-atomn",
#     "Acc-chain", "Acc-resi", "Acc-resn", "Acc-atomn", "Acc-ID",
#     "length", "angle_D_H_A"]) 

print(df_pymol.head())
df_pymol.to_csv(args.output_pdb, sep="\t", index=False, encoding='utf-8')

print(f"Hydrogen bond data saved to {args.output_pdb}")

pymol.cmd.zoom()
pymol.cmd.refresh()
