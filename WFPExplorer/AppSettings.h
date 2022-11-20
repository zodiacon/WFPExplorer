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
	END_SETTINGS

	DEF_SETTING(DarkMode, int)
	DEF_SETTING(AlwaysOnTop, int)
	DEF_SETTING(ViewToolBar, int)
	DEF_SETTING(ViewStatusBar, int)
	DEF_SETTING(SingleInstance, int)
	DEF_SETTING(MainWindowPlacement, WINDOWPLACEMENT)
};

