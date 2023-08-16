#include "scanner.hh"
#include "parser.tab.hh"
#include <fstream>
using namespace std;

SymbTab gst, gstfun, gststruct;
string filename;
std::vector<string> PrintLC;
extern std::map<string, abstract_astnode *> ast;
std::map<std::string, datatype> predefined{
	{"printf", createtype(VOID_TYPE)},
	{"scanf", createtype(VOID_TYPE)},
	{"mod", createtype(INT_TYPE)}};
int main(int argc, char **argv)
{
	fstream in_file, out_file;

	in_file.open(argv[1], ios::in);

	IPL::Scanner scanner(in_file);

	IPL::Parser parser(scanner);

#ifdef YYDEBUG
	parser.set_debug_level(1);
#endif
	parser.parse();
	// create gstfun with function entries only

	for (const auto &entry : gst.Entries)
	{
		if (entry.second.varfun == "fun")
			gstfun.Entries.insert({entry.first, entry.second});
	}
	// create gststruct with struct entries only

	for (const auto &entry : gst.Entries)
	{
		if (entry.second.varfun == "struct")
			gststruct.Entries.insert({entry.first, entry.second});
	}

	cout << "    .file \"" << argv[1] << "\"" << endl;  
	cout << "    .text" << endl;

	for (int i = 0; i < (int)PrintLC.size(); i++)
	{
		cout <<  PrintLC[i] << endl;
	}

	for (auto it = gstfun.Entries.begin(); it != gstfun.Entries.end(); ++it)
	{
		cout << "    "<< ".globl " << it->first << "\n    .type	" << it->first << ", @function" << endl;
		int ll = 0;
		for (const auto &it2 : it->second.symbtab->Entries)
		{
			if (it2.second.scope == "local")
			{
				ll += it2.second.size;
			}
		}
		ll += 4;
		cout << it->first << ":" << endl;
		cout << "    "<< "pushl	%ebp\n    movl	%esp, %ebp" << endl;
		cout << "    "<< "subl $" << ll << ", %esp" << endl;
		cout << "    " << "movl $1, -4(%ebp)" << endl;
		ast[it->first]->print(0);
		cout << "    "<< "addl $" << ll << ", %esp" << endl;
		cout << "    "<< "leave\n    ret\n    .size	" << it->first << ", .-" << it->first << endl;
	}

	cout << ".ident	\"GCC: (Ubuntu 8.1.0-9ubuntu1~16.04.york1) 8.1.0\" \n.section	.note.GNU-stack,\"\",@progbits" << endl  << ".section	.note.gnu.property,\"a\""<<endl << ".align 4"<<endl << ".long	 1f - 0f"<<endl  << ".long	 4f - 1f"<<endl << ".long	 5"<<endl << "0:" <<endl  << "	.string	 \"GNU\""<<endl  << "1:"<<endl ;
	cout << "	.align 4"<<endl;
	cout << "	.long	 0xc0000002"<<endl;
	cout << "	.long	 3f - 2f"<<endl;
	cout << "2:"<<endl;
	cout << "	.long	 0x3"<<endl;
	cout << "3:"<<endl;
	cout << "	.align 4"<<endl;
	cout << "4:"<<endl;

	fclose(stdout);
}
