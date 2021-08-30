#include "glTFLoader.h"
#include "glTFModel.h"
#include "SkinAnimator.h"
#include "core/Log.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include "tinygltf/tiny_gltf.h"
#include "vulkan/TextureLoader.h"
#include "vulkan/handles/DescriptorSetLayout.h"
#include "vulkan/handles/DescriptorSet.h"

namespace Utopian
{
   class glTFModel;
   
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
      mMeshTexturesDescriptorSetLayout = std::make_shared<Vk::DescriptorSetLayout>(mDevice);
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(0, VK_SHADER_STAGE_ALL, 1); // diffuseSampler
      mMeshTexturesDescriptorSetLayout->AddCombinedImageSampler(1, VK_SHADER_STAGE_ALL, 1); // normalSampler
      mMeshTexturesDescriptorSetLayout->Create();

      mMeshTexturesDescriptorPool = std::make_shared<Vk::DescriptorPool>(mDevice);
      mMeshTexturesDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 200);
      mMeshTexturesDescriptorPool->Create();

      mMeshSkinningDescriptorSetLayout = std::make_shared<Vk::DescriptorSetLayout>(mDevice);
      mMeshSkinningDescriptorSetLayout->AddStorageBuffer(0, VK_SHADER_STAGE_ALL, 1); // jointMatrices
      mMeshSkinningDescriptorSetLayout->Create();

      mMeshSkinningDescriptorPool = std::make_shared<Vk::DescriptorPool>(mDevice);
      mMeshSkinningDescriptorPool->AddDescriptor(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100);
      mMeshSkinningDescriptorPool->Create();
   }

   SharedPtr<glTFModel> glTFLoader::LoadModel(std::string filename, Vk::Device* device)
   {
      tinygltf::Model glTFInput;
      tinygltf::TinyGLTF gltfContext;
      std::string error, warning;

      bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);

      SharedPtr<glTFModel> model = nullptr;
      if (fileLoaded)
      {
         model = std::make_shared<glTFModel>();

         std::vector<Material2> materials = LoadMaterials(glTFInput, model.get());

         const tinygltf::Scene& scene = glTFInput.scenes[0];
         for (size_t i = 0; i < scene.nodes.size(); i++)
         {
            const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
            LoadNode(model.get(), node, materials, glTFInput, nullptr, scene.nodes[i], device);
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

      return model;
   }

   std::vector<Material2> glTFLoader::LoadMaterials(tinygltf::Model& input, glTFModel* model)
   {
      std::vector<SharedPtr<Vk::Texture>> images;
      std::vector<int32_t> imageRefs;
      std::vector<Material2> loadedMaterials;

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

         Material2 material;

         if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
            material.baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
         }
         if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
            uint32_t baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
            material.colorTexture = images[imageRefs[baseColorTextureIndex]];
         }
         if (glTFMaterial.additionalValues.find("normalTexture") != glTFMaterial.additionalValues.end()) {
            uint32_t normalTextureIndex = glTFMaterial.additionalValues["normalTexture"].TextureIndex();
            material.normalTexture = images[imageRefs[normalTextureIndex]];
         }

         material.descriptorSet = std::make_shared<Vk::DescriptorSet>(mDevice, mMeshTexturesDescriptorSetLayout.get(),
                                                                      mMeshTexturesDescriptorPool.get());
         material.descriptorSet->BindCombinedImage(0, material.colorTexture->GetDescriptor());

         // Todo: Use default normal map if not present
         if (material.normalTexture != nullptr)
            material.descriptorSet->BindCombinedImage(1, material.normalTexture->GetDescriptor());
         else 
            material.descriptorSet->BindCombinedImage(1, material.colorTexture->GetDescriptor());

         material.descriptorSet->UpdateDescriptorSets();

         loadedMaterials.push_back(material);
      }

      return loadedMaterials;
   }

   void glTFLoader::LoadNode(glTFModel* model, const tinygltf::Node& inputNode, std::vector<Material2>& loadedMaterials,
                             const tinygltf::Model& input, Node* parent, uint32_t nodeIndex, Vk::Device* device)
   {
      Node* node = new Node();
      node->parent = parent;
      node->matrix = glm::mat4(1.0f);
      node->index = nodeIndex;
      node->skin = inputNode.skin;
      node->name = inputNode.name;

      // Get the local node matrix
      // It's either made up from translation, rotation, scale or a 4x4 matrix
      if (inputNode.translation.size() == 3) {
         node->translation = glm::make_vec3(inputNode.translation.data());
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
      }

      // Load children
      if (inputNode.children.size() > 0) {
         for (size_t i = 0; i < inputNode.children.size(); i++) {
            LoadNode(model, input.nodes[inputNode.children[i]], loadedMaterials, input , node, inputNode.children[i], device);
         }
      }

      if (inputNode.mesh > -1)
      {
         // Iterate through all primitives of this node's mesh
         const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
         for (size_t i = 0; i < mesh.primitives.size(); i++)
         {
            const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
            Vk::Mesh* primitive = new Vk::Mesh(NULL); // Todo remove the Device* argument
            AppendVertexData(input, glTFPrimitive, primitive);

            bool hasIndices = (glTFPrimitive.indices != -1);
            if (hasIndices)
               AppendIndexData(input, glTFPrimitive, primitive);

            primitive->BuildBuffers(device);

            node->renderable.primitives.push_back(primitive);
            node->renderable.materials.push_back(loadedMaterials[glTFPrimitive.material]);
         }
      }

      if (parent) {
         parent->children.push_back(node);
      }
      else {
         model->AddNode(node);
      }
   }

   void glTFLoader::AppendVertexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Vk::Mesh* primitive)
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
            std::vector<Vk::Vertex>& vertexVector = primitive->vertexVector;
            glm::vec3 v1 = vertexVector[v].pos - vertexVector[v+1].pos;
            glm::vec3 v2 = vertexVector[v].pos - vertexVector[v+2].pos;
            glm::vec3 normal = glm::cross(v1, v2);
            vertexVector[v].normal = normal;
            vertexVector[v+1].normal = normal;
            vertexVector[v+2].normal = normal;
         }
      }
   }

   void glTFLoader::AppendIndexData(const tinygltf::Model& input, const tinygltf::Primitive& glTFPrimitive, Vk::Mesh* primitive)
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

   Material2 glTFLoader::GetDefaultMaterial()
   {
      Material2 material;

      material.descriptorSet = std::make_shared<Vk::DescriptorSet>(mDevice, mMeshTexturesDescriptorSetLayout.get(),
                                                                   mMeshTexturesDescriptorPool.get());

      // Todo: Move the default paths to Material.h
      material.colorTexture = Vk::gTextureLoader().LoadTexture("data/textures/prototype/Light/texture_12.png");
      material.normalTexture = Vk::gTextureLoader().LoadTexture(DEFAULT_NORMAL_MAP_TEXTURE);

      material.descriptorSet->BindCombinedImage(0, material.colorTexture->GetDescriptor());
      material.descriptorSet->BindCombinedImage(1, material.normalTexture->GetDescriptor());
      material.descriptorSet->UpdateDescriptorSets();

      return material;
   }
}
