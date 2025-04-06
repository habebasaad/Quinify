module my_minimized_logic(
 input A,
 input B,
 input C,
 input D
 output F
);

 // Negated inputs
 wire A_n;
 wire B_n;
 wire C_n;
 wire D_n;

 // Product term wires
 wire term0; // A'BD
 wire term1; // BCD'
 wire term2; // ACD
 wire term3; // AB'C'D'

 // NOT gates
 not not_A(A_n, A);
 not not_B(B_n, B);
 not not_C(C_n, C);
 not not_D(D_n, D);

 // AND gates for product terms
 and and_term0(term0, A_n, B, D);
 and and_term1(term1, B, C, D_n);
 and and_term2(term2, A, C, D);
 and and_term3(term3, A, B_n, C_n, D_n);

 // Output logic
 or or_out(F, term0, term1, term2, term3);
endmodule
