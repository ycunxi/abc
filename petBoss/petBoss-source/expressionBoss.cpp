/*
Maintained by Walter Brown
Open for modification to others in Maciej Ciesielski's team.
*/

#define DEBUG_DEPENDENCIES_UNSATISFIED 0
#define DEBUG_FIND_DEPENDENT_COUNT 0
#define DEBUG_INPUT_SIGNATURE 0
#define DEBUG_PROPAGATE_LEVEL 0
#define DEBUG_SUBSTITUTE_UNTIL_DONE 0

#include "expressionBoss.h"

ExpressionBoss::ExpressionBoss(void)
{
  doSubstitute = true;
}

bool ExpressionBoss::alwaysFalse(const string& var)
{
  return false;
}

string ExpressionBoss::blind(const string& outputSignature)
{
  /* Convert that output signature to an Expression. */
  Expression signature(outputSignature);
  /* Convert the expression into an input signature. */
  blind(signature);
  /* Return the input signature. */
  return signature.toString();
}

void ExpressionBoss::blind(Expression& outputSignature)
{
  for(string s : blindOrder)
  {
    outputSignature.substitute(s, substitutionTable.at(s).replacement);
  }
}

void ExpressionBoss::buildDependencyStacks()
{
  for(auto kv : substitutionTable)
  {
    for(string depender : kv.second.variables)
    {
      auto findDepender = substitutionTable.find(depender);
      /* Unless depender is a primary input... */
      if(findDepender != substitutionTable.end())
      {
        /* It will need to remember this wire is a dependency. */
        findDepender->second.dependencyStack.insert(kv.first);
      }
    }
    /* Substitution hasn't happened yet. */
    kv.second.happened = false;
  }
}

void ExpressionBoss::buildDependencyStacks(set<string> primaryOutputs)
{
  /* Build stacks, treating floats like primaryOutputs. */
  buildDependencyStacks();
  /*
  Now see tell all above floats that they might have their dependencies
  satisfied.
  */
  for(auto& kv : substitutionTable)
  {
    /* Filter out primary outputs. */
    if(primaryOutputs.find(kv.first) != primaryOutputs.end())
    {
      continue;
    }
    /* Floats are marked as complete. */
    if(kv.second.dependencyStack.empty())
    {
      kv.second.happened = true;
    }
    /*
    If this has primary outputs in its reverse logical cone, substitution will
    reach it.
    */
    bool inSubstitutionPath = false;
    for(string s : kv.second.dependencyStack)
    {
      if(primaryOutputs.find(s) != primaryOutputs.end())
      {
        inSubstitutionPath = true;
        break;
      }
    }
    /*
    Only need to propagate to areas that will be untouched by substitution.
    */
    if(!inSubstitutionPath)
    {
      buildDependencyStacksPropagate(kv.first);
    }
  }
}

void ExpressionBoss::buildDependencyStacksPropagate(const string& target)
{
  /* See if this wire has all its dependencies satisfied. */
  if(!dependenciesUnsatisfied(target))
  {
    substitutionTable.at(target).happened = true;
    /* If so, the wires above it might be satisfied now. */
    for(string s : substitutionTable.at(target).variables)
    {
      buildDependencyStacksPropagate(s);
    }
  }
}

string ExpressionBoss::cheapGreed(const string& outputSignature)
{
  /* Convert that output signature to an Expression. */
  Expression signature(outputSignature);
  /* Convert the expression into an input signature. */
  cheapGreed(signature);
  /* Return the input signature. */
  return signature.toString();
}

void ExpressionBoss::cheapGreed(Expression& outputSignature)
{
  set<string> varsAllowed;
  outputSignature.variablesSet(varsAllowed);
  substituteUntilDoneDependency(outputSignature, varsAllowed,
                                &ExpressionBoss::dud,
                                &ExpressionBoss::termsAddedBy);
}

void ExpressionBoss::clear(void)
{
  substitutionTable.clear();
  blindOrder.clear();
}

