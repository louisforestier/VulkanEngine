#include <vk_engine.h>
#include <logger.h>

int main(int argc, char* argv[])
{
	Logger::Get().set_time();

	if (argc != 3) 
	{
		LOG_ERROR("application must be called with shader directory path and assets directory path in command line.");
		return 1;
	}

	VulkanEngine engine(argv[1], argv[2]);

	engine.init();	
	
	engine.run();	

	engine.cleanup();	

	return 0;
}
