#include "vk_shader.h"

#include <fstream>
#include <sstream>

#include <spirv_reflect.h>

#include "vk_initializers.h"
#include "vk_descriptors.h"

bool vkutil::load_shader_module(VkDevice device, const std::string& filePath, ShaderModule* outShaderModule)
{
	const std::string path(filePath);
	//open the file, with cursor at the end
	std::ifstream file(path,std::ios::ate | std::ios::binary);
		
	if (!file.is_open())
	{
		return false;
	}

	//find the size of the file by looking up the location of the cursor
	//because it is at the end, it gives the size direcly in bytes
	size_t fileSize = (size_t)file.tellg();

	//spir-v expects the buffer to be on uint32, so we have to reserve an int vector big enough for the entire file
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	//put file cursor at the beginning
	file.seekg(0);

	//load the entire file into the buffer
	file.read((char*)buffer.data(),fileSize);

	file.close();

	//create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	//codeSize has to be in bytes, so multiply the ints in the buffer by sizeof uint32 to know the real size of the buffer
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	//check that the creation goes well
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device,& createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		return false;
	}
	outShaderModule->_code = std::move(buffer);
	outShaderModule->_module = shaderModule;
	
	return true;
}

constexpr uint32_t fnv1a_32(char const* s, std::size_t count)
{
	return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count] * 16777619u);
}

uint32_t vkutil::hash_descriptor_layout_info(VkDescriptorSetLayoutCreateInfo *info)
{
	std::stringstream ss;
	ss << info->flags;
	ss << info->bindingCount;

	for(uint32_t i = 0 ; i < info->bindingCount ; i++)
	{
		const VkDescriptorSetLayoutBinding& binding = info->pBindings[i];
		ss << binding.binding;
		ss << binding.descriptorCount;
		ss << binding.descriptorType;
		ss << binding.stageFlags;
	}
	std::string str = ss.str();

    return fnv1a_32(str.c_str(), str.length());
}

void ShaderEffect::add_stage(ShaderModule *shaderModule, VkShaderStageFlagBits stage)
{
	ShaderStage newStage = {shaderModule, stage};
	stages.push_back(newStage);
}

