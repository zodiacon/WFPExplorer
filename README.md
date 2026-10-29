# WFP Explorer

WFP Explorer is a Windows GUI tool for viewing and editing [Windows Filtering Platform](https://learn.microsoft.com/windows/win32/fwp/windows-filtering-platform-start-page) (WFP) objects: filters, layers, sublayers, providers, provider contexts, callouts, sessions and network events. It can also draw a layer as a graph, showing its sublayers and filters in the order WFP evaluates them.

![WFP Explorer showing the Hierarchy view with the ALE Connect v4 layer map](docs/images/hierarchy.png)

## Features

- **List views** for every WFP object type, with sorting, search, copy, and export to CSV.
- **Hierarchy view**: a tree of layers, each with its filters and callouts.
- **Layer maps**: a graph of a layer's sublayers (by weight) and their filters (by effective weight), with the callouts the filters use. You can color the filters by provider and hide the Windows Firewall filters.
- **Callout maps**: a graph of a callout's provider, its layer, and the filters that use it.
- **Property dialogs** for filters (including their conditions), layers (fields and filters), sublayers, providers and callouts.
- **Network events**: the events the Base Filtering Engine (BFE) has logged, such as dropped packets, with the filter and layer that caused them.
- **Editing**: create filters, providers and sublayers; delete filters, sublayers, providers and callouts.
- Save maps as PNG, SVG, JPEG, BMP, or a WFP Map file that you can open again later.
- Dark mode.

See the **[User Guide](docs/UserGuide.md)** for a tour of every view.

## Download

Get the latest version from the [Releases](https://github.com/zodiacon/WFPExplorer/releases) page. WFP Explorer runs on Windows 10 and later (x64). It needs administrator rights to access the BFE, so it asks for elevation when it starts.

## Screenshots

| Layer map | Filters |
|---|---|
| [![Layer map](docs/images/layer-map.png)](docs/images/layer-map.png) | [![Filters](docs/images/filters.png)](docs/images/filters.png) |
| **Filter properties** | **Callout map** |
| [![Filter properties](docs/images/filter-conditions.png)](docs/images/filter-conditions.png) | [![Callout map](docs/images/callout-map.png)](docs/images/callout-map.png) |

## Building

WFP Explorer is a Visual Studio 2026 solution (C++20 and WTL).

1. Clone the repository with its submodule:

   ```
   git clone --recursive https://github.com/zodiacon/WFPExplorer.git
   ```

   If you already cloned it, run `git submodule update --init`.
2. Install the dependencies with [vcpkg](https://vcpkg.io), using its classic mode and MSBuild integration (`vcpkg integrate install`):

   ```
   vcpkg install wtl:x64-windows-static wil:x64-windows-static detours:x64-windows-static
   ```
3. Build `WFPExplorer.sln` in Visual Studio, or from a Developer Command Prompt:

   ```
   msbuild WFPExplorer.sln /p:Configuration=Release /p:Platform=x64
   ```

   The executable is `x64\Release\WFPExp.exe`.

The solution also builds **wfpc**, a small console tool that lists WFP objects (`wfpc list filters`, `wfpc list layers`, and so on).

## License

[MIT](LICENSE.txt)
