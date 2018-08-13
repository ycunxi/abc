/*
Maintained by Walter Brown
Open for modification to others in Maciej Ciesielski's team.
*/
#include "stringmath.h"

string StringMath::add(string& a, string& b) {
  mpz_class ta, tb, result;
  ta = a.c_str();
  tb = b.c_str();
  result = ta+tb;
  return result.get_str();
}

#define P_DO_CHUNK if(!isNumber) collisionTable[buffer] == true;
void StringMath::allVariables(string& expression, list<string>& result) {
  /* Have a collision table to keep track of variables you've seen. */
  map<string, bool> collisionTable;

  /* Go through expression, variables will be separated by *. +, and -. */
  string buffer = "";
  buffer.reserve(expression.length());
  /* Assume buffer is a number until you find a letter. */
  bool isNumber = true;
  for(int i = 0; i < expression.length(); i++) {
    if(expression[i] == '*' || expression[i] == '+' || expression[i] == '-' ||
       expression[i] == '(' || expression[i] == ')') {
      P_DO_CHUNK
      buffer = "";
      isNumber = true;
    }
    else {
      buffer += expression[i];
      if(!isDigit(expression[i])) isNumber = false;
    }
  }
  P_DO_CHUNK

  /* Now translate this table into a list. */
  for(auto kv : collisionTable) {
    result.push_front(kv.first);
  }
}
#undef P_DO_CHUNK

bool StringMath::beforeInAlphabet(const string& a, const string& b) {
  for(int i = 0; i < a.length(); i++) {
    if(i >= b.length()) return false;
    if(a.at(i) < b.at(i)) return true;
    if(a.at(i) > b.at(i)) return false;
  }
  return false;
}

bool StringMath::beforeTerm(const string& a, const string& b) {
  mpz_class ta, tb;
  int fa = a.find('*');
  int fb = b.find('*');
  if(fa == string::npos) ta = a;
  else ta = a.substr(0,fa);
  if(fb == string::npos) tb = b;
  else tb = b.substr(0,fb);

  if(ta != tb) return ta < tb;
  else {
    if(fa == string::npos) return true;
    if(fb == string::npos) return false;
    return beforeInAlphabet(a.substr(fa), b.substr(fb));
  }
}

void StringMath::blockExpression(string& expression, list<string>& result) {
  /* This marks the start of the slice you're about to put in the list. */
  int start = 0;
  for(int i = 0; i < expression.length(); ++i) {
    /* Is this a breaker? */
    if(expression[i] == '+'||
       expression[i] == '-'||
       expression[i] == '*'||
       expression[i] == '('||
       expression[i] == ')'){
      /* Load what you have into the list! */
      if(i != start) result.push_back(expression.substr(start, i-start));
      /* Place the delimeter in the list. */
      result.push_back(expression.substr(i, 1));
      /* Update where the start of the next word will be. */
      start = i+1;
    }
  }
  /* Put the end in the list. */
  if(start != expression.length())
    result.push_back(expression.substr(start, expression.length()));
}

#define P_DO_CHUNK(END) {                                         \
  cleanTerm = target.substr(front, (END));                        \
  standardizeTerm(cleanTerm);                                     \
  int bound = cleanTerm.find('*');                                \
  if(bound == string::npos) {                                     \
    if(!coef.count("1")) coef["1"] = cleanTerm;                   \
    else {                                                        \
      string str = coef["1"];                                     \
      coef["1"] = add(str, cleanTerm);                            \
    }                                                             \
  }                                                               \
  else {                                                          \
    string mono = cleanTerm.substr(bound+1);                      \
    if(!coef.count(mono)) {                                       \
      coef[mono] =  cleanTerm.substr(0,bound);                    \
    }                                                             \
    else {                                                        \
      string str  = coef[mono];                                   \
      string str2 = cleanTerm.substr(0,bound);                    \
      coef[mono]  = add(str, str2);                               \
    }                                                             \
  }                                                               \
}
void StringMath::combineLikeTerms(string& target) {
  map<string, string> coef; /* map <variable name, coefficient> */
  string cleanTerm;
  int front = 0;
  for(int i = 0; i < target.length(); i++) {
    if(target[i] == '+') {
      P_DO_CHUNK(i-front)
      front = i+1;
    }
  }
  P_DO_CHUNK(target.length())

  /* There's an entry in coef now for every monomial type. */
  target = "";

  /* First, clear the constant term. */
  if(coef.count("1") && coef["1"] != "0") {
    target += coef["1"];
    target += "+";
    coef.erase("1");
  }
  /* Put in monomials. */
  for(auto kv : coef) {
    if(kv.second == "0") continue;
    target += kv.second;
    target += "*";
    target += kv.first;
    target += "+";
  }
  /* Either clip the extra + or set this to a zero. */
  if(target.size())
    target.resize(target.size()-1);
  else
    target = "0";
}
#undef P_DO_CHUNK

