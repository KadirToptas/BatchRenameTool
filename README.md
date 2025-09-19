# Asset/Actor Batch Rename Tool

This is an editor utility plugin for Unreal Engine, developed as a technical assessment for the Leartes Studios Internship Program. The tool is designed to simplify and accelerate the workflow of renaming multiple assets and actors within the Unreal Editor. It provides a comprehensive set of rules for renaming, including adding prefixes/suffixes, find-and-replace, sequential numbering, and case adjustments.

The entire tool is developed in C++ using the Slate UI framework to create a user-friendly and non-intrusive editor widget.

## Demo Video

[![Batch Rename Tool Demo](https://img.youtube.com/vi/oULgKO5wKxM/0.jpg)](https://www.youtube.com/watch?v=oULgKO5wKxM)

*Click the image above to watch the video on YouTube*

## Features

This tool meets all the core requirements outlined in the task document, providing a robust and practical solution for batch renaming.

### Core Functionality
* **Editor Plugin:** Developed as a standalone Unreal Editor plugin with its own module (`LeartesRenameTool`) for easy integration.
* **Content Browser Support:** Works directly with assets selected in the Content Browser. The tool collects selected asset data for processing.
* **Level Actor Support:** Works directly with actors selected in the active level viewport. The tool iterates through `GEditor`'s selected actors to build its list.
* **User-Friendly Interface:** Features a simple and intuitive UI built with Slate, contained within a dockable editor tab.

### Renaming Operations
* **Prefix & Suffix:** Easily add text to the beginning (Prefix) and end (Suffix) of each name. The UI includes `PrefixTextBox` and `SuffixTextBox` for input, and the logic concatenates these to the base name.
* **Find & Replace:** Search for a specific string within the names and replace it with another. This is handled by the `FString::Replace` function in the core logic based on input from `FindTextBox` and `ReplaceTextBox`.
* **Sequential Numbering:** Add sequential numbers to each item, with options for a starting number and digit padding (e.g., 01, 001). This is controlled by `bUseNumbering`, `StartNumber`, and `Padding` options, and the formatted number string is generated using `FString::Printf`.
* **Case Operations:** Change the case of the names. Options include `UPPERCASE`, `lowercase`, and `CapitalizeFirst`. These are managed by the `ECaseOp` enum and a dedicated `ApplyCaseOp` function.

### UI & Workflow
* **Selection Info Panel:** The UI displays the current count of selected assets and actors, so the user knows what they are modifying. This is managed by the `UpdateSelectionCounts` function, which updates the `AssetsCountText` and `ActorsCountText` UI elements.
* **Apply and Cancel Buttons:** The user can confirm the changes with an "Apply" button or reset all fields to their default state with a "Cancel" button.
* **Dry Run (Preview):** Before applying any changes, the tool generates a "Preview" list showing the old name and the proposed new name. This "Dry Run" mode is enabled by default to prevent accidental changes and can be toggled via a checkbox.
* **Collision Detection:** The preview list will indicate if a proposed new name already exists in the project or level, warning the user of a potential collision.

### Technical Requirements
* **C++ Development:** The plugin is written entirely in C++ as allowed by the technical details.
* **Undo/Redo Support:** All rename operations are wrapped in an `FScopedTransaction`, ensuring that any batch rename can be undone (Ctrl+Z) and redone (Ctrl+Y) through the editor's standard transaction system. This is implemented for both asset and actor renaming.

## Installation and Usage

1.  Clone the repository into the `Plugins` folder of your Unreal Engine project.
2.  Generate the Visual Studio project files by right-clicking the `.uproject` file.
3.  Build the project from your IDE (Visual Studio or Rider).
4.  Launch the Unreal Editor. The plugin will be enabled automatically.
5.  To open the tool, navigate to the main menu and select **Window -> LeartesRenameTool**. Alternatively, click the tool's icon in the main editor toolbar.
6.  Select assets in the Content Browser or actors in the Level Viewport.
7.  Click the **"Refresh"** button in the tool to load the selected items.
8.  Configure your desired renaming options (Prefix, Suffix, Numbering, etc.). The preview list will update automatically.
9.  Uncheck "Dry Run (Preview only)" to enable the "Apply" button.
10. Click **"Apply"** to perform the batch rename operation.

## UI Overview

<img width="310" height="838" alt="WindowTool" src="https://github.com/user-attachments/assets/f2a1ec0e-67e5-4d00-941d-8dd8afc2aa30" /><img width="1919" height="544" alt="Working" src="https://github.com/user-attachments/assets/343c5ea2-7cc8-4259-adff-8e1d4e638c97" />
<img width="870" height="147" alt="CaseOps" src="https://github.com/user-attachments/assets/881c0a39-1e34-44db-8703-c7a8a56e9be0" />
<img width="867" height="476" alt="ToolCloseUp" src="https://github.com/user-attachments/assets/7e074a58-9c18-4363-91ca-dda0479da1d9" />
<img width="1919" height="1004" alt="ToolOverwiew" src="https://github.com/user-attachments/assets/7bf88141-5317-44c5-9132-0ea827beabfb" />



* **1. Selection Counts:** Displays the number of currently selected assets and actors.
* **2. Renaming Options:** Contains all text input fields for `Prefix`, `Suffix`, `Find`, and `Replace`.
* **3. Numbering & Case:** Provides controls for enabling and configuring sequential numbering and choosing a case transformation.
* **4. Target & Mode Selection:** Checkboxes to specify whether to apply changes to `Assets`, `Actors`, and to toggle `Dry Run` mode.
* **5. Action Buttons:** Main buttons to `Refresh` the selection, `Apply` the changes, or `Cancel` and reset all options.
* **6. Preview Panel:** A scrollable list that shows a preview of all changes before they are applied.

---
### Technical Details
* **Language:** C++
* **UI Framework:** Slate UI Framework
* **Engine Version:** Unreal Engine 5.5
* **Core Engine Modules Used:** `Core`, `CoreUObject`, `Engine`, `Slate`, `SlateCore`, `EditorStyle`, `UnrealEd`, `AssetTools`, `ContentBrowser`, `AssetRegistry`, `ToolMenus`, `Projects`.
