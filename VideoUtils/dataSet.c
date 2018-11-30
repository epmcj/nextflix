#include <Python.h>
#include <string.h>
#include <stdio.h>

void exec_pycode(const char* code)
{
  Py_Initialize();
  PyRun_SimpleString(code);
  Py_Finalize();
}

void RunTerminalCommand (char* line) 
{
    PyObject *py_main, *py_dict;
    printf("ok1\n");
    py_main = PyImport_AddModule("__main__");
    printf("ok2\n");
    py_dict = PyModule_GetDict(py_main);
    PyObject * PyRes = PyRun_String(line, Py_single_input, py_dict, py_dict);
	PyRun_SimpleString("\n");
}

int main(int argc,char** argv){
	char line[50];
	strcpy(line,"print('teste')");
	RunTerminalCommand(line);
	return 0;
}
