#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "Term.h"
#include <vector>
#include <string>
#include <set>

using namespace std;

class Expression {
public:
    int numVariables;
    vector<Term> minterms;
    vector<Term> maxterms;
    vector<Term> dontCares;
    string termType;
    
    Expression(const string &filename);
    void readInputFile(ifstream &file);
    string determineTermType(const string &terms);
    void parseTerms(const string &terms, char prefix, vector<Term> &termList);
    void convertMaxtermsToMinterms(const vector<Term> &maxterms);
    void validateTermCount();
    void printTerms();
    void checkForConflicts(const string &minMaxTerms, const string &dontCareTerms);

};

#endif // EXPRESSION_H
