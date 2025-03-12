/*
	Copyright 2020 Limeoats
	Original project: https://github.com/Limeoats/L2DFileDialog

	Changes by Vladimir Sigalkin
	Original project: https://github.com/Iam1337/ImGui-FileDialog

	Changes by Marceli Antosik (Muppetsg2)
*/

#pragma once

#include <filesystem>
#include <imgui.h>
#include <string>
#include <unordered_map>
#include <vector>

typedef int ImGuiFileDialogType;	// -> enum ImGuiFileDialogType_        // Enum: A file dialog type

enum ImGuiFileDialogType_
{
	ImGuiFileDialogType_OpenFile,
	ImGuiFileDialogType_SaveFile,
	ImGuiFileDialogType_COUNT,
	ImGuiFileDialogType_SelectFolder
};

template <typename Key, typename Value>
class ordered_map {
private:
	std::unordered_map<Key, Value> map;
	std::vector<Key> keys;

public:
	void insert(const Key& key, const Value& value) {
		if (map.find(key) == map.end()) {
			keys.push_back(key);
		}
		map[key] = value;
	}

	bool contains(const Key& key) const {
		return map.find(key) != map.end();
	}

	Value& operator[](const Key& key) {
		return map[key];
	}

	void clear() {
		map.clear();
		keys.clear();
	}

	Value& at(size_t index) {
		if (index >= keys.size()) {
			throw std::out_of_range("Index out of range");
		}
		const Key& key = keys[index];
		return map.at(key);
	}

	const Key& keyAt(size_t index) const {
		if (index >= keys.size()) {
			throw std::out_of_range("Index out of range");
		}
		return keys[index];
	}

	size_t size() const {
		return keys.size();
	}

	bool empty() const {
		return keys.empty();
	}

	void remove(const Key& key) {
		auto it = map.find(key);
		if (it != map.end()) {
			map.erase(it);
			keys.erase(std::remove(keys.begin(), keys.end(), key), keys.end());
		}
	}
};

struct ImFileDialogInfo
{
	std::string title;
	ImGuiFileDialogType type;
	std::vector<std::string> filters;

	std::filesystem::path fileName;
	std::filesystem::path directoryPath;
	std::filesystem::path resultPath;

	bool refreshInfo;
	size_t currentIndex;
	size_t currentFilterIndex;
	ordered_map<std::string, std::vector<std::string>> parsedFilters;
	std::vector<std::filesystem::directory_entry> currentFiles;
	std::vector<std::filesystem::directory_entry> currentDirectories;

};

namespace ImGui
{
	IMGUI_API bool FileDialog(bool* open, ImFileDialogInfo* dialogInfo);
}