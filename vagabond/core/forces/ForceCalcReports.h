#pragma once

#include "vagabond/core/TabulatedData.h"
#include <string>
#include <vector>

namespace rope::force_calc
{

	template <typename Schema> struct ReportTable
	{
		TabulatedData data;

		static ReportTable create()
		{
			return {TabulatedData(Schema::headers())};
		}

		void add(const typename Schema::Row &row)
		{
			data.addEntry(Schema::entries(row));
		}
	};

	struct StressStrainSchema
	{
		struct Row
		{
			std::string desc;
			float stress;
			float strain;
		};
		static std::vector<TabulatedData::HeaderTypePair> headers()
		{
			return {{"desc", TabulatedData::Text},
			        {"stress", TabulatedData::Number},
			        {"strain", TabulatedData::Number}};
		}
		static std::vector<TabulatedData::StringPair> entries(const Row &row)
		{
			return {{"desc", row.desc},
			        {"stress", std::to_string(row.stress)},
			        {"strain", std::to_string(row.strain)}};
		}
	};

	struct TargetEstimateSchema
	{
		struct Row
		{
			float target;
			float estimate;
		};

		static std::vector<TabulatedData::HeaderTypePair> headers()
		{
			return {{"target", TabulatedData::Number},
			        {"estimate", TabulatedData::Number}};
		}

		static std::vector<TabulatedData::StringPair> entries(const Row &row)
		{
			return {{"target", std::to_string(row.target)},
			        {"estimate", std::to_string(row.estimate)}};
		}
	};

	struct TorsionElectromagneticSchema
	{
		struct Row
		{
			float normalized_dot_product;
			std::string desc;
		};

		static std::vector<TabulatedData::HeaderTypePair> headers()
		{
			return {{"normalized_dot_product", TabulatedData::Number},
			        {"desc", TabulatedData::Text}};
		}

		static std::vector<TabulatedData::StringPair> entries(const Row &row)
		{
			return {{"normalized_dot_product",
			         std::to_string(row.normalized_dot_product)},
			        {"desc", row.desc}};
		}
	};

	using TableStressStrain = ReportTable<StressStrainSchema>;
	using TableTargetEstimate = ReportTable<TargetEstimateSchema>;
	using TableTorsionElectromagnetic =
	    ReportTable<TorsionElectromagneticSchema>;

	struct ForceCalcResultReport
	{
		float stress_strain_correlation;

		TableStressStrain stress_strain;
		TableTargetEstimate target_estimate;
		TableTorsionElectromagnetic torsion_electromagnetic;

		static ForceCalcResultReport create()
		{
			return ForceCalcResultReport{0.f, TableStressStrain::create(),
			                             TableTargetEstimate::create(),
			                             TableTorsionElectromagnetic::create()};
		}
	};
} // namespace rope::force_calc
