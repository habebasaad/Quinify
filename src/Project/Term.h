// Term.h
#ifndef TERM_H
#define TERM_H
#include <string>
#include <algorithm>
#include <vector>
using namespace std;

class Term {
public:
    int value;
    string binary;
    bool used;
    vector<int> coveredMinterms;
    Term(int val, int numVariables);
    static int countOnes(const string &binary);
    static bool canCombine(const Term &a, const Term &b);
    static string combineTerms(const Term &a, const Term &b);
    string toExpression() const;

};

#endif