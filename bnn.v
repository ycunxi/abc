module b ( a,b,c,d,e,f,g,h,i,z);
input wire a,b,c,d,e,f,g,h,i;
output z;
wire [3:0] z_o;
wire [1:0] x1,x2,x3;
    assign x1 = a + b + c;
    assign x2 = d + e + f;
    assign x3 = g + h + i;
    assign z_o = x1 + x2 + x3;
    assign z = (z_o > 2); 
endmodule
