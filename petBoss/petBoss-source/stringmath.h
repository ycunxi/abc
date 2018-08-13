/*
Maintained by Walter Brown
Open for modification to others in Maciej Ciesielski's team.
*/

#ifndef _STRINGMATH_GUARD_
#define _STRINGMATH_GUARD_

#include <algorithm>
#include <cstdlib>
#include <gmpxx.h>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <string>
#include <vector>
using namespace std;
class StringMath {
  public:
    static string add(string&, string&);
    static void allVariables(string& expression, list<string>& result);
    template<class RandomAccessIterator>
    static void alphabeticalSort(RandomAccessIterator first,
                                 RandomAccessIterator last);
    /*
    Takes expression "dog+-cat" and adds "dog","+","-","cat" to result.
    No "" strings will be added to the list.
    */
    static void blockExpression(string& expression, list<string>& result);
    static bool isNumber(string& num);
    static void join(vector<string>& vec, char delimeter, string& result);
    static string multiply(string&, string&);
    static string power(string& base, string& exponent);
    static void removeAll(string&, char);
    static void removeAll(vector<string>& subject, const string& needle);
    static void simplify(string&);
    static void split(const string& term, char needle, vector<string>& result);
    static void substitute(string& target, string& needle, string& replacement);
    static void orderTerms(string&);
    static void outsideSplit(string& in, char needle, vector<string>& result);
    static void toUseful(string&);
    static void toHuman(string&);
    static void zeroExtend(string&, int newLength, string&result);
  private:
    static bool beforeInAlphabet(const string&, const string&);
    static bool beforeTerm(const string&, const string&);
    static void combineLikeTerms(string&);
    static void distribute(string&);
    static void distributeToList(string&, vector<string>&);
    static void dropFrontZero(string&);
    static bool isDigit(char);
    static bool notSign(char);
    static void standardizeSigns(string&);
    static void standardizeTerm(string&);
};

/* Template definitions. */
#include "stringmath.tpp"

#endif
