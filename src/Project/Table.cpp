#include "Table.h"
#include "Term.h"
#include "verilog.h"
#include <iostream>
#include <set>
#include <algorithm>
#include <sstream>
#include <iomanip>

using namespace std;

Table::Table(vector<Term>& minterms, vector<Term>& dontCares) {
    // Combine minterms and don't-cares
    terms = minterms;
    terms.insert(terms.end(), dontCares.begin(), dontCares.end());
    
    // Store don't care values
    for (const auto& dc : dontCares) {
        dont_cares.push_back(dc.value);
    }
    
    generatePrimeImplicants();
}

void Table::generatePrimeImplicants() {
    vector<vector<Term>> groups(21); // Assuming max 20 variables
    set<string> visited;
    
    // Group terms by the number of 1s in their binary representation
    for (const auto &term : terms) {
        int ones = Term::countOnes(term.binary);
        groups[ones].push_back(term);
    }
    
    bool merged = true;
    while (merged) {
        merged = false;
        vector<vector<Term>> newGroups(21);
        set<string> used;
        
        // Try to merge terms from adjacent groups
        for (size_t i = 0; i < groups.size() - 1; i++) {
            for (const auto &term1 : groups[i]) {
                for (const auto &term2 : groups[i + 1]) {
                    if (Term::canCombine(term1, term2)) {
                        // Create a new term from the combination
                        Term newTerm(-1, term1.binary.size());
                        newTerm.binary = Term::combineTerms(term1, term2);
                        
                        // Combine covered minterms
                        newTerm.coveredMinterms = term1.coveredMinterms;
                        newTerm.coveredMinterms.insert(
                            newTerm.coveredMinterms.end(), 
                            term2.coveredMinterms.begin(), 
                            term2.coveredMinterms.end()
                        );
                        
                        // Add to new groups if not already visited
                        if (visited.find(newTerm.binary) == visited.end()) {
                            newGroups[i].push_back(newTerm);
                            visited.insert(newTerm.binary);
                        }
                        
                        // Mark terms as used
                        used.insert(term1.binary);
                        used.insert(term2.binary);
                        merged = true;
                    }
                }
            }
        }
        
        // Collect prime implicants (terms that weren't merged)
        for (const auto &group : groups) {
            for (const auto &term : group) {
                if (used.find(term.binary) == used.end()) {
                    primeImplicants.push_back(term);
                    visited.insert(term.binary);
                }
            }
        }
        
        // Update groups for next iteration
        groups = newGroups;
    }
}

void Table::printPrimeImplicants() {
    cout<< "----------------------------------------------------------------------------------------------"<<endl;
    cout << "Print Prime implicants \n";
    cout << "PIs \t\t\t\t\t-> Binary\t\t\t\t\t-> String \n";
    for (const auto &pi : primeImplicants) {
        for (auto &m : pi.coveredMinterms){
            if(m == pi.coveredMinterms[pi.coveredMinterms.size()-1])
            cout<<m;
            else
            cout<<m << " & ";
        }
        cout<< "\t\t\t\t\t-> " << pi.binary <<"\t\t\t\t\t-> " << pi.toExpression() << endl;
        

}
cout<< "----------------------------------------------------------------------------------------------"<<endl;
}
// Generating EPIs from the table of PI after forming them
void Table::EPIgeneration() {
    // coverage chart
  
    for (auto &pi : primeImplicants) {
        for (auto &m : pi.coveredMinterms) {
            if (find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end())
            CoverageChart[m].push_back(pi);
        }
    }
        for (auto &[m, pi_list] : CoverageChart) {

        // Skip don't care terms when identifying EPIs
        if (find(dont_cares.begin(), dont_cares.end(), m) != dont_cares.end()) {
            cout<< "dontcare skipped : "<< m << endl;
        continue;
        }

        if (pi_list.size() == 1) {
            Term essentialPI = pi_list[0];
            
                // avoid duplicates
            bool already_included = false;
            for (const auto &epi : EPI) {
                if (epi.binary == essentialPI.binary) {
                    already_included = true;
                    break;
                }
                
            }
            if (!already_included) {
                EPI.push_back(essentialPI);
                cout << "Essential Prime Implicant: " << essentialPI.toExpression() << endl;
                
                
                // Mark all minterms covered by this EPI
                for (auto &covered : essentialPI.coveredMinterms) {
                    C_m.insert(covered);
                }
               
            }
        }
   }
cout<< "----------------------------------------------------------------------------------------------"<<endl;

   // After extracting all the EPIs, process remaining PI is generated to get the remaining PIs
  processRemainingPI();
  // Applying dominance rule on the remaining PIs
  applyDominanceRules();

 // Check and get all the remaining minterms after the dominance rule to be applied in the Petrick method
 for (auto &[m, _] : reducedChart) {
 }
 // extracting Bestfit PIs from the remaining PIs after domination rule
   BestfitPI();
   cout<<"Remaining PIs after domination rule\n";
   if(remainingPI.size()== 0)
   cout<<"There is not remaining PI \n";
   else
   for(auto & rem: remainingPI){
    cout<<rem.toExpression()<<endl;
   }
     if (!reducedChart.empty()) {
         // Apply Petrick's method for the remaining uncovered minterms
         PetrickMethod();
      }

    // Generate the final expression
    cout<< "\n----------------------------------------------------------------------------------------------\n";
    cout << "\t\t\t\tFinal Expression with EPIs and bestfit PIs\n";
    for (size_t i = 0; i < EPI.size(); i++) {
        if (i > 0) cout << " + ";
        cout << EPI[i].toExpression();
    }
    cout<< "\n----------------------------------------------------------------------------------------------\n";

FinalExpression();
}

