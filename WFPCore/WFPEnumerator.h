#pragma once

template<typename TItem, typename T = TItem*>
class WFPObjectVector {
public:
	WFPObjectVector() = default;
	WFPObjectVector(std::vector<T>&& v, TItem** items) : m_Vec(std::move(v)), m_pItems(items) {}
	WFPObjectVector(WFPObjectVector const&) = delete;
	WFPObjectVector(WFPObjectVector&& other) : m_Vec(std::move(other.m_Vec)), m_pItems(other.m_pItems) {
		other.m_pItems = nullptr;
	}

	WFPObjectVector& operator=(WFPObjectVector const&) = delete;
	WFPObjectVector& operator=(WFPObjectVector&& other) {
		if (this != &other) {
			if(m_pItems)
				::FwpmFreeMemory((void**)&m_pItems);
			m_Vec = std::move(other.m_Vec);
			m_pItems = other.m_pItems;
			other.m_pItems = nullptr;
		}
		return *this;
	}

	~WFPObjectVector() {
		if (m_pItems)
			::FwpmFreeMemory((void**)&m_pItems);
	}

	auto begin() {
		return m_Vec.begin();
	}

	auto end() {
		return m_Vec.end();
	}

	auto data() {
		return m_Vec.data();
	}

	auto const begin() const {
		return m_Vec.begin();
	}

	auto const end() const {
		return m_Vec.end();
	}

	auto size() const {
		return m_Vec.size();
	}

	auto& operator[](size_t index) {
		return m_Vec[index];
	}

	auto const& operator[](size_t index) const {
		return m_Vec[index];
	}

private:
	std::vector<T> m_Vec;
	TItem** m_pItems{ nullptr };
};


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

	WFPObjectVector<TItem> Next(uint32_t next = 1024) {
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
			for (UINT32 i = 0; i < count; i++)
				items.push_back(pItems[i]);
			return WFPObjectVector<TItem>(std::move(items), pItems);
		}
		return WFPObjectVector<TItem>();
	}

	template<typename T, typename Pred>
	WFPObjectVector<TItem, T> NextFiltered(Pred&& pred, uint32_t next = 1024) {
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
			for (UINT32 i = 0; i < count; i++) {
				if (pred(pItems[i])) {
					T item;
					item.Data = pItems[i];
					items.push_back(std::move(item));
				}
			}
		}
		return WFPObjectVector<TItem, T>(std::move(items), pItems);
	}

	template<typename T>
	WFPObjectVector<TItem, T> Next(uint32_t next = 1024) {
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
		return WFPObjectVector<TItem, T>(std::move(items), pItems);
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

