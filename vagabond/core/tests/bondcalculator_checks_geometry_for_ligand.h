int checkGeometry(std::string path, double angle_tol = 1e-3)
{
	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->possibleAnchor(0);
	std::cout << "Anchor: " << anchor->atomName() << std::endl;
	
	BondCalculator calculator;
	calculator.setPipelineType(BondCalculator::PipelineAtomPositions);
	calculator.setMaxSimultaneousThreads(1);
	calculator.addAnchorExtension(anchor);
	calculator.setup();

	calculator.start();
	
	Job job{};
	job.requests = JobExtractPositions;
	calculator.submitJob(job);

	Result *result = nullptr;
	result = calculator.acquireResult();
	result->transplantPositions();
	result->destroy();

	calculator.finish();
	
	for (size_t i = 0; i < atoms->bondLengthCount(); i++)
	{
		BondLength *constraint = atoms->bondLength(i);
		float expected = constraint->length();
		float observed = constraint->measurement();

		if (fabs(expected - observed) > 1e-3)
		{
			std::cout << "Bond length " << constraint->desc() << std::endl;
			std::cout << "Lengths do not match: " << expected << " vs "
			<< observed << " Angstroms." << std::endl;
			return 1;
		}
	}
	
	for (size_t i = 0; i < atoms->bondAngleCount(); i++)
	{
		BondAngle *constraint = atoms->bondAngle(i);
		float expected = constraint->angle();
		float observed = constraint->measurement();

		if (fabs(expected - observed) > angle_tol)
		{
			std::cout << "Bond angle " << constraint->desc() << std::endl;
			std::cout << "Angles do not match: expected " << expected << " vs "
			"observed " << observed << std::endl;
			return 1;
		}
	}

	delete atoms;
	
	return 0;
}