void Table::processRemainingPI() {
    // Clear existing collections
    remainingPI.clear();
    reducedChart.clear();
    
    // Create a map to track which minterms each PI covers (after reduction)

    // Process each prime implicant
    for (auto &pi : primeImplicants) {
        // Check if this PI is already an EPI
        bool isEPI = false;
        for (const auto &epi : EPI) {
            if (pi.binary == epi.binary) {
                isEPI = true;
                break;
            }
        }
        
        // If not an EPI, add to remaining PIs and update reduced chart
        if (!isEPI) {
            // Add to remaining PI list
            remainingPI.push_back(pi);
            cout << "Remaining pI: " << pi.toExpression() << endl;
            
            // Track uncovered minterms for this PI
            set<int> uncoveredMinterms;
            
            // Update reduced chart with uncovered minterms
            for (auto &m : pi.coveredMinterms) {
                if (C_m.find(m) == C_m.end() && 
                    find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end()) {
                    reducedChart[m].push_back(pi);
                    uncoveredMinterms.insert(m);
                }
            }
            
            // Map this PI to its uncovered minterms
            if (!uncoveredMinterms.empty())
                piToMinterms[pi.binary] = uncoveredMinterms;
        }
    }
    
    // Print the reduced coverage chart for debugging
    cout<< "----------------------------------------------------------------------------------------------"<<endl;
    cout << "\nReduced Coverage Chart\n";
    for (auto &[minterm, pis] : reducedChart) {
        cout << "Minterm " << minterm << " covered by: ";
        for (auto &pi : pis) {
            cout << pi.toExpression() << " ";
        }
        cout << endl;
    }
    cout<< "----------------------------------------------------------------------------------------------"<<endl;
}


void Table::applyDominanceRules() {
    std::cout << "\n\t\t\t\t=== Applying Dominance Rules ===" << std::endl;
    
    bool changed;
    int iteration = 0;
    do {
        changed = false;
        
        // Apply column dominance first, then row dominance
    
        bool colChanged = applyColumnDominance();
        bool rowChanged = applyRowDominance();
        
        changed = colChanged || rowChanged;
        
        // If any changes were made, update the coverage chart
        if (changed) {            
            // Update the reduced chart based on the remaining PIs
            for (auto& entry : reducedChart) {
                // Filter out any PIs that have been removed
                auto& piList = entry.second;
                int beforeSize = piList.size();
                piList.erase(
                    remove_if(piList.begin(), piList.end(), 
                        [this](const Term& pi) {
                            return find(remainingPI.begin(), remainingPI.end(), pi) == remainingPI.end();
                        }),
                    piList.end()
                );
                int afterSize = piList.size();
            }
        }
    } while (changed);
    
    cout << "Remaining PIs with minterms after domination" << std::endl;
    for (const auto& entry : reducedChart) {
        std::cout << "Minterm " << entry.first << " covered by PIs: ";
        for (const auto& pi : entry.second) {
            std::cout << pi.toExpression() << " ";
        }
        std::cout << std::endl;
    }
    if(!remainingPI.empty()){
    cout<< "\n----------------------------------------------------------------------------------------------\n";
    cout<<"Remaining PIs after domination: ";
    for(auto & rem: remainingPI){
        if(rem == remainingPI[remainingPI.size()-1])
        cout << rem.toExpression()<<endl;
        else
        cout<< rem.toExpression()<<" , ";
    }
    }
    cout<< "\n----------------------------------------------------------------------------------------------\n";
}


