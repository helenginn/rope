#include "AgreementMapView.h"
#include "ColourLegend.h"
#include "MatrixPlot.h"
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/TextButton.h>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace
{
	// |z1 * z2| at which the colours saturate
	const double Limit = 2.0;

	const std::string HelpText =
	"Each square is a residue pair (i, j), coloured by z1 x z2:\n"
	"the product of its standardised values in the two matrices.\n"
	"Red: both matrices move it the same way (agreement).\n"
	"Blue: they move it in opposite ways (disagreement).\n"
	"Blocks show regions; full rows or columns show single residues.";
}

AgreementMapView::AgreementMapView(Scene *prev, const ComparisonStats &stats)
: Scene(prev), _stats(stats)
{
	setExportHandler(this);
}

AgreementMapView::~AgreementMapView()
{
	PCA::freeMatrix(&_colours);
}

void AgreementMapView::setup()
{
	addTitle("Agreement map");
	makeMenu();

	int n = _stats.common.size();
	if (n == 0)
	{
		Text *text = new Text("No residues in common");
		text->setCentre(0.5, 0.5);
		addObject(text);
		return;
	}

	// colour scale: 0 = blue, 0.5 = white, 1 = red
	PCA::setupMatrix(&_colours, n, n);
	for (int i = 0; i < n * n; i++)
	{
		double v = _stats.agreement[i];
		v = std::isfinite(v) ? 0.5 + 0.5 * v / Limit : 0.5;
		_colours.vals[i] = std::min(std::max(v, 0.0), 1.0);
	}

	_plot = new MatrixPlot(_colours, _mutex);
	_plot->setCentre(0.45, 0.55);
	addObject(_plot);

	ColourLegend *legend = new ColourLegend(_plot->legend()->scheme(), true, this);
	legend->disableButtons();
	legend->setTitle("z1 x z2");
	legend->setLimits(-Limit, Limit);
	legend->setCentre(0.85, 0.55);
	addObject(legend);
}

void AgreementMapView::makeMenu()
{
	TextButton *text = new TextButton("Menu", this);
	text->setRight(0.95, 0.1);
	addObject(text);

	text->setReturnJob([this, text]()
	{
		Menu *m = new Menu(this);
		m->addOption("Save as CSV", [this]()
		{
			ExportsCSV::buttonPressed("export");
		});
		m->addOption("Help", [this]()
		{
			setModal(new BadChoice(this, HelpText));
		});
		m->setup(text);
		setModal(m);
	});
}

void AgreementMapView::showPairAt(double x, double y)
{
	double tx = x; double ty = y;
	convertToGLCoords(&tx, &ty);

	glm::vec3 v = glm::vec3(tx, ty, 0);
	glm::vec3 min, max;
	_plot->boundaries(&min, &max);
	v -= min;
	v /= (max - min);

	if (v.x < 0 || v.x >= 1 || v.y < 0 || v.y >= 1)
	{
		return;
	}

	int n = _stats.common.size();
	int i = v.x * n;
	int j = v.y * n;
	double value = _stats.agreement[i * n + j];

	std::ostringstream ss;
	ss << "Residues " << _stats.common[i] << " and " << _stats.common[j] << ": ";
	if (std::isfinite(value))
	{
		ss << "z1 x z2 = " << std::fixed << std::setprecision(2) << value;
	}
	else
	{
		ss << "no pair";
	}
	setInformation(ss.str());
}

void AgreementMapView::mousePressEvent(double x, double y,
                                      SDL_MouseButtonEvent button)
{
	if (_plot)
	{
		showPairAt(x, y);
	}

	Scene::mousePressEvent(x, y, button);
}

void AgreementMapView::supplyCSV(std::string indicator)
{
	// same layout as the distance matrix files: residue labels on both sides
	std::ostringstream ss;
	ss << "residue";
	for (const ResidueId &id : _stats.common)
	{
		ss << "," << id;
	}
	ss << "\n";

	size_t n = _stats.common.size();
	for (size_t i = 0; i < n; i++)
	{
		ss << _stats.common[i];
		for (size_t j = 0; j < n; j++)
		{
			ss << ",";
			double value = _stats.agreement[i * n + j];
			if (std::isfinite(value))
			{
				ss << value;
			}
		}
		ss << "\n";
	}
	_csv = ss.str();
}

void AgreementMapView::buttonPressed(std::string tag, Button *button)
{
	if (tag == "export_csv")
	{
		ExportsCSV::buttonPressed(tag, button);
		return;
	}

	Scene::buttonPressed(tag, button);
}