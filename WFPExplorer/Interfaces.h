#pragma once

struct IMainFrame abstract {
	virtual void SetStatusText(int index, PCWSTR text) = 0;
};
