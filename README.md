## Forked from berkeley-abc/abc and Modified for Algebraic Rewriting Demos

### See original ABC github https://github.com/berkeley-abc/abc


### Compiling:

Type "make" and generate binary "abc".

### Example 

Benchmarks in "test_bench" & Saved results in "test_bench_results"

#### Example 1 - pre-synthesized 
*\# Algebraic Rewriting with btor64 multiplier <br/>
./abc -c "r test_bench/btor64.aig;&get;&polyn -o -v" | tee btor64_Arwt_results.txt<br/>
\# Algebraic Rewriting with abc-64 multiplier<br/>
./abc -c "r test_bench/abc-64.aig;&get;&polyn -o -v" | tee abc-64_Arwt_results.txt<br/>*

<br/><br/>
#### Example 2 - synthesized using resyn3

*\# Algebraic Rewriting of synthesized multiplier (resyn3)<br/>
\# btor64 + resyn3<br/>
./abc -c "r test_bench/btor64.aig;resyn3;&get;&polyn -o -v" | tee btor64-resyn3_Arwt_results.txt<br/>
\# abc-64 + resyn3<br/>
./abc -c "r test_bench/abc-64.aig;resyn3;&get;&polyn -o -v" | tee abc-64-resyn3_Arwt_results.txt<br/>*

## Final remarks:

Questions related to "&polyn" in THIS version, please contact Cunxi Yu cunxi.yu@epfl.ch
