#pragma once

#include <vector>

// **** Nondeterministic finite automaton ****
// The template parameter is the input alphabet of the automaton.
// This type must support basic operations such as testing equality
// and copy.
// It allows to define NFA whose alphabet is different
// from the usual `char`.
template <typename T>
class Nfa
{
private:
	typedef struct Transition
	{
		T label;
		int to;
		Transition(const T &l, int t): label(l), to(t) { };
		bool operator==(const Transition &rhs) const { return (label == rhs.label) && (to == rhs.to); };
	} Transition;

	int n_states;
	// The number of strongly connected is stored in this variable
	// so that we only compute it when the graph has changed.
	// We set n_scc to -1 when it must be recomputed.
	int n_scc;
	std::vector<bool> initial_states_;
	std::vector<bool> final_states_;
	// Transitions are stored in an adjacency list.
	std::vector<std::vector<Transition>> transitions; 

	// The two `kosaraju` functions are auxiliary functions used 
	// in the kosaraju algorithm that counts the number of strongly connected components.
	void kosaraju_build_transpose(
		std::vector<std::vector<int>> &transpose,
		std::vector<int> &order,
		std::vector<bool> &seen, 
		int v);	
	bool kosaraju_count(const std::vector<std::vector<int>> &graph, std::vector<bool> &seen, int v);

public:

	Nfa(int q) : n_states(q), n_scc(-1), initial_states_(q), final_states_(q), transitions(q) { };

	inline int num_states() const { return n_states; };
	inline std::vector<bool> initial_states() const { return initial_states_; };
	inline std::vector<bool> final_states()   const { return final_states_; };
	// In the following functions set_STATUS and is_STATUS, 
	// `i` is expected to be between 0 and n_states-1, inclusive.
	inline void set_initial(int i) { initial_states_[i] = true; };
	inline void set_final(int i)   { final_states_[i] = true; };
	inline bool is_initial(int i) const { return initial_states_[i]; };
	inline bool is_final(int i)   const { return final_states_[i]; };

	inline void add_transition(int from, const T &label, int to);
	inline bool is_transition(int from, const T &label, int to) const
	{
		return std::find(transitions[from].begin(), transitions[from].end(), Transition(label, to)) != transitions[from].end();
	};

	// Compute the number of strongly connected components. 
	// Takes linear time, or constant time if add_transition 
	// has NOT been called since the last call to num_scc.
	int num_scc();

	// Computes the set of states reachable from any `q\in from` with any number of steps.
	// This is done efficiently using a BFS. 
	// Set of states are given as characteristic vectors.
	std::vector<bool> star_reach(const std::vector<bool> &from) const;

	// Computes the set of states reachable from any q\in states with one transition labeled by `a`.
	// Set of states are given as characteristic vectors.
	std::vector<bool> letter_reach(const std::vector<bool> &from, const T &a) const;

	// Tests whether u belongs to L(Nfa) by simulating the automaton.
	// Here, Container maybe be std::vector, std::basic_string, etc.
	template <typename Container = std::vector<T>>
	bool accepts(const Container &u) const;

};

/**** Public members ****/
template <class T>
void Nfa<T>::add_transition(int from, const T &label, int to)
{
	transitions[from].push_back(Transition(label, to));
	n_scc = -1;
}

// Compute the number of strongly connected components. 
// Takes linear time, or constant time if add_transition 
// has NOT been called since the last call to num_scc.
template <typename T>
int Nfa<T>::num_scc()
{
	if (n_scc != -1) return n_scc;
	// Use a first BFS to compute the transpose of the graph
	// and compute the order of the second BFS.
	std::vector<std::vector<int>> transpose(n_states);
	std::vector<int> order;
	std::vector<bool> seen(n_states, false);
	for (int i = 0; i < n_states; ++i)
		kosaraju_build_transpose(transpose, order, seen, i);

	// The second BFS counts the number of SCCs.
	seen = std::vector<bool>(n_states, false);
	n_scc = 0;
	for (auto it = order.rbegin(); it != order.rend(); it++)
		if (kosaraju_count(transpose, seen, *it))
			++n_scc;

	return n_scc;
} 


// Computes the set of states reachable from any q\in states with any number of steps.
// This is equivalent to computing the set of states that are reachable from
// any state in `from`.
// This is done efficiently using a BFS. 
// Set of states are given as characteristic vectors.
template <typename T>
std::vector<bool> Nfa<T>::star_reach(const std::vector<bool> &from) const
{
	std::vector<bool> seen = from;
	
	std::vector<int> stack;
	for (int i = 0; i < n_states; ++i)
		if (from[i])
			stack.push_back(i);

	while (!stack.empty())
	{
		int j = stack.back(); stack.pop_back();
		seen[j] = true;
		for (const Transition &t: transitions[j])
			if (!seen[t.to])
				stack.push_back(t.to);
	}

	return seen;
} 

// Computes the set of states reachable from any q\in states with one transition labeled by `a`.
// Set of states are given as characteristic vectors.
template <typename T>
std::vector<bool> Nfa<T>::letter_reach(const std::vector<bool> &from, const T &a) const
{
	std::vector<bool> seen(n_states, false);

	for (int i = 0; i < n_states; ++i)
		if (from[i])
			for (const Transition &t: transitions[i])
				if (t.label == a)
					seen[t.to] = true;

	return seen;
}

// Tests whether u belongs to L(Nfa)
// by simulating the automaton.
template <typename T>
template <typename Container>
bool Nfa<T>::accepts(const Container &u) const
{
	std::vector<bool> states = initial_states_;
	for (const T &c: u)
	{
		states = letter_reach(states, c);
	}	
	for (int i = 0; i < n_states; ++i)
		if (states[i] && final_states_[i])
			return true;

	return false;
}

/**** Private members ****/
template <typename T>
void Nfa<T>::kosaraju_build_transpose(
	std::vector<std::vector<int>> &transpose,
	std::vector<int> &order,
	std::vector<bool> &seen, 
	int v)
{
	if (seen[v]) return;
	seen[v] = true;
	for (const Transition &t: transitions[v])
	{
		transpose[t.to].push_back(v);
		kosaraju_build_transpose(transpose, order, seen, t.to);
	}
	order.push_back(v);
}

template <typename T>
bool Nfa<T>::kosaraju_count(const std::vector<std::vector<int>> &graph, std::vector<bool> &seen, int v)
{
	if (seen[v]) return false;
	seen[v] = true;
	for (int u: graph[v])
		kosaraju_count(graph, seen, u);

	return true;
}
