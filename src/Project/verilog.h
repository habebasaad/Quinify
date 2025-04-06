// verilog.h
#ifndef VERILOG_H
#define VERILOG_H

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cctype>

std::string generateVerilogModule(const std::string& expression, const std::string& moduleName = "boolean_logic") {
    // Identify variables in the expression
    std::set<char> variables;
    for (char c : expression) {
        if (std::isalpha(c) && c != 'F' && c != 'f') {
            variables.insert(c);
        }
    }

    // Sort variables for consistent output
    std::vector<char> orderedVars(variables.begin(), variables.end());
    std::sort(orderedVars.begin(), orderedVars.end());

    // Start building Verilog module
    std::stringstream verilog;

    // Module declaration
    verilog << "module " << moduleName << "(\n";

    // Input declarations
    for (size_t i = 0; i < orderedVars.size(); i++) {
        verilog << " input " << orderedVars[i];
        if (i < orderedVars.size() - 1) {
            verilog << ",";
        }
        verilog << "\n";
    }

    // Output declaration
    verilog << " output F\n";
    verilog << ");\n\n";

    // Parse expression into product terms (split by +)
    std::vector<std::string> terms;
    std::string currentTerm;
    for (size_t i = 0; i < expression.size(); i++) {
        char c = expression[i];
        if (c == '+') {
            if (!currentTerm.empty()) {
                // Remove whitespace - FIX: Use a lambda instead of isspace directly
                currentTerm.erase(std::remove_if(currentTerm.begin(), currentTerm.end(), 
                                 [](unsigned char c){ return std::isspace(c); }), 
                                 currentTerm.end());
                terms.push_back(currentTerm);
                currentTerm.clear();
            }
        } else {
            currentTerm += c;
        }
    }
    // Add the last term
    if (!currentTerm.empty()) {
        // FIX: Use a lambda instead of isspace directly
        currentTerm.erase(std::remove_if(currentTerm.begin(), currentTerm.end(), 
                         [](unsigned char c){ return std::isspace(c); }), 
                         currentTerm.end());
        terms.push_back(currentTerm);
    }

    // Wire declarations
    verilog << " // Negated inputs\n";
    for (char var : orderedVars) {
        verilog << " wire " << var << "_n;\n";
    }

    verilog << "\n // Product term wires\n";
    for (size_t i = 0; i < terms.size(); i++) {
        verilog << " wire term" << i << "; // " << terms[i] << "\n";
    }

    // Generate NOT gates for inputs
    verilog << "\n // NOT gates\n";
    for (char var : orderedVars) {
        verilog << " not not_" << var << "(" << var << "_n, " << var << ");\n";
    }

    // Generate AND gates for product terms
    verilog << "\n // AND gates for product terms\n";
    for (size_t i = 0; i < terms.size(); i++) {
        std::string term = terms[i];
        std::vector<std::string> inputSignals;
        for (size_t j = 0; j < term.size(); j++) {
            if (std::isalpha(term[j])) {
                char var = term[j];
                // Check for complement
                if (j + 1 < term.size() && term[j+1] == '\'') {
                    inputSignals.push_back(std::string(1, var) + "_n");
                    j++; // Skip the complement symbol
                } else {
                    inputSignals.push_back(std::string(1, var));
                }
            }
        }

        // Create appropriate gate for this term
        if (inputSignals.empty()) {
            verilog << " // Empty term (should not happen)\n";
            verilog << " assign term" << i << " = 1'b0;\n";
        } else if (inputSignals.size() == 1) {
            // Single input - use a buffer
            verilog << " assign term" << i << " = " << inputSignals[0] << ";\n";
        } else {
            // Multiple inputs - use AND gate
            verilog << " and and_term" << i << "(term" << i;
            for (const auto& signal : inputSignals) {
                verilog << ", " << signal;
            }
            verilog << ");\n";
        }
    }

    // Generate OR gate for the output
    verilog << "\n // Output logic\n";
    if (terms.empty()) {
        verilog << " assign F = 1'b0; // Empty expression\n";
    } else if (terms.size() == 1) {
        verilog << " assign F = term0;\n";
    } else {
        // Use OR gate with multiple inputs
        verilog << " or or_out(F";
        for (size_t i = 0; i < terms.size(); i++) {
            verilog << ", term" << i;
        }
        verilog << ");\n";
    }

    // Close the module
    verilog << "endmodule\n";

    return verilog.str();
}

void generateVerilogFiles(const std::vector<std::string>& minimizedExpressions) {
    for (size_t i = 0; i < minimizedExpressions.size(); i++) {
        std::string moduleName = "minimized_logic_" + std::to_string(i);
        std::string verilogCode = generateVerilogModule(minimizedExpressions[i], moduleName);
        
        // Write to a .v file
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

#endif // VERILOG_H