bool ExpressionBoss::dependenciesUnsatisfied(const string& v)
{
  #if DEBUG_DEPENDENCIES_UNSATISFIED
    cout << "dependenciesUnsatisfied(" << v << ")" << endl;
  #endif
  /* Primary inputs are never satisfied. */
  auto vPointer = substitutionTable.find(v);
  if(vPointer == substitutionTable.end())
  {
    #if DEBUG_DEPENDENCIES_UNSATISFIED
      cout << v << " is a primary input." << endl;
    #endif
    return true;
  }
  #if DEBUG_DEPENDENCIES_UNSATISFIED
    cout << v << " isn't a primary input." << endl;
  #endif
  /* Try to keep going until you've checked off all dependencies. */
  while(!vPointer->second.dependencyStack.empty())
  {
    /* See if the top dependency is unsatisfied. */
    string dependency = *vPointer->second.dependencyStack.begin();
    if(!substitutionTable.at(dependency).happened)
    {
      #if DEBUG_DEPENDENCIES_UNSATISFIED
        cout << dependency << " is blocking " << v << "." << endl;
      #endif
      return true;
    }
    #if DEBUG_DEPENDENCIES_UNSATISFIED
      cout << dependency << " is cleared for " << v << "." << endl;
    #endif
    /* This dependency is handled. */
    vPointer->second.dependencyStack.erase(
                                      vPointer->second.dependencyStack.begin());
  }
  #if DEBUG_DEPENDENCIES_UNSATISFIED
    cout << v << " has all dependencies satisfied." << endl;
  #endif
  /* The stack is empty?  Congrats!  All dependencies are satisfied. */
  return false;
}

void ExpressionBoss::describeDependency()
{
  cout << "Dependency:" << endl;
  for(auto& kv : substitutionTable)
  {
    describeDependency(kv.first);
  }
}

void ExpressionBoss::describeDependency(const string& var)
{
    cout << var << "|";
    for(string s : substitutionTable.at(var).dependencyStack)
    {
      cout << s << " ";
    }
    cout << endl;
}

void ExpressionBoss::describeTable() const
{
  for(auto& kv : substitutionTable)
  {
    cout << kv.first << '=' << kv.second.replacement << " | ";
    for(string v : kv.second.variables) cout << v << ' ';
    cout << "| L " << kv.second.level.get_str();
    cout << " | D " << kv.second.dependentCount.get_str() << endl;
  }
}

string ExpressionBoss::dijkstra(const string& outputSignature)
{
  /* Convert that output signature to an Expression. */
  Expression signature(outputSignature);
  /* Convert the expression into an input signature. */
  dijkstra(signature);
  /* Return the input signature. */
  return signature.toString();
}
void ExpressionBoss::dijkstra(Expression& outputSignature)
{
  cout << "IMPLEMENT ME!!!" << endl;

///* FROM OTHER LEVELIZATION MOJO!!! */
//  /* Get the initial set of variables you can substitute for. */
//  set<string> varsAllowed;
//  outputSignature.variablesSet(varsAllowed);
//  /* Setup dependency stack. */
//  buildDependencyStacks(varsAllowed);
//  /* Level 0 is set of wires with no dependency. */
//  set<string> levelZero;
//  for(auto kv : substitutionTable)
//  {
//    if(kv.second.dependencyStack.empty())
//    {
//      levelZero.insert(kv.first);
//    }
//  }
//  /* Assign levels to each substitution you know about. */
//  propagateLevel(levelZero);
//
//  /* Substitute until there are no variables to substitute for. */
//  substituteUntilDone(outputSignature, varsAllowed, inFunction,
//                      &ExpressionBoss::alwaysFalse, &ExpressionBoss::getLevel);
//  /* Forget levelization. */
//  for(auto& kv : substitutionTable)
//  {
//    kv.second.level = 0;
//  }
}

void ExpressionBoss::dud(const string&)
{
}

vector<string> ExpressionBoss::dumpEQN()
{
  /* Copy to result. */
  auto result = eqnBuffer;
  /* Clean up after yourself. */
  eqnBuffer.clear();
  /* Return result. */
  return result;
}

string ExpressionBoss::expensiveGreed(const string& outputSignature)
{
  /* Convert that output signature to an Expression. */
  Expression signature(outputSignature);
  /* Convert the expression into an input signature. */
  expensiveGreed(signature);
  /* Return the input signature. */
  return signature.toString();
}

void ExpressionBoss::expensiveGreed(Expression& outputSignature)
{
  set<string> varsAllowed;
  outputSignature.variablesSet(varsAllowed);
  substituteUntilDoneDependency(outputSignature, varsAllowed,
                                &ExpressionBoss::dud,
                                &ExpressionBoss::termsAfter);
}

void ExpressionBoss::findDependentCount()
{
  /* Forget all previous dependentCounts. */
  for(auto& kv : substitutionTable)
  {
    kv.second.dependentCount = -1;
  }
  /* Find the new dependent counts. */
  for(auto kv : substitutionTable)
  {
    findDependentCount(kv.first);
  }
}

