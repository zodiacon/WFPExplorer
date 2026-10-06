# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

WFP Explorer is a native Windows GUI tool (C++/WTL) that shows and edits Windows Filtering Platform (WFP) objects: filters, layers, sublayers, providers, provider contexts, callouts, sessions and network events. There is no test suite.

## Build

This is a Visual Studio solution (`WFPExplorer.sln`, toolset v145 / VS 2026). Build it from a VS Developer prompt:

```
msbuild WFPExplorer.sln /p:Configuration=Debug /p:Platform=x64
msbuild WFPExplorer\WFPExplorer.vcxproj /p:Configuration=Release /p:Platform=x64
```

The configurations are Debug, Release and ReleaseSigned. x64 is the main platform; ARM64 solution platforms map to the x64 project configurations.

Dependencies:
- **WTLHelper** (github.com/zodiacon/WTLHelper) is the git submodule at `WTLHelper\`. Run `git submodule update --init` after cloning. The solution and `WFPExplorer.vcxproj` build `WTLHelper\WTLHelper\WTLHelper.vcxproj`. Changes to WTLHelper are committed inside the submodule, and then the submodule pointer is committed in this repo.
- WTL headers and other third-party packages come from vcpkg's classic MSBuild integration (`VcpkgUseStatic=true`). There is no manifest file.
- The app links `Fwpuclnt.lib`. Its manifest requires administrator rights, so running it or debugging it from VS needs an elevated session.

## Projects

- **WFPCore** (static lib, C++20): a thin, UI-free wrapper over the `Fwpm*` user-mode API.
  - `WFPEngine` opens the BFE engine handle. It provides get/add/delete by key or ID for providers, filters, layers, sublayers and callouts. Single objects come back as `WFPObject<T>`, an RAII owner that calls `FwpmFreeMemory`. Failures show up through `LastError()`, not exceptions.
  - `WFPEnumerator<Create, Destroy, Enum, TItem>` (`WFPEnumerator.h`) is a generic wrapper around the `Fwpm*CreateEnumHandle` / `Enum` / `DestroyEnumHandle` triple. `Next<T>()` / `NextFiltered<T>()` wrap each raw pointer in a caller-defined struct with a `Data` member. The results come back in a `WFPObjectVector`, which owns the WFP-allocated arrays and frees them. Keep that vector alive as long as any raw `FWPM_*` pointers taken from it are in use.
  - `WFPEnumerators.h` defines the concrete enumerators (`WFPFilterEnumerator`, `WFPLayerEnumerator`, ...). To add a new object type, add a struct there.
  - `WFPValue<T>` builds typed `FWP_VALUE` / `FWP_CONDITION_VALUE` instances.
- **WFPExplorer** (WTL GUI, C++ latest): the main app.
- **wfpc**: a small console tool that dumps WFP objects using WFPCore. It is useful for checking engine/enumerator changes without the UI.

## GUI architecture (WFPExplorer)

- `CMainFrame` keeps a single `WFPEngine` and a `CNativeCustomTabView`. Each `ID_VIEW_*` command creates a new view, adds it as a tab and calls `Refresh()`. `COMMAND_TABVIEW_HANDLER(m_Tabs, 1)` sends commands to the active tab's `ALT_MSG_MAP(1)`. Per-view commands such as Refresh, Delete, Properties and Copy are handled there, not in the frame.
- Views talk to the frame only through `IMainFrame` (`Interfaces.h`): status text, UI update, mono font, popup menus, and the find dialog.
- Most list views derive from `CGenericListViewBase<T>` (CRTP over WTLHelper's `CFrameView` and `CVirtualListView`). They are **virtual list views**: the view keeps a vector of info structs and provides `GetColumnText`, `DoSort`, `GetRowImage`, `OnDoubleClickList`, `OnRightClickList`, `GetSaveColumnRange` and `GetDefaultSaveFile`. Find, save-as-CSV and the mono font for numeric columns (`ColumnFlags::Numeric`) come from the base.
- Property sheets and dialogs (`*Dlg`, `*Page`) open through the static `WFPHelper::Show*Properties` functions. `WFPHelper` also resolves GUIDs to display names. `StringHelper` does all formatting of WFP enums, flags, values, addresses and GUIDs to text. Add new formatting there.
- Settings (`AppSettings`, built on WTLHelper's `Settings` macros) are stored in `HKCU\SOFTWARE\ScorpioSoftware\WFPExplorer`. To add a setting, add both a `SETTING(...)` and a `DEF_SETTING(...)`.
- In dark mode, `WM_UPDATE_DARKMODE` is broadcast to views.

## Conventions

- Indent with tabs. Member variables use the `m_` prefix and PascalCase. Use WTL message-map style handlers.
- Every project uses precompiled headers (`pch.h`), and `NTDDI_VERSION` is set to `WDK_NTDDI_VERSION`. Put new system headers in the project's `pch.h`.