void StringMath::distribute(string& target) {
  vector<string> temp;
  distributeToList(target, temp);
  join(temp, '+', target);
}

void StringMath::distributeToList(string& input, vector<string>& resultList) {
  vector<string> outerTerms;
  /* Split by + signs not in ()s. */
  outsideSplit(input, '+', outerTerms);

  /* Handle each term on its own. */
  for(string currentTerm : outerTerms) {
    vector<string> multipliers;
    vector<vector<string> > listMultipliers;
    outsideSplit(currentTerm, '*', multipliers);
    listMultipliers.reserve(multipliers.size());
    /* Now, make sure that all multiplicands are distributed. */
    for(string multiplier : multipliers) {
      /* If a multiplicand with parenthesis that need distribution...*/
      vector<string> v;
      if(multiplier[0] == '(') {
        string sub = multiplier.substr(1, multiplier.size()-2);
        distributeToList(sub, v);
      }
      else {
        v.push_back(multiplier);
      }
      listMultipliers.push_back(v);
    }
    /* termResult is a running tally as the items get multiplied together. */
    vector<string> termResult;
    termResult.push_back("1");
    for(vector<string> multiplier : listMultipliers) {
      vector<string> newTermResult;
      newTermResult.reserve(multiplier.size()*termResult.size());
      for(string multiplierTerm : multiplier) {
        for(string termResultTerm : termResult) {
          newTermResult.push_back(multiplierTerm+string("*")+termResultTerm);
        }
      }
      termResult = newTermResult;
    }
    /* At this point, termResult should contain all terms. */
    for(string s : termResult) {
      standardizeTerm(s);
      resultList.push_back(s);
    }
  }
}

void StringMath::dropFrontZero(string& target) {
  int firstNonZero = 0;
  while(target[firstNonZero] == '0') {
    firstNonZero++;
  }
  target.erase(0,firstNonZero);
}

bool StringMath::isDigit(char c) {
  return c >= '0'&& c <= '9' || c == '-';
}

bool StringMath::isNumber(string& num)
{
  /* "" not a number.  Must start with digit or -. */
  if(num.length() == 0 || !isDigit(num[0])) return false;
  /* See if you can find a character that isn't a digit. */
  for(int i = 1; i < num.length(); ++i)
  {
    /* For rest of numbers, can't even be '-'! */
    if(num[i] < '0' || num[i] > '9') return false;
  }
  return true;
}

void StringMath::join(vector<string>& v, char d, string& result) {
  /* Count the contents of v to determine string size. */
  int memory = v.size();
  for(string s : v)
    memory += s.length();
  result.reserve(memory);
  result = "";

  /* Store the string. */
  for(string s : v) {
    result += s;
    result += d;
  }
  if(result.size())
    result.resize(result.size()-1);
}

string StringMath::multiply(string& a, string& b) {
  mpz_class ta, tb, result;
  ta = a.c_str();
  tb = b.c_str();
  result = ta*tb;
  return result.get_str();
}

bool StringMath::notSign(char c) {
  return !(c == '-') && !(c == '+');
}

void StringMath::orderTerms(string& target) {
  vector<string> terms;
  split(target, '+', terms);
  sort(terms.begin(), terms.end(), beforeTerm);
  join(terms, '+', target);
}

