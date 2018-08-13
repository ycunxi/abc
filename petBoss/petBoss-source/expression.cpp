/*
Maintained by Walter Brown
Open for modification to others in Maciej Ciesielski's team.
*/
#define DEBUG_EXPRESSION 0
#define DEBUG_SUBSTITUTE 0
#define TO_STRING_RESERVE 0 /* reservation proved to be inefficient here. */
#define TERM_TO_STRING_RESERVE 0
                            /* No noticable time improvement, mem increased. */

#include "expression.h"

Expression::Expression()
{
}

Expression::Expression(const string& s)
{
  string mutableString = s;
  add(mutableString);
}

void Expression::add(const Expression& e)
{
  /* Add in every element of the map. */
  for(auto kv : e.terms)
  {
    addToCoefficients(kv.first, kv.second.coefficient);
  }
}

void Expression::add(string& exp)
{
  #if DEBUG_EXPRESSION
    cout << "Expression::add, exp=\"" << exp << "\"" << endl;
  #endif
  /* Count "" as 0. */
  if(exp.length() == 0) return;

  /* Put the string in sum of products form. */
  StringMath::toUseful(exp);
  StringMath::simplify(exp);

  /* Drop 0. */
  if(exp == "0") return;

  #if DEBUG_EXPRESSION
    cout << "After stringmath, exp=\"" << exp << "\"" << endl;
  #endif

  /* Split to terms. */
  vector<string> extraTerms;
  StringMath::split(exp, '+', extraTerms);

  /* Put terms in coefficient map. */
  for(string& t : extraTerms)
  {
    int coefBorder = t.find('*');
    /* If this has no variables, goes in "1" */
    set<string> index;
    if(coefBorder == string::npos)
    {
      addToCoefficients(index, t);
    }
    else
    {
      string number = t.substr(0, coefBorder);
      /* Build index. */
      vector<string> vars;
      StringMath::split(t.substr(coefBorder+1), '*', vars);
      for(string& v : vars)
      {
        index.insert(v);
      }
      addToCoefficients(index, number);
    }
  }
  #if DEBUG_EXPRESSION
    cout << "done with Expression::add" << endl;
  #endif
}

void Expression::add(const Term& t)
{
  /* Don't bother if this is just a zero. */
  if(t.coefficient == 0) return;
  addToCoefficients(t.variables, t.coefficient);
}

void Expression::addToCoefficients(const set<string>& index,const string& value)
{
  /* Convert the string to something you can add. */
  mpz_class val;
  val = value.c_str();

  addToCoefficients(index, val);
}

void Expression::addToCoefficients(const set<string>& index,
                                                         const mpz_class& value)
{
  /* See if this element is in the coefficients map. */
  auto indexPointer = terms.find(index);
  if(indexPointer == terms.end())
  {
    makeTerm(index, value);
  }
  else
  {
    /* If so, do addition. */
    indexPointer->second.coefficient += value;
    /* Terms with a coefficient of 0 are dropped. */
    if(indexPointer->second.coefficient == 0)
    {
      clearTerm(indexPointer);
    }
  }
}

void Expression::clear(void)
{
  terms.clear();
}

void Expression::clearTerm(const map<set<string>, Term>::iterator& it)
{
  #if VARIABLE_SUBSTITUTION_MAP
    /* Dissassociate the term from the proper variables. */
    for(string v : it->first)
    {
      auto findV = variableTerms.find(v);
      findV->second.erase(it->first);
      /* Maybe that variable is completally gone. */
      if(findV->second.empty())
      {
        variableTerms.erase(findV);
      }
    }
  #endif
  /* Erase. */
  terms.erase(it);
}

mpz_class Expression::coefficientOf(const string& monomial) const
{
  #if DEBUG_EXPRESSION
    cout << "Entering Expression::coefficientOf(" << monomial << ")" << endl;
  #endif
  Term t;
  likeTerm(monomial, t);
  return t.coefficient;

  #if DEBUG_EXPRESSION
    cout << "Exiting Expression::coefficientOf" << endl;
  #endif
}