bool Table::applyColumnDominance() {
    bool changed = false;   
    // Create a copy of the keys to avoid iterator invalidation
    vector<int> minterms;
    for (const auto& entry : reducedChart) {
        minterms.push_back(entry.first);
    }
    
    // Check each pair of minterms for dominance
    for (size_t i = 0; i < minterms.size(); ++i) {
        // Skip if this minterm has been removed
        if (reducedChart.find(minterms[i]) == reducedChart.end())
            continue;
        
        for (size_t j = 0; j < minterms.size(); ++j) {
            // Skip self-comparison or if second minterm has been removed
            if (i == j || reducedChart.find(minterms[j]) == reducedChart.end()) {
                continue;
            }
            
            // Check if minterm[i]'s PIs include all of minterm[j]'s PIs
            const auto& piList1 = reducedChart[minterms[i]];
            const auto& piList2 = reducedChart[minterms[j]];
            
            if (includes(piList1.begin(), piList1.end(),
                         piList2.begin(), piList2.end())) {
                reducedChart.erase(minterms[i]);
                changed = true;
                break;
            }
        }
    }
    
    return changed;
}

bool Table::applyRowDominance() {
    bool changed = false;

    // Create a map of PIs to their covered minterms
    map<string, set<int>> piToMinterms;
    for (const auto& pi : remainingPI) {
        set<int> coveredMinterms;
        for (const auto& m : pi.coveredMinterms) {
            if (C_m.find(m) == C_m.end() && 
                find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end()) {
                coveredMinterms.insert(m);
            }
        }
        piToMinterms[pi.binary] = coveredMinterms;
    }

    // Check for row dominance
    for (auto it1 = remainingPI.begin(); it1 != remainingPI.end();) {
        bool dominated = false;
        for (auto it2 = remainingPI.begin(); it2 != remainingPI.end(); ++it2) {
            if (it1 == it2) continue;

            const auto& minterms1 = piToMinterms[it1->binary];
            const auto& minterms2 = piToMinterms[it2->binary];

            // Check if it2 dominates it1
            if (includes(minterms2.begin(), minterms2.end(), minterms1.begin(), minterms1.end()) &&
                minterms1 != minterms2) {
                it1 = remainingPI.erase(it1);
                changed = true;
                dominated = true;
                break;
            }
        }
        if (!dominated) ++it1;
    }
    return changed;
}

 // After domination rules, process remaining uncovered minterms
void Table::BestfitPI() {
    cout<<"Selecting the best fit PIs after domination rules\n";
    map<int, Term*> exclusiveCoverage;
    
    // First pass: identify minterms covered by only one PI
    for (auto &[minterm, pi_list] : reducedChart) {
        // If this minterm is covered by exactly one PI
        if (pi_list.size() == 1) {
            exclusiveCoverage[minterm] = &pi_list[0];
        }
    }
    
   // Second pass: collect unique PIs that have exclusive coverage
    for (auto &[minterm, pi_ptr] : exclusiveCoverage) {
        // Check if this PI is already included
        bool already_included = false;
        for (const auto &epi : EPI) {
            if (epi.binary == pi_ptr->binary) {
                already_included = true;
                break;
            }
        }
        
        if (!already_included) {
            EPI.push_back(*pi_ptr);
            cout << "Best fit PI: " << pi_ptr->toExpression() 
                 << " (exclusively covers minterm " << minterm << ")" << endl;
            
            // Mark all minterms covered by this PI
            for (auto &covered : pi_ptr->coveredMinterms)
                C_m.insert(covered);
        }
        // Remove this PI from remainingPI
        auto it = find_if(remainingPI.begin(), remainingPI.end(), 
        [&](const Term& t) { return t.binary == pi_ptr->binary; });
    if (it != remainingPI.end()) {
      //  cout << "Removing " << it->toExpression() << " from remaining PIs" << endl;
        remainingPI.erase(it);
    }
    
    // Update reducedChart by removing covered minterms
    for (auto &covered : pi_ptr->coveredMinterms) {
        // Skip don't care terms
        if (find(dont_cares.begin(), dont_cares.end(), covered) != dont_cares.end()) {
            continue;
        }
        // Remove the minterm from the reduced chart
        reducedChart.erase(covered);
       // cout << "Removed minterm " << covered << " from reduced chart" << endl;
    }
    }
}

