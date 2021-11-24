#pragma once

#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <vector>

namespace util
{

	template <typename K, typename V>
	void insert(std::map<K, std::set<V>>& map, K key, V value)
	{
		auto it = map.find(key);
		if (it == map.end())
		{
			it = map.insert(it, { key, std::set<V>{} });
		}
		it->second.insert(value);
	}

	template <typename K, typename V>
	void insert(std::map<K, std::unordered_set<V>>& map, K key, V value)
	{
		auto it = map.find(key);
		if (it == map.end())
		{
			it = map.insert(it, { key, std::unordered_set<V>{} });
		}
		it->second.insert(value);
	}

	template <typename K, typename V>
	void insert(std::map<K, std::vector<V>>& map, K key, V value)
	{
		auto it = map.find(key);
		if (it == map.end())
		{
			it = map.insert(it, { key, std::vector<V>{} });
		}
		it->second.push_back(value);
	}

}