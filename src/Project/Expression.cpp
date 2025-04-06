#include "Expression.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <set>

using namespace std;

Expression::Expression(const string &filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error: Unable to open file." << endl;
        exit(1);
    }
    readInputFile(file);
}

void Expression::readInputFile(ifstream &file) {
    if (!(file >> numVariables) || numVariables < 1 || numVariables > 20) {
        cerr << "Error: Invalid number of variables." << endl;
        exit(1);
    }

    string minMaxTerms, dontCareTerms;
    getline(file, minMaxTerms);
    getline(file, minMaxTerms);
    getline(file, dontCareTerms);

    checkForConflicts(minMaxTerms, dontCareTerms);

    termType = determineTermType(minMaxTerms);
    
    // Parse terms
    vector<Term> terms;
    parseTerms(minMaxTerms, termType == "minterms" ? 'm' : 'M', terms);
    parseTerms(dontCareTerms, 'd', dontCares);
    
    // If we have maxterms, convert to minterms
    if (termType == "maxterms") {
        maxterms = terms;
        convertMaxtermsToMinterms(maxterms);
    } else {
        minterms = terms;
    }
    
    validateTermCount();
}


void Expression::checkForConflicts(const string &minMaxTerms, const string &dontCareTerms) {
    // Check for mixed term types (both 'm' and 'M')
    bool hasMinterm = false;
    bool hasMaxterm = false;
    
    stringstream ss1(minMaxTerms);
    string token;
    set<int> termValues;
    
    
    while (getline(ss1, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        if (token.size() > 1) {
            if (token[0] == 'm') hasMinterm = true;
            if (token[0] == 'M') hasMaxterm = true;
            
            if (token.size() > 1) {
                int value = stoi(token.substr(1));
                termValues.insert(value);
            }
        }
    }
    
    if (hasMinterm && hasMaxterm) {
        cerr << "Error: Mixed term types (both minterms and maxterms) are not allowed." << endl;
        exit(1);
    }
    
    // Check for conflicts between terms and don't cares
    stringstream ss2(dontCareTerms);
    while (getline(ss2, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        if (token.size() > 1 && token[0] == 'd') {
            int value = stoi(token.substr(1));
            if (termValues.find(value) != termValues.end()) {
                cerr << "Error: Term " << value << " is both a " 
                     << (hasMinterm ? "minterm" : "maxterm") 
                     << " and a don't care." << endl;
                exit(1);
            }
        }
    }
}

string Expression::determineTermType(const string &terms) {
    stringstream ss(terms);
    string token;
    bool hasTerms = false;
    
    while (getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        if (!token.empty()) {
            hasTerms = true;
            if (token[0] == 'm') return "minterms";
            if (token[0] == 'M') return "maxterms";
        }
    }
    
    // If we have an empty terms list (only don't cares or no inputs at all)
    if (!hasTerms) {
        return "minterms"; // Default to minterms when only don't cares are present
    }
    
    cerr << "Error: Could not determine term type." << endl;
    exit(1);
}



void Expression::convertMaxtermsToMinterms(const vector<Term> &maxterms) {
    // Create a set of all possible term values
    set<int> allTermValues;
    for (int i = 0; i < pow(2, numVariables); i++) {
        allTermValues.insert(i);
    }
    
    // Remove maxterms and don't-cares from the set
    for (const auto &term : maxterms) {
        allTermValues.erase(term.value);
    }
    
    for (const auto &term : dontCares) {
        allTermValues.erase(term.value);
    }
    
    // The remaining values are the minterms
    for (int value : allTermValues) {
        minterms.emplace_back(value, numVariables);
    }
}


void Expression::parseTerms(const string &terms, char prefix, vector<Term> &termList) {
    stringstream ss(terms);
    string token;
    while (getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" "));
        if (token.size() > 1 && token[0] == prefix) {
            int num = stoi(token.substr(1));
            if (num < 0 || num >= pow(2, numVariables)) {
                cerr << "Error: Invalid term value: " << num << endl;
                exit(1);
            }
            termList.emplace_back(num, numVariables);
        }
    }
}

void Expression::validateTermCount() {
    int totalTerms = minterms.size() + dontCares.size();
    if (totalTerms < 0 || totalTerms > (pow(2, numVariables))) {
        cerr << "Error: Invalid number of terms." << endl;
        exit(1);
    }
    else if (totalTerms == (pow(2, numVariables))) {
        cerr << "The function has all of the input as either minterms or don't cares, so it will always result in 1. Just connect wires instead" << endl;
        exit(1);
    }

}

void Expression::printTerms() {
    cout<< "----------------------------------------------------------------------------------------------"<<endl;
    cout<<"\t\t\t\tWelcome to Quinify\n";
    cout<<"Here, you will be able to generate the logic minimization using Quine McCluskey with up to 20 variables\n";
    cout<<"Enjoy the journey with Quinify\n";
    cout<<"Now, the project is starting processing the variable you entered!!!\n\n\n";

    cout << "Number of variables: " << numVariables << endl;
    
    if (termType == "maxterms") {
        cout << "Original Maxterms: ";
        for (const auto &term : maxterms) cout << term.value << " ";
        cout << endl;
        cout << "Converted to Minterms: ";
    } else {
        cout << "Minterms: ";
    }
    
    for (const auto &term : minterms) cout << term.value << " ";
    cout << endl;
    
    cout << "Binary representation: ";
    for (const auto &term : minterms) cout << term.binary << " ";
    cout << endl;
    
    cout << "Don't-care terms: ";
    for (const auto &term : dontCares) cout << term.value << " ";
    cout << endl;
}