void Table::PetrickMethod() {
    if (reducedChart.empty()) {
        cout << "All minterms are covered by Essential Prime Implicants." << endl;
        return;
    }
    cout<< "\n----------------------------------------------------------------------------------------------\n";
    cout << "\t\t\t\tApplying Petrick's method for remaining minterms" << endl;
    map<string, int> piToIndex;
        // Assigning indices to unique prime implicants
    for (const auto& [minterm, pi_list] : reducedChart) {
        for (const auto& pi : pi_list) {
            if (piToIndex.find(pi.binary) == piToIndex.end()) { 
                piToIndex[pi.binary] = uniquePIs.size();
                uniquePIs.push_back(pi);
                for (const auto& [s, i] :piToIndex ) {
                }

                
            }
        }
    }    
    //Forming Product of Sums expression
    vector<vector<int>> petricksExpression;
    
    for (const auto& [minterm, pi_list] : reducedChart) {
        vector<int> sum;
        for (const auto& pi : pi_list) {
            sum.push_back(piToIndex[pi.binary]);
        }
        petricksExpression.push_back(sum);
    }
    
    //Expanding to Sum of Products
    vector<vector<int>> sop = expandToPetricksSOP(petricksExpression);

// Step 5: Find all minimal term solutions (without considering literal count)
int minTerms = INT_MAX;
    
// First: find the minimum number of terms
for (const auto& product : sop) {
    if (product.size() < minTerms) {
        minTerms = product.size();
    }
}
    
// Second: collect all solutions with the minimum number of terms
// (These are all considered equally minimal since we don't care about complement cost)
for (const auto& product : sop) {
    if (product.size() == minTerms) {
        minimalSolutions.push_back(product);
    }
}

// Process all minimal solutions
cout << "Found " << minimalSolutions.size() << " minimal solutions:" << endl;

// Clear previous selections
selections.clear();

// Process each minimal solution separately
vector<vector<Term>> allSolutions;

// Process each minimal solution
for (size_t solIdx = 0; solIdx < minimalSolutions.size(); solIdx++) {
    cout << "Solution " << (solIdx + 1) << ":" << endl;
    
    // Store the PIs for this solution
    vector<Term> solutionPIs;
    
    // Process each PI in this solution
    for (int piIdx : minimalSolutions[solIdx]) {
        Term selectedPI = uniquePIs[piIdx];
        
        // Check if already included in EPIs
        bool alreadyIncluded = false;
        for (const auto& epi : EPI) {
            if (epi.binary == selectedPI.binary) {
                alreadyIncluded = true;
                break;
            }
        }
        
        if (!alreadyIncluded) {
            solutionPIs.push_back(selectedPI);
            cout << "Prime Implicant in solution " << (solIdx + 1) 
                 << ": " << selectedPI.toExpression() << endl;
        }
    }
    
    // Store this solution
    allSolutions.push_back(solutionPIs);
}

// Now generate expressions for each solution
for (size_t i = 0; i < allSolutions.size(); i++) {
    // Clear previous selections
    selections = allSolutions[i];
    
    // Generate and print this expression
    cout << "Minimized Expression " << (i + 1) << ": ";
    
    string expr = "";
    // First add all EPIs
    for (size_t j = 0; j < EPI.size(); j++) {
        if (j > 0 || !expr.empty()) expr += " + ";
        expr += EPI[j].toExpression();
    }
    
    // Then add the selected PIs for this solution
    for (const auto& pi : selections) {
        if (!expr.empty()) expr += " + ";
        expr += pi.toExpression();
    }
    
    cout << expr << endl;
    
    // Add to AllExpressions map for later use
    vector<string> terms;
    stringstream ss(expr);
    string term;
    while (getline(ss, term, '+')) {
        // Trim whitespace
        term.erase(0, term.find_first_not_of(" \t"));
        term.erase(term.find_last_not_of(" \t") + 1);
        if (!term.empty()) {
            terms.push_back(term);
        }
    }
    AllExpressions[i] = terms;
}

cout << "Total minimal solutions found: " << allSolutions.size() << endl;
}

// Count literals in a term (for cost calculation)
int Table::countLiterals(const Term& term) {
    int count = 0;
    for (char c : term.binary) {
        if (c != '-') count++;
    }
    return count;
}


