#include "Table.h"
#include "Term.h"
#include <iostream>
#include <set>
#include <algorithm>
#include <sstream>

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

    cout << "dontcares " << endl;
    for (const auto &dc : dont_cares) {
        cout << dc<< " **"<<endl;
    }

    cout << "Prime Implicants: " << endl;
    for (const auto &pi : primeImplicants) {
        cout << pi.binary << " -> " << pi.toExpression() << endl;
        for (auto &m : pi.coveredMinterms) {
            cout<< "$"<<m <<endl;
    }
    cout<< "-----------"<<endl;
}
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
            cout << "remaining pi: " << pi.toExpression() << endl;
            
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
            if (!uncoveredMinterms.empty()) {
                piToMinterms[pi.binary] = uncoveredMinterms;
                
                // Print the mapping for debugging
                cout << "  Covers uncovered minterms: ";
                for (auto m : uncoveredMinterms) {
                    cout << m << " ";
                }
                cout << endl;
            }
        }
    }
    
    // Print the reduced coverage chart for debugging
    cout << "\nReduced Coverage Chart:" << endl;
    for (auto &[minterm, pis] : reducedChart) {
        cout << "Minterm " << minterm << " covered by: ";
        for (auto &pi : pis) {
            cout << pi.toExpression() << " ";
        }
        cout << endl;
    }
    
    // // Check for row dominance opportunities based on the mapping
    // cout << "\nPotential Row Dominance:" << endl;
    // for (auto it1 = piToMinterms.begin(); it1 != piToMinterms.end(); ++it1) {
    //     for (auto it2 = piToMinterms.begin(); it2 != piToMinterms.end(); ++it2) {
    //         if (it1 != it2) {
    //             if (includes(it1->second.begin(), it1->second.end(), 
    //                          it2->second.begin(), it2->second.end())) {
    //                 // Find the corresponding Term objects
    //                 Term* pi1 = nullptr;
    //                 Term* pi2 = nullptr;
    //                 for (auto &pi : remainingPI) {
    //                     if (pi.binary == it1->first) pi1 = &pi;
    //                     if (pi.binary == it2->first) pi2 = &pi;
    //                 }
                    
    //                 if (pi1 && pi2) {
    //                     cout << "PI " << pi1->toExpression() << " dominates " 
    //                          << pi2->toExpression() << endl;
    //                 }
    //             }
    //         }
    //     }
    // }
}


void Table::applyDominanceRules() {
    std::cout << "\n=== Applying Dominance Rules ===" << std::endl;
    
    bool changed;
    int iteration = 0;
    do {
        std::cout << "\nIteration " << ++iteration << std::endl;
        changed = false;
        
        // Apply column dominance first, then row dominance
      

        bool colChanged = applyColumnDominance();
        bool rowChanged = applyRowDominance();
        
        changed = colChanged || rowChanged;
        
        // If any changes were made, update the coverage chart
        if (changed) {
            std::cout << "\nUpdating reduced chart after dominance rules..." << std::endl;
            
            // Update the reduced chart based on the remaining PIs
            for (auto& entry : reducedChart) {
                // Filter out any PIs that have been removed
                auto& piList = entry.second;
                int beforeSize = piList.size();
                piList.erase(
                    std::remove_if(piList.begin(), piList.end(), 
                        [this](const Term& pi) {
                            return std::find(remainingPI.begin(), remainingPI.end(), pi) == remainingPI.end();
                        }),
                    piList.end()
                );
                int afterSize = piList.size();
                
                if (beforeSize != afterSize) {
                    std::cout << "Updated minterm " << entry.first << ": removed " 
                              << (beforeSize - afterSize) << " PIs" << std::endl;
                }
            }
        }
    } while (changed);
    
    std::cout << "\n Dominance Rules Application Complete" << std::endl;
    std::cout << "Final remaining pis" << std::endl;
    for (const auto& entry : reducedChart) {
        std::cout << "Minterm " << entry.first << " covered by PIs: ";
        for (const auto& pi : entry.second) {
            std::cout << pi.toExpression() << " ";
        }
        std::cout << std::endl;
    }
}


