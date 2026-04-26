#pragma once

#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/vsttypes.h"
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace VstCommon {

/**
 * Generic list-backed ParameterModel.
 *
 * The Definition type must provide:
 *   - kId           : Steinberg::Vst::ParamID
 *   - kTitle        : const Steinberg::Vst::TChar*
 *   - kUnits        : const Steinberg::Vst::TChar*
 *   - kFlags        : Steinberg::int32
 *   - kValues       : std::array<Steinberg::Vst::ParamValue, N>
 *   - kLabels       : std::array<const Steinberg::Vst::TChar*, N>
 *   - kDefaultIndex : std::size_t
 *
 * This model quantizes normalized values to the nearest list entry, which makes
 * sweeping/automation snap between options.
 */
template <typename Definition> struct GenericListModel {
	static constexpr Steinberg::Vst::ParamID	  kId	 = Definition::kId;
	static constexpr const Steinberg::Vst::TChar *kTitle = Definition::kTitle;
	static constexpr const Steinberg::Vst::TChar *kUnits = Definition::kUnits;
	static constexpr Steinberg::int32			  kFlags = Definition::kFlags | Steinberg::Vst::ParameterInfo::kIsList;

	static constexpr auto kValues		= Definition::kValues;
	static constexpr auto kLabels		= Definition::kLabels;
	static constexpr auto kDefaultIndex = Definition::kDefaultIndex;

	static constexpr std::size_t kOptionCount = kValues.size();
	static_assert(kOptionCount > 0, "GenericListModel requires at least one option");
	static_assert(kLabels.size() == kOptionCount, "GenericListModel labels and values must have the same size");
	static_assert(kDefaultIndex < kOptionCount, "GenericListModel default index is out of range");

	static constexpr Steinberg::Vst::ParamValue kMinPlain = [] {
		auto min_value = kValues[0];
		for (std::size_t index = 1; index < kOptionCount; ++index)
			min_value = std::min(min_value, kValues[index]);
		return min_value;
	}();
	static constexpr Steinberg::Vst::ParamValue kMaxPlain = [] {
		auto max_value = kValues[0];
		for (std::size_t index = 1; index < kOptionCount; ++index)
			max_value = std::max(max_value, kValues[index]);
		return max_value;
	}();
	static constexpr Steinberg::Vst::ParamValue kDefaultPlain = kValues[kDefaultIndex];
	static constexpr Steinberg::int32			kStepCount	  = (kOptionCount > 1) ? static_cast<Steinberg::int32>(kOptionCount - 1) : 0;

	static Steinberg::Vst::ParamValue toPlain(Steinberg::Vst::ParamValue in_norm) {
		auto index = toIndex(in_norm);
		return kValues[index];
	}

	static Steinberg::Vst::ParamValue toNormalized(Steinberg::Vst::ParamValue in_plain) {
		auto index = findClosestValueIndex(in_plain);
		return indexToNormalized(index);
	}

	static void toString(Steinberg::Vst::ParamValue in_norm, Steinberg::Vst::String128 out_string) {
		auto index = toIndex(in_norm);
		Steinberg::UString(out_string, USTRINGSIZE(Steinberg::Vst::String128)).assign(kLabels[index]);
	}

	static bool fromString(const Steinberg::Vst::TChar *in_str, Steinberg::Vst::ParamValue &out_norm) {
		if (in_str == nullptr)
			return false;

		for (std::size_t index = 0; index < kOptionCount; ++index) {
			if (equals(in_str, kLabels[index])) {
				out_norm = indexToNormalized(index);
				return true;
			}
		}

		double			   plain = 0.0;
		Steinberg::UString in_string(const_cast<Steinberg::Vst::TChar *>(in_str), USTRINGSIZE(Steinberg::Vst::String128));
		if (!in_string.scanFloat(plain))
			return false;

		out_norm = toNormalized(plain);
		return true;
	}

	static std::size_t toIndex(Steinberg::Vst::ParamValue in_norm) {
		if constexpr (kOptionCount <= 1)
			return 0;

		auto clamped = std::clamp(in_norm, 0.0, 1.0);
		auto scaled	 = clamped * static_cast<Steinberg::Vst::ParamValue>(kStepCount);
		auto index	 = static_cast<std::size_t>(std::lround(scaled));
		if (index >= kOptionCount)
			return kOptionCount - 1;
		return index;
	}

  private:
	static Steinberg::Vst::ParamValue indexToNormalized(std::size_t in_index) {
		if constexpr (kOptionCount <= 1)
			return 0.0;
		return static_cast<Steinberg::Vst::ParamValue>(in_index) / static_cast<Steinberg::Vst::ParamValue>(kStepCount);
	}

	static std::size_t findClosestValueIndex(Steinberg::Vst::ParamValue in_plain) {
		auto best_index = static_cast<std::size_t>(0);
		auto best_delta = std::abs(in_plain - kValues[0]);

		for (std::size_t index = 1; index < kOptionCount; ++index) {
			auto delta = std::abs(in_plain - kValues[index]);
			if (delta < best_delta) {
				best_delta = delta;
				best_index = index;
			}
		}

		return best_index;
	}

	static bool equals(const Steinberg::Vst::TChar *in_lhs, const Steinberg::Vst::TChar *in_rhs) {
		if (in_lhs == nullptr || in_rhs == nullptr)
			return false;

		while (*in_lhs != 0 && *in_rhs != 0) {
			if (*in_lhs != *in_rhs)
				return false;
			++in_lhs;
			++in_rhs;
		}

		return *in_lhs == *in_rhs;
	}
};

} // namespace VstCommon
