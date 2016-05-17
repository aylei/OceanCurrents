#include <iostream>
#include <algorithm>
#include "NetCDFArray.h"

NetCDFArray::NetCDFArray(std::string path) 
	: NetCDFArray::GeoArray<float>(), height_num_(0)
{
	file_full_path_ = path;

	int result = nc_open(path.c_str(), NC_NOWRITE, &id_netcdf_);
	if(result != NC_NOERR)
	{
		status_ = ARRAY_STATUS_FILE_NOT_FOUND;
		std::cout << "[GEOARRAY] netcdf format read error during opening the file: " << nc_strerror(result) << std::endl;
		return;
	}

	char name[NC_MAX_NAME];
	nc_type type;
	size_t length;

	result = nc_inq(id_netcdf_, &count_dimensions_, &count_variables_, &count_attributes_, &count_unlimited_dimensions_);
	if(result != NC_NOERR)
	{
		status_ = ARRAY_STATUS_DIMENSION_NOT_FOUND;
		std::cout << "[GEOARRAY] netcdf format read error during querying the dimensions: " << nc_strerror(result) << std::endl;
		return;
	}
	
	//query the dimensions
	for(int i = 0; i < count_dimensions_; ++i)
	{
		result = nc_inq_dim(id_netcdf_, i, name, &length);
		if(result != NC_NOERR)
		{
			status_ = ARRAY_STATUS_DIMENSION_NOT_FOUND;
			std::cout << "[GEOARRAY] netcdf format read error during querying the dimension's name: " << nc_strerror(result) << std::endl;
			return;
		}
#ifdef SOUTH_SEA
		if(strcmp(name, "y") == 0) 
#else
		if(strcmp(name, "lat") == 0) 
#endif
		{
			latitude_num_ = length;
			//latitude_num_ = latitude_end_ - latitude_start_;
			//latitude_interval_ = latitude_num_ / length;
		}
#ifdef SOUTH_SEA
		else if(strcmp(name, "x") == 0)
#else
		else if(strcmp(name, "lon") == 0)
#endif
		{
			longitude_num_ = length;
			//longitude_num_ = longitude_end_ - longitude_start_;
			//longitude_interval_ = longitude_num_ / length;
		}
#ifdef SOUTH_SEA
		else if(strcmp(name, "z") == 0)
#else
		else if(strcmp(name, "lev1") == 0 || strcmp(name, "lev") == 0) 
#endif
		{
			height_num_ = length;
			//height_num_ = height_end_ - height_start_;
			//height_interval_= height_num_ / length;
		}
#ifdef SOUTH_SEA
		else if(strcmp(name, "t") == 0)
#else
		else if(strcmp(name, "time") == 0) 
#endif
		{
			//date_interval_ = 120;
			date_num_ = length;
		}
	}
		
	getVariableList();
	for(std::vector<std::string>::iterator it = variables_list_.begin(); it != variables_list_.end(); ++it)
	{
		if(it->compare("lat") == 0)
		{
			bool res = readFromFile(*it);
			latitude_start_ = firstVal_;
			latitude_end_ = lastVal_;
			latitude_interval_ = (lastVal_ - firstVal_) / latitude_num_;
		}
		else if(it->compare("lon") == 0)
		{
			bool res = readFromFile(*it);
			longitude_start_ = firstVal_;
			longitude_end_ = lastVal_;
			longitude_interval_ = (lastVal_ - firstVal_) / longitude_num_;
		}
		else if(it->compare("lev1") == 0 || it->compare("lev") == 0)
		{
			bool res = readFromFile(*it);
			height_start_ = firstVal_;
			height_end_ = lastVal_;
			height_interval_ = (lastVal_ - firstVal_) / height_num_;
		}
		else if(it->compare("time") == 0)
		{
			bool res = readFromFile(*it);
			date_start_ = firstVal_;
			date_end_ = lastVal_;
			date_interval_ = (lastVal_ - firstVal_) / date_num_;
		}
	}

#ifdef SOUTH_SEA
	//height_start_ = 0;
	//height_interval_ = 1;
	//height_end_ = height_start_ + height_interval_ * (23 -1);
#endif

	status_ = ARRAY_STATUS_SUCCEED;
}

NetCDFArray::~NetCDFArray()
{
	int result = nc_close(id_netcdf_);
	if(result != NC_NOERR)
	{
		status_ = GeoArray::ARRAY_STATUS_CLOSE_FILE_FAILED;
		std::cout << "[GEOARRAY] close netcdf file failed due to: " << nc_strerror(result) << std::endl;
		return;
	}
	status_ = ARRAY_STATUS_SUCCEED;
	std::cout << "close return value = " << result << std::endl << "nc id = " << id_netcdf_ << std::endl;
}

