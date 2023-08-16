#include "symbtab.hh"

char separator = ' ';
int nameWidth = 10;
int numWidth = 8;

SymbTabEntry::SymbTabEntry()
{
}

SymbTabEntry::SymbTabEntry(std::string vf, std::string sc,
						   datatype ty, int sz, int off, SymbTab *st)
{
	varfun = vf;
	scope = sc;
	type = ty;
	size = sz;
	offset = off;
	symbtab = st;
}

void SymbTabEntry::print()
{
	cout << right << setw(nameWidth) << setfill(separator) << "\"" << varfun << "\""
		 << ",";
	cout << right << setw(nameWidth) << setfill(separator) << "\"" << scope << "\""
		 << ",";
	cout << right << setw(numWidth) << setfill(separator) << size << ",";
	if (varfun != "struct")
	{
		cout << right << setw(numWidth) << setfill(separator) << offset << ",";
		type.print();
	}
	else {
		cout << right << setw(numWidth) << setfill(separator) << "\"-\"" << ",";
		cout << "\"-\"" << endl;
	}
}

int SymbTab::push(std::string symbol, SymbTabEntry *entry)
{
	if (Entries.find(symbol) != Entries.end())
	{
		return -1; // Variable already exists
	}
	Entries[symbol] = *entry; // entry inserted in symbol table
	return 0;
}

vector<pair<int, datatype>> SymbTab::getparameters()
{
	vector<pair<int, datatype>> v;
	for (map<std::string, SymbTabEntry>::iterator it = Entries.begin(); it != Entries.end(); ++it)
	{
		if (it->second.scope == "param")
			v.push_back({it->second.offset, it->second.type});
	}
	sort(v.begin(), v.end());
	reverse(v.begin(), v.end());
	return v;
}

SymbTab::SymbTab()
{
}

void SymbTab::print()
{
	vector<pair<string, SymbTabEntry>> temp;

	for (map<std::string, SymbTabEntry>::iterator it = Entries.begin(); it != Entries.end(); ++it)
	{
		temp.push_back(make_pair(it->first, it->second));
	}
	cout << "[" << endl;
	for (vector<pair<string, SymbTabEntry>>::iterator it = temp.begin(); it != temp.end(); ++it)
	{
		cout << "[";
		std::cout << "\"" << it->first << "\",";
		it->second.print();
		std::cout << "]" << std::endl;
		if (next(it, 1) != temp.end())
			cout << "," << endl;
	}
	cout << "]" << endl;
}

void SymbTab::printgst()
{

	std::vector<pair<string, SymbTab *>> v;

	cout << "[";
	for (map<std::string, SymbTabEntry>::iterator it = Entries.begin(); it != Entries.end(); ++it)
	{
		cout << "[";
		std::cout << right << setw(nameWidth) << setfill(separator) << "\"" << it->first << "\"";
		cout << ",";
		it->second.print();
		cout << "]";
		if (next(it, 1) != Entries.end())
			cout << ",";
		std::cout << std::endl;
	}
	cout << "]";
	cout << endl;
}

SymbTabEntry *SymbTab::search(std::string symbol)
{
	if (Entries.find(symbol) != Entries.end())
	{
		return &Entries[symbol];
	}
	else // symbol not found in this symbol table
	{
		return NULL;
	}
}