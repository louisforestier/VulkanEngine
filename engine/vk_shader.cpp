#include "vk_shader.h"

#include <fstream>

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
	
		for(auto& set : layoutsData)
		{
			if(set.setNumber == i)
			{
				for(auto& binding : set.bindings)
				{
					layout.bindings.push_back(binding);
				}
			}			
		}
		layout.createInfo.bindingCount = layout.bindings.size();
		layout.createInfo.pBindings = layout.bindings.data();
		layout.createInfo.flags = 0;
		layout.createInfo.pNext = nullptr;

		if(layout.createInfo.bindingCount > 0)
		{
			setLayouts[i] = descriptorLayoutCache.createDescriptorLayout(&layout.createInfo);
			layoutCount++;
		}
		else
		{
			setLayouts[i] = VK_NULL_HANDLE;
		}
	}
	//create default layout info
	VkPipelineLayoutCreateInfo meshPipelineLayoutInfo = vkinit::pipeline_layout_create_info();
	meshPipelineLayoutInfo.pushConstantRangeCount = constantRanges.size();
	meshPipelineLayoutInfo.pPushConstantRanges = constantRanges.data();
	meshPipelineLayoutInfo.setLayoutCount = layoutCount;
	meshPipelineLayoutInfo.pSetLayouts = setLayouts.data();

	vkCreatePipelineLayout(device,&meshPipelineLayoutInfo,nullptr,&_builtLayout);

}