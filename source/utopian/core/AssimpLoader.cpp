#include <vector>

#include "AssimpLoader.h"
#include "core/renderer/Primitive.h"
#include "core/Log.h"
#include "core/ModelLoader.h"
#include "core/renderer/Model.h"
#include "vulkan/handles/Device.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"
#include "vulkan/TextureLoader.h"

// TODO: Note that the format should be #include <assimp/Importer.hpp> but something in the project settings is wrong
#include "../external/assimp/assimp/Importer.hpp"
#include "../external/assimp/assimp/cimport.h"
#include "../external/assimp/assimp/material.h"
#include "../external/assimp/assimp/ai_assert.h"
#include "../external/assimp/assimp/postprocess.h"
#include "../external/assimp/assimp/scene.h"

namespace Utopian
{
   AssimpLoader::AssimpLoader(Vk::Device* device)
      : mDevice(device)
   {
   }

   AssimpLoader::~AssimpLoader()
   {
   }

   SharedPtr<Model> AssimpLoader::LoadModel(std::string filename)
   {
      SharedPtr<Model> model = std::make_shared<Model>();
      model->SetFilename(filename);

      // Load scene from the file.
      Assimp::Importer importer;
      const aiScene* scene = importer.ReadFile(filename, aiProcess_FlipUVs | aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

      if (scene != nullptr)
      {
         Mesh mesh;

         // Loop over all meshes
         for (unsigned int meshId = 0u; meshId < scene->mNumMeshes; meshId++)
         {
            Primitive primitive;
            aiMesh* assimpMesh = scene->mMeshes[meshId];

            // Get the diffuse color
            aiColor3D color(0.f, 0.f, 0.f);
            scene->mMaterials[assimpMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);

            // Load vertices
            for (unsigned int vertexId = 0u; vertexId < assimpMesh->mNumVertices; vertexId++)
            {
               aiVector3D pos = assimpMesh->mVertices[vertexId];
               aiVector3D normal = assimpMesh->mNormals[vertexId];
               aiVector3D uv = aiVector3D(0, 0, 0);
               aiVector3D tangent = aiVector3D(0, 0, 0);

               if (assimpMesh->HasTextureCoords(0))
                  uv = assimpMesh->mTextureCoords[0][vertexId];

               if (assimpMesh->HasTangentsAndBitangents())
                  tangent = assimpMesh->mTangents[vertexId];

               normal = normal.Normalize();
               Vk::Vertex vertex = {};
               vertex.pos = glm::vec3(pos.x, pos.y, pos.z);
               vertex.normal = glm::vec3(normal.x, normal.y, normal.z);
               vertex.tangent = glm::vec4(tangent.x, tangent.y, tangent.z, 1.0f);
               vertex.uv = glm::vec2(uv.x, uv.y);
               vertex.color = glm::vec3(color.r, color.g, color.b);
               primitive.AddVertex(vertex);
            }

            // Load indices
            for (unsigned int faceId = 0u; faceId < assimpMesh->mNumFaces; faceId++)
            {
               for (unsigned int indexId = 0u; indexId < assimpMesh->mFaces[faceId].mNumIndices; indexId+=3)
               {
                  primitive.AddTriangle(assimpMesh->mFaces[faceId].mIndices[indexId], assimpMesh->mFaces[faceId].mIndices[indexId+1], assimpMesh->mFaces[faceId].mIndices[indexId+2]);
               }
            }

            // Get texture path
            aiMaterial* aiMaterial = scene->mMaterials[assimpMesh->mMaterialIndex];
            int numTextures = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
            int numNormalMaps = aiMaterial->GetTextureCount(aiTextureType_NORMALS);
            int numHeightMaps = aiMaterial->GetTextureCount(aiTextureType_HEIGHT);
            int numSpecularMaps = aiMaterial->GetTextureCount(aiTextureType_SPECULAR);

            std::string diffuseTexturePath = DEFAULT_COLOR_TEXTURE_PATH;
            std::string normalTexturePath = DEFAULT_NORMAL_MAP_TEXTURE;
            std::string specularTexturePath = DEFAULT_SPECULAR_MAP_TEXTURE;

            /* Diffuse texture */
            if (numTextures > 0)
            {
               aiString texPath;
               aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
               FindValidPath(&texPath, filename);
               diffuseTexturePath = texPath.C_Str();
               SharedPtr<Vk::Texture> texture = Vk::gTextureLoader().LoadTexture(diffuseTexturePath);

               // Workaround for Unity assets
               // Note: Instead of calling LoadTexture() something similar to FindFile(diffuseTexturePath) could be used
               if (texture == nullptr)
               {
                  uint32_t idx = (uint32_t)diffuseTexturePath.rfind("\\");
                  std::string textureName = diffuseTexturePath.substr(idx+1);
                  idx = (uint32_t)filename.rfind("/");
                  diffuseTexturePath = filename.substr(0, idx) + "/Textures/" + textureName;

                  texture = Vk::gTextureLoader().LoadTexture(diffuseTexturePath);

                  // Try removing _H from the filename
                  if (texture == nullptr)
                  {
                     idx = (uint32_t)diffuseTexturePath.rfind("_H.tga");
                     if (idx != std::string::npos)
                     {
                        diffuseTexturePath = diffuseTexturePath.substr(0, idx) + ".tga";
                        texture = Vk::gTextureLoader().LoadTexture(diffuseTexturePath);
                     }

                     if (texture == nullptr)
                     {
                        idx = (uint32_t)diffuseTexturePath.rfind("_H.png");
                        if (idx != std::string::npos)
                        {
                           diffuseTexturePath = diffuseTexturePath.substr(0, idx) + ".png";
                           texture = Vk::gTextureLoader().LoadTexture(diffuseTexturePath);
                        }
                     }
                  }
               }

               if (texture == nullptr)
               {
                  texPath = DEFAULT_COLOR_TEXTURE_PATH;
               }
            }

            /* Normal texture */
            if (numNormalMaps > 0)
            {
               normalTexturePath = GetPath(aiMaterial, aiTextureType_NORMALS, filename);
            }

            /* Heightmap texture, in some formats this is the same as the normal map */
            if (numHeightMaps > 0)
            {
               assert(numNormalMaps == 0);
               normalTexturePath = GetPath(aiMaterial, aiTextureType_HEIGHT, filename);
            }

            /* Specular texture */
            if (numSpecularMaps > 0)
            {
               specularTexturePath = GetPath(aiMaterial, aiTextureType_SPECULAR, filename);
            }

            Material material = gModelLoader().GetDefaultMaterial();

            material.colorTexture = Vk::gTextureLoader().LoadTexture(diffuseTexturePath);
            material.normalTexture = Vk::gTextureLoader().LoadTexture(normalTexturePath);
            material.specularTexture = Vk::gTextureLoader().LoadTexture(specularTexturePath);
            material.BindTextureDescriptors(mDevice);

            primitive.SetDebugName(filename);
            primitive.BuildBuffers(mDevice);

            Primitive* prim = model->AddPrimitive(primitive);
            Material* mat = model->AddMaterial(material);

            mesh.AddPrimitive(prim, mat);
         }

         Node* node = model->CreateNode();
         node->mesh = mesh;
         model->AddRootNode(node);
         model->Init();
      }
      else {
         // Loading of model failed
         UTO_LOG("Failed to load model: " + filename);
         model = nullptr;
      }

      return model;
   }

   std::string AssimpLoader::GetPath(aiMaterial* material, aiTextureType textureType, std::string filename)
   {
      std::string path;

      aiString texPath;
      material->GetTexture(textureType, 0, &texPath);
      FindValidPath(&texPath, filename);
      path = texPath.C_Str();

      return path;
   }

   // From /assimp/assimp/tools/assimp_view/Material.cpp
   int AssimpLoader::FindValidPath(aiString* texturePath, std::string modelPath)
   {
      ai_assert(NULL != texturePath);
      aiString pcpy = *texturePath;
      if ('*' == texturePath->data[0]) {
         // '*' as first character indicates an embedded file
         return 5;
      }

      // first check whether we can directly load the file
      FILE* pFile = fopen(texturePath->data, "rb");
      if (pFile)fclose(pFile);
      else
      {
         // check whether we can use the directory of  the asset as relative base
         char szTemp[MAX_PATH * 2], tmp2[MAX_PATH * 2];
         strcpy(szTemp, modelPath.c_str());
         strcpy(tmp2, szTemp);

         char* szData = texturePath->data;
         if (*szData == '\\' || *szData == '/')++szData;

         char* szEnd = strrchr(szTemp, '\\');
         if (!szEnd)
         {
            szEnd = strrchr(szTemp, '/');
            if (!szEnd)szEnd = szTemp;
         }
         szEnd++;
         *szEnd = 0;
         strcat(szEnd, szData);


         pFile = fopen(szTemp, "rb");
         if (!pFile)
         {
            // convert the string to lower case
            for (unsigned int i = 0;; ++i)
            {
               if ('\0' == szTemp[i])break;
               szTemp[i] = (char)tolower(szTemp[i]);
            }

            if (TryLongerPath(szTemp, texturePath))return 1;
            *szEnd = 0;

            // search common sub directories
            strcat(szEnd, "tex\\");
            strcat(szEnd, szData);

            pFile = fopen(szTemp, "rb");
            if (!pFile)
            {
               if (TryLongerPath(szTemp, texturePath))return 1;

               *szEnd = 0;

               strcat(szEnd, "textures\\");
               strcat(szEnd, szData);

               pFile = fopen(szTemp, "rb");
               if (!pFile)
               {
                  if (TryLongerPath(szTemp, texturePath))return 1;
               }

               // patch by mark sibly to look for textures files in the asset's base directory.
               const char *path = pcpy.data;
               const char *p = strrchr(path, '/');
               if (!p) p = strrchr(path, '\\');
               if (p) {
                  char *q = strrchr(tmp2, '/');
                  if (!q) q = strrchr(tmp2, '\\');
                  if (q) {
                     strcpy(q + 1, p + 1);
                     if (pFile = fopen(tmp2, "r")) {
                        fclose(pFile);
                        strcpy(texturePath->data, tmp2);
                        texturePath->length = strlen(tmp2);
                        return 1;
                     }
                  }
               }
               return 0;
            }
         }
         fclose(pFile);

         // copy the result string back to the aiString
         const size_t iLen = strlen(szTemp);
         size_t iLen2 = iLen + 1;
         iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
         memcpy(texturePath->data, szTemp, iLen2);
         texturePath->length = iLen;

      }
      return 1;
   }

   // From /assimp/assimp/tools/assimp_view/Material.cpp
   bool AssimpLoader::TryLongerPath(char* szTemp, aiString* p_szString)
   {
      char szTempB[MAX_PATH];
      strcpy(szTempB, szTemp);

      // go to the beginning of the file name
      char* szFile = strrchr(szTempB, '\\');
      if (!szFile)szFile = strrchr(szTempB, '/');

      char* szFile2 = szTemp + (szFile - szTempB) + 1;
      szFile++;
      char* szExt = strrchr(szFile, '.');
      if (!szExt)return false;
      szExt++;
      *szFile = 0;

      strcat(szTempB, "*.*");
      const unsigned int iSize = (const unsigned int)(szExt - 1 - szFile);

      HANDLE          h;
      WIN32_FIND_DATA info;

      // build a list of files
      h = FindFirstFile(szTempB, &info);
      if (h != INVALID_HANDLE_VALUE)
      {
         do
         {
            if (!(strcmp(info.cFileName, ".") == 0 || strcmp(info.cFileName, "..") == 0))
            {
               char* szExtFound = strrchr(info.cFileName, '.');
               if (szExtFound)
               {
                  ++szExtFound;
                  if (0 == _stricmp(szExtFound, szExt))
                  {
                     const unsigned int iSizeFound = (const unsigned int)(
                        szExtFound - 1 - info.cFileName);

                     for (unsigned int i = 0; i < iSizeFound; ++i)
                        info.cFileName[i] = (CHAR)tolower(info.cFileName[i]);

                     if (0 == memcmp(info.cFileName, szFile2, glm::min(iSizeFound, iSize)))
                     {
                        // we have it. Build the full path ...
                        char* sz = strrchr(szTempB, '*');
                        *(sz - 2) = 0x0;

                        strcat(szTempB, info.cFileName);

                        // copy the result string back to the aiString
                        const size_t iLen = strlen(szTempB);
                        size_t iLen2 = iLen + 1;
                        iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
                        memcpy(p_szString->data, szTempB, iLen2);
                        p_szString->length = iLen;
                        return true;
                     }
                  }
                  // check whether the 8.3 DOS name is matching
                  if (0 == _stricmp(info.cAlternateFileName, p_szString->data))
                  {
                     strcat(szTempB, info.cAlternateFileName);

                     // copy the result string back to the aiString
                     const size_t iLen = strlen(szTempB);
                     size_t iLen2 = iLen + 1;
                     iLen2 = iLen2 > MAXLEN ? MAXLEN : iLen2;
                     memcpy(p_szString->data, szTempB, iLen2);
                     p_szString->length = iLen;
                     return true;
                  }
               }
            }
         } while (FindNextFile(h, &info));

         FindClose(h);
      }
      return false;
   }
}
