#pragma once

template<typename Create, typename Destroy, typename Enum, typename TItem>
class WFPEnumerator abstract {
public:
	virtual ~WFPEnumerator() {
		Close();
	}
	void Close() {
		if (m_hEnum) {
			m_destroy(m_hEngine, m_hEnum);
			m_hEnum = nullptr;
		}
	}

	DWORD LastError() const {
		return m_LastError;
	}

	std::vector<TItem*> Next(uint32_t next = 1024) {
		if (m_hEnum == nullptr) {
			m_LastError = m_create(m_hEngine, nullptr, &m_hEnum);
			if (m_LastError != ERROR_SUCCESS)
				return {};
		}

		TItem** pItems;
		UINT32 count;
		std::vector<TItem*> items;
		m_LastError = m_enum(m_hEngine, m_hEnum, next, &pItems, &count);
		if (ERROR_SUCCESS == m_LastError) {
			items.reserve(count);
			for (UINT i = 0; i < count; i++)
				items.push_back(pItems[i]);
			return items;
		}
		return items;
	}

	template<typename T>
	std::vector<T> Next(uint32_t next = 1024) {
		if (m_hEnum == nullptr) {
			m_LastError = m_create(m_hEngine, nullptr, &m_hEnum);
			if (m_LastError != ERROR_SUCCESS)
				return {};
		}

		TItem** pItems;
		UINT32 count;
		std::vector<T> items;
		m_LastError = m_enum(m_hEngine, m_hEnum, next, &pItems, &count);
		if (ERROR_SUCCESS == m_LastError) {
			items.reserve(count);
			for (UINT i = 0; i < count; i++) {
				T item;
				item.Data = pItems[i];
				items.push_back(std::move(item));
			}
		}
		return items;
	}

protected:
	WFPEnumerator(HANDLE hEngine, Create fcreate, Destroy fdestroy, Enum fenum) :
		m_hEngine(hEngine), m_create(fcreate), m_destroy(fdestroy), m_enum(fenum) {
	}

	HANDLE m_hEngine;
	HANDLE m_hEnum{ nullptr };
	DWORD m_LastError{ 0 };

private:
	Create m_create;
	Destroy m_destroy;
	Enum m_enum;
};

