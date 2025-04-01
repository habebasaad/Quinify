#include "Table.h"
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
    set<int> coveredMinterms;
    for (auto &epi : EPI) {
        for (auto &m : epi.coveredMinterms) {
            if (find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end())
                coveredMinterms.insert(m);
        }
    }

    remainingPI.clear();
    reducedChart.clear();
    // for (auto &pi : primeImplicants) {
    //     if (find(EPI.begin(), EPI.end(), pi) == EPI.end()) {
    //         remainingPI.push_back(pi);
    //         for (auto &m : pi.coveredMinterms) {
    //             if (coveredMinterms.find(m) == coveredMinterms.end() && 
    //                 find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end())
    //                 reducedChart[m].push_back(pi);
    //         }
    //     }
    // }

    // Find remaining prime implicants and uncovered minterms
    for (auto &pi : primeImplicants) {
        // Skip PIs already in EPI list
        bool isEPI = false;
        for (const auto &epi : EPI) {
            if (pi.binary == epi.binary) {
                isEPI = true;
                break;
            }
        }
        if (!isEPI) {
            // Add to remaining PI list
            remainingPI.push_back(pi);
            
            // Update reduced chart with uncovered minterms
            for (auto &m : pi.coveredMinterms) {
                if (coveredMinterms.find(m) == coveredMinterms.end() && 
                    find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end())
                    reducedChart[m].push_back(pi);
            }
        }
    }
}

void Table::applyDominanceRules() {
    bool changed;
    do {
        changed = false;
        // bitwise or operator to track the changes.
        changed |= applyColumnDominance();
        changed |= applyRowDominance();
    } while (changed);
}

bool Table::applyColumnDominance() {
    bool changed = false;
    for (auto it1 = reducedChart.begin(); it1 != reducedChart.end();) {
        bool erased = false;
        for (auto it2 = reducedChart.begin(); it2 != reducedChart.end();++it2) {
            if (it1 == it2) { continue; }
            if (includes(it1->second.begin(), it1->second.end(), 
                         it2->second.begin(), it2->second.end())) {
                it1 = reducedChart.erase(it1);
                erased = true;
                changed = true;
                break;
            } }
            if(!erased) ++it1;
    }
    return changed;
}

bool Table::applyRowDominance() {
    bool changed = false;
        for (auto it1 = remainingPI.begin(); it1 != remainingPI.end();) {
            bool erased = false;
            for (auto it2 = remainingPI.begin(); it2 != remainingPI.end();) {
                if (it1 == it2) { ++it2; continue; }
                
                // check if t1 includes t2 (all t2 in t1)
                if (includes(it1->coveredMinterms.begin(), it1->coveredMinterms.end(),
                             it2->coveredMinterms.begin(), it2->coveredMinterms.end())) {
                    it2 = remainingPI.erase(it2);
                    changed = true;
                    erased = true;  // Restart the process after making changes
                } else {
                    ++it2;
                }
            }
            if(erased) it1 = remainingPI.begin();
            else ++it1;
    }
    
    return changed;
}


