#ifndef GEO_VOLUME_H
#define GEO_VOLUME_H

#include <vector>
#include <string>

template<typename T>
struct GeoVolume
{
	double longitudeStart_;

	double longitudeStep_;

	int longitudeNum_;

	double latitudeStart_;

	double latitudeStep_;

	int latitudeNum_;

	std::vector<double> heightOfLevels_;

	std::vector<T> volData_;

	std::string timeStr_;
};

template<typename T>
bool isSameGeoInfo(const GeoVolume<T>& gv0, const GeoVolume<T>& gv1)
{
	bool ret = gv0.longitudeStart_ == gv1.longitudeStart_
		&& gv0.longitudeStep_ == gv1.longitudeStep_
		&& gv0.longitudeNum_ == gv1.longitudeNum_
		&& gv0.latitudeStart_ == gv1.latitudeStart_
		&& gv0.latitudeStep_ == gv1.latitudeStep_
		&& gv0.latitudeNum_ == gv1.latitudeNum_
		&& gv0.heightOfLevels_ == gv1.heightOfLevels_;
	return ret;
}

template<typename T>
void rotateLat(GeoVolume<T>& gv)
{
	const auto lonNum = gv.longitudeNum_;
	const auto latNum = gv.latitudeNum_;
	const auto hNum = gv.heightOfLevels_.size();
	gv.latitudeStep_ = -gv.latitudeStep_;
	for(int h=0; h<hNum; ++h)
	{	
		T* pos0 = gv.volData_.data() + h*lonNum*latNum;
		for(int lat=0; lat<latNum / 2; ++lat)
		{
			T* p0 = pos0 + lonNum * lat;
			T* p1 = pos0 + lonNum * (latNum - lat - 1);
			std::vector<T> vec(p0, p0 + lonNum);
			std::copy(p1, p1+lonNum, p0);
			std::copy(vec.begin(), vec.end(), p1);
		}
	}
}

std::vector<float> GetNormalizedVolumeData(const GeoVolume<float>& gv);

std::vector<float> GetNormalizedPerLevelVolumeData(const GeoVolume<float>& gv);

#endif
