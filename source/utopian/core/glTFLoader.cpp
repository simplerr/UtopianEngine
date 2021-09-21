#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include "glTFLoader.h"
#include "core/renderer/Model.h"
#include "core/renderer/SkinAnimator.h"
#include "core/Log.h"
#include "core/ModelLoader.h"
#include "tinygltf/tiny_gltf.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"

namespace Utopian
{
   class Model;
   
   glTFLoader::glTFLoader(Vk::Device* device)
      : mDevice(device)
   {
      CreateDescriptorPools();
   }

   glTFLoader::~glTFLoader()
   {
   }

   void glTFLoader::CreateDescriptorPools()
   {
      mMeshSkinningDescriptorSetLayout = std::make_shared<Vk::DescriptorSetLayout>(mDevice);
      mMeshSkinningDescriptorSetLayout->AddStorageBuffer(0, VK_SHADER_STAGE_ALL, 1); // jointMatrices
      mMeshSkinningDescriptorSetLayout->Create();

      mMeshSkinningDescriptorPool = std::make_shared<Vk::DescriptorPool>(mDevice);
      mMeshSkinningDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100);
      mMeshSkinningDescriptorPool->Create();
   }

   SharedPtr<Model> glTFLoader::LoadModel(std::string filename, Vk::Device* device)
   {
      tinygltf::Model glTFInput;
      tinygltf::TinyGLTF gltfContext;
      std::string error, warning;

      bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

      SharedPtr<Model> model = nullptr;
      if (fileLoaded)
      {
         model = std::make_shared<Model>();
         model->SetFilename(filename);

         LoadMaterials(glTFInput, model.get());

         const tinygltf::Scene& scene = glTFInput.scenes[0];
         for (size_t i = 0; i < scene.nodes.size(); i++)
         {
            const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
            LoadNode(model.get(), node, glTFInput, nullptr, scene.nodes[i], device);
         }

         if (glTFInput.skins.size() > 0)
         {
            SharedPtr<SkinAnimator> skinAnimator = std::make_shared<SkinAnimator>(glTFInput, model.get(), device);
            skinAnimator->CreateSkinningDescriptorSet(device, mMeshSkinningDescriptorSetLayout.get(), mMeshSkinningDescriptorPool.get());
            model->AddSkinAnimator(skinAnimator);
         }
      }
      else
      {
         UTO_LOG("Failed to load glTF model " + filename);
      }

      model->Init();

      return model;
   }

   void glTFLoader::LoadMaterials(tinygltf::Model& input, Model* model)
   {
      std::vector<SharedPtr<Vk::Texture>> images;
      std::vector<int32_t> imageRefs;

      for (size_t i = 0; i < input.images.size(); i++)
      {
         tinygltf::Image& glTFImage = input.images[i];

         SharedPtr<Vk::Texture> texture = Vk::gTextureLoader().CreateTexture(&glTFImage.image[0], VK_FORMAT_R8G8B8A8_UNORM,
                                                                             glTFImage.width, glTFImage.height, 1, sizeof(uint32_t));
         images.push_back(texture);
      }

      imageRefs.resize(input.textures.size());
      for (size_t i = 0; i < input.textures.size(); i++)
      {
         imageRefs[i] = input.textures[i].source;
      }

      for (size_t i = 0; i < input.materials.size(); i++)
      {
         tinygltf::Material glTFMaterial = input.materials[i];

         Material material = GetDefaultMaterial();

         if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
            material.properties->data.baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
         }
         if (glTFMaterial.values.find("roughnessFactor") != glTFMaterial.values.end()) {
            material.properties->data.roughnessFactor = static_cast<float>(glTFMaterial.values["roughnessFactor"].Factor());
         }
         if (glTFMaterial.values.find("metallicFactor") != glTFMaterial.values.end()) {
            material.properties->data.metallicFactor = static_cast<float>(glTFMaterial.values["metallicFactor"].Factor());
         }
         if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
            uint32_t baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
            material.colorTexture = images[imageRefs[baseColorTextureIndex]];
         }
         if (glTFMaterial.values.find("metallicRoughnessTexture") != glTFMaterial.values.end()) {
            uint32_t metallicRoughnessTextureIndex = glTFMaterial.values["metallicRoughnessTexture"].TextureIndex();
            material.metallicRoughnessTexture = images[imageRefs[metallicRoughnessTextureIndex]];
         }
         if (glTFMaterial.additionalValues.find("normalTexture") != glTFMaterial.additionalValues.end()) {
            uint32_t normalTextureIndex = glTFMaterial.additionalValues["normalTexture"].TextureIndex();
            material.normalTexture = images[imageRefs[normalTextureIndex]];
         }

         material.properties->UpdateMemory();

         material.descriptorSet->BindCombinedImage(0, material.colorTexture->GetDescriptor());
         material.descriptorSet->BindCombinedImage(1, material.normalTexture->GetDescriptor());
         material.descriptorSet->BindCombinedImage(2, material.specularTexture->GetDescriptor());
         material.descriptorSet->BindCombinedImage(3, material.metallicRoughnessTexture->GetDescriptor());
         material.descriptorSet->BindUniformBuffer(20, material.properties->GetDescriptor());
         material.descriptorSet->UpdateDescriptorSets();

         model->AddMaterial(material);
      }
   }

   void glTFLoader::LoadNode(Model* model, const tinygltf::Node& inputNode, const tinygltf::Model& input,
                             Node* parent, uint32_t nodeIndex, Vk::Device* device)
   {
      Node* node = model->CreateNode();
      node->parent = parent;
      node->matrix = glm::mat4(1.0f);
      node->index = nodeIndex;
      node->skin = inputNode.skin;
      node->name = inputNode.name;

      // Get the local node matrix
      // It's either made up from translation, rotation, scale or a 4x4 matrix
      if (inputNode.translation.size() == 3) {
         node->translation = glm::make_vec3(inputNode.translation.data());

         // Todo: remove
         if (mInverseTranslation)
            node->translation *= -1;

      }
      if (inputNode.rotation.size() == 4) {
         glm::quat q = glm::make_quat(inputNode.rotation.data());
         node->rotation = glm::mat4(q);
      }
      if (inputNode.scale.size() == 3) {
         node->scale = glm::make_vec3(inputNode.scale.data());
      }
      if (inputNode.matrix.size() == 16) {
         node->matrix = glm::make_mat4x4(inputNode.matrix.data());

         // Todo: remove
         if (mInverseTranslation)
         {
            node->matrix[3][0] = -node->matrix[3][0];
            node->matrix[3][1] = -node->matrix[3][1];
            node->matrix[3][2] = -node->matrix[3][2];
         }
      }

      // Load children
      if (inputNode.children.size() > 0) {
         for (size_t i = 0; i < inputNode.children.size(); i++) {
            LoadNode(model, input.nodes[inputNode.children[i]], input , node, inputNode.children[i], device);
         }
      }

      if (inputNode.mesh > -1)
      {
         // Iterate through all primitives of this node's mesh
         const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
         for (size_t i = 0; i < mesh.primitives.size(); i++)
         {
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            Primitive primitive;
            AppendVertexData(input, glTFPrimitive, &primitive);

            bool hasIndices = (glTFPrimitive.indices != -1);
            if (hasIndices)
               AppendIndexData(input, glTFPrimitive, &primitive);

            primitive.BuildBuffers(device);

            Primitive* addedPrimitive = model->AddPrimitive(primitive);
            Material* material = nullptr;

            // Use the default material of missing from the primitive in the .gltf file
            if (glTFPrimitive.material == -1)
               material = model->AddMaterial(GetDefaultMaterial());
            else
               material = model->GetMaterial(glTFPrimitive.material);

            node->mesh.AddPrimitive(addedPrimitive, material); 
         }
      }

      if (parent) {
         parent->children.push_back(node);
      }
      else {
         model->AddRootNode(node);
      }
   }

   void glTFLoader::AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Primitive* primitive)
   {
      const float* positionBuffer = nullptr;
      const float* normalsBuffer = nullptr;
      const float* texCoordsBuffer = nullptr;
      const float* tangentsBuffer = nullptr;
      const uint16_t* jointIndicesBuffer = nullptr;
      const float* jointWeightsBuffer = nullptr;
      size_t vertexCount = 0;

      if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
         vertexCount = accessor.count;
      }
      if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }
      if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }
      if (glTFPrimitive.attributes.find("TANGENT") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TANGENT")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         tangentsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }
      if (glTFPrimitive.attributes.find("JOINTS_0") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("JOINTS_0")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         jointIndicesBuffer = reinterpret_cast<const uint16_t*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }
      if (glTFPrimitive.attributes.find("WEIGHTS_0") != glTFPrimitive.attributes.end())
      {
         const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("WEIGHTS_0")->second];
         const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
         jointWeightsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
      }

      bool hasSkin = (jointIndicesBuffer && jointWeightsBuffer);

      for (size_t v = 0; v < vertexCount; v++)
      {
         Vk::Vertex vert = {};
         vert.pos = glm::make_vec3(&positionBuffer[v * 3]);
         vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
         vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
         vert.tangent = tangentsBuffer ? glm::make_vec4(&tangentsBuffer[v * 4]) : glm::vec4(0.0f);
         vert.color = glm::vec3(1.0f);
         vert.jointIndices = hasSkin ? glm::vec4(glm::make_vec4(&jointIndicesBuffer[v * 4])) : glm::vec4(0.0f);
         vert.jointWeights = hasSkin ? glm::make_vec4(&jointWeightsBuffer[v * 4]) : glm::vec4(0.0f);
         primitive->AddVertex(vert);
      }

      if (normalsBuffer == nullptr)
      {
         UTO_LOG("Missing normals for model, calculating flat normals");
         for (size_t v = 0; v < primitive->GetNumVertices(); v += 3)
         {
            std::vector<Vk::Vertex>& vertices = primitive->vertices;
            glm::vec3 v1 = vertices[v].pos - vertices[v+1].pos;
            glm::vec3 v2 = vertices[v].pos - vertices[v+2].pos;
            glm::vec3 normal = glm::cross(v1, v2);
            vertices[v].normal = normal;
            vertices[v+1].normal = normal;
            vertices[v+2].normal = normal;
         }
      }
   }

   void glTFLoader::AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Primitive* primitive)
   {
      const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
      const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
      const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

      uint32_t indexCount = static_cast<uint32_t>(accessor.count);

      // glTF supports different component types of indices
      switch (accessor.componentType) {
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
         uint32_t* buf = new uint32_t[accessor.count];
         memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
         for (size_t index = 0; index < accessor.count; index += 3) {
            primitive->AddTriangle(buf[index], buf[index+1], buf[index+2]);
         }
         break;
      }
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
         uint16_t* buf = new uint16_t[accessor.count];
         memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
         for (size_t index = 0; index < accessor.count; index += 3) {
            primitive->AddTriangle(buf[index], buf[index+1], buf[index+2]);
         }
         break;
      }
      case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
         uint8_t* buf = new uint8_t[accessor.count];
         memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
         for (size_t index = 0; index < accessor.count; index += 3) {
            primitive->AddTriangle(buf[index], buf[index+1], buf[index+2]);
         }
         break;
      }
      default:
         UTO_LOG("Index component type " + std::to_string(accessor.componentType) + " not supported!");
         assert(0);
      }
   }

   Material glTFLoader::GetDefaultMaterial()
   {
      Material material;

      material.descriptorSet = std::make_shared<Vk::DescriptorSet>(mDevice, gModelLoader().GetMeshTextureDescriptorSetLayout(),
                                                                   gModelLoader().GetMeshTextureDescriptorPool());

      material.properties->Create(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
      material.properties->UpdateMemory();

      material.colorTexture = Vk::gTextureLoader().LoadTexture(DEFAULT_COLOR_TEXTURE_PATH);
      material.normalTexture = Vk::gTextureLoader().LoadTexture(DEFAULT_NORMAL_MAP_TEXTURE);
      material.specularTexture = Vk::gTextureLoader().LoadTexture(DEFAULT_SPECULAR_MAP_TEXTURE);
      material.metallicRoughnessTexture = Vk::gTextureLoader().LoadTexture(DEFAULT_METALLIC_ROUGHNESS_TEXTURE);

      material.descriptorSet->BindCombinedImage(0, material.colorTexture->GetDescriptor());
      material.descriptorSet->BindCombinedImage(1, material.normalTexture->GetDescriptor());
      material.descriptorSet->BindCombinedImage(2, material.specularTexture->GetDescriptor());
      material.descriptorSet->BindCombinedImage(3, material.metallicRoughnessTexture->GetDescriptor());
      material.descriptorSet->BindUniformBuffer(20, material.properties->GetDescriptor());
      material.descriptorSet->UpdateDescriptorSets();

      return material;
   }

   void glTFLoader::SetInverseTranslation(bool inverse)
   {
      mInverseTranslation = inverse;
   }
}
