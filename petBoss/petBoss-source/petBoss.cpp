/*
Maintained by Walter Brown
Open for modification to others in Maciej Ciesielski's team.
*/
#include "expressionBoss.h"
#include "stringmath.h"
#include <unistd.h>
using namespace std;

bool blind          = false;
bool cheapGreed     = false;
bool expensiveGreed = false;
bool help           = true;
bool levelization   = false;
bool maxDependents  = false;
bool toEQN          = false;
bool doSubstitute   = true;
bool satMiter       = false;

void commentCout(string in)
{
  if(toEQN)
  {
    cout << "# ";
  }
  cout << in << endl;
}

void usage()
{
  commentCout(
"Usage: ./petBoss -b < *.eqn"
  );
}

void welcome()
{
  char *login;
  if((login = getlogin()) != NULL)
  {
    commentCout(string("Welcome to petBoss, ") + login + "!");
  }
  else
  {
    commentCout("Welcome to petBoss!");
  }
}

void prompt()
{
  if(!toEQN)
  {
    cout << ">>>";
  }
}

int main(int argc, char **argv)
{
  /* Get options. */
  char c;
  while ((c = getopt(argc, argv, "bceHlmpSs")) != -1)
  {
    switch (c)
    {
      case 'b':
        blind = true;
        break;
      case 'c':
        cheapGreed = true;
        break;
      case 'e':
        expensiveGreed = true;
        break;
      case 'H':
        help = false;
        break;
      case 'l':
        levelization = true;
        break;
      case 'm':
        maxDependents = true;
        break;
      case 'p':
        toEQN = true;
        break;
      case 'S':
        doSubstitute = false;
	break;
      case 's':
        satMiter = true;
	break;
      case '?':
        toEQN = false;
        usage();
        return EXIT_FAILURE;
    }
  }
  /* Make sure options are fine. */
  if(blind+cheapGreed+expensiveGreed+levelization+maxDependents > 1)
  {
    cout << "-b, -c, -e, -l and -m are incompatible." << endl;
    return EXIT_FAILURE;
  }
  if(help)
  {
    welcome();
    usage();
  }
  bool heardSignature = false; /* Used as a safety for $miter. */
  string input;
  ExpressionBoss boss;
  Expression lastResult, oldResult;
  boss.setDoSubstitute(doSubstitute);
  prompt();
  while(getline(cin, input)) {
    /* Remove comments. */
    auto comment = input.find('#');
    if(comment != string::npos) input = input.substr(0, comment);
    /* Remove spaces. */
    StringMath::removeAll(input, ' ');
    /* Do nothing with blank lines. */
    if(input.length() == 0) continue;
    /* Handle exit. */
    if(input == "$exit") break;
    if(input == "$help")
    {
      usage();
      prompt();
      continue;
    }
    /* Can forget all substitutions. */
    if(input == "$new")
    {
      boss.clear();
      prompt();
      continue;
    }
    /* Compares last two expressions. */
    if(input == "$miter")
    {
      /* First, make sure that the user didn't forget signatures. */
      if(heardSignature)
      {
        /* Figure out the difference between these two. */
        Expression diff = oldResult;
        diff.multiplyBy(-1);
        diff.add(lastResult);
        Expression zero("0");
        if(diff != zero)
        {
          cout << "No match.  Difference when " << diff << " is non-zero."
               << endl;
          if(satMiter)
          {
            /* Produce an example of failure. */
            cout << "Consider the following case:" << endl;
            map<string, bool> satis = diff.sat();
            for(const auto & s : satis)
            {
              cout << s.first << " = " << s.second << endl;
            }
          }
        }
        else
        {
          cout << "Match." << endl;
        }
      }
      else
      {
        cout << "No signature was detected." << endl;
      }
      prompt();
      continue;
    }
    #define DEBUG_AUTO_SIG 0
    /* Handle automatic signatures. */
    string tempLead = input.substr(0,5);
    #if DEBUG_AUTO_SIG
      cout << "tempLead = \"" << tempLead << "\"." << endl;
    #endif
    if(tempLead == "$usig" || tempLead == "$ssig")
    {
      string listString = input.substr(5);
      #if DEBUG_AUTO_SIG
        cout << "Attempting auto-sig..." << endl;
	cout << "listString = \"" << listString << "\"." << endl;
      #endif
      vector<string> outputs;
      StringMath::split(listString, ',', outputs);
      mpz_class coefficient(1);
      Expression autoSignature;
      for(int i = 0; i < outputs.size(); ++i)
      {
        Expression::Term outputTerm;
	outputTerm.coefficient = coefficient;
	outputTerm.variables.insert(outputs.at(i));
        autoSignature.add(outputTerm);
	coefficient *= 2;
      }
      /* If this is a signed circuit, then flip the msb. */
      if(input.at(1) == 's' && outputs.size() > 0)
      {
        Expression::Term flipper;
	flipper.coefficient = -1*coefficient;
	flipper.variables.insert(outputs.at(outputs.size()-1));
        autoSignature.add(flipper);
      }
      /* Take this as the new input you must parse. */
      input = autoSignature;
      #if DEBUG_AUTO_SIG
        cout << "Using signature " << input << endl;
      #endif
    }
    auto equalsSign = input.find('=');
    /* If this is a signature... */
    if(equalsSign == string::npos)
    {
      heardSignature = true;
      /* Shift previous result back. */
      oldResult = lastResult;
      /* Use the requested technique. */
      if(blind)
      {
        lastResult = boss.blind(input);
      }
      else if(cheapGreed)
      {
        lastResult = boss.cheapGreed(input);
      }
      else if(expensiveGreed)
      {
        lastResult = boss.expensiveGreed(input);
      }
      else if(levelization)
      {
        lastResult = boss.levelization(input);
      }
      else if(maxDependents)
      {
        lastResult = boss.maxDependents(input);
      }
      else
      {
        lastResult = boss.inputSignature(input);
      }
      /* Produce the eqn file for this signature. */
      if(toEQN)
      {
        for(auto s : boss.dumpEQN())
        {
          cout << s << endl;
        }
      }
      /* Otherwise, just print the signature. */
      else
      {
        cout << lastResult << endl;
      }
    }
    /* If this is a substitution... */
    else
    {
      /* Record it so it can be used when boss.inputSignature is called. */
      boss.learnSubstitution(input.substr(0, equalsSign),
                                                    input.substr(equalsSign+1));
    }
    prompt();
  }
  commentCout("How'd I do?");
  return EXIT_SUCCESS;
}

