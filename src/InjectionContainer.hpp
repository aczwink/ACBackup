/*
 * Copyright (c) 2020 Amir Czwink (amir130@hotmail.de)
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
#include <typeindex>

#include <Std++.hpp>
using namespace StdXX;

class InjectionContainer
{
public:
	//Inline
	template<typename T>
	inline T& Get()
	{
		return *(T*)this->instances[typeid(T)];
	}

	template<typename T>
	inline void Register(T& instance)
	{
		this->instances[typeid(T)] = &instance;
	}

	inline void UnregisterAll()
	{
		this->instances.Release();
	}

	//Static
	inline static InjectionContainer& Instance()
	{
		static InjectionContainer instance;
		return instance;
	}

private:
	//Members
	Map<std::type_index, void*> instances;

	//Constructor
	InjectionContainer() = default;
};