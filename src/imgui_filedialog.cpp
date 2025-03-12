/*
	Copyright 2020 Limeoats
	Original project: https://github.com/Limeoats/L2DFileDialog

	Changes by Vladimir Sigalkin
	Original project: https://github.com/Iam1337/ImGui-FileDialog

	Changes by Marceli Antosik (Muppetsg2)
*/

#include <chrono>
#include <sstream>

#include <imgui.h>

#include "imgui_filedialog.h"

using namespace std::chrono_literals;

typedef int ImGuiFileDialogSortOrder;

enum ImGuiFileDialogSortOrder_
{
	ImGuiFileDialogSortOrder_Up,
	ImGuiFileDialogSortOrder_Down,
	ImGuiFileDialogSortOrder_None
};

void ParseFilters(ImFileDialogInfo* dialogInfo)
{
	dialogInfo->parsedFilters.clear();
	dialogInfo->currentFilterIndex = 0;

	std::vector<std::string> actualFilters = dialogInfo->filters;
	if (actualFilters.empty()) {
		actualFilters.push_back("All Files (*.*)|.*");
	}

	for (size_t i = 0; i < actualFilters.size(); ++i) {
		std::string activeFilter = actualFilters[i];
		std::string filterName = activeFilter.substr(0, activeFilter.find("|"));
		std::string filterExtensions = activeFilter.substr(activeFilter.find("|") + 1);

		dialogInfo->parsedFilters.insert(filterName, {});

		bool hasWildcard = false;
		std::vector<std::string> extensions;

		size_t pos = 0;
		while ((pos = filterExtensions.find("|")) != std::string::npos) {
			std::string ext = filterExtensions.substr(0, pos);
			filterExtensions.erase(0, pos + 1);

			if (ext == ".*") {
				hasWildcard = true;
				break;
			}

			extensions.push_back(ext);
		}

		if (!hasWildcard && !filterExtensions.empty()) {
			std::string ext = filterExtensions;

			if (ext == ".*") {
				hasWildcard = true;
			}
			else {
				extensions.push_back(ext);
			}
		}

		if (hasWildcard) {
			dialogInfo->parsedFilters[filterName].push_back("*");
		}
		else {
			dialogInfo->parsedFilters[filterName].insert(dialogInfo->parsedFilters[filterName].end(), extensions.begin(), extensions.end());

			if (dialogInfo->parsedFilters[filterName].empty())
				dialogInfo->parsedFilters.remove(filterName);
		}
	}
}

void RefreshInfo(ImFileDialogInfo* dialogInfo)
{
	dialogInfo->refreshInfo = false;
	dialogInfo->currentIndex = 0;
	dialogInfo->currentFiles.clear();
	dialogInfo->currentDirectories.clear();

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dialogInfo->directoryPath))
	{
		if (entry.is_directory())
		{
			dialogInfo->currentDirectories.push_back(entry);
		}
		else
		{
			if (dialogInfo->type != ImGuiFileDialogType_::ImGuiFileDialogType_SelectFolder) {
				std::string extension = entry.path().extension().string();
				bool validExtension = false;

				if (dialogInfo->parsedFilters.size() > 0) {
					std::vector extensions = dialogInfo->parsedFilters.at(dialogInfo->currentFilterIndex);
					for (size_t i = 0; i < extensions.size(); ++i)
					{
						if (extensions[i] == "*" || extensions[i] == extension)
						{
							validExtension = true;
							break;
						}
					}
				}
				else {
					validExtension = true;
				}

				if (validExtension)
				{
					dialogInfo->currentFiles.push_back(entry);
				}
			}
		}
	}
}

