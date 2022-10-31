#pragma once

struct ProcessHelper abstract final {
	static CString GetProcessName(DWORD pid);
};

