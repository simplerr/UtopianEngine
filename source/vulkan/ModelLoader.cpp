#include <vector>

#include "ModelLoader.h"
#include "vulkan/Mesh.h"
#include "StaticModel.h"
#include "Device.h"

// TODO: Note that the format should be #include <assimp/Importer.hpp> but something in the project settings is wrong
#include "../external/assimp/assimp/Importer.hpp"
#include "../external/assimp/assimp/cimport.h"
#include "../external/assimp/assimp/material.h"
#include "../external/assimp/assimp/ai_assert.h"
#include "../external/assimp/assimp/postprocess.h"
#include "../external/assimp/assimp/scene.h"

using namespace glm;

namespace VulkanLib
{
	void ModelLoader::CleanupModels(VkDevice device)
	{
		for (auto& model : mModelMap)
		{
			// Free vertex and index buffers
			for (int i = 0; i < model.second->mMeshes.size(); i++)
			{
				vkDestroyBuffer(device, model.second->mMeshes[i]->vertices.buffer, nullptr);
				vkFreeMemory(device, model.second->mMeshes[i]->vertices.memory, nullptr);
				vkDestroyBuffer(device, model.second->mMeshes[i]->indices.buffer, nullptr);
				vkFreeMemory(device, model.second->mMeshes[i]->indices.memory, nullptr);
			}

			delete model.second;
		}
	}

