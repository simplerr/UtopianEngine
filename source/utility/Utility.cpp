#include "utility/Utility.h"

glm::vec4 ColorRGB(uint32_t r, uint32_t g, uint32_t b)
{
	return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
}

string ReadFile(string filename)
{
	string text;
	std::ifstream is(filename.c_str(), std::ios::binary | std::ios::in | std::ios::ate);

	if (is.is_open())
	{
		size_t size = is.tellg();
		is.seekg(0, std::ios::beg);
		char* data = new char[size];
		is.read(data, size);
		is.close();

		text = data;

		delete[] data;

		assert(size > 0);
	}

	return text;
}