void Table::EPIgeneration() {
    // coverage chart
    set<int> coveredMinterms;
    for (auto &pi : primeImplicants) {
        for (auto &m : pi.coveredMinterms) {
            if (find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end())
            CoverageChart[m].push_back(pi);
        }
    }
        for (auto &[m, pi_list] : CoverageChart) {

        // Skip don't care terms when identifying EPIs
        if (find(dont_cares.begin(), dont_cares.end(), m) != dont_cares.end()) {
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
                    coveredMinterms.insert(covered);
                }
               
            }
        }
   }
   
  processRemainingPI();
  applyDominanceRules();

 // Check if all minterms are covered
 set<int> uncoveredMinterms;
 for (auto &[m, _] : reducedChart) {
     if (find(dont_cares.begin(), dont_cares.end(), m) == dont_cares.end()) {
         uncoveredMinterms.insert(m);
     }
 }
 
 // If there are still uncovered minterms after dominance rules
 if (!uncoveredMinterms.empty()) {
    Term* bestPI = nullptr;
      int maxCoverage = 0;
      auto bestIt = remainingPI.end();

      for (auto pi = remainingPI.begin(); pi!= remainingPI.end(); ++pi) {
          int coverage = count_if(pi->coveredMinterms.begin(), pi->coveredMinterms.end(),
              [&](int m) { return uncoveredMinterms.find(m) != uncoveredMinterms.end(); });
          if (coverage > maxCoverage) {
              maxCoverage = coverage;
              bestPI =& (*pi);
              bestIt = pi;
          }
      }
      if (bestPI && maxCoverage>0) {
          EPI.push_back(*bestPI);
          cout<<"PI from the dominance: "<< bestPI->toExpression() <<endl;
          for (auto &m : bestPI->coveredMinterms) {
              uncoveredMinterms.erase(m);
          }
          if(bestIt!= remainingPI.end()) remainingPI.erase(bestIt);
      } //else break;
     
     // If there are still uncovered minterms, use Petrick's method
    } if (!uncoveredMinterms.empty()) {
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

void Table::applyPetrickMethod(const map<int, vector<Term>>& uncoveredChart) {
    if (uncoveredChart.empty()) return;
    
    cout << "Applying Petrick's method for remaining minterms..." << endl;
    
    // Map each prime implicant to a unique index
    map<string, int> piToIndex;
    vector<Term> uniquePIs;
    
    for (const auto& [minterm, pi_list] : uncoveredChart) {
        for (const auto& pi : pi_list) {
            if (piToIndex.find(pi.binary) == piToIndex.end()) {
                piToIndex[pi.binary] = uniquePIs.size();
                uniquePIs.push_back(pi);
            }
        }
    }
    
    // Form Product of Sums expression
    vector<vector<int>> petricksExpression;
    
    for (const auto& [minterm, pi_list] : uncoveredChart) {
        vector<int> sum;
        for (const auto& pi : pi_list) {
            sum.push_back(piToIndex[pi.binary]);
        }
        petricksExpression.push_back(sum);
    }
    
    // Expand to Sum of Products
    vector<vector<int>> sop = expandToPetricksSOP(petricksExpression);
    
    // Find minimum-cost solution
    vector<int> bestSolution;
    int minTerms = INT_MAX;
    
    for (const auto& product : sop) {
        if (product.size() < minTerms) {
            minTerms = product.size();
            bestSolution = product;
        }
    }
    
    // Add selected PIs to solution
    for (int piIdx : bestSolution) {
        Term selectedPI = uniquePIs[piIdx];
        
        // Check if already included
        bool alreadyIncluded = false;
        for (const auto& epi : EPI) {
            if (epi.binary == selectedPI.binary) {
                alreadyIncluded = true;
                break;
            }
        }
        if (!alreadyIncluded) {
            EPI.push_back(selectedPI);
            cout << "Prime Implicant selected by Petrick: " << selectedPI.toExpression() << endl;
            selections.push_back(selectedPI);
            }
    }
}

////////////////////******************************************************************9999 */

void Table::PetrickMethod() {
    // Step 1: Find uncovered minterms after EPIs
    map<int, vector<Term>> uncoveredChart;
    
    for (const auto& [minterm, pi_list] : CoverageChart) {
        // Skip don't cares
        if (find(dont_cares.begin(), dont_cares.end(), minterm) != dont_cares.end()) {
            continue;
        }
        
        // Check if already covered by EPIs
        bool covered = false;
        for (const auto& epi : EPI) {
            for (const auto& m : epi.coveredMinterms) {
                if (m == minterm) {
                    covered = true;
                    break;
                }
            }
            if (covered) break;
        }
        
        if (!covered) {
            uncoveredChart[minterm] = pi_list;
        }
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
            }
        }
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
    
    // Step 5: Find minimum-cost solution
    vector<int> bestSolution;
    int minTerms = INT_MAX;
    
    for (const auto& product : sop) {
        if (product.size() < minTerms) {
            minTerms = product.size();
            bestSolution = product;
        } else if (product.size() == minTerms) {
            // If tied for number of terms, choose the one with fewer literals
            int cost1 = 0, cost2 = 0;
            for (int i : bestSolution) {
                cost1 += countLiterals(uniquePIs[i]);
            }
            for (int i : product) {
                cost2 += countLiterals(uniquePIs[i]);
            }
            if (cost2 < cost1) {
                bestSolution = product;
            }
        }
    }
    
    // Step 6: Add selected PIs to solution
    for (int piIdx : bestSolution) {
        Term selectedPI = uniquePIs[piIdx];
        
        // Check if already included
        bool alreadyIncluded = false;
        for (const auto& epi : EPI) {
            if (epi.binary == selectedPI.binary) {
                alreadyIncluded = true;
                break;
            }
        }
        
        if (!alreadyIncluded) {
          //  EPI.push_back(selectedPI);
            selections.push_back(selectedPI);
            cout << "Prime Implicant selected by Petrick: " << selectedPI.toExpression() << endl;
        }
    }
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
    
    // Apply absorption law (if A⊆B, then A+B=A)
    vector<vector<int>> minimalResult;
    for (const auto& product1 : result) {
        bool isMinimal = true;
        
        for (const auto& product2 : result) {
            if (product1 == product2) continue;
            
            // If product2 is a subset of product1, product1 is redundant
            if (includes(product1.begin(), product1.end(), product2.begin(), product2.end())) {
                isMinimal = false;
                break;
            }
        }
        
        if (isMinimal) {
            minimalResult.push_back(product1);
        }
    }
    
    return minimalResult;
}





// void Table:: FinalExpression(){
//     stringstream final;
    
//     set <string> possible_F;
//     for(auto &epi: EPI){
    
//         cout<<epi.toExpression();

//       final<<epi.toExpression();
      
//        final<< " + "; 
//    }

//     for(auto &[m, pi] : CoverageChart){
//        { 
//         for(auto &pi : primeImplicants)
//     {
//         for (auto &min: pi.coveredMinterms)
//         {
            
//         if (min==m)
//        { 
//         stringstream temp;
//         temp.str(final.str());
//         temp << pi.toExpression();
        
//         temp<< " + "; 
//         possible_F.insert(temp.str()); // vector of all possible minimized funtion expression
//          }
           
//          }
//     }
//          }


//     }
//  cout << "Minimized Expressions:" <<"----------------" <<endl;

//     for (auto &expr: possible_F)
//     cout<< expr << endl << endl;
    
// }

// void Table::FinalExpression() {
//     set<string> unique_expressions;
    
//     // Construct base expression from Essential Prime Implicants
//     string final;
//     for (size_t i = 0; i < EPI.size(); i++) {
//         if (i > 0) final += " + "; // Append "+" only if it's NOT the first term
//         final += EPI[i].toExpression();
//     }

//     // Generate possible minimized function expressions
//     for (auto &[m, pi_list] : CoverageChart) {
//         for (auto &pi : pi_list) {  // Only iterate over relevant prime implicants
//             string temp = final;
//             if (!pi.toExpression().empty()) { // Ensure expression is not empty
//                 if (!temp.empty()) temp += " + "; // Append "+" only if necessary
//                 temp += pi.toExpression();
//             }
//             unique_expressions.insert(temp); // Store unique expressions
//         }
//     }

//     // Print unique minimized expressions
//     cout << "Minimized Expressions:\n";
//     for (const auto &expr : unique_expressions) {
//         cout << expr << endl;
//     }
// }
