#include "Term.h"
#include <bitset>
#include <iostream>

Term::Term(int val, int numVariables) : value(val), used(false) {
    binary = bitset<20>(val).to_string().substr(20 - numVariables);
    coveredMinterms.push_back(val);
}

int Term::countOnes(const string &binary) {
    return count(binary.begin(), binary.end(), '1');
}

bool Term::canCombine(const Term &a, const Term &b) {
    int diffCount = 0;
    for (size_t i = 0; i < a.binary.size(); i++) {
        if (a.binary[i] != b.binary[i]) diffCount++;
        if (diffCount > 1) return false;
    }
    return diffCount == 1;
}

string Term::combineTerms(const Term &a, const Term &b) {
    string combined = a.binary;
    for (size_t i = 0; i < combined.size(); i++) {
        if (a.binary[i] != b.binary[i]) {
            combined[i] = '-';
        }
    }
    return combined;
}
string Term::toExpression() const {
    string expr;
    char var = 'A';
    
    for (size_t i = 0; i < binary.size(); i++) {
        if (binary[i] == '1') {
            expr += var;
        } else if (binary[i] == '0') {
            expr += var;
            expr += "'";
        }
        // Don't add anything to the expression if binary[i] is '-'
        var++;  // Always increment the variable name
    }
    
    return expr;
}
