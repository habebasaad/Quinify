module minimized_logic_1(
 input A,
 input B,
 input C,
 input D,
 input E
 output F
);

 // Negated inputs
 wire A_n;
 wire B_n;
 wire C_n;
 wire D_n;
 wire E_n;

 // Product term wires
 wire term0; // C'E'
 wire term1; // B'CE
 wire term2; // BC'
 wire term3; // AD'
 wire term4; // A'E'
 wire term5; // A'CD
 wire term6; // B'D'

 // NOT gates
 not not_A(A_n, A);
 not not_B(B_n, B);
 not not_C(C_n, C);
 not not_D(D_n, D);
 not not_E(E_n, E);

 // AND gates for product terms
 and and_term0(term0, C_n, E_n);
 and and_term1(term1, B_n, C, E);
 and and_term2(term2, B, C_n);
 and and_term3(term3, A, D_n);
 and and_term4(term4, A_n, E_n);
 and and_term5(term5, A_n, C, D);
 and and_term6(term6, B_n, D_n);

 // Output logic
 or or_out(F, term0, term1, term2, term3, term4, term5, term6);
endmodule