struct DescriptorSetLayoutData
{
	uint32_t setNumber;
	VkDescriptorSetLayoutCreateInfo createInfo;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

void ShaderEffect::reflect_layout(VkDevice device,vkutil::DescriptorLayoutCache& descriptorLayoutCache, ReflectionOverrides *overrides, int overrideCount)
{
	std::vector<DescriptorSetLayoutData> layoutsData;
	std::vector<VkPushConstantRange> constantRanges;
	int layoutCount = 0;

	for(auto& stage : stages)
	{
		SpvReflectShaderModule spvModule;
		SpvReflectResult result = spvReflectCreateShaderModule(stage._module->_code.size() * sizeof(uint32_t), stage._module->_code.data(), &spvModule);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		uint32_t count = 0;
		result = spvReflectEnumerateDescriptorSets(&spvModule, &count, nullptr);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		std::vector<SpvReflectDescriptorSet*> sets(count);
		result = spvReflectEnumerateDescriptorSets(&spvModule, &count, sets.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);

		for (const auto& set : sets)
		{
			DescriptorSetLayoutData layout = {};
			layout.bindings.resize(set->binding_count);
			for (size_t i_binding = 0; i_binding < set->binding_count; i_binding++)
			{
				const SpvReflectDescriptorBinding& binding = *(set->bindings[i_binding]);
				VkDescriptorSetLayoutBinding& layoutBinding = layout.bindings[i_binding];
				layoutBinding.binding = binding.binding;
				layoutBinding.descriptorType = static_cast<VkDescriptorType>(binding.descriptor_type);
				for (size_t ov = 0; ov < overrideCount; ov++)
				{
					if(strcmp(binding.name, overrides[ov]._name) == 0)
					{
						layoutBinding.descriptorType = overrides[ov]._type;
					}
				}
				layoutBinding.descriptorCount = 1;
				for (size_t i_dim = 0; i_dim < binding.array.dims_count; i_dim++)
				{
					layoutBinding.descriptorCount *= binding.array.dims[i_dim];
				}
				layoutBinding.stageFlags = static_cast<VkShaderStageFlagBits>(spvModule.shader_stage);

				ReflectedBinding reflected;
				reflected.binding = layoutBinding.binding;
				reflected.set = set->set;
				reflected.type = layoutBinding.descriptorType;
				bindings[binding.name] = reflected;
			}
			layout.setNumber = set->set;
			layout.createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layout.createInfo.bindingCount = set->binding_count;
			layout.createInfo.pBindings = layout.bindings.data();
			layoutsData.push_back(layout);
		}
		
		//pushconstants

		result = spvReflectEnumeratePushConstants(&spvModule,&count,nullptr);
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		std::vector<SpvReflectBlockVariable*> pConstants(count);
		result = spvReflectEnumeratePushConstants(&spvModule,&count,pConstants.data());
		assert(result == SPV_REFLECT_RESULT_SUCCESS);
		if(count > 0)
		{
			VkPushConstantRange pcs{};
			pcs.offset = pConstants[0]->offset;
			pcs.size = pConstants[0]->size;
			pcs.stageFlags = stage.stage;
			constantRanges.push_back(pcs);
		}
	}

	std::array<DescriptorSetLayoutData,4> mergedLayouts;

	for (size_t i = 0; i < 4; i++)
	{
		DescriptorSetLayoutData& layout = mergedLayouts[i];
		layout.setNumber = i;
		layout.createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	
		std::unordered_map<int, VkDescriptorSetLayoutBinding> binds;
		for(auto& set : layoutsData)
		{
			if(set.setNumber == i)
			{
				for(auto& binding : set.bindings)
				{
					auto it = binds.find(binding.binding);
					if (it == binds.end())
					{
						binds[binding.binding] = binding;
					}
					else
					{
						binds[binding.binding].stageFlags |= binding.stageFlags;
					}
				}
			}			
		}
		for (auto [k, v] : binds)
		{
			layout.bindings.push_back(v);
		}

		std::sort(layout.bindings.begin(), layout.bindings.end(),[](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b){
			return a.binding < b.binding;
		});

		layout.createInfo.bindingCount = uint32_t(layout.bindings.size());
		layout.createInfo.pBindings = layout.bindings.data();
		layout.createInfo.flags = 0;
		layout.createInfo.pNext = nullptr;

		if(layout.createInfo.bindingCount > 0)
		{
			setHashes[i] = vkutil::hash_descriptor_layout_info(&layout.createInfo);
			setLayouts[i] = descriptorLayoutCache.createDescriptorLayout(&layout.createInfo);
			layoutCount++;
		}
		else
		{
			setHashes[i] = 0;
			setLayouts[i] = VK_NULL_HANDLE;
		}
	}
	//create default layout info
	VkPipelineLayoutCreateInfo meshPipelineLayoutInfo = vkinit::pipeline_layout_create_info();
	meshPipelineLayoutInfo.pushConstantRangeCount = uint32_t(constantRanges.size());
	meshPipelineLayoutInfo.pPushConstantRanges = constantRanges.data();

	std::array<VkDescriptorSetLayout, 4> compactedLayouts;
	int s = 0;
	for(int i = 0 ; i < 4 ; i++)
	{
		if (setLayouts[i] != VK_NULL_HANDLE)
		{
			compactedLayouts[s] = setLayouts[i];
			s++;
		}
	}
	meshPipelineLayoutInfo.setLayoutCount = s;
	meshPipelineLayoutInfo.pSetLayouts = compactedLayouts.data();

	vkCreatePipelineLayout(device,&meshPipelineLayoutInfo,nullptr,&_builtLayout);

}

void ShaderEffect::fill_stages(std::vector<VkPipelineShaderStageCreateInfo> &pipelineStages)
{
	for (auto& s : stages)
	{
		pipelineStages.push_back(vkinit::pipeline_shader_stage_create_info(s.stage,s._module->_module));
	}
}

void ShaderDescriptorBinder::bind_buffer(const char *name, VkDescriptorBufferInfo &bufferInfo)
{
	bind_dynamic_buffer(name, -1, bufferInfo);
}

void ShaderDescriptorBinder::bind_dynamic_buffer(const char *name, uint32_t offset, const VkDescriptorBufferInfo &bufferInfo)
{
	auto found = shaders->bindings.find(name);
	if (found != shaders->bindings.end())
	{
		const ShaderEffect::ReflectedBinding& bind = (*found).second;

		for (auto& write : bufferWrites)
		{
			if (write.dstBinding == bind.binding && write.dstSet == bind.set)
			{
				if(write.bufferInfo.buffer != bufferInfo.buffer
				|| write.bufferInfo.range != bufferInfo.range
				|| write.bufferInfo.offset != bufferInfo.offset)
				{
					write.bufferInfo = bufferInfo;
					write.dynamicOffset = offset;

					cachedDescriptorSets[write.dstSet] = VK_NULL_HANDLE;
				}
				else
				{
					write.dynamicOffset = offset;					
				}
				return;
			}
		}
		BufferWriteDescriptor newWrite;
		newWrite.dstSet = bind.set;
		newWrite.dstBinding = bind.binding;
		newWrite.type = bind.type;
		newWrite.bufferInfo = bufferInfo;
		newWrite.dynamicOffset = offset;
		cachedDescriptorSets[bind.set] = VK_NULL_HANDLE;

		bufferWrites.push_back(newWrite);
	}
	else
	{
		LOG_ERROR("ShaderDescriptorBinder::bind_dynamic_buffer: Binding not found");
	}
}

void ShaderDescriptorBinder::apply_binds(VkCommandBuffer cmd)
{
	for (int i = 0 ; i < 4 ; i++)
	{
		if(cachedDescriptorSets[i] != VK_NULL_HANDLE)
		{
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaders->_builtLayout, i, 1, &cachedDescriptorSets[i], setOffsets[i].count, setOffsets[i].offsets.data());
		}
	}
}

