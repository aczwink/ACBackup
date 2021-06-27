/*
 * Copyright (c) 2020-2021 Amir Czwink (amir130@hotmail.de)
 *
 * This file is part of ACBackup.
 *
 * ACBackup is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ACBackup is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ACBackup.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "config/ConfigManager.hpp"
#include "config/CompressionStatistics.hpp"

using namespace StdXX;

class InjectionContainer
{
public:
	//Properties
	inline CompressionStatistics& CompressionStats()
	{
		return *this->compressionStatistics;
	}

	inline void CompressionStats(CompressionStatistics* compressionStatistics)
	{
		this->compressionStatistics = compressionStatistics;
	}

	inline const struct Config& Config() const
	{
		return this->ConfigManager().Config();
	}

	inline void ConfigManager(class ConfigManager* configManager)
	{
		this->configManager = configManager;
	}

	inline const class ConfigManager& ConfigManager() const
	{
		return *this->configManager;
	}

	inline class StatusTracker& StatusTracker()
	{
		return *this->statusTracker;
	}

	inline void StatusTracker(class StatusTracker* statusTracker)
	{
		this->statusTracker = statusTracker;
	}

	inline StaticThreadPool& TaskQueue()
	{
		return *this->taskQueue;
	}

	inline void TaskQueue(uint32 nWorkers)
	{
		this->taskQueue = new StaticThreadPool(nWorkers);
	}

	//Inline
	inline void UnregisterAll()
	{
		this->compressionStatistics = nullptr;
		this->configManager = nullptr;
		this->statusTracker = nullptr;
		this->taskQueue = nullptr;
	}

	//Static
	inline static InjectionContainer& Instance()
	{
		static InjectionContainer instance;
		return instance;
	}

private:
	//Members
	CompressionStatistics* compressionStatistics;
	class ConfigManager* configManager;
	UniquePointer<class StatusTracker> statusTracker;
	UniquePointer<StaticThreadPool> taskQueue;

	//Constructor
	InjectionContainer() = default;
};