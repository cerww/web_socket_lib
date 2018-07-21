#pragma once
#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <algorithm>

template<typename K,typename V>
struct trie_like_thing{

	V& operator[](K&& k){
		
	}



private:


	struct node{
		uint8_t val = 0;
		union { V thing; };
		std::vector<int> child_indexes;
	};

	node& add_to_node(node& n,uint8_t val) {
		m_nodes.push_back(node{ val });
		n.child_indexes.push_back(m_nodes.size());
		return m_nodes.back();
	}

	std::vector<node> m_nodes;
	int m_end_of_roots_idx = 0;
};



template<typename V>
struct trie{

	trie() {
		m_nodes.emplace_back();
	}

	template<typename rng>
	auto operator[](rng&& range)->std::enable_if_t<std::is_same_v<std::decay_t<decltype(*range.begin() == char{})>,bool>,V&>  {
		return get_node(std::forward<rng>(range)).thing;
	}

private:
	static constexpr int root_node_idx = 0;
	struct node {
		char c = 0;
		V thing;
		std::vector<int> child_indexes;
	};

	int add_to_node(node& n, char c) {
		n.child_indexes.push_back(m_nodes.size());
		m_nodes.push_back(node{ c });
		return m_nodes.size() - 1;
	}

	template<typename rng>
	node& get_node(rng&& range) {
		int idx = root_node_idx;

		auto start = range.begin();
		auto stop = range.end();
		for (; start != stop;++start) {
			const auto&& item = *start;
			auto it = std::find_if(m_nodes[idx].child_indexes.begin(), m_nodes[idx].child_indexes.end(),[&](const auto& id){
				return m_nodes[id].c == item;
			});
			if(it == m_nodes[idx].child_indexes.end())
				break;
			idx = *it;			
		}
		for (; start != stop; ++start)
			idx = add_to_node(m_nodes[idx], *start);
		return m_nodes[idx];
	}
	
	std::vector<node> m_nodes;
};


