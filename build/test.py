import os
import sys

# Get the absolute path to the directory containing module.so
module_dir = os.path.abspath('/Users/vapostolop/Desktop/pyRoPE/python_src')

# Add the module directory to sys.path
sys.path.append(module_dir)

# Now you can import functions from module.so
from module import greet, get_atom_position

kk = greet()
print(kk)

position = get_atom_position()
print("Atom Position:", position)
