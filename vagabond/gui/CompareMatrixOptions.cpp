#include "CompareMatrixOptions.h"
#include <vagabond/gui/elements/Menu.h>
#include <vagabond/gui/elements/Scene.h>
#include <vagabond/gui/elements/BadChoice.h>
#include <vagabond/gui/elements/Window.h>
#include <vagabond/gui/ComparisonView.h>
#include <vagabond/core/CompareMatrices.h>
#include <vagabond/core/ComparisonResults.h>
#include <atomic>
#include <thread>
#include <iostream>

namespace
{
	// only one comparison at the time
	std::atomic<bool> comparisonRunning{false};
	void showMessage(const std::string &text)
	{
		Window::window()->addMainThreadJob([text]()
		{
			Scene *scene = Window::currentScene();
			if (scene)
			{
				scene->setModal(new BadChoice(scene, text));
			}
		});
	}

	void finishOnMainThread(const ComparisonStats &stats)
	{
		Window::window()->addMainThreadJob([stats]()
		{
			std::string message = "comparison done!";

			if (stats.pairs.size() < 3)
			{
				message = "Comparison done, but the matrices have fewer than "
							"3 residues pairs in common";
			}

			ComparisonResults::instance().set(stats);
			Scene *scene = Window::currentScene();
			if (scene)
			{
				scene->setModal(new BadChoice(scene, message));
			}
		});
	}

	void startComparison()
	{
		if (comparisonRunning.exchange(true))
		{
			std::cout << "[compare] a comparison is already running"
				      << std::endl;
			return;
		}

		MatrixCache &cache = MatrixCache::instance();
		SlotMatrix m1 = cache.get(MatrixCache::Matrix1);
		SlotMatrix m2 = cache.get(MatrixCache::Matrix2);

		std::thread worker([m1, m2]()
		{
			CompareMatrices comparison(m1, m2);
			comparison.run();
			const ComparisonStats &stats = comparison.stats();
			comparisonRunning = false; 

			std::cout << "[compare] " << stats.common.size() << " residues,"
					  << stats.pairs.size() << " pairs, r = " << stats.pearson 
					  << " , rho = " << stats.spearman << std::endl;

			finishOnMainThread(stats);
		});
		worker.detach();
	}
}


void addCompareOptions(Menu *menu, Scene *scene, 
		               const std::function<SlotMatrix()> &getMatrix)
{
	MatrixCache &slots = MatrixCache::instance();

	std::string label = "Compare this matrix"; // shows which slot this matrix would go into 
	if (!slots.isFull(MatrixCache::Matrix1))
	{
		label += " (matrix1)";
	}
	else if (!slots.isFull(MatrixCache::Matrix2))
	{
		label += " (matrix2)";
	}

	auto compare = [scene, getMatrix]()
	{
		MatrixCache &slots = MatrixCache::instance();
		if (slots.bothFull())
		{
			scene->setModal(new BadChoice(scene, "Two matrices already chosen. "
											"Clear one first."));
			return;
		}


		SlotMatrix matrix = getMatrix();
		int slot = slots.fillNext(matrix);
		if (slot < 0)
		{
			scene->setModal(new BadChoice(scene, "Could not keep this matrix: "
											"it is empty or does not match its residues."));
		}

		std::string name = (slot == MatrixCache::Matrix1) ? "matrix1" : "matrix2"; 
		scene->setInformation("Kept as " + name + ": " + matrix.label);
		std::cout << "[compare] " << name << " = " << matrix.label << " ("
		          << matrix.ids.size() << " residues)" << std::endl;
		          
		if (slots.bothFull())
		{
			std::cout << "[compare] both matrices chosen" << std::endl;
			startComparison();
		}

	};

	menu->addOption(label, compare);
	if (slots.isFull(MatrixCache::Matrix1))
	{
		menu->addOption("Clear matrix1", [scene]()
		{
			MatrixCache::instance().clear(MatrixCache::Matrix1);
			scene->setInformation("matrix1 cleared");
		});
	}

	if (slots.isFull(MatrixCache::Matrix1))
	{
		menu->addOption("Clear matrix2", [scene]()
		{
			MatrixCache::instance().clear(MatrixCache::Matrix2);
			scene->setInformation("matrix2 cleared");
		});
	}

	if (ComparisonResults::instance().has())
	{
		menu->addOption("Explore comparison", [scene]()
		{
			ComparisonView *view = new ComparisonView(scene, ComparisonResults::instance().get());
			view->show();
		});
	}
}




