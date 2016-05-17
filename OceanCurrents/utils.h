#pragma once
#include <map>

template <typename T, typename U>
class create_map
{
private:
	std::map<T, U> m_map;
public:
	create_map(const T& key, const U& val)
	{
		m_map[key] = val;
	}

	create_map<T, U>& operator()(const T& key, const U& val)
	{
		m_map[key] = val;
		return *this;
	}

	operator std::map<T, U>()
	{
		return m_map;
	}
};

template<typename T>
double getRatio(T val, T valMin, T valMax)
{
	return (val - valMin) * 1.0 / (valMax - valMin);
}