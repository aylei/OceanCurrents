/**
 * @file GeoArray.h
 *
 * @brief the dot array data struct
 *
 * @author Leonard
 * @date 2014-09-03
 */
#ifndef  METEOROLOGYATAWAPPER_GEOARRAY_H
#define  METEOROLOGYATAWAPPER_GEOARRAY_H

#include <string>
#include <io.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <type_traits>
#include <assert.h>
#include <string.h>

template <typename T>
struct GeoArray
{
	typedef T elem_t;
	typedef enum Status{
		ARRAY_STATUS_UNKNOW = 0,
		ARRAY_STATUS_NOTFIND,
		ARRAY_STATUS_NONUMS,
		ARRAY_STATUS_ERRORMODE,
		ARRAY_STATUS_SUCCEED,
		ARRAY_STATUS_FILE_NOT_FOUND,
		ARRAY_STATUS_DIMENSION_NOT_FOUND,
		ARRAY_STATUS_VARIABLE_NOT_FOUND,
		ARRAY_STATUS_READ_VARIABLE_DESC_ERROR,
		ARRAY_STATUS_CLOSE_FILE_FAILED,
		ARRAY_STATUS_READ_VARIABLE_ERROR
	}Status;
	
	GeoArray()
		: array_p_(nullptr), status_(ARRAY_STATUS_UNKNOW)
	{}

	GeoArray(const GeoArray& ga)
		: array_p_(nullptr)
	{
		*this = ga;
	}

	// move ctor
	GeoArray(GeoArray&& ga)
	{
		operator=(std::move(ga));
	}

	GeoArray& operator=(const GeoArray& ga)
	{
		if( this == &ga )
			return *this;
		if(array_p_)
			delete []array_p_;
		longitude_start_ = ga.longitude_start_;
		longitude_end_ = ga.longitude_end_;
		latitude_start_ = ga.latitude_start_;
		latitude_end_ = ga.latitude_end_;
		longitude_interval_ = ga.longitude_interval_;
		latitude_interval_ = ga.latitude_interval_;
		latitude_num_ = ga.latitude_num_;
		longitude_num_ = ga.longitude_num_;
		maxVal_ = ga.maxVal_;
		minVal_ = ga.minVal_;
		status_ = ga.status_;
		file_full_path_ = ga.file_full_path_;
		type_ = ga.type_;
		int cnt = longitude_num_ * latitude_num_;
		array_p_ = new T[cnt];
		memcpy(array_p_, ga.array_p_, cnt*sizeof(T));
		return *this;
	}

	GeoArray& operator=(GeoArray&& ga)
	{
		if( this == &ga )
			return *this;
		
		longitude_start_ = ga.longitude_start_;
		longitude_end_ = ga.longitude_end_;
		latitude_start_ = ga.latitude_start_;
		latitude_end_ = ga.latitude_end_;
		longitude_interval_ = ga.longitude_interval_;
		latitude_interval_ = ga.latitude_interval_;
		latitude_num_ = ga.latitude_num_;
		longitude_num_ = ga.longitude_num_;
		maxVal_ = ga.maxVal_;
		minVal_ = ga.minVal_;
		status_ = ga.status_;
		file_full_path_ = ga.file_full_path_;
		type_ = ga.type_;
		array_p_ = ga.array_p_;
		ga.array_p_ = nullptr;
		return *this;
	}

	/*
	 * @brief release the array
	 */
	~GeoArray()
	{
		delete[] array_p_;
	}

	/*
	 * @brief get datas 
	 * m is latitude_num ,n is longitude_num_
	 */
	T operator()(int m, int n) const
	{
		assert(m < latitude_num_ && n < longitude_num_);
		return array_p_[m * longitude_num_ + n];
	}

	Status getStatus() const
	{
		return status_;
	}

	double longitude_start_;
	double longitude_end_;
	double latitude_start_;
	double latitude_end_;
	double longitude_interval_;
	double latitude_interval_;
	int latitude_num_;	//the max of m
	int longitude_num_; //the max of n
	T* array_p_;
	T maxVal_;
	T minVal_;
	bool has_invalid_value_;
	float invalid_value_;
	Status status_;

	std::string file_full_path_;

protected:
	std::shared_ptr<std::ifstream> if_stream_ptr_;
	int type_;
};

template<typename T>
bool readFromFile(std::shared_ptr<std::ifstream>& sptr,GeoArray<T>& Geo)
{
	int m = 0, n = 0;
	T dataNum;
	bool flag = false;
	if (!sptr.get())
	{
		Geo.status_ = GeoArray<T>::ARRAY_STATUS_NONUMS;
		sptr->close();
		return false;
	}
	while(!sptr->eof() && m < Geo.latitude_num_)
	{
		*sptr >> dataNum;
		if (!flag)
		{
			Geo.maxVal_ = dataNum;
			Geo.minVal_ = dataNum;
			flag = true;
		}
		Geo.maxVal_ = std::max(Geo.maxVal_, dataNum);
		Geo.minVal_ = std::min(Geo.minVal_, dataNum);
		Geo.array_p_[m * Geo.longitude_num_ + n] = dataNum;
		++n;
		if (n >= Geo.longitude_num_ )
		{
			++m;
			n = 0;
		}
	}
	sptr->close();
	return true;
}