bool Table::applyColumnDominance() {
    bool changed = false;
    
    std::cout << "\n--- Column Dominance Debug ---" << std::endl;
    
    // Create a copy of the keys to avoid iterator invalidation
    std::vector<int> minterms;
    for (const auto& entry : reducedChart) {
        minterms.push_back(entry.first);
        std::cout << "Minterm " << entry.first << " covered by PIs: ";
        for (const auto& pi : entry.second) {
            std::cout << pi.toExpression() << " ";
        }
        std::cout << std::endl;
    }
    
    // Check each pair of minterms for dominance
    for (size_t i = 0; i < minterms.size(); ++i) {
        // Skip if this minterm has been removed
        if (reducedChart.find(minterms[i]) == reducedChart.end()) {
            continue;
        }
        
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
                std::cout << "Column dominance: Minterm " << minterms[i] 
                          << " dominates minterm " << minterms[j] << std::endl;
                std::cout << "Removing minterm " << minterms[i] << std::endl;
                reducedChart.erase(minterms[i]);
                changed = true;
                break;
            }
        }
    }
    
    if (!changed) {
        std::cout << "No column dominance found" << std::endl;
    }
    
    return changed;
}

// bool Table::applyRowDominance() {
//     bool changed = false;
    
//     std::cout << "\n--- Row Dominance Debug ---" << std::endl;
    
//     // First, populate the piToMinterms map with uncovered minterms for each PI
//     piToMinterms.clear();
//     for (const auto& pi : remainingPI) {
//         set<int> uncoveredMinterms;
//         for (const auto& m : pi.coveredMinterms) {
//             // Only consider minterms that are in the reduced chart (uncovered by EPIs)
//             if (C_m.find(m) == C_m.end() && 
//                 find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end()) {
//                 uncoveredMinterms.insert(m);
//             }
//         }
//         piToMinterms[pi.binary] = uncoveredMinterms;
//     }
    
//     // Print all remaining PIs before applying dominance
//     std::cout << "Remaining PIs before dominance check:" << std::endl;
//     for (const auto& pi : remainingPI) {
//         std::cout << "PI: " << pi.toExpression() << " covers uncovered minterms: ";
//         for (const auto& m : piToMinterms[pi.binary]) {
//             std::cout << m << " ";
//         }
//         std::cout << std::endl;
//     }
    
//     for (auto it1 = remainingPI.begin(); it1 != remainingPI.end();) {
//         bool erased = false;
//         for (auto it2 = remainingPI.begin(); it2 != remainingPI.end();) {
//             if (it1 == it2) { ++it2; continue; }
            
//             // Check if pi1 dominates pi2 based on uncovered minterms
//             const auto& minterms1 = piToMinterms[it1->binary];
//             const auto& minterms2 = piToMinterms[it2->binary];
            
//             // If minterms2 is empty, it doesn't cover any uncovered minterms
//             if (minterms2.empty()) {
//                 ++it2;
//                 continue;
//             }
            
//             // If minterms1 includes all of minterms2, then pi1 dominates pi2
//             if (includes(minterms1.begin(), minterms1.end(), minterms2.begin(), minterms2.end())) {
//                 cout << "Row dominance: PI " << it1->toExpression() 
//                      << " dominates PI " << it2->toExpression() << endl;
//                 cout << "Removing PI: " << it2->toExpression() << endl;
//                 it2 = remainingPI.erase(it2);
                        
//                 changed = true;
//                 erased = true;  // Restart the process after making changes
//             } else {
//                 ++it2;
//             }
//         }
//         if(erased) it1 = remainingPI.begin();
//         else ++it1;
//     }
    
//     if (!changed) {
//         std::cout << "No row dominance found" << std::endl;
//     } else {
//         // Print remaining PIs after applying dominance
//         std::cout << "Remaining PIs after dominance check:" << std::endl;
//         for (const auto& pi : remainingPI) {
//             std::cout << "PI: " << pi.toExpression() << " covers uncovered minterms: ";
//             for (const auto& m : piToMinterms[pi.binary]) {
//                 std::cout << m << " ";
//             }
//             std::cout << std::endl;
//         }
//     }
    
