#pragma once

#include <boost/graph/adjacency_list.hpp>
#include <vector>
#include <sstream>
#include <iostream>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <chrono>
#include <mutex>
#include <condition_variable>


inline unsigned long long bit_extractor_ull(int i) {
	return ((unsigned long long)1 << ((i)));
}

inline unsigned long long cut_bit(int i, unsigned long long mask) {
	return ((mask >> (i + 1)) << i) + (mask & (bit_extractor_ull(i) - (unsigned long long)1));
}

inline unsigned long long insert_bit(int pos, bool bit, unsigned long long mask) {
	return ((mask >> pos) << (pos + 1)) + ((unsigned long long)bit << pos) + (mask & (((unsigned long long)1 << pos) - 1));
}


using namespace std::chrono_literals;



typedef std::vector<std::pair<int, int>> AdjG;
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS> G; // need undirected?
// typedef typename treedec::graph_traits<G>::treedec_type T;
// typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, treedec::bag_t> T;
// typedef int T;
// typedef typename boost::graph_traits<T>::vertex_descriptor Vert;
typedef int Vert;

namespace wrap {

enum NodeType {
	NONE, INTRODUCE, FORGET, MERGE, LEAF
};




struct TreeNode {
	Vert v;
	NodeType node_type;
	int distinguished_vertex;

	int idx;

	std::vector<Vert> children;
	std::vector<int> children_idx;

	std::vector<int> bag;

	bool arb_flag1;
	bool arb_flag2;

	TreeNode(): node_type(NONE),  arb_flag1(false), arb_flag2(false), idx(-1) {}


};
}


const int MAX_BAG_LIMIT = 20;