#define P_STORE_PART(END) {                      \
  result.push_back(input.substr(front, (END)));  \
}
void StringMath::outsideSplit(string& input, char needle,
                            vector<string>& result) {
  /* First count number of multipliers so space can be locked in. */
  int chunks = 1;
  for(char c : input) {
    if(c == needle) chunks++;
  }
  result.reserve(result.capacity()+chunks);

  /* Split oustide ()s. */
  int parenCount = 0;
  int front = 0;
  for(int i = 0; i < input.length(); i++) {
    if(input[i] == '(') {
      parenCount++;
    }
    else if(input[i] == ')') {
      parenCount--;
    }
    else if(parenCount == 0 && input[i] == needle) {
      P_STORE_PART(i-front);
      front = i+1;
    }
  }
  P_STORE_PART(input.length());

  /* Drop unnecessary space. */
  result.reserve(result.size());
}
#undef P_STORE_PART

string StringMath::power(string& base, string& exponent) {
  mpz_class result;
  long unsigned int b, e;
  b = stoi(base);
  e = stoi(exponent);

  mpz_ui_pow_ui(result.get_mpz_t(), b, e);
  return result.get_str();
}

void StringMath::removeAll(string& target, char c) {
  int last = 0;
  for(int i = 0; i < target.length(); i++) {
    if(target[i] != c)
      target[last++] = target[i];
  }
  target.resize(last);
}

void StringMath::removeAll(vector<string>& subject, const string& needle) {
  auto iter = subject.begin();
  while(iter != subject.end()) {
    if(*iter == needle) {
      if(iter == subject.end()) {
        subject.pop_back();
        return;
      }
      iter = subject.erase(iter);
    }
    else
      iter++;
  }
}

void StringMath::simplify(string& target) {
  distribute(target);
  combineLikeTerms(target);
}

#define P_STORE_PART(END) {                     \
  result.push_back(term.substr(front, (END)));  \
}
void StringMath::split(const string& term, char needle, vector<string>& result){
  /* First count number of multipliers so space can be locked in. */
  int chunks = 1;
  for(char c : term) {
    if(c == needle) chunks++;
  }
  result.reserve(result.capacity()+chunks);

  /* Next break this into strings. */
  int front = 0;
  for(int i = 0; i < term.length(); i++) {
    if(term[i] == needle) {
      P_STORE_PART(i-front);
      front = i+1;
    }
  }
  P_STORE_PART(term.length());
}
#undef P_STORE_PART


void StringMath::standardizeSigns(string& target) {
  string in = target;
  target = "";
  target.reserve(in.length()*2);
  bool negative = false;
  for(int i = 0; i < in.length(); i++) {
    if(in[i] == '+') {
      
    }
    else if(in[i] == '-') {
      negative = !negative;
    }
    /* If this isn't a sign... */
    else {
      /* If you just finished reading in the signs... */
      if(i == 0 || !notSign(in[i-1])) {
        /* Dump the total sign. */
        /* Only place + if there's a variable on the other side. */
        if(target.length() != 0 && target.back() != '(')
          target += "+";
        if(negative) target += "-";
        /* Prepare for the next round of counting. */
        negative = false;
      }
      /* Always jot down variable names. */
      target += in[i];
    }
  }
}

#define P_DO_CHUNK(END) {                                 \
  if(isNumber) {                                          \
    string str = target.substr(front, END);               \
    coef = multiply(coef, str);                           \
  }                                                       \
  else {                                                  \
    vars.push_back(target.substr(front, END));            \
  }                                                       \
}
void StringMath::standardizeTerm(string& target) {
  vector<string> vars;
  string coef = "1";
  int front = 0;
  bool isNumber = true;
  for(int i = 0; i < target.length(); i++) {
    if(target.at(i) == '*') {
      P_DO_CHUNK(i-front)
      isNumber = true;
      front = i+1;
    }
    else if(!isDigit(target.at(i))) {
      isNumber = false;
    }
  }
  P_DO_CHUNK(target.length()-front)

  /* At this point, vars contains all variables.  coef is the coefficient. */
  alphabeticalSort(vars.begin(), vars.end());

  /* By now, vars is in the proper order for loading. */
  target = "";
  target.append(coef);
  for(int i = 0; i < vars.size(); i++) {
    /* Drop boolean powers. */
    if(i != 0 && vars[i] == vars[i-1]) continue;
    target.append("*"+vars[i]);
  }
}
#undef P_DO_CHUNK


