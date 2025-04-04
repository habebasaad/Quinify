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
    bool operator==(const Term& other) const {
        // For basic comparison, check if the binary representations match
        return binary == other.binary;}
    bool operator<(const Term& other) const {
        return binary < other.binary;}
   // int countLiterals() const;  // helper in petrick method to count literal

};

#endif