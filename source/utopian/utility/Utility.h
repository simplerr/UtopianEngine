#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <glm/glm.hpp>

glm::vec4 ColorRGB(uint32_t r, uint32_t g, uint32_t b);

std::string ReadFile(std::string filename);

namespace Utopian
{
   std::string ExtractFilename(std::string path);
   std::string ExtractFileDirectory(std::string path);
   std::string GetFileExtension(std::string filename);
}