//     return changed;
// }

     bool Table::applyRowDominance() {
    bool changed = false;
    std::cout << "\n--- Row Dominance Debug ---" << std::endl;

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

    // Print all remaining PIs before applying dominance
    std::cout << "Remaining PIs before dominance check:" << std::endl;
    for (const auto& [binary, minterms] : piToMinterms) {
        std::cout << "PI: " << binary << " covers minterms: ";
        for (const auto& m : minterms) {
            std::cout << m << " ";
        }
        std::cout << std::endl;
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
                cout << "Row dominance: PI " << it2->toExpression() 
                     << " dominates PI " << it1->toExpression() << endl;
                cout << "Removing PI: " << it1->toExpression() << endl;
                it1 = remainingPI.erase(it1);
                changed = true;
                dominated = true;
                break;
            }
        }
        if (!dominated) ++it1;
    }

    if (!changed) {
        std::cout << "No row dominance found" << std::endl;
    } else {
        // Print remaining PIs after applying dominance
        std::cout << "Remaining PIs after dominance check:" << std::endl;
        for (const auto& pi : remainingPI) {
            std::cout << "PI: " << pi.toExpression() << " covers minterms: ";
            for (const auto& m : piToMinterms[pi.binary]) {
                std::cout << m << " ";
            }
            std::cout << std::endl;
        }
    }

    return changed;
}


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
   
  processRemainingPI();
  applyDominanceRules();

 // Check if all minterms are covered

 for (auto &[m, _] : reducedChart) {
     if (find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end()) {
         uncoveredMinterms.insert(m);
     }
 }
   BestfitPI();
   cout<<"^^^^^^^^^^^\n";
   for(auto & rem: remainingPI){
    cout<<rem.toExpression()<<endl;
   }
     if (!reducedChart.empty()) {
         // Apply Petrick's method for the remaining uncovered minterms
         PetrickMethod();
      }

    // Generate the final expression
    cout << "Final Expression: ";
    for (size_t i = 0; i < EPI.size(); i++) {
        if (i > 0) cout << " + ";
        cout << EPI[i].toExpression();
    }
    cout << endl<<"<<--------------------";

FinalExpression();
 }
