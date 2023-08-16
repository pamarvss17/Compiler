#include <map>
#include <algorithm>
#include <iomanip>
#include "type.hh"
#include <cstring>

using namespace std;

class SymbTab; // forward declaration for symbtab pointer in the symbol table entry

class SymbTabEntry
{
public:
	// var or fun or struct
	std::string varfun;
	// (local or param) or global 
	std::string scope; 
	// 
	datatype type;
	// size of the entry
	int size;
	// offset of entry from basepointer
	int offset;
    //If gst, may contain a local symbtab
	SymbTab *symbtab;
	SymbTabEntry();
	SymbTabEntry(std::string vf, std::string sc, datatype ty, int sz, int off, SymbTab *st);
	void print();
	bool operator<(const SymbTabEntry &str) const
	{
		return (offset > str.offset);
	}
};

class SymbTab
{
public:
	// symbtab entries
	map<std::string, SymbTabEntry> Entries;
	SymbTab();
	void print();
	void printgst();
	int push(std::string symbol, SymbTabEntry *entry);
	SymbTabEntry *search(std::string symbol);
	vector<pair<int, datatype>> getparameters();
};