CXXFLAGS = -O2 -std=c++14 -Wall -Wextra -Werror

all: tests example

%:%.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS)

clean: 
	$(RM) tests
	$(RM) example