void ShaderDescriptorBinder::build_sets(VkDevice device, vkutil::DescriptorAllocator &allocator)
{
	std::array<std::vector<VkWriteDescriptorSet>, 4> writes{};
	std::sort(bufferWrites.begin(), bufferWrites.end(), [](BufferWriteDescriptor& a, BufferWriteDescriptor& b){
		if(a.dstSet == b.dstSet)
			return a.dstBinding < b.dstBinding;
		else return a.dstSet < b.dstSet;
	});

	for(auto& offset : setOffsets)
	{
		offset.count;
	}

	for(auto& w : bufferWrites)
	{
		uint32_t set = w.dstSet;
		VkWriteDescriptorSet write = vkinit::write_descriptor_buffer(w.type, VK_NULL_HANDLE, &w.bufferInfo, w.dstBinding);
		writes[set].push_back(write);

		if(w.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC || w.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
		{
			DynOffsets& offsetSet = setOffsets[set];
			offsetSet.offsets[offsetSet.count] = w.dynamicOffset;
			offsetSet.count++;
		}
	}

	for(int i = 0 ; i < 4 ; i++)
	{
		if(writes[i].size() > 0)
		{
			if(cachedDescriptorSets[i] == VK_NULL_HANDLE)
			{
				auto& layout = shaders->setLayouts[i];
				VkDescriptorSet newDescriptor;
				allocator.allocate(&newDescriptor, layout);

				for(auto& w : writes[i])
				{
					w.dstSet = newDescriptor;
				}
				vkUpdateDescriptorSets(device, uint32_t(writes[i].size()), writes[i].data(), 0, nullptr);
				cachedDescriptorSets[i] = newDescriptor;
			}
		}
	}	
}

void ShaderDescriptorBinder::set_shader(ShaderEffect *newShader)
{
	if(shaders && shaders != newShader)
	{
		for(int i = 0 ; i < 4 ; i++)
		{
			if(newShader->setHashes[i]  != shaders->setHashes[i])
			{
				cachedDescriptorSets[i] = VK_NULL_HANDLE;
			}
			else if (newShader->setHashes[i] == 0)
			{
				cachedDescriptorSets[i] = VK_NULL_HANDLE;
			}
			
		}
	}
	else 
	{
		for(int i = 0 ; i < 4 ; i++)
			cachedDescriptorSets[i] = VK_NULL_HANDLE;
	}
	shaders = newShader;
}

ShaderModule *ShaderCache::get_shader(const std::string &path)
{
	auto it  = module_cache.find(path);
	if(it == module_cache.end())
	{
		ShaderModule newShader;
		bool result = vkutil::load_shader_module(_device, path.c_str(), &newShader);
		if(!result)
		{
			LOG_ERROR("Error when compiling shader {}", path);
			return nullptr;
		}
		module_cache[path] = newShader;
	}
    return &module_cache[path];
}
