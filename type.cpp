#include "type.hh"
#include "symbtab.hh"

extern SymbTab gst;

extern char separator;
extern int nameWidth;
extern int numWidth;

datatype createtype(type_specifier_class *typedata, declarator_class *decldata)
{
	datatype t;
	t.type = typedata->type;
	t.structname = typedata->structname;
	t.deref = decldata->deref;
	if (decldata->array.size() == 0)
	{
		t.is_array = false;
	}
	else
	{
		t.is_array = true;
		t.array = decldata->array;
	}
	return t;
}

datatype createtype(type_specifier_class *typedata, int nstars)
{
	// std::cout<<"Entered"<<std::endl;
	datatype t;
	t.type = typedata->type;
	// std::cout<<t.type<<std::endl;
	t.structname = typedata->structname;
	t.deref = nstars;
	t.is_array = false;
	std::vector<int> v;
	t.array = v;

	return t;
}

datatype createtype(int type)
{
	datatype t;
	t.type = type;
	t.deref = 0;
	t.is_array = false;
	return t;
}

datatype createVoidStarType()
{
	datatype t;
	t.deref = 1;
	t.type = VOID_TYPE;
	t.is_array = false;
	t.structname = "";
	return t;
}

//////////////////////////////////////////////////////////////////////////////////

bool datatype::operator==(const datatype &t2)
{
	return (type == t2.type) && (structname == t2.structname) && (deref == t2.deref) && (is_array == t2.is_array) && (array == t2.array);
}

bool datatype::operator<(const datatype &t2) const
{
	return true;
}

void datatype::addpointer()
{
	reverse(array.begin(), array.end());
	array.push_back(-1);
	reverse(array.begin(), array.end());
	is_array = true;
}

string datatype::sprint()
{
	string str = "";
	int i = 0;
	while (i < (int)(array.size)())
	{
		if (array[i] == -1)
		{
			str = "(*" + str;
			i++;
			while (array[i] == -1)
			{
				str = str + "*";
				i++;
			}
			str = str + ")";
		}
		while (i < (int)(array.size)() && array[i] >= 0)
		{
			str = str + "[" + to_string(array[i]) + "]";
			i++;
		}
	}
	for (int i = 0; i < deref; ++i)
	{
		str = "*" + str;
	}
	switch (type)
	{
	case VOID_TYPE:
		str = "\"void" + str + "\"";
		break;
	case INT_TYPE:
		str = "\"int" + str + "\"";
		break;
	case FLOAT_TYPE:
		str = "\"float" + str + "\"";
		break;
	case STRUCT_TYPE:
		str = "\"" + structname + str + "\"";
		break;
	case STRING_TYPE:
		str = "\"string" + str + "\"";
		break;
	}
	return str;
}
void datatype::print()
{
	cout << sprint() << endl;
}

datatype extract_type(datatype dt, int method)
{
	datatype t = dt;
	if (method == TYPE_ARRAY_REF)
	{
		t.array.erase(t.array.begin());
		if (t.array.size() == 0)
		{
			t.is_array = false;
		}
	}
	else if (method == TYPE_DEREF)
	{
		if (t.is_array)
		{
			t.array.erase(t.array.begin());
			if (t.array.size() == 0)
			{
				t.is_array = false;
			}
		}
		else
		{
			t.deref--;
		}
	}

	return t;
}

bool isVoidType(datatype t)
{
	if (t.type == VOID_TYPE && t.deref == 0 && t.is_array == false)
	{
		return true;
	}
	else
		return false;
}

bool isIntType(datatype t)
{
	if (t.type == INT_TYPE && t.deref == 0 && t.is_array == false)
	{
		return true;
	}
	else
		return false;
}

bool isFloatType(datatype t)
{
	if (t.type == FLOAT_TYPE && t.deref == 0 && t.is_array == false)
	{
		return true;
	}
	else
		return false;
}

bool isStructType(datatype t)
{
	if (t.type == STRUCT_TYPE && t.deref == 0 && t.is_array == false)
	{
		return true;
	}
	else
		return false;
}

bool isStructPointerType(datatype t)
{
	if (t.type == STRUCT_TYPE && t.deref == 1 && t.is_array == false)
	{
		return true;
	}
	else
		return false;
}

bool isDereferenceAllowed(datatype t)
{
	if (t.type == -1)
		return false;
	if (t.is_array || t.deref > 1)
		return true;
	else if (t.deref == 1 && t.type != VOID_TYPE)
		return true;
	else
		return false;
}

bool isIntOrFloatType(datatype t)
{
	return isIntType(t) || isFloatType(t);
}
bool isScalarOrPointerType(datatype t)
{
	return isIntOrFloatType(t) || isPointer(t);
}

bool isPointer(datatype t)
{
	if (t.is_array && t.array[0] == -1)
		return true;
	if (t.deref > 0 && !t.is_array)
		return true;
	return false;
}
bool isArray(datatype t)
{
	return (t.is_array && t.array[0] != -1);
}

/////////////////////////////////////////////////////////////////////////////////
datatype::datatype()
{
	array = std::vector<int>{};
}

int datatype::size()
{
	int size;

	switch (type)
	{
	case VOID_TYPE:
		size = 0;
		break;
	case INT_TYPE:
	case FLOAT_TYPE:
		size = 4;
		break;
	case STRUCT_TYPE:
		if (structname == "")
			size = 4;
		else
		{
			if (gst.Entries.find(structname) != gst.Entries.end())
			{
				size = gst.Entries[structname].size;
			}
		};
		break;
	default:;
	};
	if (deref > 0 || (is_array && array[0] == -1))
		size = 4;
	if (is_array && array[0] != -1)
		for (int i = 0; i < (int)array.size(); i++)
			size *= array[i];
	return size;
};