bool ImGui::FileDialog(bool* open, ImFileDialogInfo* dialogInfo)
{
	if (!*open) return false;

	static float initialSpacingColumn0 = 230.0f;
	static float initialSpacingColumn1 = 80.0f;
	static float initialSpacingColumn2 = 90.0f;
	static ImGuiFileDialogSortOrder fileNameSortOrder = ImGuiFileDialogSortOrder_None;
	static ImGuiFileDialogSortOrder sizeSortOrder = ImGuiFileDialogSortOrder_None;
	static ImGuiFileDialogSortOrder dateSortOrder = ImGuiFileDialogSortOrder_None;
	static ImGuiFileDialogSortOrder typeSortOrder = ImGuiFileDialogSortOrder_None;

	static ImVec4 textErrorColor = ImVec4(1.0, 0.0, 0.0, 1.0);

	static bool folderSelectError = false;
	static bool fileChooseError = false;

	assert(dialogInfo != nullptr);

	bool complete = false;

	ImGui::PushID(dialogInfo);
	ImGui::SetNextWindowSize(ImVec2(740.0f, 410.0f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin(dialogInfo->title.c_str(), open)) {
		ImGui::End();
		ImGui::PopID();
		return false;
	}

	if (dialogInfo->parsedFilters.empty())
		ParseFilters(dialogInfo);

	if (dialogInfo->currentFiles.empty() && dialogInfo->currentDirectories.empty() || dialogInfo->refreshInfo)
		RefreshInfo(dialogInfo);

	// Draw path
	ImGui::Text("Path: %s", dialogInfo->directoryPath.string().c_str());

	ImGui::BeginChild("##browser", ImVec2(ImGui::GetContentRegionAvail().x, 300), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
	ImGui::Columns(4);

	// Columns size
	if (initialSpacingColumn0 > 0)
	{
		ImGui::SetColumnWidth(0, initialSpacingColumn0);
		initialSpacingColumn0 = 0.0f;
	}
	if (initialSpacingColumn1 > 0)
	{
		ImGui::SetColumnWidth(1, initialSpacingColumn1);
		initialSpacingColumn1 = 0.0f;
	}
	if (initialSpacingColumn2 > 0)
	{
		ImGui::SetColumnWidth(2, initialSpacingColumn2);
		initialSpacingColumn2 = 0.0f;
	}

	// File Columns
	if (ImGui::Selectable("Name"))
	{
		sizeSortOrder = ImGuiFileDialogSortOrder_None;
		dateSortOrder = ImGuiFileDialogSortOrder_None;
		typeSortOrder = ImGuiFileDialogSortOrder_None;
		fileNameSortOrder = fileNameSortOrder == ImGuiFileDialogSortOrder_Down ? ImGuiFileDialogSortOrder_Up : ImGuiFileDialogSortOrder_Down;
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Size"))
	{
		fileNameSortOrder = ImGuiFileDialogSortOrder_None;
		dateSortOrder = ImGuiFileDialogSortOrder_None;
		typeSortOrder = ImGuiFileDialogSortOrder_None;
		sizeSortOrder = sizeSortOrder == ImGuiFileDialogSortOrder_Down ? ImGuiFileDialogSortOrder_Up : ImGuiFileDialogSortOrder_Down;
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Type"))
	{
		fileNameSortOrder = ImGuiFileDialogSortOrder_None;
		dateSortOrder = ImGuiFileDialogSortOrder_None;
		sizeSortOrder = ImGuiFileDialogSortOrder_None;
		typeSortOrder = typeSortOrder == ImGuiFileDialogSortOrder_Down ? ImGuiFileDialogSortOrder_Up : ImGuiFileDialogSortOrder_Down;
	}
	ImGui::NextColumn();
	if (ImGui::Selectable("Date"))
	{
		fileNameSortOrder = ImGuiFileDialogSortOrder_None;
		sizeSortOrder = ImGuiFileDialogSortOrder_None;
		typeSortOrder = ImGuiFileDialogSortOrder_None;
		dateSortOrder = dateSortOrder == ImGuiFileDialogSortOrder_Down ? ImGuiFileDialogSortOrder_Up : ImGuiFileDialogSortOrder_Down;
	}
	ImGui::NextColumn();

	// File Separator
	ImGui::Separator();

	// Sort directories
	auto* directories = &dialogInfo->currentDirectories;

	if (fileNameSortOrder != ImGuiFileDialogSortOrder_None || sizeSortOrder != ImGuiFileDialogSortOrder_None || typeSortOrder != ImGuiFileDialogSortOrder_None)
	{
		std::sort(directories->begin(), directories->end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
			{
				if (fileNameSortOrder == ImGuiFileDialogSortOrder_Down)
				{
					return a.path().filename() > b.path().filename();
				}

				return a.path().filename() < b.path().filename();
			});
	}
	else if (dateSortOrder != ImGuiFileDialogSortOrder_None)
	{
		std::sort(directories->begin(), directories->end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
			{
				if (dateSortOrder == ImGuiFileDialogSortOrder_Down)
				{
					return a.last_write_time() > b.last_write_time();
				}
				return a.last_write_time() < b.last_write_time();
			});
	}

	// Sort files
	auto* files = &dialogInfo->currentFiles;

	if (fileNameSortOrder != ImGuiFileDialogSortOrder_None)
	{
		std::sort(files->begin(), files->end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
			{
				if (fileNameSortOrder == ImGuiFileDialogSortOrder_Down)
				{
					return a.path().filename() > b.path().filename();
				}

				return a.path().filename() < b.path().filename();
			});
	}
	else if (sizeSortOrder != ImGuiFileDialogSortOrder_None)
	{
		std::sort(files->begin(), files->end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
			{
				if (sizeSortOrder == ImGuiFileDialogSortOrder_Down)
				{
					return a.file_size() > b.file_size();
				}
				return a.file_size() < b.file_size();
			});
	}
	else if (typeSortOrder != ImGuiFileDialogSortOrder_None)
	{
		std::sort(files->begin(), files->end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
			{
				if (typeSortOrder == ImGuiFileDialogSortOrder_Down)
				{
					return a.path().extension() > b.path().extension();
				}

				return a.path().extension() < b.path().extension();
			});
	}
	else if (dateSortOrder != ImGuiFileDialogSortOrder_None)
	{
		std::sort(files->begin(), files->end(), [](const std::filesystem::directory_entry& a, const std::filesystem::directory_entry& b)
			{
				if (dateSortOrder == ImGuiFileDialogSortOrder_Down)
				{
					return a.last_write_time() > b.last_write_time();
				}
				return a.last_write_time() < b.last_write_time();
			});
	}

	size_t index = 0;

	// Draw parent
	if (dialogInfo->directoryPath.has_parent_path())
	{
		if (ImGui::Selectable("..", dialogInfo->currentIndex == index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			dialogInfo->currentIndex = index;

			if (ImGui::IsMouseDoubleClicked(0))
			{
				dialogInfo->directoryPath = dialogInfo->directoryPath.parent_path();
				dialogInfo->refreshInfo = true;
			}
		}
		ImGui::NextColumn();
		ImGui::TextUnformatted("-");
		ImGui::NextColumn();
		ImGui::TextUnformatted("<parent>");
		ImGui::NextColumn();
		ImGui::TextUnformatted("-");
		ImGui::NextColumn();

		index++;
	}

	// Draw directories
	for (size_t i = 0; i < directories->size(); ++i)
	{
		auto directoryEntry = dialogInfo->currentDirectories[i];
		auto directoryPath = directoryEntry.path();
		auto directoryName = directoryPath.filename();

		if (ImGui::Selectable(directoryName.string().c_str(), dialogInfo->currentIndex == index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			dialogInfo->currentIndex = index;
			if (dialogInfo->type == ImGuiFileDialogType_SelectFolder) dialogInfo->fileName = directoryName;

			if (ImGui::IsMouseDoubleClicked(0))
			{
				dialogInfo->directoryPath = directoryPath;
				dialogInfo->refreshInfo = true;
			}
		}

		ImGui::NextColumn();
		ImGui::TextUnformatted("-");
		ImGui::NextColumn();
		ImGui::TextUnformatted("<directory>");
		ImGui::NextColumn();

		auto lastWriteTime = directoryEntry.last_write_time();
		auto st = std::chrono::time_point_cast<std::chrono::system_clock::duration>(lastWriteTime - decltype(lastWriteTime)::clock::now() + std::chrono::system_clock::now());
		std::time_t tt = std::chrono::system_clock::to_time_t(st);
		std::tm mt;
#ifdef _WIN32
		localtime_s(&mt, &tt);
#elif __linux__
		localtime_r(&tt, &mt);
#endif
		std::stringstream ss;
		ss << std::put_time(&mt, "%F %R");
		ImGui::TextUnformatted(ss.str().c_str());
		ImGui::NextColumn();

		index++;
	}

	// Draw files
	for (size_t i = 0; i < files->size(); ++i)
	{
		auto fileEntry = dialogInfo->currentFiles[i];
		auto filePath = fileEntry.path();
		auto fileName = filePath.filename();

		if (ImGui::Selectable(fileName.string().c_str(), dialogInfo->currentIndex == index, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(ImGui::GetContentRegionAvail().x, 0)))
		{
			dialogInfo->currentIndex = index;
			dialogInfo->fileName = fileName;
		}

		ImGui::NextColumn();
		ImGui::TextUnformatted(std::to_string(fileEntry.file_size()).c_str());
		ImGui::NextColumn();
		ImGui::TextUnformatted(filePath.extension().string().c_str());
		ImGui::NextColumn();

		auto lastWriteTime = fileEntry.last_write_time();
		auto st = std::chrono::time_point_cast<std::chrono::system_clock::duration>(lastWriteTime - decltype(lastWriteTime)::clock::now() + std::chrono::system_clock::now());
		std::time_t tt = std::chrono::system_clock::to_time_t(st);
		std::tm mt;
#ifdef _WIN32
		localtime_s(&mt, &tt);
#elif __linux__
		localtime_r(&tt, &mt);
#endif
		std::stringstream ss;
		ss << std::put_time(&mt, "%F %R");
		ImGui::TextUnformatted(ss.str().c_str());
		ImGui::NextColumn();

		index++;
	}
	ImGui::EndChild();

	// Draw filename
	static const size_t fileNameBufferSize = 200;
	static char fileNameBuffer[fileNameBufferSize];

	std::string fileNameStr = dialogInfo->fileName.string();
	size_t fileNameSize = fileNameStr.size();

	if (fileNameSize >= fileNameBufferSize)	fileNameSize = fileNameBufferSize - 1;
	std::memcpy(fileNameBuffer, fileNameStr.c_str(), fileNameSize);
	fileNameBuffer[fileNameSize] = 0;

	float textWidth = ImGui::CalcTextSize("File Name:").x + 5.0f;
	ImGui::Text("File Name:");
	ImGui::SameLine(textWidth + ImGui::GetStyle().ItemSpacing.x);
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::InputText("##File Name", fileNameBuffer, fileNameBufferSize))
	{
		dialogInfo->fileName = std::string(fileNameBuffer);
		dialogInfo->currentIndex = 0;
	}

	if (ImGui::Button("Cancel"))
	{
		fileNameSortOrder = ImGuiFileDialogSortOrder_None;
		sizeSortOrder = ImGuiFileDialogSortOrder_None;
		typeSortOrder = ImGuiFileDialogSortOrder_None;
		dateSortOrder = ImGuiFileDialogSortOrder_None;

		dialogInfo->refreshInfo = false;
		dialogInfo->currentIndex = 0;
		dialogInfo->currentFiles.clear();
		dialogInfo->currentDirectories.clear();

		*open = false;
	}

	ImGui::SameLine();

	if (dialogInfo->type == ImGuiFileDialogType_OpenFile)
	{
		if (ImGui::Button("Open"))
		{
			dialogInfo->resultPath = dialogInfo->directoryPath / dialogInfo->fileName;

			if (std::filesystem::exists(dialogInfo->resultPath))
			{
				if (std::filesystem::is_regular_file(dialogInfo->resultPath)) {
					fileNameSortOrder = ImGuiFileDialogSortOrder_None;
					sizeSortOrder = ImGuiFileDialogSortOrder_None;
					typeSortOrder = ImGuiFileDialogSortOrder_None;
					dateSortOrder = ImGuiFileDialogSortOrder_None;

					fileChooseError = false;

					dialogInfo->refreshInfo = false;
					dialogInfo->currentIndex = 0;
					dialogInfo->currentFiles.clear();
					dialogInfo->currentDirectories.clear();

					complete = true;
					*open = false;
				}
				else {
					fileChooseError = true;
				}
			}
		}

		if (fileChooseError) {
			ImGui::SameLine();
			ImGui::TextColored(textErrorColor, "The selected item is not a regular file");
		}

	}
	else if (dialogInfo->type == ImGuiFileDialogType_SaveFile)
	{
		if (ImGui::Button("Save"))
		{
			dialogInfo->resultPath = dialogInfo->directoryPath / dialogInfo->fileName;

			if (std::filesystem::exists(dialogInfo->resultPath))
			{
				ImGui::OpenPopup("File Exists");
			}
			else {
				fileNameSortOrder = ImGuiFileDialogSortOrder_None;
				sizeSortOrder = ImGuiFileDialogSortOrder_None;
				typeSortOrder = ImGuiFileDialogSortOrder_None;
				dateSortOrder = ImGuiFileDialogSortOrder_None;

				dialogInfo->refreshInfo = false;
				dialogInfo->currentIndex = 0;
				dialogInfo->currentFiles.clear();
				dialogInfo->currentDirectories.clear();

				complete = true;
				*open = false;
			}
		}

		if (ImGui::BeginPopupModal("File Exists", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// Wyśrodkowanie tekstów
			float windowWidth = ImGui::GetWindowSize().x;
			float textWidth = ImGui::CalcTextSize("The file already exists.").x;
			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::Text("The file already exists.");

			textWidth = ImGui::CalcTextSize("Do you want to overwrite it?").x;
			ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
			ImGui::Text("Do you want to overwrite it?");

			ImGui::Spacing();

			// Wyśrodkowanie przycisków
			float buttonWidth = ImGui::CalcTextSize("Yes").x + ImGui::GetStyle().FramePadding.x * 2;
			float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
			float totalButtonWidth = (buttonWidth * 2) + buttonSpacing;

			ImGui::SetCursorPosX((windowWidth - totalButtonWidth) * 0.5f);

			if (ImGui::Button("Yes"))
			{
				fileNameSortOrder = ImGuiFileDialogSortOrder_None;
				sizeSortOrder = ImGuiFileDialogSortOrder_None;
				typeSortOrder = ImGuiFileDialogSortOrder_None;
				dateSortOrder = ImGuiFileDialogSortOrder_None;

				dialogInfo->refreshInfo = false;
				dialogInfo->currentIndex = 0;
				dialogInfo->currentFiles.clear();
				dialogInfo->currentDirectories.clear();

				complete = true;
				*open = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("No"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
	else if (dialogInfo->type == ImGuiFileDialogType_SelectFolder)
	{
		if (ImGui::Button("Select"))
		{
			dialogInfo->resultPath = dialogInfo->directoryPath / dialogInfo->fileName;

			if (std::filesystem::exists(dialogInfo->resultPath))
			{
				if (std::filesystem::is_directory(dialogInfo->resultPath)) {
					fileNameSortOrder = ImGuiFileDialogSortOrder_None;
					sizeSortOrder = ImGuiFileDialogSortOrder_None;
					typeSortOrder = ImGuiFileDialogSortOrder_None;
					dateSortOrder = ImGuiFileDialogSortOrder_None;

					folderSelectError = false;

					dialogInfo->refreshInfo = false;
					dialogInfo->currentIndex = 0;
					dialogInfo->currentFiles.clear();
					dialogInfo->currentDirectories.clear();

					complete = true;
					*open = false;
				}
				else {
					folderSelectError = true;
				}
			}
		}

		if (folderSelectError) {
			ImGui::SameLine();
			ImGui::TextColored(textErrorColor, "The selected item is not a folder");
		}
	}

	if (!dialogInfo->parsedFilters.empty() && dialogInfo->type != ImGuiFileDialogType_SelectFolder) {

		float textWidth = ImGui::CalcTextSize("Filter:").x + 5.0f;
		ImGui::SameLine(ImGui::GetWindowWidth() - 150.0f - textWidth - ImGui::GetStyle().ItemSpacing.x);
		ImGui::Text("Filter:");
		ImGui::SetNextItemWidth(150.0f);
		ImGui::SameLine(ImGui::GetWindowWidth() - 150.0f - ImGui::GetStyle().ItemSpacing.x);

		const char* currentFilterName = dialogInfo->parsedFilters.keyAt(dialogInfo->currentFilterIndex).c_str();
		if (ImGui::BeginCombo("##Filters", currentFilterName)) {
			for (int i = 0; i < dialogInfo->parsedFilters.size(); ++i) {
				bool isSelected = (dialogInfo->currentFilterIndex == i);

				if (ImGui::Selectable(dialogInfo->parsedFilters.keyAt(i).c_str(), isSelected)) {
					dialogInfo->currentFilterIndex = i;
					dialogInfo->refreshInfo = true;
				}
			}
			ImGui::EndCombo();
		}
	}

	ImGui::End();
	ImGui::PopID();

	return complete;
}