void Expression::debugPrint()
{
  cout << "debugPrint():" << endl;
  /* For each term... */
  for(auto i : terms)
  {
    cout << "{";
    for(auto v : i.first)
    {
      cout << '"' << v << "\" ";
    }
    cout << ": " << i.second.coefficient.get_str() << " | ";
    /* Print all variables. */
    for(auto v : i.second.variables)
    {
      cout << '"' << v << "\" ";
    }
    cout << "}" << endl;
  }
}

void Expression::likeTerm(const string& similarTo, Term& result) const
{
  /* Break string into variables. */
  set<string> vars = setFrom(similarTo);
  /* If the term is here, return it. */
  auto termPointer = terms.find(vars);
  if(termPointer != terms.end())
  {
    result = termPointer->second;
  }
  /* Otherwise, build a dud term */
  else
  {
    result.coefficient = 0;
  }
}

void Expression::makeTerm(const set<string>& variables, const mpz_class& value)
{
  /* If not, just add this to the map. */
  Term* t = &terms[variables];
  t->coefficient = value;
  /* As a new element, you should also give it a variable list. */
  t->variables = variables;
  #if VARIABLE_SUBSTITUTION_MAP
    /* Associate the term with the proper variables. */
    for(string v : variables)
    {
      variableTerms[v].insert(variables);
    }
  #endif
}

 void Expression::multiplyBy(const mpz_class& factor)
{
  for(auto & t : terms)
  {
    t.second.coefficient = t.second.coefficient * factor;
  }
}

unsigned int Expression::numberOfTerms(void) const
{
  return terms.size();
}

unsigned int Expression::numberOfTermsWith(const string& variableName) const
{
  #if VARIABLE_SUBSTITUTION_MAP
    /* Be defensive... make sure the variable is here. */
    auto findVar = variableTerms.find(variableName);
    if(findVar == variableTerms.end())
    {
      return 0;
    }
    return findVar->second.size();
  #else
    unsigned int result = 0;
    /* Count the terms with the variable. */
    for(auto kv : terms)
    {
      /* If this term has the variable, count it. */
      if(kv.first.find(variableName) != kv.first.end())
      {
        ++result;
      }
    }
  #endif
}

unsigned int Expression::numberOfVariables(void) const
{
  #if VARIABLE_SUBSTITUTION_MAP
    return variableTerms.size();
  #else
    /* This set will contain all variables present. */
    set<string> allMyVariables;
    variablesSet(allMyVariables);
    return allMyVariables.size();
  #endif
}

#define DEBUG_SAT 0
map<string, bool> Expression::sat(void)
{
  /*
  General Plan:
  Represent each combination of trues and falses with an integer.  Evaluate the
  cases one by one.  First non-zero case is reported!
  */
  Expression zero("0");
  map<string, bool> result;
  /* Get variables that must be set. */
  set<string> varsSet;
  variablesSet(varsSet);
  /* Figure out how many cases you have. */
  mpz_class maxNum = 1;
  for(mpz_class i = 0; i < varsSet.size(); ++i) maxNum *= 2;
  for(mpz_class i = 0; i < maxNum; ++i)
  {
    Expression trial = *this;
    /* Set all the variables. */
    mpz_class setter = i;
    for(string v : varsSet)
    {
      trial.substitute(v, (setter%2==0)?"0":"1");
      setter /= 2;
    }
    #if DEBUG_SAT
      cout << "{" << trial << "} {" << zero << "}" << endl;
    #endif
    /* See if we're satisfied. */
    if(trial != zero)
    {
      /* Convert to a map! */
      mpz_class setter = i;
      for(string v : varsSet)
      {
        result[v] = setter % 2 == 1;
        setter /= 2;
      }
      break;
    }
  }
  return result;
}

