#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>

using namespace std;

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
