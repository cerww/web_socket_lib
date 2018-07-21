#pragma once
#include <utility>
#include <memory>
#include <algorithm>


struct middle{
	template<typename T>
	static T* fn(void* start, void* end, size_t size)noexcept {
		return static_cast<T*>(start) + ((char*)end - (char*)start) / 2 - size / 2;
	}
};

struct right{
	template<typename T>
	static T* fn(void* start, void* end, size_t size) noexcept{
		return static_cast<T*>(end) - size;
	}
};

struct left{
	template<typename T>
	static T* fn(void* start, void* end, size_t size)noexcept {
		return static_cast<T*>(start);
	}
};

struct n_from_start{
	
	template<typename T>
	T* fn(void* start,void* end,size_t size) const noexcept{
		return static_cast<T*>(start) + n;
	}

	size_t n = 0;
};

struct n_from_last {
	template<typename T>
	T* fn(void* start, void* end, size_t size) const noexcept {
		return static_cast<T*>(end) - n;
	}
	size_t n = 0;
};


template<typename T>
struct vector{
	vector() = default;

	vector(const vector& other) {
		reserve(other.size(), left{});
		m_end = std::copy(other.begin(), other.end, m_begin);
	}

	vector(vector&& other) noexcept :
		m_start(std::exchange(other.m_start, nullptr)),
		m_stop(std::exchange(other.m_stop, nullptr)),
		m_begin(std::exchange(other.m_begin,nullptr)),
		m_end(std::exchange(other.m_end,nullptr)){
		
	}

	vector& operator=(const vector& other) {
		reserve(other.size(), left{});
		m_end = std::copy(other.begin(), other.end, m_begin);
		return *this;
	}

	vector& operator=(vector&& other) noexcept{
		m_start = std::exchange(other.m_start, nullptr);
		m_end = std::exchange(other.m_end, nullptr);
		m_begin = std::exchange(other.m_begin, nullptr);
		m_stop = std::exchange(other.m_stop, nullptr);
		return *this;
	}

	vector& operator=(std::initializer_list<T> li) {
		reserve(li.size(), left{});
		m_end = std::move(li.begin(), li.end(), m_begin);
		return *this;
	}

	vector(std::initializer_list<T> li) {
		reserve(li.size(), left{});
		m_end = std::move(li.begin(), li.end(), m_begin);
	}

	template<typename it, typename sen, typename std::enable_if_t<std::is_void_v<std::void_t<decltype(std::distance(std::declval<sen>(), std::declval<it>())),decltype(*std::declval<T>()),decltype(std::declval<T>()++)>>, int> = 0>
	vector(it&& begin,sen&& end) {
		reserve(std::distance(end, begin));
		m_end = std::uninitialized_copy(begin, send, m_begin);
	}

	~vector() {
		clean_up();
	}

	void push_front(T a) {
		emplace_front(std::move(a));
	}

	void push_back(T item) {
		emplace_back(std::move(item));
	}

	template<typename ...args>
	T& emplace_front(args&&... Args) {
		if (m_begin == (T*)m_start) {
			reserve(capacity() * 2 + 1);
			return *(new (--m_begin) T(std::forward<args>(Args)...));
		}else {
			return *(new (--m_begin) T(std::forward<args>(Args)...));
		}
	}

	template<typename ...args>
	T& emplace_back(args&&... Args) {
		if (m_end == (T*)m_stop) {
			reserve(capacity() * 2 + 1);
			return *(new (m_end++) T(std::forward<args>(Args)...));
		}else {
			return *(new (m_end++) T(std::forward<args>(Args)...));
		}
	}
	
	void resize(size_t new_size,T fill = T{}) {
		if (new_size <= size())
			return;
		if(new_size>capacity()) {
			reserve(new_size, left{});
		}else if(m_end+new_size>(T*)m_stop) {
			shift_elems(left{});
		}
		m_end = std::uninitialized_fill_n(m_end, new_size - size(), fill);
	}

