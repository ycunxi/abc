/*
Maintained by Walter Brown
Open for modification to others in Maciej Ciesielski's team.
*/

#ifndef _EXPRESSION_GUARD_
#define _EXPRESSION_GUARD_

#include <algorithm>
#include <exception>
#include <gmpxx.h>
#include <iterator>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include "stringmath.h"
#include <vector>

#define VARIABLE_SUBSTITUTION_MAP 1

using namespace std;

class Expression
{
  public:
    class Term
    {
      public:
        /* If representing the term 4*cow*fish... */
        /* 4 */
        mpz_class coefficient;
        /* "4*cow*fish" */
        string toString (void) const;
        /* "cow", "fish" */
        set<string> variables;
        /* Multiplies this with another term.  Handles pseudoboolean. */
        void multiplyBy(const Term&);
        /*
        Allows typecasting to string.
        */
        operator string();
    };

    Expression();
    Expression(const string&);

    /*
    Adds an expression to this one.
    */
    void add(const Expression&);

    /*
    Adds a string representing an expression to the Expression object.
    Warning:  The input string will be mutated.
    */
    void add(string&);

    /*
    Adds a term.
    */
    void add(const Term&);

    /*
    Resets the expression to the state it was constructed in. "0".
    */
    void clear(void);

    /*
    Returns the coefficient associated with a particular monomial.
    For an Expression("5*cat+3*moose*dog"), coefficientOf("dog*moose")=3.
    Constants are ignored while variables are present, so
    coefficientOf("195*dog*moose")=3 as well.
    To get the constant, do coefficientOf("") or input any constant.
    */
    mpz_class coefficientOf(const string& monomial) const;

    /*
    Prints the datastructure in a form that makes it easier to examine
    critically.
    */
    void debugPrint();

    /*
    Produces a copy of the term in the expression that has the same variables as
    the term presented.  Just sets result coefficient to 0 if term not present.
    */
    void likeTerm(const string& similarTo, Term& result) const;

    /*
    Multiplies this whole expression by a coefficient.
    */
    void multiplyBy(const mpz_class&);

    /*
    Returns the number of terms that don't have a coefficient of 0.
    */
    unsigned int numberOfTerms(void) const;

    /*
    Returns the number of terms that have this variable.
    */
    unsigned int numberOfTermsWith(const string& variableName) const;

    /*
    Returns the number of variables present in the expression.
    */
    unsigned int numberOfVariables(void) const;

    /*
    Returns a vector where the expression is non-zero.  If this is a
    zero-function,  Return an empty map.
    */
    map<string, bool> sat(void);

    /*
    Performs a substitution in the expression.
    */
    void substitute(const string& variable, const string& replacement);
    /*
    This is the more efficient version, which assumes variable has no spaces
    */
    void substitute(const string& variable, const Expression& replacement);

    /*
    Appends all Terms to the given vector.
    */
    void toVector(vector<Expression::Term>& result) const;

    /*
    Returns a string representing the expression
    */
    string toString (void) const;

    /*
    Adds the variables of the expression to the input set.
    */
    void variablesSet(set<string>& result) const;

    /*
    Allows typecasting to string.
    */
    operator string();

    /*
    Allows you to compare expressions to see if they have identical terms,
    ignoring order.
    */
    friend bool operator == (const Expression&, const Expression&);
    friend bool operator != (const Expression&, const Expression&);

  private:
    /*
    Records the term associated with each term in sum of products form.
    */
    map<set<string>, Term> terms;

    /*
    Adds a particular value to the coefficients map.
    */
    void addToCoefficients(const set<string>& index, const string& value);
    void addToCoefficients(const set<string>& index, const mpz_class& value);
    /*
    Removes the term being pointed to from the term map..
    */
    void clearTerm(const map<set<string>, Term>::iterator&);
    /*
    Puts a term into the terms map.
    */
    void makeTerm(const set<string>& variables, const mpz_class& value);
    /*
    Returns a set containing the variables that were separated by * signs in
    the input.
    */
    set<string> setFrom(const string&) const;
    #if VARIABLE_SUBSTITUTION_MAP
      /*
      A map with keys as variable names and values as sets of terms that contain
      that variable.
      */
      map<string, set<set<string> > > variableTerms;
    #endif
};

/*
Called by streams, allowing cout << someExpression << endl; to be done.
*/
ostream& operator << (ostream& strm, const Expression& expr);
ostream& operator << (ostream& strm, const Expression::Term& term);

#endif