// After identifying EPIs, process remaining uncovered minterms
void Table::BestfitPI() {
    map<int, Term*> exclusiveCoverage;
    
    // First pass: identify minterms covered by only one PI
    for (auto &[minterm, pi_list] : reducedChart) {
        // Skip don't care terms
        if (find(dont_cares.begin(), dont_cares.end(), minterm) != dont_cares.end()) {
            continue;
        }
        
        // If this minterm is covered by exactly one PI
        if (pi_list.size() == 1) {
            exclusiveCoverage[minterm] = &pi_list[0];
           // cout<<"Bestfit: "<<exclusiveCoverage[minterm]->toExpression()<<endl;
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
            cout << "Exclusive Prime Implicant: " << pi_ptr->toExpression() 
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

void Table::FinalExpression() {
    set<string> unique_expressions;
    set<string> unique;
    // Construct base expression from Essential Prime Implicants
    string base_expr;
    for (size_t i = 0; i < EPI.size(); i++) {
        if (i > 0) base_expr += " + ";
        base_expr += EPI[i].toExpression();
    }
    // Generate possible minimized function expressions
    for (auto &[m, pi_list] : reducedChart) {
        for (auto &pi : pi_list) {
            string temp = base_expr;
            if (!pi.toExpression().empty()) {
                if (!temp.empty()) temp += " + ";
                temp += pi.toExpression();
            }
            unique_expressions.insert(temp);
        }
    }
    // Print unique minimized expressions    -->has repititions  
    cout << "Minimized Expressions:" << endl;
    for(auto & pi: selections){
        cout<<base_expr << " + " << pi.toExpression() <<endl;

    }
    
    
}

void Table::PetrickMethod() {
    // Step 1: Find uncovered minterms after EPIs
    map<int, vector<Term>> uncoveredChart;
    
    for (const auto& [minterm, pi_list] : reducedChart) { //copy reduced chart
            uncoveredChart[minterm] = pi_list;
            cout<< minterm << "---->" ;
    for (auto &p: pi_list)
    cout<<p.toExpression()<<endl;
        }
   
    
    if (uncoveredChart.empty()) {
        cout << "All minterms are covered by Essential Prime Implicants." << endl;
        return;
    }
    
    cout << "Applying Petrick's method for remaining minterms..." << endl;
    // Step 2: Assign indices to unique prime implicants
    map<string, int> piToIndex;

    vector<Term> uniquePIs;
    
    for (const auto& [minterm, pi_list] : uncoveredChart) {
        
        for (const auto& pi : pi_list) {
            if (piToIndex.find(pi.binary) == piToIndex.end()) { 

                piToIndex[pi.binary] = uniquePIs.size();
                uniquePIs.push_back(pi);
                for (const auto& [s, i] :piToIndex ) {
                }

                
            }
        }
    }


    for (auto & x:uniquePIs){

    cout<<"hereeee"<<x.toExpression()<<endl; 
    }
    
    // Step 3: Form Product of Sums expression
    vector<vector<int>> petricksExpression;
    
    for (const auto& [minterm, pi_list] : uncoveredChart) {
        vector<int> sum;
        for (const auto& pi : pi_list) {
            sum.push_back(piToIndex[pi.binary]);
        }
        petricksExpression.push_back(sum);
    }
    
  
    // Step 4: Expand to Sum of Products
    vector<vector<int>> sop = expandToPetricksSOP(petricksExpression);
    for (auto &x: sop){
        for(auto &y:x)
        cout<<"debug min: "<< y<<endl;
    }

    // Step 5: Find minimum-cost solution
   // Step 5: Find all minimal cost solutions
vector<vector<int>> minimalSolutions;
int minTerms = INT_MAX;
    
// First pass: find the minimum number of terms
for (const auto& product : sop) {
    if (product.size() < minTerms) {
        minTerms = product.size();
    }
}
    
// Second pass: collect all solutions with the minimum number of terms
vector<vector<int>> minTermCandidates;
for (const auto& product : sop) {
    if (product.size() == minTerms) {
        minTermCandidates.push_back(product);
    }
}

// Third pass: find the minimum literal count among these candidates
int minLiterals = INT_MAX;
for (const auto& product : minTermCandidates) {
    int literalCount = 0;
    for (int i : product) {
        literalCount += countLiterals(uniquePIs[i]);
    }
    if (literalCount < minLiterals) {
        minLiterals = literalCount;
    }
}

// Fourth pass: collect all solutions with minimum terms and minimum literals
for (const auto& product : minTermCandidates) {
    int literalCount = 0;
    for (int i : product) {
        literalCount += countLiterals(uniquePIs[i]);
    }
    if (literalCount == minLiterals) {
        minimalSolutions.push_back(product);
    }
}

    // Process all minimal solutions
cout << "Found " << minimalSolutions.size() << " minimal solutions:" << endl;

// Clear previous selections
selections.clear();

// Track unique PIs across all minimal solutions
set<string> uniqueSelectedPIs;

// Process each minimal solution
for (size_t solIdx = 0; solIdx < minimalSolutions.size(); solIdx++) {
    cout << "Solution " << (solIdx + 1) << ":" << endl;
    
    // For debugging
    for (auto &x : minimalSolutions[solIdx]) {
        cout << "debug best: " << x << endl;
    }
    
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
            uniqueSelectedPIs.insert(selectedPI.binary);
            cout << "Prime Implicant in solution " << (solIdx + 1) 
                 << ": " << selectedPI.toExpression() << endl;
        }
    }
    
    // Add this solution's PIs to selections
    for (const auto& pi : solutionPIs) {
        selections.push_back(pi);
    }
}

cout << "Total unique PIs selected across all minimal solutions: " 
     << uniqueSelectedPIs.size() << endl;

}


// void Table:: PetrickMethod(){
//         // Map each prime implicant to a unique index
//         map<string, int> piToIndex;
//         vector<Term> uniquePIs;
        
//         for (const auto& [minterm, pi_list] : reducedChart) {
//             for (const auto& pi : pi_list) {
//                 if (piToIndex.find(pi.binary) == piToIndex.end()) {
//                     piToIndex[pi.binary] = uniquePIs.size();
//                     uniquePIs.push_back(pi);
//                 }
//             }
//         }
        
//         // Form Product of Sums expression
//         vector<vector<int>> petricksExpression;
        
//         for (const auto& [minterm, pi_list] : reducedChart) {
//             vector<int> sum;
//             for (const auto& pi : pi_list) {
//                 sum.push_back(piToIndex[pi.binary]);
//             }
//             petricksExpression.push_back(sum);
//         }
        
//         // Expand to Sum of Products
//         vector<vector<int>> sop = expandToPetricksSOP(petricksExpression);
        
//         // Find minimum-cost solution
//         vector<int> bestSolution;
//         int minTerms = INT_MAX;
        
//         for (const auto& product : sop) {
//             if (product.size() < minTerms) {
//                 minTerms = product.size();
//                 bestSolution = product;
//             } else if (product.size() == minTerms) {
//                 // If tied for number of terms, choose the one with fewer literals
//                 int cost1 = 0, cost2 = 0;
                
//                 for (int i : bestSolution) {
//                     cost1 += countLiterals(uniquePIs[i]);
//                 }
                
//                 for (int i : product) {
//                     cost2 += countLiterals(uniquePIs[i]);
//                 }
                
//                 if (cost2 < cost1) {
//                     bestSolution = product;
//                 }
//             }
//         }
        
//         // Add selected PIs to solution
//         for (int piIdx : bestSolution) {
//             Term selectedPI = uniquePIs[piIdx];
            
//             // Check if already included
//             bool alreadyIncluded = false;
//             for (const auto& epi : EPI) {
//                 if (epi.binary == selectedPI.binary) {
//                     alreadyIncluded = true;
//                     break;
//                 }
//             }
            
//             if (!alreadyIncluded) {
//                 selections.push_back(selectedPI);
//                 cout << "Prime Implicant selected by Petrick: " << selectedPI.toExpression() << endl;
                
//                 // Update covered minterms
//                 for (auto &covered : selectedPI.coveredMinterms) {
//                     C_m.insert(covered);
//                 }
//             }
//         }
// }

// Count literals in a term (for cost calculation)
int Table::countLiterals(const Term& term) {
    int count = 0;
    for (char c : term.binary) {
        if (c != '-') count++;
    }
    return count;
}


// // Expand Petrick's expression to SOP form
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
    vector<vector<int>> minimalResult;
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
// for (auto &x: minimalResult){
//     for(auto &y:x)
//     cout<<"debug min: "<< y<<endl;
// }
    return minimalResult;
}




