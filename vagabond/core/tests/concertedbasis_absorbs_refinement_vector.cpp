#include "../ConcertedBasis.h"
#include "../BondSequence.h"
#include "../BondTorsion.h"
#include "../Atom.h"
#include "../AtomGroup.h"
#include "../CifFile.h"

void fill_mask(std::vector<bool> &mask, int count)
{
	for (size_t i = mask.size(); i < count; i++)
	{
		mask.push_back(false);
	}
}

bool tweak_torsion(TorsionBasis *b, int num)
{
	std::cout << "Original torsion: " << b->torsion(num)->startingAngle() << std::endl;
	float vec[] = {0.5, -0.5, 1.0};
	float mod = b->torsionForVector(num, vec, 3);
	std::cout << "Modified torsion: " << mod << std::endl;
	
	std::cout << "Absorbing vector..." << std::endl;
	b->absorbVector(vec, 3);
	float absorbed = b->torsion(num)->startingAngle();
	std::cout << "Absorbed torsion: " << absorbed << std::endl;
	
	return (fabs(mod - absorbed) > 1e-6);
}

int main()
{
	std::string path = "/assets/geometry/ATP.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup &glycine = *geom.atoms();
	BondSequence *sequence = new BondSequence();
	sequence->setTorsionBasisType(TorsionBasis::TypeConcerted);
	sequence->addToGraph(glycine.possibleAnchor(0), UINT_MAX);

	TorsionBasis *b = sequence->torsionBasis();
	int count = b->torsionCount();
	
	std::cout << "Torsion count: " << count << std::endl;
	
	{
		std::vector<bool> mask = {false, false, true, true, true};
		fill_mask(mask, count);

		b->supplyMask(mask);
		b->prepare();

		if (tweak_torsion(b, 3))
		{
			std::cout << "Does not match!" << std::endl;
			return 1;
		}
	}

	return 0;
}