set<string> Expression::setFrom(const string& in) const
{
  set<string> result;
  /* Break string into variables. */
  vector<string> multipliers;
  StringMath::split(in, '*', multipliers);
  /* We want to focus on the variables. */
  for(string i : multipliers)
  {
    if(!StringMath::isNumber(i))
    {
      result.insert(i);
    }
  }
  return result;
}

void Expression::substitute(const string& var, const Expression& replacement)
{
  #if DEBUG_SUBSTITUTE
    cout << "Entered Expression::substitute(" << var << "," << replacement
         << ")" << endl;
  #endif
  /* Assumes var has no spaces. */
  /* This will store the transformed terms. */
  Expression bonus;
  /* Extract the right bonus from each term. */
  #if VARIABLE_SUBSTITUTION_MAP
    /* If no terms have this variable, do nothing. */
    auto termsOfVar = variableTerms.find(var);
    if(termsOfVar == variableTerms.end())
    {
      return;
    }
    /* Make a copy of the old term list so you can mess with the old one. */
    auto constantTermsOfVar = termsOfVar->second;
    /* Otherwise, handle all terms with the variable. */
    for(auto it = constantTermsOfVar.begin(); it != constantTermsOfVar.end();)
    {
      /* Jump to the term being examined. */
      auto term = terms.find(*it);
      /* We'll reuse the term as a base for its bonuses. */
      /* No bonus will have a "var".  Remove it now */
      term->second.variables.erase(var);
      /* A bonus term will be added for each term of the replacement. */
      for(auto t : replacement.terms)
      {
        /* Make a copy of original term to work with. */
        Term bonusTerm = term->second;
        /* Multiply this bonus term with the term of the replacement. */
        bonusTerm.multiplyBy(t.second);
        /* Now this is ready to be in the bonus. */
        bonus.add(bonusTerm);
      }
      /* This term is being replaced.  It must go. */
      clearTerm(term);
      it++;
    }
  #else
    for(auto it = terms.begin(); it != terms.end();)
    {
      /* If this term contains x, it will be replaced by a bonus. */
      auto varPointer = it->second.variables.find(var);
      if(varPointer != it->second.variables.end())
      {
        /* We'll reuse the "it" term as a base for its bonuses. */
        /* No bonus will have a "var".  Remove it now */
        it->second.variables.erase(varPointer);
        /* A bonus term will be added for each term of the replacement. */
        for(auto t : replacement.terms)
        {
          /* Make a copy of original term to work with. */
          Term bonusTerm = it->second;
          /* Multiply this bonus term with the term of the replacement. */
          bonusTerm.multiplyBy(t.second);
          /* Now this is ready to be in the bonus. */
          bonus.add(bonusTerm);
        }
        /* This term is being replaced.  It must go. */
        clearTerm(it++);
      }
      /* Otherwise, move on. */
      else
      {
        ++it;
      }
    }
  #endif
  /* Finally, add the bonus. */
  add(bonus);
  #if DEBUG_SUBSTITUTE
    cout << "Exit Expression::substitute()" << endl;
  #endif
}

void Expression::substitute(const string& variable, const string& replacement)
{
  Expression rep(replacement);
  /* Remove spaces. */
  string var = variable;
  StringMath::removeAll(var, ' ');
  substitute(var, rep);
}

Expression::operator string()
{
  return toString();
}