int datatype::struct_size()
{
	int size;

	switch (type)
	{
	case STRUCT_TYPE:
		if (structname == "")
			size = 4;
		else
		{
			if (gst.Entries.find(structname) != gst.Entries.end())
			{
				size = gst.Entries[structname].size;
			}
		};
		break;
	default:;
	};
	
	return size;
};

//////////////////////////////////////////////////////////////////////////////////

type_specifier_class::type_specifier_class(int x)
{
	type = x;
	structname = "";
}

type_specifier_class::type_specifier_class(int x, string y)
{
	type = x;
	structname = y;
}

//////////////////////////////////////////////////////////////////////////////////

fun_declarator_class::fun_declarator_class()
{
	deref = 0;
	identifier = "";
	param = NULL;
}

fun_declarator_class::fun_declarator_class(string s)
{
	deref = 0;
	identifier = s;
	param = NULL;
}

fun_declarator_class::fun_declarator_class(string s, parameter_list_class *p)
{
	deref = 0;
	identifier = s;
	param = p;
}

void fun_declarator_class::addstar()
{
	deref++;
}

//////////////////////////////////////////////////////////////////////////////////

parameter_list_class::parameter_list_class()
{
	length = 0;
}

void parameter_list_class::push(parameter_declaration_class *data)
{
	param.push_back(data);
	length++;
}

//////////////////////////////////////////////////////////////////////////////////

parameter_declaration_class::parameter_declaration_class()
{
}

parameter_declaration_class::parameter_declaration_class(type_specifier_class *typedata, declarator_class *decldata)
{
	t = *new datatype();
	t.type = typedata->type;
	t.structname = typedata->structname;
	t.deref = decldata->deref;
	if (decldata->array.size() == 0)
	{
		t.is_array = false;
	}
	else
	{
		t.is_array = true;
		t.array = decldata->array;
	}
	identifier = decldata->identifier;
}

//////////////////////////////////////////////////////////////////////////////////

declarator_class::declarator_class()
{
	deref = 0;
}

declarator_class::declarator_class(string name)
{
	identifier = name;
	deref = 0;
}

void declarator_class::addstar()
{
	deref++;
}

void declarator_class::add_array_index(int index)
{
	array.push_back(index);
}

//////////////////////////////////////////////////////////////////////////////////

declaration_class::declaration_class()
{
}

declaration_class::declaration_class(type_specifier_class *t1, declarator_list_class *d1)
{
	type = t1;
	decl_list = d1;
}

//////////////////////////////////////////////////////////////////////////////////

declarator_list_class::declarator_list_class()
{
}

void declarator_list_class::push(declarator_class *data)
{
	v.push_back(data);
}

//////////////////////////////////////////////////////////////////////////////////

declaration_list_class::declaration_list_class()
{
}

//////////////////////////////////////////////////////////////////////////////////

bool is_compatible(int x, int y, string s, string t)
{
	// cout << "In is_compatible" << endl;
	// cout << x << y << s << t << endl;
	if (x == y && s == t)
		return true;
	if ((x == INT_TYPE && y == FLOAT_TYPE) || (y == INT_TYPE && x == FLOAT_TYPE))
		return true;
	return false;
}

bool compatible(datatype t1, datatype t2, bool is_assign)
{
	//cout << t1.sprint() << t2.sprint() << endl;
	if (t1 == t2)
		return true;

	// already checked that t1 is not array

	// void* to any * thingy only in the context of assignment
	if (is_assign)
	{
		if (t1.type == VOID_TYPE && t1.deref == 1)
		{
			if (t2.is_array || t2.deref > 0)
				return true;
		}
		if (t2.type == VOID_TYPE && t2.deref == 1)
		{
			if (t1.is_array || t1.deref > 0)
				return true;
		}
	}
	// normal int - float compatibility
	if (t1.deref == 0 && !t1.is_array && t2.deref == 0 && !t2.is_array)
	{
		return is_compatible(t1.type, t2.type, t1.structname, t2.structname);
	}
	// arrays
	if (t1.is_array && t2.is_array)
	{
		if (t1.array.size() != t2.array.size())
			return false;
		for (int i = 1; i < (int)t1.array.size(); i++)
		{
			if (t1.array[i] != t2.array[i])
				return false;
		}
		return (t1.type == t2.type) && (t1.deref == t2.deref) && (t1.structname == t2.structname);
	}
    
	if (isPointer(t1))
	{
		if (isPointer(t2))
			return (normalize(t1) == normalize(t2));
		else if (isArray(t2))
			return (normalize(t1) == normalize(convert((t2))));
	}

		return false;
}

datatype normalize(datatype t)
{
	datatype tnew;
	if (!isPointer(t) || !t.is_array)
		return t;
	else if (!all_negative(t.array))
		return t;
	else
	{
		tnew.deref = t.deref + (int)t.array.size();
		tnew.type = t.type;
		tnew.is_array = false;
		tnew.structname = t.structname;
		tnew.array = {};
		return tnew;
	}
}

bool all_negative(vector<int> v)
{
	bool result = true;
	for (int i = 0; i < (int)v.size(); i++)
		if (v[i] != -1)
			result = false;
	return result;
}

datatype convert(datatype t)
{
	datatype tnew;
	if (!isArray(t))
		return t;
	else
	{
		tnew.deref = t.deref;
		tnew.type = t.type;
		tnew.is_array = t.is_array;
		tnew.structname = t.structname;
		tnew.array = t.array;
		tnew.array[0] = -1;
		return normalize(tnew);
	}
}