#ifndef TABLE_H
#define TABLE_H

#include "Term.h"
#include <vector>
#include <string>
#include <map>

using namespace std;

class Table {
public:
    vector<Term> terms;
    vector<Term> primeImplicants;
    vector<int> dont_cares;
    vector <Term> EPI;
    vector <Term> remainingPI;
    map <int, vector<Term> > CoverageChart;
    map <int, vector<Term> > reducedChart; //for uncovered minterms


    Table(vector<Term> &minterms, vector<Term> &dontCares);
    void generatePrimeImplicants();
    void printPrimeImplicants();
    void EPIgeneration();
    void FinalExpression();
    //for dominance
    void processRemainingPI();
    bool applyRowDominance();
    bool applyColumnDominance();
    void applyDominanceRules();
    //for Petrick Method
    vector<vector<int>> expandToPetricksSOP(const vector<vector<int>>& pos);
    int countLiterals(const Term& term);
    void PetrickMethod();  //the collection
};

#endif


//counter for each input minterm increments 
//when its numeber appear in the prime implicants
// Iff the counter is one --> extract this cell and
// mark ok for the minterms corresponding to this cell as visited
//the remaining uncovered minterms we need to find which prime implicant 
//will cover them to add them to the 
//expression in addition to the essential prime implicants
// that where already covered