#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <bitset>
#include <cmath>

using namespace std;

void readInputFile(ifstream &file, int &numVariables, string &minMaxTerms, string &dontCareTerms) {
    if (!(file >> numVariables)) {
        cerr << "Error: Failed to read the number of variables." << endl;
        exit(1);
    }
    string s;
    getline(file, s); // Consumes the \n character    
    if (!getline(file, minMaxTerms)) {
        cerr << "Error: Failed to read the second line." << endl;
        exit(1);
    }
    
    if (!getline(file, dontCareTerms)) {
        cerr << "Error: Failed to read the third line." << endl;
        exit(1);
    }
}

void parseTerms(const string &terms, char prefix, unordered_set<int> &termSet) {
    stringstream ss(terms);
    string token;
    while (getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" ")); // Trim leading spaces
        if (token.size() > 1 && token[0] == prefix) {
            int num = stoi(token.substr(1));
            termSet.insert(num);
        }
    }
}
string determineTermType(const string &minMaxTerms) {
    stringstream ss(minMaxTerms);
    string token;

    while (getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" ")); // Trim leading spaces
        
        if (!token.empty()) {
            if (token[0] == 'm') return "minterms";
            if (token[0] == 'M') return "maxterms";
        }
    }

    cerr << "Error: Could not determine if terms are minterms or maxterms." << endl;
    exit(1);
}
vector<string> convertToBinary(const unordered_set<int> &termSet, int numVariables) {
    vector<string> binaryTerms;
    for (int num : termSet) {
        string binary = bitset<20>(num).to_string(); // Convert number to a 20-bit binary string (since 20 is the max num of variables)
        binaryTerms.push_back(binary.substr(20 - numVariables)); // Extract only relevant bits
    }
    return binaryTerms;
}

int main() {
    ifstream file("D:/AUC/Spring 2025/DD1/Project/input.txt");
    if (!file) {
        cerr << "Error: Unable to open file." << endl;
        return 1;
    }

    int numVariables;
    string minMaxTerms, dontCareTerms;
    readInputFile(file, numVariables, minMaxTerms, dontCareTerms);
    if(numVariables < 1 || numVariables > 20) {
        cerr << "Error: Number of variables must be between 1 and 20." << endl;
        return 1;
    }
    string termType = determineTermType(minMaxTerms);
    
    unordered_set<int> minMaxSet, dontCareSet;
    parseTerms(minMaxTerms, termType == "minterms" ? 'm' : 'M', minMaxSet);
    parseTerms(dontCareTerms, 'd', dontCareSet);
    


    int noOfTerms = minMaxSet.size() + dontCareSet.size();
    if (noOfTerms < 0 || noOfTerms > (pow(2, numVariables) - 1)) {
        cerr << "Error: Number of terms must be between 0 and 2^numVariables - 1." << endl;
        return 1;
    }
    
    
    vector<string> binaryMinMax = convertToBinary(minMaxSet, numVariables);
    vector<string> binaryDontCare = convertToBinary(dontCareSet, numVariables);

    cout << "Number of variables: " << numVariables << endl;
    if(termType == "minterms") 
        cout << "Minterms: ";
    else 
        cout << "Maxterms: " ;
    for (int num : minMaxSet) cout << num << " ";
    cout << endl;
    
    if(termType == "minterms") 
        cout << "Minterms: ";
    else 
        cout << "Maxterms: " ;
    for (const string &bin : binaryMinMax) cout << bin << " ";
    cout << endl;

    cout << "Don't-care terms in decimal: ";
    for (int num : dontCareSet) cout << num << " ";
    cout << endl;
    
    cout << "Don't-care terms in binary: ";
    for (const string &bin : binaryDontCare) cout << bin << " ";
    cout << endl;
    
    return 0;
}




