#pragma once
#include <unordered_map>
#include <string>

namespace assets 
{
	enum class TransparencyMode:uint8_t 
    {
		Opaque,
		Transparent,
		Masked
	};

	struct MaterialInfo
	{
		std::string baseEffect;
		std::unordered_map<std::string, std::string> textures; //name to path 
		std::unordered_map<std::string, std::string> customProperties;
		TransparencyMode transparency; 
	};
}