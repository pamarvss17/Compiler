CXX   = g++-9
EXE = iplC
CXXDEBUG = -O0  -fno-inline-small-functions  -Wall 
CXXSTD = -std=c++11
GCC="g++ -fno-asynchronous-unwind-tables -fno-exceptions -fno-pic -no-pie -fno-stack-protector -mmanual-endbr -mpreferred-stack-boundary=2 -m32 -O0"

.PHONY: all	
all: parser.o scanner.o  ast.o type.o symbtab.o main.o	
	$(CXX) $(CXXDEBUG) parser.o scanner.o  type.o symbtab.o ast.o main.o -o $(EXE)
	
main.o: main.cpp
	$(CXX) $(CXXDEBUG) -c main.cpp -o main.o

type.o: type.cpp
	$(CXX) $(CXXDEBUG) -c type.cpp -o type.o

symbtab.o: symbtab.cpp
	$(CXX) $(CXXDEBUG) -c symbtab.cpp -o symbtab.o

ast.o: ast.hh ast.cpp
	$(CXX) $(CXXDEBUG) -c ast.cpp -o ast.o

parser.o: parser.yy 
	bison  --graph -v $<  
	$(CXX) $(CXXDEBUG) -c parser.tab.cc -o parser.o 

scanner.o: scanner.l scanner.hh parser.tab.hh parser.tab.cc	
	flex++ --outfile=scanner.yy.cc  $<
	$(CXX)  $(CXXDEBUG) -c scanner.yy.cc -o scanner.o

.PHONY: clean
clean:
	rm -f a.out parser.tab.cc parser.tab.hh position.hh scanner.o iplC parser.o parser.output scanner.yy.cc stack.hh Fl.h *.o *.gv *.s 