// void Table::PetrickMethod() {
//     cout << "Applying Petrick's Method for remaining uncovered minterms..." << endl;
    
//     // Step 1: Create Product of Sums (POS) expression
//     // Each sum term represents the PIs that cover a specific minterm
//     vector<vector<int>> POS; // Product of Sums expression
//     map<int, Term> piIndex; // Maps PI index to actual Term object
    
//     // Assign indices to remaining PIs
//     int index = 0;
//     for (const auto& pi : remainingPI) {
//         piIndex[index] = pi;
//         index++;
//     }
    
//     // For each uncovered minterm, create a sum term
//     for (auto& [minterm, pi_list] : reducedChart) {
        
//         // Create a sum term for this minterm
//         vector<int> sumTerm;
//         for (int i = 0; i < remainingPI.size(); i++) {
//             // Check if this PI covers the minterm
//             auto& pi = remainingPI[i];
//             if (find(pi.coveredMinterms.begin(), pi.coveredMinterms.end(), minterm) != pi.coveredMinterms.end()) {
//                 sumTerm.push_back(i);
//             }
//         }
        
//         if (!sumTerm.empty()) {
//             POS.push_back(sumTerm);
//         }
//     }
    
//     // Step 2: Multiply out the POS to get SOP (Sum of Products)
//     vector<vector<int>> SOP = MultiplyOutPOS(POS);
    
