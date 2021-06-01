#include "core/glTFModel.h"
#include "core/Log.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

namespace Utopian
{
   glTFModel::glTFModel()
   {

   }

   glTFModel::~glTFModel()
   {

   }

   void glTFModel::LoadFromFile(std::string filename)
   {
      tinygltf::Model glTFInput;
      tinygltf::TinyGLTF gltfContext;
      std::string error, warning;

      bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

      if (fileLoaded)
      {

      }
      else
      {
         UTO_LOG("Failed to load glTF model " + filename);
      }
   }
}