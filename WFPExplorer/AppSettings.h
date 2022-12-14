#pragma once

#include <Settings.h>

class AppSettings : public Settings {
public:
	BEGIN_SETTINGS(AppSettings)
		SETTING(MainWindowPlacement, WINDOWPLACEMENT{}, SettingType::Binary);
		SETTING(Font, LOGFONT{}, SettingType::Binary);
		SETTING(AlwaysOnTop, 0, SettingType::Bool);
		SETTING(ViewToolBar, 1, SettingType::Bool);
		SETTING(ViewStatusBar, 1, SettingType::Bool);
		SETTING(DarkMode, 0, SettingType::Bool);
		SETTING(SingleInstance, 0, SettingType::Bool);
		SETTING(HideEmptyLayers, 1, SettingType::Bool);
		SETTING(AppendNetworkEvents, 0, SettingType::Bool);
		SETTING(ResolveNetworkAddresses, 0, SettingType::Bool);
	END_SETTINGS

	DEF_SETTING(DarkMode, bool)
	DEF_SETTING(AlwaysOnTop, bool)
	DEF_SETTING(ViewToolBar, bool)
	DEF_SETTING(ViewStatusBar, bool)
	DEF_SETTING(SingleInstance, bool)
	DEF_SETTING(MainWindowPlacement, WINDOWPLACEMENT)
	DEF_SETTING(HideEmptyLayers, bool)
	DEF_SETTING(AppendNetworkEvents, bool)
	DEF_SETTING(ResolveNetworkAddresses, bool)
};

