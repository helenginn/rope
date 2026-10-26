#include <doctest/doctest.h>
#include <vagabond/core/FileManager.h>
#include <vagabond/core/Environment.h>
#include <vagabond/core/GeometryTable.h>
#include <vagabond/core/AtomContent.h>
#include <vagabond/core/files/File.h>
#include <vagabond/core/forces/ForceAnalysis.h>
#include <cstdlib>
#include <memory>
#include <filesystem>

TEST_CASE("REFERENCE GEOMETRY" * doctest::test_suite("force-e2e"))
{
	const char *directory = std::getenv("ROPE_FORCES_FIXTURES");
	REQUIRE(directory != nullptr);
	const std::filesystem::path fixtures{directory};

  const char *dataDirectory = std::getenv("ROPE_DATA_DIR");
  REQUIRE(dataDirectory != nullptr);
  Environment::fileManager()->setDataDirectory(dataDirectory);

  const auto dictionaryPath = fixtures / "MLE.cif";
  const auto pdbPath = fixtures / "6mw1.h.pdb";
  REQUIRE(std::filesystem::is_regular_file(dictionaryPath));
  REQUIRE(std::filesystem::is_regular_file(pdbPath));

  GeometryTable::loadExtraGeometries({dictionaryPath.string()});

  std::unique_ptr<File> file{File::loadUnknown(pdbPath.string())};
  REQUIRE(file != nullptr);

  std::unique_ptr<AtomContent> atoms{file->atoms()};
  REQUIRE(atoms != nullptr);

	ForceAnalysis analysis(atoms.get());
  analysis.convert(false);

	analysis.toggleReason(AbstractForce::ReasonBondLength, false);
	analysis.toggleReason(AbstractForce::ReasonBondTorsion, false);
	analysis.toggleReason(AbstractForce::ReasonVdwContact, false);
	analysis.toggleReason(AbstractForce::ReasonElectrostaticContact, false);
	analysis.toggleReason(AbstractForce::ReasonBondAngle, true);

	auto report = analysis.calculate(false);

  CHECK(report.stress_strain.size() == 42);
  CHECK(report.target_estimate.size() == 516);
  REQUIRE(report.stress_strain_correlation.has_value());
  CHECK(*report.stress_strain_correlation == doctest::Approx(0.60945).epsilon(1e-4));
}