mpz_class ExpressionBoss::findDependentCount(const string& target)
{
  /* Primary inputs have no dependents. */
  auto findTarget = substitutionTable.find(target);
  if(findTarget == substitutionTable.end())
  {
    #if DEBUG_FIND_DEPENDENT_COUNT
      cout << target << " is a primary input." << endl;
    #endif
    return 0;
  }
  #if DEBUG_FIND_DEPENDENT_COUNT
    cout << target << " isn't a primary input." << endl;
    cout << target << " has a dependentCount of "
         << findTarget->second.dependentCount << endl;
  #endif
  /* Only do work if dependentCount hasn't been found yet. */
  if(findTarget->second.dependentCount == -1)
  {
    #if DEBUG_FIND_DEPENDENT_COUNT
      cout << target << " needs work." << endl;
    #endif
    /*
    dependentCount =
             sum of direct dependent's dependent counts + direct dependents
    */
    findTarget->second.dependentCount = findTarget->second.variables.size();
    for(string v : findTarget->second.variables)
    {
      findTarget->second.dependentCount += findDependentCount(v);
    }
  }
  #if DEBUG_FIND_DEPENDENT_COUNT
    cout << target << " is " << findTarget->second.dependentCount << endl;
  #endif
  return findTarget->second.dependentCount;
}

mpz_class ExpressionBoss::getLevel(const string& needle, const Expression& e)
{
  return substitutionTable.at(needle).level;
}

unordered_map<string, ExpressionBoss::Substitution>
ExpressionBoss::getSubstitutionTable(void)
{
  return substitutionTable;
}

void ExpressionBoss::substituteUntilDoneLevel(Expression& outputSignature,
                     void (ExpressionBoss::*inFunction)(const string&))
{
  /* Get the initial set of variables you can substitute for. */
  set<string> varsAllowed;
  outputSignature.variablesSet(varsAllowed);
  /* Setup dependency stack. */
  buildDependencyStacks(varsAllowed);
  /* Level 0 is set of wires with no dependency. */
  set<string> levelZero;
  for(auto kv : substitutionTable)
  {
    if(kv.second.dependencyStack.empty())
    {
      levelZero.insert(kv.first);
    }
  }
  /* Assign levels to each substitution you know about. */
  propagateLevel(levelZero);

  /* Substitute until there are no variables to substitute for. */
  substituteUntilDone(outputSignature, varsAllowed, inFunction,
                      &ExpressionBoss::alwaysFalse, &ExpressionBoss::getLevel);
  /* Forget levelization. */
  for(auto& kv : substitutionTable)
  {
    kv.second.level = 0;
  }
}

string ExpressionBoss::inputSignature(const string& outputSignature)
{
  /* Convert that output signature to an Expression. */
  Expression signature(outputSignature);
  /* Convert the expression into an input signature. */
  inputSignature(signature);
  /* Return the input signature. */
  return signature.toString();
}

void ExpressionBoss::inputSignature(Expression& outputSignature) 
{
  #if DEBUG_INPUT_SIGNATURE
    cout << "Entering ExpressionBoss::inputSignature(" << outputSignature << ")"
         << endl;
  #endif
  levelization(outputSignature);
  #if DEBUG_INPUT_SIGNATURE
    cout << "Exiting ExpressionBoss::inputSignature.  Result="
         << outputSignature << endl;
  #endif
}

bool ExpressionBoss::isPrimaryInput(const string var)
{
  return substitutionTable.find(var) == substitutionTable.end();
}

void ExpressionBoss::learnSubstitution(const string& var, const string& rep)
{
  /* Avoid messing with original var and rep. */
  string variable = var;
  string replacement = rep;
  /* Clean inputs. */
  StringMath::removeAll(variable, ' ');
  StringMath::toUseful(replacement);
  StringMath::simplify(replacement);
  /* Store the substitution in a table. */
  substitutionTable[variable] = Substitution(replacement);
  /* Record this in the blind order. */
  blindOrder.push_back(variable);
}

string ExpressionBoss::levelization(const string& outputSignature)
{
  /* Convert that output signature to an Expression. */
  Expression signature(outputSignature);
  /* Convert the expression into an input signature. */
  levelization(signature);
  /* Return the input signature. */
  return signature.toString();
}

