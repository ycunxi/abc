module miter ( a, b, z );

input wire  [31:0] a,b;
output wire [63:0] z;
    assign z = a * b;
endmodule
