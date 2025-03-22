#ifndef TABLE_H
#define TABLE_H

#include "Term.h"
#include <vector>
#include <string>

using namespace std;

class Table {
public:
    vector<Term> terms;
    vector<Term> primeImplicants;
    
    Table(vector<Term> &minterms, vector<Term> &dontCares);
    void generatePrimeImplicants();
    void printPrimeImplicants();
};

#endif