/* t=target, n=needle, r=replacement */
#define P_CHECK_MATCH(END) {           \
  string subst = in.substr(front, END);\
  if(n == subst) {                     \
    /* You've found a needle! */       \
    t += "(" + r + ")";                \
  }                                    \
  else {                               \
    t += subst;                        \
  }                                    \
}
void StringMath::substitute(string& t, string& n, string& r) {
#if 1
  string in = t;
  t = "";

  int front = 0;
  for(int i = 0; i < in.length(); i++) {
    /* If this is a proper variable splitter... */
    if(in[i] == '+'    ||
       in[i] == '-'    ||
       in[i] == '*'    ||
       in[i] == '('    ||
       in[i] == ')'    ){
      P_CHECK_MATCH(i-front)
      front = i+1;
      /* Put in the splitting symbol. */
      t += in[i];
    }
  }
  P_CHECK_MATCH(in.length());
#else /* Tests suggest this approach is slower, but it uses .reserve function */
  /* Convert the expression into a list of variable names and operators. */
  list<string> breakDown;
  blockExpression(t, breakDown);

  /* Go through the list. */
  int expectedSize = 0;
  string replacement = "(" + r + ")";
  for(string& i : breakDown) {
    /* If this is what you want to substitute for, substitute! */
    if(i == n) i = replacement;

    /* Record how much space this element will take. */
    expectedSize += i.length();
  }

  /* Allocate the space you'll need. */
  t = "";
  t.reserve(expectedSize);

  /* Fill the string. */
  for(string i : breakDown) {
    t += i;
  }
#endif
}
#undef P_CHECK_MATCH

#define P_LOAD() {                        \
  if(base == "") {                        \
    target += loadingBuffer;              \
  }                                       \
  else {                                  \
    target += power(base, loadingBuffer); \
    base = "";                            \
  }                                       \
}
void StringMath::toUseful(string& target) {
  removeAll(target, ' ');
  standardizeSigns(target);
  /* Convert -cow to -1*cow.  Also convert frog to 1*frog. */
  string in = target;
  target = "";
  target.reserve(in.length()*3);
  for(int i = 0; i < in.length(); i++) {
    if((i == 0 || in[i-1] == '-' || in[i-1] == '+') && !isDigit(in[i]))
    {
      target += "1*";
    }
    target += in[i];
  }

  /* Replace 2^10 with 1024 */
  /* I have no idea how to predict the size at this point... */
  in = target;
  target = "";
  string loadingBuffer = "";
  string base = "";
  loadingBuffer.reserve(in.length());
  for(char c : in) {
    if(c == '(' || c == ')' || c == '*' || c == '+') {
      P_LOAD()
      target += c;
      loadingBuffer = "";
    }
    else if(c == '^') {
      base = loadingBuffer;
      loadingBuffer = "";
    }
    else loadingBuffer += c;
  }
  P_LOAD();
  target.resize(target.length());

  /* If there's nothing here at this point, it's a zero. */
  if(target.empty()) target = "0";
}

void StringMath::toHuman(string& target) {
  int writeIn = 0;
  for(int i = 0; i < target.length(); i++) {
    if(i != target.length()-1) {
      /* Don't write 1* if leading term.*/
      if((target[i] == '1' && target[i+1] == '*') &&
         (i == 0 || target[i-1] == '+' || target[i-1] == '-')) {
        /* Skip next character. */
        i++;
        /* Don't write. */
        continue;
      }
      /* Skip the + in +-. */
      if(target[i] == '+' && target[i+1] == '-')
        continue;
    }
    /* If no issues so far, draw the character. */
    target[writeIn++] = target[i];
  }
  /* Cut off unnecessary target. */
  target.resize(writeIn);
}

void StringMath::zeroExtend(string& target, int newLength, string& result) {
  string zero = string(newLength-target.length(), '0');
  result = zero + target;
}
