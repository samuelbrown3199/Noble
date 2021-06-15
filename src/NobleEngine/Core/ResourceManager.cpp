#include "ResourceManager.h"

namespace NobleCore
{
	std::vector<std::shared_ptr<Resource>> ResourceManager::resources;

	void ResourceManager::UnloadUnusedResources()
	{
		for (size_t re = 0; re < resources.size(); re++)
		{
			if (resources.at(re).use_count() == 1)
			{
				resources.erase(resources.begin() + re);
			}
		}
	}
}