string Expression::toString (void) const
{
  #if DEBUG_EXPRESSION
    cout << "Entering Expression::toString" << endl;
  #endif
  
  /* If there's nothing in the map, return 0. */
  if(terms.empty()) return "0";
  #if DEBUG_EXPRESSION
    cout << "Moved passed empty 0 check." << endl;
  #endif

  string result;
  #if TO_STRING_RESERVE == 1
    /*
    Each entry will have a * between coef and vars and a + in back. Also,
    it's possible that the coefficient might need room for a sign.
    */
    size_t reserve = 3*terms.size();
    for(auto kv : terms)
    {
      /* Need room for coefficient. */
      reserve +=  mpz_sizeinbase(kv.second.coefficient.get_mpz_t(), 10);
      /* Need room for variables. */
      reserve += kv.first.size();
    }
    result.reserve(reserve);
  #endif
  #if TO_STRING_RESERVE == 2 /* Precalculate toStrings and put them in list. */
    /* Reserve space for + signs. */
    size_t reserve = terms.size();
    list<string> toStrings;
    for(auto kv : terms)
    {
      toStrings.push_back(kv.second.toString());
    }
    for(string s : toStrings)
    {
      reserve += toStrings.size();
    }
    result.reserve(reserve);
    for(string s : toStrings)
    {
      result += s + '+';
    }
  #endif
  #if TO_STRING_RESERVE != 2 /* Method 2 handles result construction. */
    /* Concatanate map back into a string. */
    for(auto kv : terms)
    {
      result += kv.second.toString() + '+';
    }
  #endif
  /* Oops!  We have an extra '+' at the end! */
  result.pop_back();
  #if DEBUG_EXPRESSION
    cout << "Finished pop_back.  result="<< result << endl;
  #endif

  /* Use stringmath to clean up the result. */
  StringMath::orderTerms(result);
  #if DEBUG_EXPRESSION
    cout << "Finished ordereTerms.  result="<< result << endl;
  #endif
  StringMath::toHuman(result);
  #if DEBUG_EXPRESSION
    cout << "Finished toHuman.  result="<< result << endl;
  #endif

  /* Return. */
  #if DEBUG_EXPRESSION
    cout << "About to exit Expression::toString" << endl;
  #endif
  return result;
}

void Expression::toVector(vector<Expression::Term>& result) const
{
  /* Reserve enough space to store the new items. */
  result.reserve(result.size() + numberOfTerms());
  /* Load. */
  for(auto kv : terms)
  {
    result.push_back(kv.second);
  }
}

void Expression::variablesSet(set<string>& result) const
{
  /* Add variables of all terms to the set. */
  for(auto t : terms)
  {
    result.insert(t.second.variables.begin(), t.second.variables.end());
  }
}

void Expression::Term::multiplyBy(const Expression::Term& mult)
{
  /* Multiply coefficient. */
  coefficient *= mult.coefficient;
  /* Add the multiplier's variables to your set. */
  for(string i : mult.variables)
  {
    variables.insert(i);
  }
}

string Expression::Term::toString(void) const
{
  /* Start with the coefficient. */
  string result = coefficient.get_str();
  #if TERM_TO_STRING_RESERVE
  /* Reserve your space. */
    int reserveNeeded = result.size();
    for(auto i : variables)
    {
      reserveNeeded += i.size() + 1;
    }
    result.reserve(reserveNeeded);
  #endif
  /* Append all variables. */
  for(auto i : variables)
  {
    result += '*' + i;
  }
  return result;
}

Expression::Term::operator string()
{
  return toString();
}

ostream& operator << (ostream& strm, const Expression& expr)
{
  return strm << expr.toString();
}

ostream& operator << (ostream& strm, const Expression::Term& term)
{
  return strm << term.toString();
}

#define DEBUG_EQEQ 0
bool operator == (const Expression& a, const Expression& b)
{
  #if DEBUG_EQEQ
    cout << "a.numberOfTerms() = " << a.numberOfTerms()
         << ", b.numberOfTerms() = " << b.numberOfTerms() << endl;
  #endif
  /* These can't be the same if they have a different number of terms. */
  if(a.numberOfTerms() != b.numberOfTerms()) return false;
  #if DEBUG_EQEQ
    cout << "Didn't return false by term count." << endl;
  #endif

  /* Compare term by term. */
  for(auto kv : a.terms)
  {
    /* If a has an element that's not in b, these don't match. */
    if(b.terms.find(kv.first) == b.terms.end()) return false;

    /* If b's version of the term has a different coefficient, no match. */
    if(b.terms.at(kv.first).coefficient != kv.second.coefficient) return false;
  }

  /* If no differences were found, these match. */
  return true;
}

bool operator != (const Expression& a, const Expression& b)
{
  return !(a == b);
}