	StaticModel* ModelLoader::LoadModel(Device* device, std::string filename)
	{
		// Check if the model already is loaded
		if (mModelMap.find(filename) != mModelMap.end())
			return mModelMap[filename];

		StaticModel* model = new StaticModel();

		Assimp::Importer importer;

		// Load scene from the file.
		const aiScene* scene = importer.ReadFile(filename, aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

		if (scene != nullptr)
		{
			std::vector<AssimpMesh> assimpMeshes;

			// Loop over all meshes
			for (int meshId = 0; meshId < scene->mNumMeshes; meshId++)
			{
				Mesh* mesh = new Mesh(device);
				aiMesh* assimpMesh = scene->mMeshes[meshId];

				// Get the diffuse color
				aiColor3D color(0.f, 0.f, 0.f);
				scene->mMaterials[assimpMesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, color);

				// Load vertices
				for (int vertexId = 0; vertexId < assimpMesh->mNumVertices; vertexId++)
				{
					aiVector3D v = assimpMesh->mVertices[vertexId];
					aiVector3D n = assimpMesh->mNormals[vertexId];
					aiVector3D t = aiVector3D(0, 0, 0);

					if (assimpMesh->HasTextureCoords(0))
						t = assimpMesh->mTextureCoords[0][vertexId];

					n = n.Normalize();
					Vertex vertex(v.x, v.y, v.z, n.x, n.y, n.z, 0, 0, 0, t.x, t.y, color.r, color.g, color.b);
					mesh->AddVertex(vertex);
				}

				// Load indices
				for (int faceId = 0; faceId < assimpMesh->mNumFaces; faceId++)
				{
					for (int indexId = 0; indexId < assimpMesh->mFaces[faceId].mNumIndices; indexId+=3)
					{
						mesh->AddIndex(assimpMesh->mFaces[faceId].mIndices[indexId], assimpMesh->mFaces[faceId].mIndices[indexId+1], assimpMesh->mFaces[faceId].mIndices[indexId+2]);
					}
				}

				// Get texture path
				aiMaterial* material = scene->mMaterials[assimpMesh->mMaterialIndex];
				int numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
				if (numTextures > 0)
				{
					aiString texturePath;
					material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
					FindValidPath(&texturePath, filename);
					mesh->SetTexturePath(texturePath.C_Str());
				}
				else
				{
					mesh->SetTexturePath("NO_TEXTURE");
				}

				mesh->BuildBuffers(device);		// Build the models buffers here
				model->AddMesh(mesh);
			}

			// Add the model to the model map
			model->Init(device);
			mModelMap[filename] = model;
		}
		else {
			// Loading of model failed
			assert(scene);
		}

		return model;
	}

	StaticModel* ModelLoader::GenerateTerrain(Device* device, std::string filename)
	{
		return nullptr;
		//// Check if the model already is loaded
		//if (mModelMap.find(filename) != mModelMap.end())
		//	return mModelMap[filename];

		//// Load the terrain froma .tga file
		//TextureData texture;
		//LoadTGATextureData((char*)filename.c_str(), &texture);

		//StaticModel* terrain = new StaticModel;
		//Mesh mesh;

		//int vertexCount = texture.width * texture.height;
		//int triangleCount = (texture.width - 1) * (texture.height - 1) * 2;
		//int x, z;

		//mesh.vertices.resize(vertexCount);
		//mesh.indices.resize(triangleCount * 3);

		//printf("bpp %d\n", texture.bpp);
		//for (x = 0; x < texture.width; x++)
		//	for (z = 0; z < texture.height; z++)
		//	{
		//		// Vertex array. You need to scale this properly
		//		float height = texture.imageData[(x + z * texture.width) * (texture.bpp / 8)] / 15.0f;

		//		vec3 pos = vec3(x / 1.0, height, z / 1.0);
		//		vec3 normal = vec3(0, 0, 0);
		//		vec2 uv = vec2(x / (float)texture.width, z / (float)texture.height);

		//		Vertex vertex = Vertex(pos, normal, uv, vec3(0, 0, 0), vec3(1.0, 1.0, 1.0));
		//		mesh.vertices[x + z * texture.width] = vertex;
		//	}

		//// Normal vectors. You need to calculate these.
		//for (x = 0; x < texture.width; x++)
		//{
		//	for (z = 0; z < texture.height; z++)
		//	{
		//		vec3 p1, p2, p3;
		//		vec3 edge = { 0.0f, 0.0f, 0.0f };
		//		int i1;

		//		// p1 [x-1][z-1]
		//		if (x < 1 && z < 1)
		//			i1 = (x + 1 + (z + 1) * texture.width);
		//		else
		//			i1 = (x - 1 + (z - 1) * texture.width);

		//		// TODO: NOTE: HAX
		//		if (i1 < 0)
		//			i1 = 0;

		//		p1 = mesh.vertices[i1].Pos;

		//		// p1 [x-1][z] (if on the edge use [x+1] instead of [x-1])
		//		int i2;
		//		if (x < 1)
		//			i2 = (x + 1 + (z)* texture.width);
		//		else
		//			i2 = (x - 1 + (z)* texture.width);

		//		p2 = mesh.vertices[i2].Pos;

		//		// p1 [x][z-1]
		//		int i3;
		//		if (z < 1)
		//			i3 = (x + (z + 1) * texture.width);
		//		else
		//			i3 = (x + (z - 1) * texture.width);

		//		p3 = mesh.vertices[i3].Pos;

		//		vec3 e1 = p2 - p1;
		//		vec3 e2 = p3 - p1;
		//		vec3 normal = glm::cross(e2, e1);

		//		if (normal != vec3(0, 0, 0))
		//			int asda = 1;

		//		normal = glm::normalize(normal);

		//		//i = (x + 1 + (z + 1) * texture.width);
		//		mesh.vertices[i1].Normal += normal;
		//		mesh.vertices[i2].Normal += normal;
		//		mesh.vertices[i3].Normal += normal;

		//		// NOTE: Testing
		//		//mesh.vertices[i].Normal = vec3(0, 0, 0);
		//	}
		//}

		//for (x = 0; x < texture.width - 1; x++)
		//{
		//	for (z = 0; z < texture.height - 1; z++)
		//	{
		//		// Triangle 1
		//		mesh.indices[(x + z * (texture.width - 1)) * 6 + 0] = x + z * texture.width;
		//		mesh.indices[(x + z * (texture.width - 1)) * 6 + 1] = x + (z + 1) * texture.width;
		//		mesh.indices[(x + z * (texture.width - 1)) * 6 + 2] = x + 1 + z * texture.width;
		//		// Triangle 2
		//		mesh.indices[(x + z * (texture.width - 1)) * 6 + 3] = x + 1 + z * texture.width;
		//		mesh.indices[(x + z * (texture.width - 1)) * 6 + 4] = x + (z + 1) * texture.width;
		//		mesh.indices[(x + z * (texture.width - 1)) * 6 + 5] = x + 1 + (z + 1) * texture.width;
		//	}
		//}

		//// Now loop through each vertex vector, and average out all the normals stored.
		//for (int i = 0; i < mesh.vertices.size(); ++i)
		//{
		//	mesh.vertices[i].Normal = glm::normalize(mesh.vertices[i].Normal);
		//}

		//terrain->AddMesh(mesh);
		//terrain->BuildBuffers(vulkanBase);

		//// Add to the map
		//mModelMap[filename] = terrain;

		//return terrain;
	}

	StaticModel* ModelLoader::LoadDebugBox(Device* device)
	{
		// Check if the model already is loaded
		if (mModelMap.find("debug_box") != mModelMap.end())
			return mModelMap["debug_box"];

		StaticModel* model = new StaticModel();
		Mesh* mesh = new Mesh(device);

		// Front
		mesh->AddVertex(-0.5f, -0.5f, 0.5f);
		mesh->AddVertex(0.5f, -0.5f, 0.5f);
		mesh->AddVertex(0.5f, 0.5f, 0.5f);
		mesh->AddVertex(-0.5f, 0.5f, 0.5f);

		// Back
		mesh->AddVertex(-0.5f, -0.5f, -0.5f);
		mesh->AddVertex(0.5f, -0.5f, -0.5f);
		mesh->AddVertex(0.5f, 0.5f, -0.5f);
		mesh->AddVertex(-0.5f, 0.5f, -0.5f);

		// Front
		mesh->AddIndex(0, 2, 1);
		mesh->AddIndex(2, 0, 3);

		// Top
		mesh->AddIndex(1, 6, 5);
		mesh->AddIndex(6, 1, 2);

		// Back
		mesh->AddIndex(7, 5, 6);
		mesh->AddIndex(5, 7, 4);

		// Bottom
		mesh->AddIndex(4, 3, 0);
		mesh->AddIndex(3, 4, 7);

		// Left
		mesh->AddIndex(4, 1, 5);
		mesh->AddIndex(1, 4, 0);

		// Right
		mesh->AddIndex(3, 6, 2);
		mesh->AddIndex(6, 3, 7);

		mesh->BuildBuffers(device);		
		model->AddMesh(mesh);

		model->Init(device);
		mModelMap["debug_box"] = model;
		return model;
	}

	// From /assimp/assimp/tools/assimp_view/Material.cpp
	int ModelLoader::FindValidPath(aiString* texturePath, std::string modelPath)
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
	bool ModelLoader::TryLongerPath(char* szTemp, aiString* p_szString)
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

							if (0 == memcmp(info.cFileName, szFile2, min(iSizeFound, iSize)))
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
}	// VulkanLib namespace