//     // Step 3: Find the product term with minimum cost
//     vector<int> bestProduct;
//     int minCost = INT_MAX;
//     int minLiterals = INT_MAX;
    
//     for (const auto& product : SOP) {
//         // Calculate cost (number of PIs) and literals
//         int cost = product.size();
//         int literals = 0;
        
//         for (int piIdx : product) {
//             literals += piIndex[piIdx].countLiterals();
//         }
        
//         // Update if this product has lower cost or same cost but fewer literals
//         if (cost < minCost || (cost == minCost && literals < minLiterals)) {
//             minCost = cost;
//             minLiterals = literals;
//             bestProduct = product;
//         }
//     }
    
//     // Step 4: Add the selected PIs to the final solution
//     cout << "Selected PIs from Petrick's method: " << endl;
//     for (int piIdx : bestProduct) {
//         Term selectedPI = piIndex[piIdx];
        
//         // Check if already included
//         bool already_included = false;
//         for (const auto& epi : EPI) {
//             if (epi.binary == selectedPI.binary) {
//                 already_included = true;
//                 break;
//             }
//         }
        
//         if (!already_included) {
//           //  EPI.push_back(selectedPI);
//           selections.push_back(selectedPI);
//             cout << "  " << selectedPI.toExpression() << endl;
            
//             // Mark all minterms covered by this PI
//             for (auto& covered : selectedPI.coveredMinterms) {
//                 C_m.insert(covered);
//             }
//         }
//     }
// }

// // Helper function to multiply out the Product of Sums expression
// vector<vector<int>> Table::MultiplyOutPOS(const vector<vector<int>>& POS) {
//     if (POS.empty()) {
//         return {};
//     }
    
//     vector<vector<int>> result = {{}};  // Start with empty product
    
//     for (const auto& sumTerm : POS) {
//         vector<vector<int>> newResult;
        
//         for (const auto& product : result) {
//             for (int literal : sumTerm) {
//                 vector<int> newProduct = product;
                
//                 // Only add the literal if it's not already in the product
//                 if (find(newProduct.begin(), newProduct.end(), literal) == newProduct.end()) {
//                     newProduct.push_back(literal);
//                     newResult.push_back(newProduct);
//                 } else {
//                     // If literal is already in the product, just keep the product as is
//                     newResult.push_back(product);
//                 }
//             }
//         }
        
//         // Remove duplicates from newResult
//         sort(newResult.begin(), newResult.end());
//         newResult.erase(unique(newResult.begin(), newResult.end()), newResult.end());
        
//         result = newResult;
//     }
    
//     // Apply absorption law to simplify the expression
//     ApplyAbsorptionLaw(result);
    
//     return result;
// }

// // Helper function to apply the absorption law to simplify the SOP
// void Table::ApplyAbsorptionLaw(vector<vector<int>>& SOP) {
//     bool changed = true;
    
//     while (changed) {
//         changed = false;
        
//         for (int i = 0; i < SOP.size(); i++) {
//             for (int j = 0; j < SOP.size(); j++) {
//                 if (i == j) continue;
                
//                 // Check if SOP[i] is a subset of SOP[j]
//                 bool isSubset = true;
//                 for (int term : SOP[i]) {
//                     if (find(SOP[j].begin(), SOP[j].end(), term) == SOP[j].end()) {
//                         isSubset = false;
//                         break;
//                     }
//                 }
                
//                 // If SOP[i] is a subset of SOP[j], remove SOP[j]
//                 if (isSubset && SOP[i].size() < SOP[j].size()) {
//                     SOP.erase(SOP.begin() + j);
//                     changed = true;
                    
//                     // Adjust indices after removal
//                     if (i > j) i--;
//                     j--;
//                     break;
//                 }
//             }
//         }
//     }
// }
