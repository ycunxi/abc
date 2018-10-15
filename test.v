module b ( a,b,c,z);
input wire a,b,c;
output z;
wire [1:0] x1;
    assign x1 = a + b + c;
    assign z = (x1 >= 2); 
endmodule