const std::vector<std::string> NetCDFArray::getVariableList()
{
	if(variables_list_.empty())
	{
		int result = nc_inq_nvars(id_netcdf_, &count_variables_);
		if(result != NC_NOERR)
		{
			status_ = ARRAY_STATUS_VARIABLE_NOT_FOUND;
			std::cout << "[GEOARRAY] read netcdf file variable count failed due to: " << nc_strerror(result) << std::endl;
			return variables_list_;
		}
		
		char name[NC_MAX_NAME];
		nc_type type;
		int count_dims;
		int count_attr;
		int ids_dims[NC_MAX_DIMS]; 
		for(int i = 0; i < count_variables_; ++i)
		{
			result = nc_inq_var(id_netcdf_, i, name, &type, &count_dims, ids_dims, &count_attr); 
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_VARIABLE_NOT_FOUND;
				std::cout << "[GEOARRAY] read netcdf file variable description failed due to: " << nc_strerror(result) << std::endl;
				return variables_list_;
			}
			else
			{
				variables_list_.push_back(name);
				variables_type_list_.push_back(type);
				variables_dimensions_count_list_.push_back(count_dims);
			}
		}
	}
	return variables_list_;
}

bool NetCDFArray::getGeoArrayData(GeoArray<float>& geoArray, std::string variable_name, size_t ticks, size_t level, int sparse_rate)
{
	auto updateGaInfo = [&](GeoArray<float>& ga){
		ga.longitude_start_ = longitude_start_;
		ga.latitude_start_ = latitude_start_;
		if(sparse_rate > 0)
		{
			ga.longitude_interval_ = longitude_interval_ * sparse_rate;
			ga.longitude_num_ = (longitude_num_ - 1) / sparse_rate + 1;
			ga.longitude_end_ = ga.longitude_start_ + ga.longitude_interval_ * sparse_rate * (ga.longitude_num_ - 1);
			ga.latitude_interval_ = latitude_interval_ * sparse_rate;
			ga.latitude_num_ = (latitude_num_ - 1) / sparse_rate + 1;
			ga.latitude_end_ = ga.latitude_start_ + ga.latitude_interval_ * sparse_rate * (ga.latitude_num_ - 1);
		}
		else
		{
			ga.longitude_interval_ = longitude_interval_;
			ga.longitude_end_ = longitude_end_;
			ga.longitude_num_ = longitude_num_;
			ga.latitude_interval_ = latitude_interval_;
			ga.latitude_end_ = latitude_end_;
			ga.latitude_num_ = latitude_num_;
		}
	};

	delete[] array_p_;
	
	size_t size = 0;
	if(variable_name.compare("lat") == 0)
		size = latitude_num_;
	else if(variable_name.compare("lon") == 0)
		size = longitude_num_;
	else if(variable_name.compare("lev1") == 0 || variable_name.compare("lev") == 0)
		size = height_num_;
	else if(variable_name.compare("time") == 0)
		size = date_num_;
	else
		size = longitude_num_ * latitude_num_;

	array_p_ = new float[size];

	int id_var;
	char name[NC_MAX_NAME];
	nc_type type;
	int count_dims;
	int ids_dims[NC_MAX_DIMS]; 
	int count_attrs;
	int result = nc_inq_varid(id_netcdf_, variable_name.c_str(), &id_var);
	if(result != NC_NOERR)
	{
		status_ = ARRAY_STATUS_VARIABLE_NOT_FOUND;
		std::cout << "[GEOARRAY] netcdf format read error during querying the variable: " << nc_strerror(result) << std::endl;
		return false;
	}
	result = nc_inq_var(id_netcdf_, id_var, name, &type, &count_dims, ids_dims, &count_attrs);
	if(result != NC_NOERR)
	{
		status_ = ARRAY_STATUS_READ_VARIABLE_DESC_ERROR;
		std::cout << "[GEOARRAY] netcdf format read error during querying the variable's metadata: " << nc_strerror(result) << std::endl;
		return false;
	}
	size_t* index_dim_start = new unsigned int [count_dims];
	size_t* index_dim_count = new unsigned int [count_dims];
	memset(index_dim_start, 0, count_dims * sizeof(int));

	for(int i = 0; i < count_dims; ++i)
	{
		result = nc_inq_dimname(id_netcdf_, ids_dims[i], name);
		if(result != NC_NOERR)
		{
			status_ = ARRAY_STATUS_READ_VARIABLE_DESC_ERROR;
			std::cout << "[GEOARRAY] netcdf format read error during querying the dimension's name: " << nc_strerror(result) << std::endl;
			return false;
		}
#ifdef SOUTH_SEA
		if(strcmp(name, "x") == 0)
#else
		if(strcmp(name, "lon") == 0)
#endif
		{
			index_dim_count[i] = longitude_num_;
		}
#ifdef SOUTH_SEA
		else if(strcmp(name, "y") == 0)
#else
		else if(strcmp(name, "lat") == 0)
#endif
		{
			index_dim_count[i] = latitude_num_;
		}
#ifdef SOUTH_SEA
		else if(strcmp(name, "z") == 0)
#else
		else if(strcmp(name, "lev1") == 0 || strcmp(name, "lev") == 0)
#endif
		{
			//query the levels, range from 0 to height_num_
#ifdef SOUTH_SEA
			if(variable_name.compare("z") == 0)
#else
			if(variable_name.compare("lev1") == 0 || variable_name.compare("lev") == 0)
#endif
			{
				index_dim_start[i] = 0;
				index_dim_count[i] = height_num_;
			}
			//query the variable, specify the input level
			else
			{
				index_dim_start[i] = level;
				index_dim_count[i] = 1;
			}
		}
#ifdef SOUTH_SEA
		else if(strcmp(name, "t") == 0)
#else
		else if(strcmp(name, "time") == 0)
#endif
		{
			index_dim_start[i] = ticks;
			index_dim_count[i] = 1;//date_num_;
		}
		else
		{
			std::cout << "[NETCDF ERROR] it's an unknown dimension: " << name << std::endl;
		}
	}

	switch(type)
	{
	case NC_FLOAT:
		{
			float* buffer = new float [size];
			result = nc_get_vara_float(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
				std::cout << "[GEOARRAY] netcdf format read error during reading the variable data: " << nc_strerror(result) << std::endl;
				return false;
			}
			//经向速度用的是余纬，需要反一下
			if(strcmp(name, "vv") != 0)
			{
				for(int i = 0; i < size; ++i)
				{
					array_p_[i] = buffer[i];
				}
			}
			else
			{
				for(int i = 0; i < size; ++i)
				{
					array_p_[i] = -buffer[i];
				}
			}
			delete[] buffer;
			status_ = ARRAY_STATUS_SUCCEED;
		}
		break;
	case NC_DOUBLE:
		{
			double* buffer = new double [size];
			result = nc_get_vara_double(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
				std::cout << "[GEOARRAY] netcdf format read error during quering the varibale data: " << nc_strerror(result) << std::endl;
				return false;
			}
			if(strcmp(name, "vv") != 0)
			{
				for(int i = 0; i < size; ++i)
				{
					array_p_[i] = buffer[i];
				}
			}
			else
			{
				for(int i = 0; i < size; ++i)
				{
					array_p_[i] = -buffer[i];
				}
			}
			delete[] buffer;
			status_ = ARRAY_STATUS_SUCCEED;
		}
		break;
	case NC_INT:
		{
			int* buffer = new int [size];
			result = nc_get_vara_int(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
				std::cout << "[GEOARRAY] netcdf format read error during querying the varibale data: " << nc_strerror(result) << std::endl;
				return false;
			}
			if(strcmp(name, "vv") != 0)
			{
				for(int i = 0; i < size; ++i)
				{
					array_p_[i] = buffer[i];
				}
			}
			else
			{
				for(int i = 0; i < size; ++i)
				{
					array_p_[i] = -buffer[i];
				}
			}
			delete[] buffer;
			status_ = ARRAY_STATUS_SUCCEED;
		}
		break;
	case NC_SHORT:
		{
			short* buffer = new short [size];
			result = nc_get_vara_short(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
				std::cout << "[GEOARRAY] netcdf format read error during querying the varibale data: " << nc_strerror(result) << std::endl;
				return false;
			}
			if(strcmp(name, "vv") != 0)
			{
				for(int i = 0; i < size; ++i)
				{
#ifdef SOUTH_SEA
					array_p_[i] = buffer[i] / 100.0;
#else
					array_p_[i] = buffer[i];
#endif
				}
			}
			else
			{
				for(int i = 0; i < size; ++i)
				{
					array_p_[i] = -buffer[i];
				}
			}
			delete[] buffer;
			status_ = ARRAY_STATUS_SUCCEED;
		}
		break;
	default:
		status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
		std::cout << "the varaible your queried has not been supported yet";
		return false;
	}
	this->maxVal_ = -10000;
	this->minVal_ = 10000;
	for(int i = 0; i < size; ++i)
	{
#ifdef SOUTH_SEA
		if(_isnan(array_p_[i]) || array_p_[i] == 0)
#else
		if(array_p_[i] > 1e+34)
#endif
			continue;
		//if(array_p_[i] > 0)
		//	std::cout << array_p_[i] << std::endl;

		if(this->maxVal_ < array_p_[i])
			this->maxVal_ = array_p_[i];
		if(this->minVal_ > array_p_[i])
			this->minVal_ = array_p_[i];
	}

	this->firstVal_ = array_p_[0];
	this->lastVal_ = array_p_[size - 1];

	status_ = ARRAY_STATUS_SUCCEED;

	updateGaInfo(geoArray);
	delete [] geoArray.array_p_;
	if(sparse_rate > 1)
	{
		geoArray.array_p_ = new float[geoArray.latitude_num_ * geoArray.longitude_num_];
		for(int i = 0, count = 0; i < latitude_num_; i += 3)
			for(int j = 0; j < longitude_num_; j += 3)
#ifdef SOUTH_SEA
				geoArray.array_p_[count++] = _isnan(array_p_[i * longitude_num_ + j]) ? this->minVal_ : array_p_[i * longitude_num_ + j];
#else
				geoArray.array_p_[count++] = array_p_[i * longitude_num_ + j] > 1e+34 ? 0 : array_p_[i * longitude_num_ + j];
#endif
	}
	else
	{
		geoArray.array_p_ = new float[size];
		for(int i = 0; i < size; ++i)
#ifdef SOUTH_SEA
			geoArray.array_p_[i] = _isnan(array_p_[i]) || (variable_name == "SSH" && array_p_[i] == 0) ? this->minVal_ : array_p_[i];
#else
			geoArray.array_p_[i] = array_p_[i] > 1e+34 ? 0 : array_p_[i];
#endif
	}
	
	geoArray.maxVal_ = maxVal_;
	geoArray.minVal_ = minVal_;
	geoArray.status_ = ARRAY_STATUS_SUCCEED;

	return true;
}

