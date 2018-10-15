module miter ( a, b, z );

input wire  [255:0] a,b;
output wire [511:0] z;
    assign z = a * b;
endmodule
