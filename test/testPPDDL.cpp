#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <cerrno>
#include <cstring>

#include "../lib/mdpsim-2.2.2/states.h"
#include "../lib/mdpsim-2.2.2/problems.h"
#include "../lib/mdpsim-2.2.2/domains.h"
#include "../lib/mdpsim-2.2.2/actions.h"

using namespace std;

extern int yyparse();
extern FILE* yyin;
string current_file;
int warning_level = 0;
int verbosity = 0;

/* Parses the given file, and returns true on success. */
static bool read_file(const char* name)
{
  yyin = fopen(name, "r");
  if (yyin == 0) {
    cerr << "mdpclient:" << name << ": " << strerror(errno)
              << endl;
    return false;
  } else {
    current_file = name;
    bool success = (yyparse() == 0);
    fclose(yyin);
    return success;
  }
}

int main(int argc, char **argv)
{
    read_file(argv[1]);

    /* Display domains and problems */
    cerr << "----------------------------------------"<< endl << "domains:" << endl;
    for (Domain::DomainMap::const_iterator di = Domain::begin(); di != Domain::end(); di++) {
        cerr << endl << *(*di).second << endl;
    }
    cerr << "----------------------------------------"<< endl << "problems:" << endl;
    for (Problem::ProblemMap::const_iterator pi = Problem::begin(); pi != Problem::end(); pi++) {
        const Problem* problem = (*pi).second;

        cerr << "***************** Initial Atoms *****************" << endl;
        const AtomSet& atoms = problem->init_atoms();
        for (const Atom *a : atoms)
            cerr << *a << endl;

        cerr << "***************** Actions *****************" << endl;
        ValueMap values = problem->init_values();
        ActionList alist;
        problem->enabled_actions(alist, atoms, values);
        for (const Action* action : alist) {
            cerr << *action << endl;
            cerr << "---------------------------" << endl;
            for (int i = 0; i < 5; i++) {
                AtomSet atoms2 = atoms;
                action->affect(problem->terms(), atoms2, values);
                for (const Atom *a : atoms2)
                    cerr << *a << endl;
                cerr << "........" << endl;
                for (const Atom *a : atoms)
                    cerr << *a << endl;
                cerr << "---------------------------" << endl;
            }
            cerr << "+++++++++++++++++++++++++++++++" << endl;
        }
    }
    cerr << "******************************************************"<< endl;

    Problem::clear();
    Domain::clear();
}
