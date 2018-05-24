## Forked from berkeley-abc/abc and Modified for Algebraic Rewriting Demos

### See original ABC github https://github.com/berkeley-abc/abc


### Compiling:

Type "make" and generate binary "abc".

### Example

\# Algebraic Rewriting with btor64 multiplier <\b>
./abc -c "r test_bench/btor64.aig;&get;&polyn -o -v" | tee btor64_Arwt_results.txt<\b>
\# Algebraic Rewriting with abc-64 multiplier<\b>
./abc -c "r test_bench/abc-64.aig;&get;&polyn -o -v" | tee abc-64_Arwt_results.txt<\b>

<\b><\b><\b>
\# Algebraic Rewriting of synthesized multiplier (resyn3)<\b>
\# btor64 + resyn3<\b>
./abc -c "r test_bench/btor64.aig;resyn3;&get;&polyn -o -v" | tee btor64-resyn3_Arwt_results.txt<\b>
\# abc-64 + resyn3<\b>
./abc -c "r test_bench/abc-64.aig;resyn3;&get;&polyn -o -v" | tee abc-64-resyn3_Arwt_results.txt<\b>

## Final remarks:

Unfortunately, there is no comprehensive regression test. Good luck!                                
Questions related to "&polyn" in THIS version, please contact Cunxi Yu cunxi.yu@epfl.ch
