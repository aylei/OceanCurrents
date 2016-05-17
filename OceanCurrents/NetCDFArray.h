/**
 * @file NetCDFArray.h
 *
 * @brief the NetCDF data struct
 *
 * @author Leonard
 * @date 2014-12-07
 */
#ifndef  METEOROLOGYATAWAPPER_NETCDFARRAY_H
#define  METEOROLOGYATAWAPPER_NETCDFARRAY_H
#include <vector>
#include <netcdf.h>
#include "GeoArray.h"
#include "GeoVolume.h"

#define SOUTH_SEA

typedef float data_t;
typedef double levels_t;

class NetCDFArray : public GeoArray<float>
{
public:
	NetCDFArray(std::string path);
	virtual ~NetCDFArray();

	const std::vector<std::string> getVariableList();
	inline void resetVariableList()
	{
		variables_list_.clear();
		variables_type_list_.clear();
		variables_dimensions_count_list_.clear();
	}

	bool readFromFile(const std::string& variable_name, size_t height_level = 0);
	bool getGeoArrayData(GeoArray<float>& geoArray, std::string variable_name, size_t ticks = 1, size_t level = 0, int sparse_rate = 0);
	bool getGeoVolumeData(GeoVolume<float>& geoArray, std::string type_str, size_t ticks = 1, size_t levels_count = 1, int sparse_rate = 0);

	std::vector<levels_t> getLevelsList(int count = 0);
	size_t getLevelIndex(std::string level);
	size_t getTickIndex(std::string date) const;

public:
	std::string date_start_;
	std::string date_end_;
	float date_interval_;
	size_t date_num_;
	double height_start_;
	double height_end_;
	double height_interval_;
	size_t height_num_;
	float firstVal_;
	float lastVal_;

private:
	int id_netcdf_; //the id of the netcdf dataset
	int count_dimensions_;
	int count_variables_;
	int count_attributes_;
	int count_unlimited_dimensions_;

	std::vector<std::string> variables_list_;
	std::vector<nc_type> variables_type_list_;
	std::vector<int> variables_dimensions_count_list_;
};

#endif