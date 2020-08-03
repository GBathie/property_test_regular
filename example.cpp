#include "property_tester.h"
#include "nfa.h"

#include <iostream>
#include <string>

using namespace std;

int main()
{
	// Automaton for (ab)^*
	// with characters
	Nfa<char> a_b_star(3);
	a_b_star.add_transition(0, 'a', 1);
	a_b_star.add_transition(1, 'b', 0);
	a_b_star.set_initial(0);
	a_b_star.set_final(0);

	// Here it is very important to specify the that Container type 
	// is `std::string`, otherwise the string literal is has the type
	// const char[], which include the trailing '\0' symbol, 
	// which will make the automaton reject the word.
	cout << "'abab': " << a_b_star.accepts<std::string>("abab") << endl;
	cout << "'abb': " << a_b_star.accepts<std::string>("abb") << endl;

	double eps = 0.3;
	double p = 0.5;
	string close = "";
	for (int i = 0; i < 10000; ++i)
		close += "ab";
	cout << "close (exact): " << a_b_star.accepts(close) << endl;
	cout << "close (approx eps = " << eps 
		 << ", p = " << p << "): " << property_test(a_b_star, close, eps, p) << endl;

	string far = "";
	far.append(10000, 'a');
	far.append(10000, 'b');
	cout << "far (exact): " << a_b_star.accepts(far) << endl;
	cout << "far (approx eps = " << eps 
		 << ", p = " << p << "): " << property_test(a_b_star, far, eps, p) << endl;
	

	// We can also make an NFA with transitions labeled with integers,
	// or even with a custom struct.
	// Automaton for 0^* 1^+, with integers
	Nfa<int> zero_s_one_p(2); 
	zero_s_one_p.add_transition(0, 0, 0);
	zero_s_one_p.add_transition(0, 1, 1);
	zero_s_one_p.add_transition(1, 1, 1);
	zero_s_one_p.set_initial(0);
	zero_s_one_p.set_final(1);

	cout << "'000': " << zero_s_one_p.accepts({0,0,0}) << endl;
	cout << "'0001': " << zero_s_one_p.accepts({0,0,0,1}) << endl;
	
	return 0;
}