bool NetCDFArray::getGeoVolumeData(GeoVolume<float>& gv, std::string type_str, size_t ticks, size_t levels_count, int sparse_rate)
{
	std::string data_path = file_full_path_;
	//std::map<levels_t,std::pair<offset_t,int>>  type_result_map;
	//if (getVarMap(type_result_map,type))
	{
		gv.longitudeStart_ = longitude_start_;
		gv.latitudeStart_ = latitude_start_;
		if(sparse_rate > 0)
		{
			gv.longitudeStep_ = longitude_interval_ * sparse_rate;
			gv.longitudeNum_ = (longitude_num_ - 1) / sparse_rate + 1;
			gv.latitudeStep_ = latitude_interval_ * sparse_rate;
			gv.latitudeNum_ = (latitude_num_ - 1) / sparse_rate + 1;
		}
		else
		{
			gv.longitudeStep_ = longitude_interval_;
			gv.longitudeNum_ = longitude_num_;
			gv.latitudeStep_ = latitude_interval_;
			gv.latitudeNum_ = latitude_num_;
		}
		gv.heightOfLevels_ = getLevelsList(levels_count);
		if(gv.heightOfLevels_.front() > gv.heightOfLevels_.back())
			std::reverse(gv.heightOfLevels_.begin(), gv.heightOfLevels_.end());
		
		//if (type_result_map.size() > 1)
		{
			//LEVELS_OFFSET_MAP::const_iterator it = type_result_map.find(height_layer);
			//if(it == type_result_map.end())
			//	return false;

			//const int offset = it->second.first + time_layer_offset_ * time_layer;
			//const int length = it->second.second;

			gv.volData_.clear();
			const int voxelNum = longitude_num_ * latitude_num_ * levels_count;//gv.heightOfLevels_.size();
			const int size = gv.latitudeNum_ * gv.longitudeNum_ * levels_count;
			gv.volData_.resize(size);
			////gv.volData_.resize(length);
			//std::ifstream ifs(data_path, std::ios::binary);
			//if(!ifs.good())
			//	return false;
			//const int offset = type_result_map[level_vector_[0]].first;
			//ifs.seekg(offset);
			//ifs.read((char*)gv.volData_.data(), voxelNum*sizeof(data_t));
			////ifs.read((char*)gv.volData_.data(), length * sizeof(data_t));
			delete[] array_p_;
	
			//size_t size = 0;
			//if(variable_name.compare("lat") == 0)
			//	size = latitude_num_;
			//else if(variable_name.compare("lon") == 0)
			//	size = longitude_num_;
			//else if(variable_name.compare("lev1") == 0 || variable_name.compare("lev") == 0)
			//	size = height_num_;
			//else if(variable_name.compare("time") == 0)
			//	size = date_num_;
			//else
			//	size = longitude_num_ * latitude_num_;

			array_p_ = new float[voxelNum];

			int id_var;
			char name[NC_MAX_NAME];
			nc_type type;
			int count_dims;
			int ids_dims[NC_MAX_DIMS]; 
			int count_attrs;
			int result = nc_inq_varid(id_netcdf_, type_str.c_str(), &id_var);
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_VARIABLE_NOT_FOUND;
				std::cout << "[GEOARRAY] netcdf format read error during querying the variable: " << nc_strerror(result) << std::endl;
				return false;
			}
			result = nc_inq_var(id_netcdf_, id_var, name, &type, &count_dims, ids_dims, &count_attrs);
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_READ_VARIABLE_DESC_ERROR;
				std::cout << "[GEOARRAY] netcdf format read error during querying the variable's metadata: " << nc_strerror(result) << std::endl;
				return false;
			}
			size_t* index_dim_start = new unsigned int [count_dims];
			size_t* index_dim_count = new unsigned int [count_dims];
			memset(index_dim_start, 0, count_dims * sizeof(int));

			for(int i = 0; i < count_dims; ++i)
			{
				result = nc_inq_dimname(id_netcdf_, ids_dims[i], name);
				if(result != NC_NOERR)
				{
					status_ = ARRAY_STATUS_READ_VARIABLE_DESC_ERROR;
					std::cout << "[GEOARRAY] netcdf format read error during querying the dimension's name: " << nc_strerror(result) << std::endl;
					return false;
				}
#ifdef SOUTH_SEA
				if(strcmp(name, "x") == 0)
#else
				if(strcmp(name, "lon") == 0)
#endif
				{
					index_dim_count[i] = gv.longitudeNum_;
				}
#ifdef SOUTH_SEA
				else if(strcmp(name, "y") == 0)
#else
				else if(strcmp(name, "lat") == 0)
#endif
				{
					index_dim_count[i] = gv.latitudeNum_;
				}
#ifdef SOUTH_SEA
				else if(strcmp(name, "z") == 0)
#else
				else if(strcmp(name, "lev1") == 0 || strcmp(name, "lev") == 0)
#endif
				{
					index_dim_start[i] = 0;
					index_dim_count[i] = height_num_ < levels_count ? height_num_ : levels_count;//height_num_;
				}
#ifdef SOUTH_SEA
				else if(strcmp(name, "t") == 0)
#else
				else if(strcmp(name, "time") == 0)
#endif
				{
					index_dim_start[i] = ticks;
					index_dim_count[i] = 1;//date_num_;
				}
				else
				{
					std::cout << "[NETCDF ERROR] it's an unknown dimension: " << name << std::endl;
				}
			}

			switch(type)
			{
			case NC_FLOAT:
				{
					float* buffer = new float [voxelNum];
					result = nc_get_vara_float(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
					if(result != NC_NOERR)
					{
						status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
						std::cout << "[GEOARRAY] netcdf format read error during reading the variable data: " << nc_strerror(result) << std::endl;
						return false;
					}
					//经向速度用的是余纬，需要反一下
					if(strcmp(name, "vv") != 0)
					{
						if(height_num_ < levels_count)
						{
							for(int i = 0; i < levels_count / height_num_; ++i)
							{
								for(int j = 0; j < voxelNum / (levels_count / height_num_); ++j)
								{
									array_p_[i * levels_count / height_num_ + j] = buffer[j];
								}
							}
						}
						else
						{
							for(int i = 0; i < voxelNum; ++i)
							{
								array_p_[i] = buffer[i];
							}
						}
					}
					else
					{
						if(height_num_ < levels_count)
						{
							for(int i = 0; i < levels_count / height_num_; ++i)
							{
								for(int j = 0; j < voxelNum / (levels_count / height_num_); ++j)
								{
									array_p_[i * levels_count / height_num_ + j] = -buffer[j];
								}
							}
						}
						else
						{
							for(int i = 0; i < voxelNum; ++i)
							{
								array_p_[i] = -buffer[i];
							}
						}
					}
					delete[] buffer;
					status_ = ARRAY_STATUS_SUCCEED;
				}
				break;
			case NC_DOUBLE:
				{
					double* buffer = new double [voxelNum];
					result = nc_get_vara_double(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
					if(result != NC_NOERR)
					{
						status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
						std::cout << "[GEOARRAY] netcdf format read error during quering the varibale data: " << nc_strerror(result) << std::endl;
						return false;
					}
					if(strcmp(name, "vv") != 0)
					{
						for(int i = 0; i < voxelNum; ++i)
						{
							array_p_[i] = buffer[i];
						}
					}
					else
					{
						for(int i = 0; i < voxelNum; ++i)
						{
							array_p_[i] = -buffer[i];
						}
					}
					delete[] buffer;
					status_ = ARRAY_STATUS_SUCCEED;
				}
				break;
			case NC_INT:
				{
					int* buffer = new int [voxelNum];
					result = nc_get_vara_int(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
					if(result != NC_NOERR)
					{
						status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
						std::cout << "[GEOARRAY] netcdf format read error during querying the varibale data: " << nc_strerror(result) << std::endl;
						return false;
					}
					if(strcmp(name, "vv") != 0)
					{
						for(int i = 0; i < voxelNum; ++i)
						{
							array_p_[i] = buffer[i];
						}
					}
					else
					{
						for(int i = 0; i < voxelNum; ++i)
						{
							array_p_[i] = -buffer[i];
						}
					}
					delete[] buffer;
					status_ = ARRAY_STATUS_SUCCEED;
				}
				break;
			case NC_SHORT:
				{
					short* buffer = new short [voxelNum];
					result = nc_get_vara_short(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
					if(result != NC_NOERR)
					{
						status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
						std::cout << "[GEOARRAY] netcdf format read error during querying the varibale data: " << nc_strerror(result) << std::endl;
						return false;
					}
					if(strcmp(name, "vv") != 0)
					{
						for(int i = 0; i < voxelNum; ++i)
						{
#ifdef SOUTH_SEA
							array_p_[i] = buffer[i] / 100.0; 
#else
							array_p_[i] = buffer[i];
#endif
						}
					}
					else
					{
						for(int i = 0; i < voxelNum; ++i)
						{
							array_p_[i] = -buffer[i];
						}
					}
					delete[] buffer;
					status_ = ARRAY_STATUS_SUCCEED;
				}
				break;
			default:
				status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
				std::cout << "the varaible your queried has not been supported yet";
				return false;
			}
			this->maxVal_ = -10000;
			this->minVal_ = 10000;
			for(int i = 0; i < voxelNum; ++i)
			{
#ifdef SOUTH_SEA
				if(_isnan(array_p_[i]) || array_p_[i] == 0)
#else
				if(array_p_[i] > 1e+34)
#endif
					continue;

				if(this->maxVal_ < array_p_[i])
					this->maxVal_ = array_p_[i];
				if(this->minVal_ > array_p_[i])
					this->minVal_ = array_p_[i];
			}

			this->firstVal_ = array_p_[0];
			this->lastVal_ = array_p_[voxelNum - 1];

			status_ = ARRAY_STATUS_SUCCEED;

			//updateGaInfo(geoArray);
			//delete [] geoArray.array_p_;
			//geoArray.array_p_ = new float[size];
			//for(int i = 0; i < voxelNum; ++i)
			//	gv.volData_[i] = array_p_[i] > 1e+34 ? 0 : array_p_[i];

			if(sparse_rate > 1)
			{
				for(int i = 0, count = 0; i < levels_count; ++i)
					for(int j = 0; j < latitude_num_; j += 3)
						for(int k = 0; k < longitude_num_; k += 3)
#ifdef SOUTH_SEA
							gv.volData_[count++] = _isnan(array_p_[i * latitude_num_ * longitude_num_ + j * longitude_num_ + k]) ? 0 : array_p_[i * latitude_num_ * longitude_num_ + j * longitude_num_ + k];
#else
							gv.volData_[count++] = array_p_[i * latitude_num_ * longitude_num_ + j * longitude_num_ + k] > 1e+34 ? 0 : array_p_[i * latitude_num_ * longitude_num_ + j * longitude_num_ + k];
#endif
			}
			else
			{
				for(int i = 0; i < size; ++i)
#ifdef SOUTH_SEA
					gv.volData_[i] = _isnan(array_p_[i]) ? 0 : array_p_[i];
#else
					gv.volData_[i] = array_p_[i] > 1e+34 ? 0 : array_p_[i];
#endif
			}
	
			//geoArray.maxVal_ = maxVal_;
			//geoArray.minVal_ = minVal_;
			//geoArray.status_ = ARRAY_STATUS_SUCCEED;
			//std::for_each(gv.volData_.begin(), gv.volData_.end(), 
			//	[=](data_t& val){ // endian convert
			//		char buf[sizeof(val)];
			//		const char* p = (char*)&val;
			//		for(int i=0; i<sizeof(val); ++i)
			//		{
			//			buf[i] = *(p+sizeof(val)-i-1);
			//		}
			//		val = *(data_t*)(&buf[0]);
			//		if(has_invalid_value_ && val == invalid_value_)
			//			val = 0;
			//	}
			//);
			return true;
		}
	}
	
	return false;
}

bool NetCDFArray::readFromFile(const std::string& variable_name, size_t level)
{
	delete[] array_p_;
	
	size_t size = 0;
	if(variable_name.compare("lat") == 0)
		size = latitude_num_;
	else if(variable_name.compare("lon") == 0)
		size = longitude_num_;
	else if(variable_name.compare("lev1") == 0 || variable_name.compare("lev") == 0 || variable_name.compare("depth") == 0)
		size = height_num_;
	else if(variable_name.compare("time") == 0)
		size = date_num_;
	else
		size = longitude_num_ * latitude_num_;

	int id_var;
	char name[NC_MAX_NAME];
	nc_type type;
	int count_dims;
	int ids_dims[NC_MAX_DIMS]; 
	int count_attrs;
	int result = nc_inq_varid(id_netcdf_, variable_name.c_str(), &id_var);
	if(result != NC_NOERR)
	{
		status_ = ARRAY_STATUS_VARIABLE_NOT_FOUND;
		std::cout << "[GEOARRAY] netcdf format read error during querying the variable: " << nc_strerror(result) << std::endl;
		return false;
	}
	result = nc_inq_var(id_netcdf_, id_var, name, &type, &count_dims, ids_dims, &count_attrs);
	if(result != NC_NOERR)
	{
		status_ = ARRAY_STATUS_READ_VARIABLE_DESC_ERROR;
		std::cout << "[GEOARRAY] netcdf format read error during querying the variable's metadata: " << nc_strerror(result) << std::endl;
		return false;
	}
//#ifdef SOUTH_SEA
//	if(strcmp(name, "lon") == 0
//		|| strcmp(name, "lat") == 0
//		|| strcmp(name, "z") == 0
//		|| strcmp(name, "t") == 0)
//		count_dims = 1;
//#endif
	size_t* index_dim_start = new unsigned int [count_dims];
	size_t* index_dim_count = new unsigned int [count_dims];
	memset(index_dim_start, 0, count_dims * sizeof(int));

	for(int i = 0; i < count_dims; ++i)
	{
		result = nc_inq_dimname(id_netcdf_, ids_dims[i], name);
		if(result != NC_NOERR)
		{
			status_ = ARRAY_STATUS_READ_VARIABLE_DESC_ERROR;
			std::cout << "[GEOARRAY] netcdf format read error during querying the dimension's name: " << nc_strerror(result) << std::endl;
			return false;
		}
#ifdef SOUTH_SEA
		if(strcmp(name, "x") == 0)
#else
		if(strcmp(name, "lon") == 0)
#endif
		{
			index_dim_count[i] = longitude_num_;
		}
#ifdef SOUTH_SEA
		else if(strcmp(name, "y") ==0)
#else
		else if(strcmp(name, "lat") == 0)
#endif
		{
			index_dim_count[i] = latitude_num_;
		}
#ifdef SOUTH_SEA
		else if(strcmp(name, "z") == 0 || strcmp(name, "depth") == 0)
#else
		else if(strcmp(name, "lev1") == 0 || strcmp(name, "lev") == 0)
#endif
		{
			//query the levels, range from 0 to height_num_
			if(variable_name.compare("lev1") == 0 || variable_name.compare("lev") == 0 || variable_name.compare("z") == 0 || variable_name.compare("depth") == 0)
			{
				index_dim_start[i] = 0;
				index_dim_count[i] = height_num_;
			}
			//query the variable, specify the input level
			else
			{
				index_dim_start[i] = level;
				index_dim_count[i] = 1;
			}
		}
#ifdef SOUTH_SEA
		else if(strcmp(name, "t") == 0)
#else
		else if(strcmp(name, "time") == 0)
#endif
		{
			index_dim_count[i] = date_num_;
		}
		else
		{
			std::cout << "[NETCDF ERROR] it's an unknown dimension: " << name << std::endl;
		}
	}
#ifdef SOUTH_SEA
	if(count_dims == 2)
		size = longitude_num_ * latitude_num_;
#endif
	array_p_ = new float[size];

	switch(type)
	{
	case NC_FLOAT:
		{
			float* buffer = new float [size];
			result = nc_get_vara_float(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
				std::cout << "[GEOARRAY] netcdf format read error during reading the variable data: " << nc_strerror(result) << std::endl;
				return false;
			}
			for(int i = 0; i < size; ++i)
			{
				array_p_[i] = buffer[i];
			}
			delete[] buffer;
			status_ = ARRAY_STATUS_SUCCEED;
		}
		break;
	case NC_DOUBLE:
		{
			double* buffer = new double [size];
			result = nc_get_vara_double(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
				std::cout << "[GEOARRAY] netcdf format read error during quering the varibale data: " << nc_strerror(result) << std::endl;
				return false;
			}
			for(int i = 0; i < size; ++i)
			{
				array_p_[i] = buffer[i];
			}
			delete[] buffer;
			status_ = ARRAY_STATUS_SUCCEED;
		}
		break;
	case NC_INT:
		{
			int* buffer = new int [size];
			result = nc_get_vara_int(id_netcdf_, id_var, index_dim_start, index_dim_count, buffer);
			if(result != NC_NOERR)
			{
				status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
				std::cout << "[GEOARRAY] netcdf format read error during querying the varibale data: " << nc_strerror(result) << std::endl;
				return false;
			}
			for(int i = 0; i < size; ++i)
			{
				array_p_[i] = buffer[i];
			}
			delete[] buffer;
			status_ = ARRAY_STATUS_SUCCEED;
		}
		break;
	default:
		status_ = ARRAY_STATUS_READ_VARIABLE_ERROR;
		std::cout << "the varaible your queried has not been supported yet";
		return false;
	}
	this->maxVal_ = -10000;
	this->minVal_ = 10000;
	for(int i = 0; i < size; ++i)
	{
		if(this->maxVal_ < array_p_[i])
			this->maxVal_ = array_p_[i];
		if(this->minVal_ > array_p_[i])
			this->minVal_ = array_p_[i];
	}

	this->firstVal_ = array_p_[0];
	this->lastVal_ = array_p_[size - 1];

	status_ = ARRAY_STATUS_SUCCEED;
	return true;
}

std::vector<levels_t> NetCDFArray::getLevelsList(int count)
{
	std::vector<levels_t> lv_vec;
#ifdef SOUTH_SEA
	//lv_vec.push_back(0.01);
	//lv_vec.push_back(15);
	//lv_vec.push_back(30);
	//lv_vec.push_back(50);
	//lv_vec.push_back(100);
	//lv_vec.push_back(150);
	//lv_vec.push_back(200);
	//lv_vec.push_back(250);
	//lv_vec.push_back(300);
	//lv_vec.push_back(350);
	//lv_vec.push_back(400);
	//lv_vec.push_back(450);
	//lv_vec.push_back(500);
	//lv_vec.push_back(600);
	//lv_vec.push_back(700);
	//lv_vec.push_back(800);
	//lv_vec.push_back(1000);
	//lv_vec.push_back(1200);
	//lv_vec.push_back(1500);
	//lv_vec.push_back(2000);
	//lv_vec.push_back(2500);
	//lv_vec.push_back(3000);
	//lv_vec.push_back(4000);
	//count = count > 0 ? count : 23;
	//for(int i = 0; i < 23 - count; ++i)
	//	lv_vec.pop_back();
	std::vector<std::string>::const_iterator it = variables_list_.begin();
	for(; it != variables_list_.end(); ++it)
	{
		if(it->compare("depth") == 0)
		{
			bool res = readFromFile(*it);
			for(int i = 0; i < height_num_; ++i)
				lv_vec.push_back(array_p_[i]);
			break;
		}
		else if(it->compare("pressure") == 0)
		{
			for(int i = 1000; i >= 100; i-=50)
				lv_vec.push_back(i);
			break;
		}
	}
	if(it == variables_list_.end())
	{
		int i = 0;
		do
		{
			lv_vec.push_back(i++);
			if(count > 0 && count <= i)
				break;
		}while(i < height_num_);
	}
#else
	if(height_num_ < count)
	{
		for(int i = 0; i < count; ++i)
		{
			lv_vec.push_back(height_start_ + (i % height_num_) * height_interval_);
		}
	}
	else
	{
		for(int i = 0; i < height_num_; ++i)
		{
			if(count > 0 && i == count)
				break;
			lv_vec.push_back(height_start_ + i * height_interval_);
		}
	}
#endif
	return lv_vec;
}

size_t NetCDFArray::getLevelIndex(std::string level)
{
	double d_level = stod(level);
#ifdef SOUTH_SEA
	std::vector<levels_t> levels_list = getLevelsList();
	//std::vector<levels_t>::iterator it = std::find(levels_list.begin(), levels_list.end(), d_level);
	std::vector<levels_t>::iterator it = levels_list.begin();
	while(true)
	{
		if(*it - d_level < 0.001 && d_level - *it < 0.001)
			break;
		else
			++it;
	}
	return it - levels_list.begin();
#else
	size_t index = (d_level - height_start_) / height_interval_;
	return abs(height_start_ + index * height_interval_ - d_level) 
		<= abs(height_start_ + (index + 1) * height_interval_ - d_level) ? index : index + 1;
#endif
}

size_t NetCDFArray::getTickIndex(std::string date) const
{
	if(date == "")
		return 1;

	std::string hour_str = date.substr(11, 2);
	//std::string min_str = date.substr(14, 2);
	double d_hour = stod(hour_str);
	//double d_min = stod(min_str);
	return d_hour;
}