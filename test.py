import os
import sys

# Get the absolute path to the directory containing module.so
module_dir = os.path.abspath('/Users/vapostolop/Desktop/pyRoPE/build')

# Add the module directory to sys.path
sys.path.append(module_dir)

# Now you can import functions from module.so
from module import greet, get_one_atom_position, get_atom_positions, fish_positions

def main():
	kk = greet()
	print(kk)

	positions = get_atom_positions()
	print("Atom Position:", positions)

	position = get_one_atom_position()
	print("One atom position:", position)

	withPos = fish_positions()
	if withPos:
		samples = withPos['samples']
		ave = withPos['ave']
		target = withPos['target']
		colour = withPos['colour']

		print("WithPos Samples:", samples)
		print("WithPos Ave:", ave)
		print("WithPos Target:", target)
		print("WithPos Colour:", colour)


if __name__ == "__main__":
    main()