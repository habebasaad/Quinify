#include "Expression.h"
#include "Table.h"
#include <iostream>

using namespace std;

int main() {
    Expression expr("input.txt");
    expr.printTerms();
    
    Table table(expr.minterms, expr.dontCares);
    table.printPrimeImplicants();
    table.EPIgeneration();
    
    return 0;
}