// Expand Petrick's expression to SOP form
vector<vector<int>> Table::expandToPetricksSOP(const vector<vector<int>>& pos) {
    if (pos.empty()) return {{}};
    
    // Initialize with first clause
    vector<vector<int>> result;
    for (int term : pos[0]) {
        result.push_back({term});
    }
    
    // Process remaining clauses
    for (size_t i = 1; i < pos.size(); i++) {
        vector<vector<int>> newResult;
        for (const auto& existingProduct : result) {
            for (int term : pos[i]) {
                vector<int> newProduct = existingProduct;
                // Add term if not already present
                if (find(newProduct.begin(), newProduct.end(), term) == newProduct.end()) {
                    newProduct.push_back(term);
                }
                newResult.push_back(newProduct);
            }
        }
        result = newResult;
    }
    
    // Sort each product for consistent comparison
    for (auto& product : result) {
        sort(product.begin(), product.end());
    }
    
    // Remove duplicates
    sort(result.begin(), result.end());
    result.erase(unique(result.begin(), result.end()), result.end());
    
    // Apply absorption law properly
for (size_t i = 0; i < result.size(); i++) {
    bool isRedundant = false;
    for (size_t j = 0; j < result.size(); j++) {
        if (i != j && includes(result[i].begin(), result[i].end(), 
                              result[j].begin(), result[j].end())) {
            // If result[j] is a subset of result[i], then result[i] is redundant
            isRedundant = true;
            break;
        }
    }
    if (!isRedundant) {
        minimalResult.push_back(result[i]);
    }
}

    return minimalResult;
}


void Table::FinalExpression() {
    // Construct base expression from Essential Prime Implicants
    string base_expr;
    vector<string> exp;
    for (size_t i = 0; i < EPI.size(); i++) {
        if (i > 0) base_expr += " + ";
        base_expr += EPI[i].toExpression();
        exp.push_back(EPI[i].toExpression());
    }

    // Process all minimal solutions from Petrick's method
    cout << "\t\t\tPossible Function Minimizations" << endl;
    
    if (minimalSolutions.empty()) {
        // If no solutions from Petrick's method, just print the EPIs
        cout << "F = " << base_expr << endl;
        AllExpressions[0] = exp;
        
        // Generate Verilog for the base expression
        std::string verilogCode = generateVerilogModule(base_expr, "minimized_logic");
        std::string filename = "minimized_logic.v";
        std::ofstream outFile(filename);
        if (outFile.is_open()) {
            outFile << verilogCode;
            outFile.close();
            std::cout << "Generated Verilog module written to " << filename << std::endl;
        } else {
            std::cerr << "Failed to open " << filename << " for writing" << std::endl;
        }
    } else {
        // Store all possible expressions
        vector<vector<Term>> allSolutions;
        
        // Process each minimal solution
        for (size_t solIdx = 0; solIdx < minimalSolutions.size(); solIdx++) {
            // Store the PIs for this solution
            vector<Term> solutionPIs;
            
            // Process each PI in this solution
            for (int piIdx : minimalSolutions[solIdx]) {
                Term selectedPI = uniquePIs[piIdx];
                
                // Check if already included in EPIs
                bool alreadyIncluded = false;
                for (const auto& epi : EPI) {
                    if (epi.binary == selectedPI.binary) {
                        alreadyIncluded = true;
                        break;
                    }
                }
                
                if (!alreadyIncluded) {
                    solutionPIs.push_back(selectedPI);
                }
            }
            
            // Add this solution to all selections
            allSolutions.push_back(solutionPIs);
        }
        
        // Print each possible function minimization
        for (size_t i = 0; i < allSolutions.size(); i++) {
            string expr = base_expr;
            vector<string> terms = exp;  // Start with EPIs
            
            // Add the selected PIs for this solution
            for (const auto& pi : allSolutions[i]) {
                if (!expr.empty()) expr += " + ";
                expr += pi.toExpression();
                terms.push_back(pi.toExpression());
            }
            
            cout << "F = " << expr << endl;
            
            // Store in AllExpressions map
            AllExpressions[i] = terms;
            
            // Generate Verilog for each minimized expression
            std::string moduleName = "minimized_logic_" + std::to_string(i);
            std::string verilogCode = generateVerilogModule(expr, moduleName);
            std::string filename = moduleName + ".v";
            std::ofstream outFile(filename);
            if (outFile.is_open()) {
                outFile << verilogCode;
                outFile.close();
                std::cout << "Generated Verilog module written to " << filename << std::endl;
            } else {
                std::cerr << "Failed to open " << filename << " for writing" << std::endl;
            }
        }
    }
}