#include "Expression.h"
#include "Table.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>

using namespace std;
//for comparing maps -> to compare the vaues in the text files and the code output
bool compareMaps(const map <int, vector<string>>& mp1, const map <int, vector<string>>& mp2){
    if(mp1.size()!= mp2.size())
    return false;
    for(const auto & [num, word]: mp1){
        if(mp2.find(num)== mp2.end())
        return false;
        const vector <string> word2 = mp2.at(num);
        set <string> s1 (word.begin(), word.end());
        set <string> s2 (word2.begin(), word2.end());
        if(s1!= s2)
        return false;
    }
    return true;
}
 //for comparing results with the ones in the test cases
void compareResults(const Table& table){
    bool foundFile = false;
    string correctFile;
    // Track around each file output
    for (int i = 1; i <= 10; i++){
        string name = "output" + to_string(i) + ".txt";
        correctFile = name;
        ifstream file(name);
        if(!file) {
            cerr << "Error: Unable to open file to compare results." << endl;
            //exit(1);
            continue; // to trace the next file
            
        }
        // reading the output file
        map <int, vector<string>> out;
        vector<string> words;
        string line;   
        int lineNum = 0;
    
    while (getline(file, line)) {
        vector<string> terms;
        stringstream ss(line);
        string term;
        
        // Split the line by '+' to get individual terms
        while (getline(ss, term, '+')) {
            // Trim whitespace from the beginning and end of the term
            term.erase(0, term.find_first_not_of(" \t"));
            term.erase(term.find_last_not_of(" \t") + 1);
            
            if (!term.empty()) {
                terms.push_back(term);
            }
        }
        
        out[lineNum] = terms;
        lineNum++;
    }
    // to compare each file with the code output
        foundFile = compareMaps(out, table.AllExpressions);
        // break the loop if found the solution
        if (foundFile)
        break;
        file.close();  //close the file
    }
    
    if(!foundFile) 
    cerr<<"No such output is matching the output test cases\n";
   else 
    cout <<"In the correct file: "<<correctFile<<" , your solution is correct\n";
}


int main() {
    Expression expr("../../tests/test2.txt");
    expr.printTerms();
    
    Table table(expr.minterms, expr.dontCares);
    table.printPrimeImplicants();
    table.EPIgeneration();
    cout<< "\n----------------------------------------------------------------------------------------------\n";
    cout<<"Now, that's the time to compare the code results with the actual output\n";
    compareResults(table);
    cout<<"We are done now, hope you enjoyed!\n\n";
    return 0;
}