template<typename T>
bool readFromBinaryFile(std::shared_ptr<std::ifstream>& sptr,GeoArray<T>& Geo)
{
	int n = 0, m = 0;
	bool flag = false;
	T dataNUm;
	sptr->seekg(0, std::ios::end);
	int size = sptr->tellg();
	sptr->seekg(size - sizeof(T) * Geo.longitude_num_ * Geo.latitude_num_);
	if (!sptr.get())
	{
		Geo.status_ = GeoArray<T>::ARRAY_STATUS_NONUMS;
		sptr->close();
		return false;
	}
	while(!sptr->eof() && m < Geo.latitude_num_)
	{
		assert(sizeof(T) > 0);
		char buffer[sizeof(T)];
		sptr->read(buffer, sizeof(T));
		memcpy(Geo.array_p_ + m * Geo.longitude_num_ + n, buffer, sizeof(T));
		dataNUm = *(Geo.array_p_ + m * Geo.longitude_num_ + n);
		if (!flag)
		{
			Geo.maxVal_ = dataNUm;
			Geo.minVal_ = dataNUm;
			flag = true;
		}
		Geo.maxVal_ = std::max(Geo.maxVal_, dataNUm);
		Geo.minVal_ = std::min(Geo.minVal_, dataNUm);
		++n;
		if (n >= Geo.longitude_num_ )
		{
			++m;
			n = 0;
		}
	}
	return true;
}

// make a GeoArray object that using -latitudeStep_
template<typename T>
void rotateLat(GeoArray<T>& ga)
{
	const auto lonNum = ga.longitude_num_;
	const auto latNum = ga.latitude_num_;
	ga.latitude_interval_ = -ga.latitude_interval_;
	std::swap(ga.latitude_start_, ga.latitude_end_);
	for(int lat=0; lat<latNum / 2; ++lat)
	{
		T* p0 = ga.array_p_ + lonNum * lat;
		T* p1 = ga.array_p_ + lonNum * (latNum - lat - 1);
		std::vector<T> vec(p0, p0 + lonNum);
		std::copy(p1, p1+lonNum, p0);
		std::copy(vec.begin(), vec.end(), p1);
	}
}

template<typename T>
bool isSameGeoInfo(const GeoArray<T>& ga0, const GeoArray<T>& ga1)
{
	bool ret = (ga0.longitude_start_ == ga1.longitude_start_)
		&& (ga0.longitude_end_ == ga1.longitude_end_)
		&& (ga0.latitude_start_ == ga1.latitude_start_)
		&& (ga0.latitude_end_ == ga1.latitude_end_)
		&& (ga0.longitude_interval_ == ga1.longitude_interval_)
		&& (ga0.latitude_interval_ == ga1.latitude_interval_)
		&& (ga0.latitude_num_ == ga1.latitude_num_)
		&& (ga0.longitude_num_ == ga1.longitude_num_);
	return ret;
}

template<typename vecT, typename T>
bool getGeoArray_UV(GeoArray<vecT>& gauv, const GeoArray<T>& gaU, const GeoArray<T>& gaV)
{
	static_assert(std::is_arithmetic<T>::value);
	if( !isSameGeoInfo(gaU, gaV)
		|| gaU.getStatus() != GeoArray<T>::ARRAY_STATUS_SUCCEED
		|| gaV.getStatus() != GeoArray<T>::ARRAY_STATUS_SUCCEED )
	{
		return false;
	}
	gauv.longitude_start_ = gaU.longitude_start_;
	gauv.longitude_end_ = gaU.longitude_end_;
	gauv.latitude_start_ = gaU.latitude_start_;
	gauv.latitude_end_ = gaU.latitude_end_;
	gauv.longitude_interval_ = gaU.longitude_interval_;
	gauv.latitude_interval_ = gaU.latitude_interval_;
	gauv.latitude_num_ = gaU.latitude_num_;
	gauv.longitude_num_ = gaU.longitude_num_;
	if(gauv.array_p_)
		delete []gauv.array_p_;
	const int cnt = gauv.longitude_num_ * gauv.latitude_num_;
	if(cnt <= 0)
		return false;
	gauv.array_p_ = new GeoArray<vecT>::elem_t[cnt];
	for(int i=0; i<cnt; ++i)
	{
		gauv.array_p_[i].x = gaU.array_p_[i];
		gauv.array_p_[i].y = gaV.array_p_[i];
	}
	gauv.status_ = GeoArray<vecT>::ARRAY_STATUS_SUCCEED;
	return true;
}
#endif