void ExpressionBoss::levelization(Expression& outputSignature)
{
  substituteUntilDoneLevel(outputSignature, &ExpressionBoss::dud);
}

void ExpressionBoss::loadToEQN(const string& variable)
{
  string result = variable + " = ";
  result += substitutionTable[variable].replacement;
  eqnBuffer.push_back(result);
}

void ExpressionBoss::makePrimaryInput(const string& newPrimaryInput)
{
  substitutionTable.erase(newPrimaryInput);
  blindOrder.remove(newPrimaryInput);
}

bool ExpressionBoss::match(const string& expA, const string& expB)
{
  /* Convert signatures to Expressions. */
  Expression expressionA(expA);
  Expression expressionB(expB);
  return match(expressionA, expressionB);
}

bool ExpressionBoss::match(Expression& expressionA, Expression& expressionB)
{
  /* Apply substitutions. */
  inputSignature(expressionA);
  inputSignature(expressionB);
  /* Compare expressions. */
  return expressionA == expressionB;
}

string ExpressionBoss::maxDependents(const string& outputSignature)
{
  /* Convert that output signature to an Expression. */
  Expression signature(outputSignature);
  /* Convert the expression into an input signature. */
  maxDependents(signature);
  /* Return the input signature. */
  return signature.toString();
}

void ExpressionBoss::maxDependents(Expression& outputSignature)
{
  /* Calculate dependentCount. */
  findDependentCount();
  set<string> varsAllowed;
  outputSignature.variablesSet(varsAllowed);
  substituteUntilDoneDependency(outputSignature, varsAllowed,
                                &ExpressionBoss::dud,
                                &ExpressionBoss::negativeDependentCount);
}

mpz_class ExpressionBoss::negativeDependentCount(const string& var,
                                                 const Expression& e)
{
  return -substitutionTable.at(var).dependentCount;
}

void ExpressionBoss::propagateLevel(const set<string> logicCone)
{
  /* Consider propagating to all variables in replacement expression. */
  for(auto v : logicCone)
  {
    /* Keep track of when you know this v isn't settled yet. */
    bool propagate = true;
    /* If v is a primary input, don't touch it. */
    auto vPointer = substitutionTable.find(v);
    if(vPointer == substitutionTable.end()) continue;
    /* Try to keep going until you've checked off all dependencies. */
    while(!vPointer->second.dependencyStack.empty())
    {
      /* See if the top dependency is settled. */
      string dependency = *vPointer->second.dependencyStack.begin();
      if(substitutionTable.at(dependency).dependencyStack.empty())
      {
        #if DEBUG_PROPAGATE_LEVEL
          cout << v << "(" << vPointer->second.level << "?) observes "
               << dependency <<"(" << substitutionTable.at(dependency).level
               << ") below it." << endl;
        #endif
        /* See if this dependency offers a better level. */
        mpz_class offeredLevel;
        offeredLevel = substitutionTable.at(dependency).level+1;
        if(offeredLevel > vPointer->second.level)
        {
          vPointer->second.level = offeredLevel;
        }
        /* This dependency is handled. */
        vPointer->second.dependencyStack.erase(dependency);
      }
      /* If not, you have to wait. */
      else
      {
        propagate = false;
        break;
      }
    }
    #if DEBUG_PROPAGATE_LEVEL
      cout << v << "(" << vPointer->second.level
           << ") has selected its level." << endl;
    #endif
    /* Maybe this was the last dependency for the wires above. */
    if(propagate)
    {
      propagateLevel(vPointer->second.variables);
    }
  }
}

void ExpressionBoss::substituteFor(const string& target, Expression& signature,
                  set<string>& varsAllowed,
                  void (ExpressionBoss::*inFunction)(const string&))
{
  /* This substitution will add new possibilities for substitution. */
  Expression* rep = &substitutionTable.at(target).replacement;
  rep->variablesSet(varsAllowed);
  /* Substitute. */
  if(doSubstitute) signature.substitute(target, *rep);
  /* Because levelization enforces dependency, target is now invalid. */
  varsAllowed.erase(target);
  /* Finally, call the function. */
  ((*this).*inFunction)(target);
  /* Substitution just happened. */
  substitutionTable.at(target).happened = true;
  loadToEQN(target);
  /* Constants get priority. */
  for(string possibleInstantSub : substitutionTable.at(target).variables)
  {
    /* Never do primary inputs. */
    auto findPossibleInstantSub = substitutionTable.find(possibleInstantSub);
    if(findPossibleInstantSub == substitutionTable.end())
    {
      varsAllowed.erase(possibleInstantSub);
      continue;
    }
    if(findPossibleInstantSub->second.variables.empty())
    {
      substituteFor(possibleInstantSub, signature, varsAllowed, inFunction);
    }
  }
}

