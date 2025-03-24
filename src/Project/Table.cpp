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

void Table::EPIgeneration() {
    // coverage chart
    set<int> coveredMinterms;
    for (auto &pi : primeImplicants) {
        for (auto &m : pi.coveredMinterms) {
            CoverageChart[m].push_back(pi);
        }
    }

    // extract essential prime implicants

        for (auto &[m, pi_list] : CoverageChart) {
            
//delete the dontcares
//             for (auto &dc:dont_cares)
// {
// CoverageChart.erase(dc);
// }

                    // Skip don't care terms when identifying EPIs
                    if (find(dont_cares.begin(), dont_cares.end(), m) != dont_cares.end()) {
                        continue;
                    }

        if (pi_list.size() == 1) {
            Term essentialPI = pi_list[0];
            
                // avoid duplicates
            bool already_included = false;
            for (auto &epi : EPI) {
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

    // Generate the final expression
    cout << "Final Expression: ";
    for (size_t i = 0; i < EPI.size(); i++) {
        if (i > 0) cout << " + ";
        cout << EPI[i].toExpression();
    }
    cout << endl<<"<<--------------------";

FinalExpression();
}

// void Table::FinalExpression() {
//     set<string> unique_expressions;

//     // Construct base expression from Essential Prime Implicants
//     string base_expr;
//     for (size_t i = 0; i < EPI.size(); i++) {
//         if (i > 0) base_expr += " + ";
//         base_expr += EPI[i].toExpression();
//     }

//     // Generate possible minimized function expressions
//     for (auto &[m, pi_list] : CoverageChart) {
//         for (auto &pi : pi_list) {
//             string temp = base_expr;
//             if (!pi.toExpression().empty()) {
//                 if (!temp.empty()) temp += " + ";
//                 temp += pi.toExpression();
//             }
//             unique_expressions.insert(temp);
//         }
//     }

//     // Print unique minimized expressions
//     cout << "Minimized Expressions:" << endl;
//     for (const auto &expr : unique_expressions) {
//         cout << expr << endl;
//     }
// }

void Table::FinalExpression() {
    set<string> unique_expressions;

    string base_expr;
    for (size_t i = 0; i < EPI.size(); i++) {
        if (i > 0) base_expr += " + ";
        base_expr += EPI[i].toExpression();
    }

    for (const auto& epi : EPI) {
        for (const auto& m : epi.coveredMinterms) {
            CoverageChart.erase(m);
        }
    }

    if (CoverageChart.empty()) {
        unique_expressions.insert(base_expr);
    } else {
        for (const auto& [m, pi_list] : CoverageChart) {
            for (const auto& pi : pi_list) {
                string temp = base_expr;
                if (!pi.toExpression().empty()) {
                    if (!temp.empty()) temp += " + ";
                    temp += pi.toExpression();
                }
                unique_expressions.insert(temp);
            }
        }
    }

    // Print unique minimized expressions
    cout << "Minimized Expressions:" << endl;
    for (const auto& expr : unique_expressions) {
        cout << expr << endl;
    }
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
