#pragma once
#include <vector>
#include <array>

class Color
{
public:
	//could do this as glm::vec4
	float r;
	float g;
	float b;
	float a;

	Color(float c)
		:r(c), g(c), b(c), a(1)
	{
	}

	Color()
		:r(1), g(1), b(1), a(1)
	{
	}

	Color(float r, float g, float b, float a)
		:r(r), g(g), b(b), a(a)
	{
	}

	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
		:r(r* (1.f / 255.f)), g(g* (1.f / 255.f)), b(b* (1.f / 255.f)), a(a* (1.f / 255.f))
	{
	}

	Color(std::vector<double> c)
		:r((float)c[0]), g((float)c[1]), b((float)c[2]), a((float)c[3])
	{
	}

	Color(std::array<unsigned char, 4> c)
		:r(c[0] * (1.f / 255.f)), g(c[1] * (1.f / 255.f)), b(c[2] * (1.f / 255.f)), a(c[3] * (1.f / 255.f))
	{
	}

	inline std::array<unsigned char, 4> getCharArray()
	{
		return { (unsigned char)(r * 255),
		(unsigned char)(g * 255),
		(unsigned char)(b * 255),
		(unsigned char)(a * 255) };
	}

	inline void scale(const float& s)
	{
		r *= s;
		g *= s;
		b *= s;
	}

	inline void scale(const Color& c)
	{
		r *= c.r;
		g *= c.g;
		b *= c.b;
		a *= c.a;
	}
};