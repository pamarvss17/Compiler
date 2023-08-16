#ifndef type_HH_INCLUDED
#define type_HH_INCLUDED

#include <iostream>
#include <string>
#include <vector>

#define VOID_TYPE 0
#define INT_TYPE 1
#define FLOAT_TYPE 2
#define STRUCT_TYPE 3

#define TYPE_ARRAY_REF 5
#define TYPE_DEREF 6
#define STRING_TYPE 7

class declarator_class;
class type_specifier_class;

using namespace std;

class datatype
{
public:
	int type;				// the basic type as a number, VOID_TYPE 0 INT_TYPE 1
	std::string structname;	// identifying the name of the struct
	int struct_size();
	int deref;				// how many derefs
	bool is_array;			// is it an array of
	std::vector<int> array; // the indices of the array

	datatype();
	int size();             //size of the datatype
	//datatype operator=(const datatype &t2);
	bool operator==(const datatype &t2);
	bool operator<(const datatype &t2) const;
	void print();
	string sprint();
	void addpointer();
};

datatype createtype(type_specifier_class *t, declarator_class *d);
datatype createtype(type_specifier_class *t, int nstars);
datatype createtype(int type);
datatype extract_type(datatype dt, int method);
bool is_compatible(int x, int y);
bool compatible(datatype t1, datatype t2, bool is_assign = false);
bool isVoidType(datatype t);
bool isIntType(datatype t);
bool isFloatType(datatype t);
bool isIntOrFloatType(datatype t);
bool isScalarOrPointerType(datatype t);
bool isStructType(datatype t);
bool isStructPointerType(datatype t);
bool isDereferenceAllowed(datatype t);
bool isPointer(datatype t);
bool isArray(datatype t);
datatype normalize (datatype t); //make pointers have a unique representation
bool all_negative(vector<int> v); //checks whether all elements of v are -1.
datatype convert(datatype t); 


class type_specifier_class////////////
{
public:
	int type;	 // types have been given numbers
	string structname; // 
	type_specifier_class(int type);
	type_specifier_class(int type, string structname);
};

class declarator_list_class ///////////////
{
public:
	std::vector <declarator_class *> v;

	declarator_list_class();
	void push(declarator_class *data);
};

class declaration_class  //////////////////
{
public:
	type_specifier_class *type;
	declarator_list_class *decl_list;

	declaration_class();
	declaration_class(type_specifier_class *t, declarator_list_class *d);
};

class declarator_class  /////////////////
{
public:
	string identifier;
	int deref;
	std::vector<int> array;
	declarator_class();
	declarator_class(string name);
	void addstar();
	void add_array_index(int index);
};

class parameter_declaration_class//////////////////
{
public:
	datatype t;
	string identifier;

	parameter_declaration_class();
	parameter_declaration_class(type_specifier_class *typedata, declarator_class *decldata);
};

class parameter_list_class ////////////////
{
public:
	std::vector<parameter_declaration_class *> param;
	int length;

	parameter_list_class();
	void push(parameter_declaration_class *data);
};

class fun_declarator_class  /////////////////
{
public:
	int deref;
	string identifier;
	parameter_list_class *param;

	fun_declarator_class();
	fun_declarator_class(string name);
	fun_declarator_class(string name, parameter_list_class *param);

	void addstar();
};

class declaration_list_class ////////////////
{
public:
	int offset;

	declaration_list_class();
};
#endif