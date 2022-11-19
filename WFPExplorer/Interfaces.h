#pragma once

struct IMainFrame abstract {
	virtual void SetStatusText(int index, PCWSTR text) = 0;
	virtual CUpdateUIBase& UI() = 0;
	virtual HFONT GetMonoFont() const = 0;
};

