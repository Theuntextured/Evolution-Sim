#pragma once

#include <iostream>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

//#define THREAD_SAFE

#define PI 3.1415926535897932384626433832795
#define PI_F 3.1415926535897932384626433832795f

#ifdef _DEBUG
#define DEBUG_VALUE_SWITCH(DEBUG_VALUE, RELEASE_VALUE) DEBUG_VALUE
#else
#define DEBUG_VALUE_SWITCH(DEBUG_VALUE, RELEASE_VALUE) RELEASE_VALUE
#endif

extern float time_speed_modifier;
extern sf::Vector2f world_extent;
extern sf::Font global_font;

#ifndef THREAD_SAFE
inline
#endif
int random()
{
#ifdef THREAD_SAFE
	static std::mutex mtx;
	mtx.lock();
#endif
	const auto r = rand();  // NOLINT(concurrency-mt-unsafe)
#ifdef THREAD_SAFE
	mtx.unlock();
#endif
	return r;
}

inline float random_float(const float lo, const float hi)
{
	return lo + static_cast <float> (random()) /( static_cast <float> (RAND_MAX/(hi-lo)));
}

inline float random_float(const float hi)
{
	return static_cast <float> (random()) / (static_cast <float> (RAND_MAX/hi));
}

inline float random_float()
{
	return static_cast <float> (random()) / static_cast <float> (RAND_MAX);
}

inline bool random_chance(const float chance)
{
	return random_float() < chance;
}

template <typename T>
sf::Vector2<T> clamp_vec_size(const sf::Vector2<T>& in, const T size)
{
	T len_squared = in.x * in.x + in.y * in.y;
	if(len_squared <= size*size)
		return in;
	sf::Vector2<T> out;
	auto len = sqrt(len_squared);
	out.x = in.x / len;
	out.y = in.y / len;
	return out;
}

template <typename T>
T vector_length(const sf::Vector2<T>& in)
{
	return sqrt(in.x * in.x + in.y * in.y);
}

template <typename T>
T vector_length_squared(const sf::Vector2<T>& in)
{
	return in.x * in.x + in.y * in.y;
}

template <typename T>
void print(const T& in)
{
	std::cout << in << std::endl;
}

template <typename T>
sf::Vector2<T> normalize(const sf::Vector2<T> v, const sf::Vector2<T> def = sf::Vector2<T>(0.0f, 0.0f))
{
	const auto len = sqrt(v.x * v.x + v.y * v.y);
	if(len == 0)
		return def;
	return v / len;
}

template <typename T>
T dot(const sf::Vector2<T> a, const sf::Vector2<T> b)
{
	return a.x * b.x + a.y * b.y;
}

template <typename T>
T remove_at_swap(std::vector<T>& vec, size_t pos)
{
	T back = vec.back();
	T ret = vec.at(pos);
	vec[pos] = back;
	vec.pop_back();
	return ret;
}

template <typename T>
T square(T val)
{
	return val * val;
}

template <typename T>
T clamp(T value, T min, T max)
{
	return std::max(min, std::min(value, max));
}

template <typename T>
T acos_deg(const T x)
{
	return acos(x) / PI_F * 180.0f;
}

template <typename T>
T atan_deg(const T x)
{
	return atan(x) / PI_F * 180.0f;
}