	size_t capacity()const noexcept{
		return static_cast<T*>((void*)m_stop) - static_cast<T*>((void*)m_start);
	}
	
	size_t size()const noexcept {
		return m_end - m_begin;
	}

	template<typename policy>
	void reserve(size_t new_capacity,policy&& p) {
		if (new_capacity < capacity())
			return;
		void* new_start = malloc(new_capacity * sizeof(T));
		void* new_stop = (char*)new_start + (new_capacity * sizeof(T));
		T* new_begin = p.template fn<T>(new_start,new_start,size());
		T* new_end = std::uninitialized_move(m_begin, m_end,
#if defined(_MSC_VER) && _DEBUG
			stdext::make_checked_array_iterator(new_begin, size())
#else 
			new_begin
#endif
		)
#if defined(_MSC_VER) && _DEBUG
		.base();
#endif;	
		clean_up();
		m_start = (char*)new_start;
		m_begin = new_begin;
		m_end = new_end;
		m_stop = (char*)new_stop;
	}

	void reserve(size_t c) {
		reserve(c, middle{});
	}

	template<typename policy>
	void shift_elems(policy&& p) {
		T* new_begin = p.template fn<T>(m_start, m_end, size());
		size_t size = size();		
		if (new_begin > m_begin && new_begin < m_end) {
			auto middle = std::swap_ranges(new_begin, m_end, m_begin);
			m_end = std::uninitialized_move(m_begin, middle, m_end);
			std::destroy(m_begin,middle);
			m_begin = new_begin;
		}else {
			m_begin = std::uninitialized_move(m_begin, m_end, new_begin);
			m_end = m_begin + size;
		}
	}
	
	template<typename value_type>
	struct templated_iterator{
		templated_iterator() = default;
		templated_iterator(value_type* ptr) :m_ptr(ptr){}

		value_type& operator*() const noexcept{
			return *m_ptr;
		}

		value_type* operator->() const noexcept {
			return m_ptr;
		}

		bool operator!=(templated_iterator other) const noexcept{
			return m_ptr != other.m_ptr;
		}

		bool operator==(templated_iterator other) const noexcept {
			return m_ptr == other.m_ptr;
		}
		bool operator<(templated_iterator other) const noexcept {
			return m_ptr < other.m_ptr;
		}
		bool operator>(templated_iterator other) const noexcept {
			return m_ptr >other.m_ptr;
		}
		bool operator>=(templated_iterator other) const noexcept {
			return m_ptr >= other.m_ptr;
		}
		bool operator<=(templated_iterator other) const noexcept {
			return m_ptr <= other.m_ptr;
		}

	private:
		value_type* m_ptr = nullptr;
		friend struct vector;
	};

	using iterator = templated_iterator<T>;
	using const_iterator = templated_iterator<const T>;

	iterator begin() noexcept {
		return iterator(m_begin);
	}

	iterator end() noexcept {
		return iterator(end);
	}

	const_iterator begin()const noexcept {
		return const_iterator(m_begin);
	}

	const_iterator end()const noexcept {
		return const_iterator(end);
	}

	const_iterator cbegin()const noexcept {
		return const_iterator(m_begin);
	}

	const_iterator cend()const noexcept {
		return const_iterator(end);
	}
	
	template<typename iterator_value_type>
	iterator insert(templated_iterator<iterator_value_type> it,T item) {
		size_t distance_from_begin = it.m_ptr - m_begin;
		if(distance_from_begin>size()/2) {
			push_back(std::move(item));
			std::rotate(m_begin + distance_from_begin, m_end - 1, m_end);
		}else {
			push_front(std::move(item));
			std::rotate(m_begin + distance_from_begin, m_begin + distance_from_begin + 1, m_end);
		}return iterator(m_begin + distance_from_begin);
	}

private:
	void clean_up() {
		if (!m_start) return;
		std::destroy(m_begin, m_end);
		free(m_start);
	}
	char* m_start = nullptr;
	char* m_stop = nullptr;
	T* m_begin = nullptr;
	T* m_end = nullptr;
};