void ExpressionBoss::substituteUntilDone(Expression& outputSignature,
       set<string> varsAllowed,
       void (ExpressionBoss::*inFunction)(const string&),
       bool (ExpressionBoss::*filterFunction)(const string&),
       mpz_class(ExpressionBoss::*minimizeMe)(const string&, const Expression&))
{
  #if DEBUG_SUBSTITUTE_UNTIL_DONE
    cout << "Enter substituteUntilDone" << endl;
  #endif
  /*
  Most of the time, dependency will put a maximum bound on the number
  of substitutions that can happen in this process.
  */
  eqnBuffer.reserve(substitutionTable.size()+eqnBuffer.size()+1);
  eqnBuffer.push_back(outputSignature);
  /* Repeat until there are no variables to substitute for. */
  while(true)
  {
    /* Filter out primary inputs. */
    for(auto v = varsAllowed.begin(); v != varsAllowed.end();)
    {
      auto vPointer = substitutionTable.find(*v);
      if(vPointer != substitutionTable.end())
      {
        ++v;
      }
      else
      {
        v = varsAllowed.erase(v);
      }
    }
    /* Abort if it's only primary inputs. */
    if(varsAllowed.empty())
    {
      break;
    }
    /* Find the target of substitution. */
    string target;
    mpz_class minLevel;
    /* Will record the current wire's level. */
    mpz_class current;
    for(auto v = varsAllowed.begin(); v != varsAllowed.end(); ++v)
    {
      /* Apply filter. */
      if(!((*this).*filterFunction)(*v))
      {
        /* Apply low water mark algorithm. */
        current = ((*this).*minimizeMe)(*v, outputSignature);
        if(minLevel > current || target.empty())
        {
          minLevel = current;
          target = *v;
        }
      }
    }
    /* If everyone got filtered, just do the first substitution. */
    if(target.empty())
    {
      target = *varsAllowed.begin();
      #if DEBUG_SUBSTITUTE_UNTIL_DONE
        cout << "No good target, target=" << target << endl;
      #endif
    }
    #if DEBUG_SUBSTITUTE_UNTIL_DONE
      cout << "in ";
      for(auto i : varsAllowed)
      {
        cout << i << "(" << ((*this).*minimizeMe)(i, outputSignature).get_str()
             << ") ";
      }
      cout << "minlevel was=" << minLevel.get_str() << ", came from "
           << target << endl;
    #endif
    substituteFor(target, outputSignature, varsAllowed, inFunction);
  }
  /* Store result in eqn. */
  eqnBuffer.push_back("# Expected Output: " + outputSignature.toString());
}

void ExpressionBoss::substituteUntilDoneDependency(Expression& outputSignature,
      set<string> varsAllowed,
      void (ExpressionBoss::*inFunction)(const string&),
      mpz_class(ExpressionBoss::*minimizeMe)(const string&, const Expression&))
{
  /* Setup dependency stack. */
  buildDependencyStacks(varsAllowed);
  substituteUntilDone(outputSignature, varsAllowed, inFunction,
                      &ExpressionBoss::dependenciesUnsatisfied, minimizeMe);
}

mpz_class ExpressionBoss::termsAddedBy(const string& var, const Expression& exp)
{
  mpz_class result = substitutionTable.at(var).replacement.numberOfTerms()
                   * exp.numberOfTermsWith(var);
  return result;
}

mpz_class ExpressionBoss::termsAfter(const string& var, const Expression& exp)
{
  /* Make a copy of the expression. */
  Expression temp = exp;
  /* Try the substitution. */
  temp.substitute(var, substitutionTable.at(var).replacement);
  /* Count the terms. */
  return temp.numberOfTerms();
}

ExpressionBoss::Substitution::Substitution()
{
  /* Start off knowing nothing about levels. */
  level = 0;
}

/* This is a "delegating constructor". */
ExpressionBoss::Substitution::Substitution(const string& rep) : Substitution()
{
  /* Build expression. */
  new (&replacement) Expression(rep);
  /* Get variables. */
  variables.clear();
  replacement.variablesSet(variables);
} 

void ExpressionBoss::setDoSubstitute(bool ds)
{
  doSubstitute = ds;
}
