#include "GeoVolume.h"
#include "utils.h"
#include "GeoArray.h"
#include <algorithm>
#include <tuple>

std::vector<float> GetNormalizedVolumeData( const GeoVolume<float>& gv )
{
	typedef float T;
	std::vector<T> ret(gv.volData_);
	std::vector<T>::const_iterator minIt, maxIt;
	std::tie(minIt, maxIt) = std::minmax_element(ret.begin(), ret.end());

	const T minVal = *minIt;
	const T maxVal = *maxIt;
	std::for_each(ret.begin(), ret.end(), 
		[=](T& val){
			val = static_cast<T>(getRatio(val, minVal, maxVal));
	});
	return ret;
}

std::vector<float> GetNormalizedPerLevelVolumeData( const GeoVolume<float>& gv )
{
	typedef float T;
	std::vector<T> ret(gv.volData_);
	std::vector<T>::const_iterator minIt, maxIt;
	auto itBeg = ret.begin();
	auto itEnd = itBeg + gv.longitudeNum_*gv.latitudeNum_;

	while(itBeg < ret.end())
	{
		std::tie(minIt, maxIt) = std::minmax_element(itBeg, itEnd);

		const T minVal = *minIt;
		const T maxVal = *maxIt;
		std::for_each(itBeg, itEnd, 
			[=](T& val){
				val = static_cast<T>(getRatio(val, minVal, maxVal));
		});

		itBeg = itEnd;
		itEnd = itBeg + gv.longitudeNum_*gv.latitudeNum_;
	}
	return ret;
}