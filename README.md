[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/Iam1337/ImGui-FileDialog/blob/master/LICENSE)

# File Dialog for Dear ImGui 🖥️

A simple and lightweight file dialog for Dear ImGui, based on [L2DFileDialog](https://github.com/Limeoats/L2DFileDialog).

![File Dialog Without Filter](/screenshots/screenshot1.png?raw=true)

### 📋 Table of Contents
- [✨ Features](#-features)
- [⚙️ Adding to your project](#%EF%B8%8F-adding-to-your-project)
- [💡 Example Usage](#-example-usage)
- [🔄 Changes](#%EF%B8%8F-changes)
- [📜 License](#-license)

### ✨ Features
- **Multiple dialog types**:
  - 📂 Open File
  - 💾 Save File
  - 📁 Select Folder
- **File sorting options**:
  - 📝 By Name
  - 📏 By Size
  - 🔠 By Type
  - 🕒 By Last Modified Date
- 🔍 **Customizable file filters** (e.g., support for specific file types like .json).
- ⚠️ **Error handling** (e.g., alert when a file already exists during a "Save File" operation).

### ⚙️ Adding to your project

1. Download `imgui_filedialog.h` and `imgui_filedialog.cpp`
2. Add downloaded files to your C++ project.
3. Include the header file where you want to use ImGuiFileDialogs.

### 💡 Example Usage

```c++
// Declare variables outside of the draw loop
bool m_fileDialogOpen;
ImFileDialogInfo m_fileDialogInfo;

// Trigger file dialog on button click
if (ImGui::Button("Save File"))
{
    m_fileDialogOpen = true;
    m_fileDialogInfo.type = ImGuiFileDialogType_SaveFile;
    m_fileDialogInfo.title = "Save File";
    m_fileDialogInfo.fileName = "test.json";
    m_fileDialogInfo.directoryPath = std::filesystem::current_path();

    // Optional: Define file filters
    m_fileDialogInfo.filters = 
    { 
      "All Files (*.*)|.*", 
      "JSON (*.json)|.json"
    };
}

// Show file dialog in the render loop
if (ImGui::FileDialog(&m_fileDialogOpen, &m_fileDialogInfo))
{
    // Handle result path: m_fileDialogInfo.resultPath
}
```

### 🔄 Changes
- 📁 Added **SelectFolder** dialog type.
- 🔧 Changed the **file filtering** system.
- ⬇️ Introduced a **filter dropdown** for easier selection.
- 📑 Updated the `ImFileDialogInfo` structure.
- 🔄 Refactored the method to refresh paths: `void RefreshInfo(ImFileDialogInfo* dialogInfo)` (replacing `ImFileDialogInfo::refreshPaths()`).
- ⚠️ Added **Error Text** during "Open File" operation.
- 🛑 Introduced a **popup** that appears if a file already exists when attempting to save.
- 📚 Updated the README.

### 📜 License
📝 This project is licensed under the **Apache License 2.0**.

📖 **Apache License 2.0 Overview:**
- ✅ Free to use, modify, and distribute.
- ✅ Can be used in **commercial** and **non-commercial** projects.
- ✅ Provides an express grant of patent rights from contributors.
- ❗ Must include original license, copyright, and NOTICE file.

See the [LICENSE](./LICENSE) file for more details.