#include <iostream>
#include <string>
#include <stack>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <unordered_set>
#include <regex>
#include <queue>

using namespace std;

extern unordered_set<string> varList;
extern int tVar; //For temporary variables
extern vector<string> lines;
bool errorCatched = false;
string exprFunc(string str,stack<string> &postfixStack);
void moretermsFunc(string moreterms,stack<string> &postfixStack);
void termFunc(string term,stack<string> &postfixStack);
void factorFunc(string factor,stack<string> &postfixStack);
void morefactorsFunc(string morefactors,stack<string> &postfixStack);
bool checkParanthesis(string expr);
bool varCheck(string factor);
bool chooseCheck(string factor,string &var);
extern string chooseCalculate(string exp1, string exp2, string exp3, string exp4);
extern void calculator(char opr,stack<string> &postfixStack);
extern bool instanceOfInt(string var);


/*  Calculates an expression using postfix form and returns a variable name that holds the result
    expr -> term moreterms
*/
string exprFunc(string str,stack<string> &postfixStack){

    if(!checkParanthesis(str)){//checking number of parantheses
        throw std::runtime_error("unmatched parantheses");
    }
    string LHS, RHS;
    stack<string> parenthesesStack;
    bool insideParantheses = false;
    int tokenIndex = 0;
    //Traversing the string to find the + and - signs, the operators inside the parantheses are excluded
    for (int tokenIndex = 0; tokenIndex < str.length(); ++tokenIndex)
    {
        //when encountered a parantheses opening, exclude the rest until finding the closure
        if(str[tokenIndex]== '('){
            parenthesesStack.push(str[tokenIndex]+"");
            insideParantheses = true;
        }
        else if(str[tokenIndex]== ')'){
            if(!insideParantheses) throw std::runtime_error("close paranthesesbefore open");
            parenthesesStack.pop();
            if(parenthesesStack.empty()){
                insideParantheses = false;
            }
        }

        else if(!insideParantheses && (str[tokenIndex]== '+' ||  str[tokenIndex] == '-' )){
            //when finding a + or -, separating string to left and right hand sides of operator 
            LHS = str.substr(0,tokenIndex);//doesn't include any + or -
            RHS = str.substr(tokenIndex);//right hand side includes the found operator itself
            termFunc(LHS,postfixStack);//calculate the left hand and push to stack
            moretermsFunc(RHS,postfixStack);//find the next term, use the included operator and apply it to top 2 element of the stack
            break;
        }
    }
    // if there is not any + or - in the string, it might be a term that includes * or -
    if(LHS == ""){
        LHS = str;  
        termFunc(LHS,postfixStack);
    }
    return postfixStack.top() ;// return the output variable name
}
/*
    Calculates a <term> using postfix form 
    term -> factor morefactors
*/
void termFunc(string str,stack<string> &postfixStack){
    string LHS, RHS;
    stack<string> parenthesesStack;
    bool insideParantheses = false;
    int tokenIndex = 0;
    //Similar to exprFunc, traversing the string, looking for * or /, the operators inside the parantheses are excluded 
    for (int tokenIndex = 0; tokenIndex < str.length(); ++tokenIndex)
    {
        if(str[tokenIndex]== '('){
            parenthesesStack.push(str[tokenIndex]+"");
            insideParantheses = true;
        }
        else if(str[tokenIndex]== ')'){
            parenthesesStack.pop();
            if(parenthesesStack.empty()){
                insideParantheses = false;
            }
        }
        else if(!insideParantheses && (str[tokenIndex]== '*' ||  str[tokenIndex] == '/' )){
            LHS = str.substr(0,tokenIndex);
            RHS = str.substr(tokenIndex);//inludes the found operator itself
            factorFunc(LHS,postfixStack);
            morefactorsFunc(RHS,postfixStack);
            return;
        }
    }
    if(LHS == ""){// if no operator found, it must be a factor
        LHS = str;
        factorFunc(LHS,postfixStack);
    }
}
/*
    Finds the next + or - operator, again separates the left and right hand sides, calculates and pushes the result of left hand side first,
    and then pops the top 2 elements from the operands stack and applies the current operator on them and pushes the result back.
    Also calls itself for the right hand side again for rest of the calculation
    moreterms -> + term {print(+)} moreterms |
                 - term {print(-)} moreterms |
                 NULL
*/
void moretermsFunc(string expr,stack<string> &postfixStack){

    char currentOpr = expr[0];
    //check if opr is valid
    if(currentOpr != '+' && currentOpr != '-'){
        throw std::runtime_error("invalid operator");
    }
    expr = expr.substr(1);//REMOVING THE OPERATOR
    //check if expr is not empty
    if(expr == ""){
        throw std::runtime_error( "right hand side is empty" );
    }
    string term, moreterms;
    stack<string> parenthesesStack;
    bool insideParantheses = false;
    int tokenIndex = 0;
    for (int tokenIndex = 0; tokenIndex < expr.length(); ++tokenIndex)
    {
        if(expr[tokenIndex]== '('){
            parenthesesStack.push(expr[tokenIndex]+"");
            insideParantheses = true;
        }
        else if(expr[tokenIndex]== ')'){
            parenthesesStack.pop();
            if(parenthesesStack.empty()){
                insideParantheses = false;
            }
        }
        else if(!insideParantheses && (expr[tokenIndex]== '+' ||  expr[tokenIndex] == '-' )){
            term = expr.substr(0,tokenIndex);
            moreterms = expr.substr(tokenIndex);
            termFunc(term,postfixStack);//calculate the left hand, push it to stack
            calculator(currentOpr,postfixStack);//pop the top 2 elements from stack and apply the current operator 
            //cout << currentOpr << endl;
            moretermsFunc(moreterms,postfixStack);//calculate the rest
            return;
        }
    }
    if(term == ""){// if there is not any + or -, expression must be a term
        term = expr;
        termFunc(term,postfixStack);//calculate the term, push it to stack
        calculator(currentOpr,postfixStack);//pop the top 2 element and apply the current operator, push the result back
        //cout << currentOpr << endl;
    }
}
/*
    Pushes factors to stack properly, if a variable or number directly push to stack, 
    if an <expression>, calculate it and and push the result to the stack
    factor -> (expr) |
              var {print(var)} |
              number {print(num)}
*/
void factorFunc(string factor,stack<string> &postfixStack){ 
    if(factor.empty()) throw std::runtime_error( "empty operand");
    //cout << factor << endl;
    int paranthesIndex = -1;
    string beforePar;//if there is a string before the beginning of parantheses
    bool isChoose = false;// whether this factor is a choose expression or not
    //traverse the factor until finding an open parantheses '('
    for (int i = 0; i < factor.length(); ++i)
    {
        if(!beforePar.empty() || ( factor[i]!=' ' && factor[i] != '\t')){
            if(factor[i] != '('){
                beforePar+= factor[i];
            }
            else{// found the opening parantheses
                if(!beforePar.empty()){
                    int i = beforePar.find("choose");
                    if(i!=-1){// if the string before parantheses is "choose" 
                        beforePar.replace(i,6,"");
                        beforePar.erase(std::remove(beforePar.begin(), beforePar.end(), ' '), beforePar.end());
                        beforePar.erase(std::remove(beforePar.begin(), beforePar.end(), '\t'), beforePar.end());
                        if(beforePar.empty()){
                            isChoose = true;// this factor is a choose expression
                        }
                        else{
                            throw std::runtime_error( "invalid factor,there is \"" + beforePar +"\" before paranthesess");
                        }
                    }
                    else{
                        throw std::runtime_error( "invalid factor,there is \"" + beforePar +"\" before paranthesess");
                    }
                }
                paranthesIndex = i;
                break;
            }
        }
    }

    int len = factor.find_last_of(')')-paranthesIndex-1;
    string spaceCheck = factor.substr(factor.find_last_of(')')+1);
    stringstream check(spaceCheck);
    string spaceCheck2;
    check >> spaceCheck2;

    if(paranthesIndex!= -1){//there is some parantheses choose() or (expr)
        if(!spaceCheck2.empty()){
            throw std::runtime_error( "invalid factor,there is \"" + spaceCheck2 +"\" after parantheses");
        
        }
            if(isChoose){
                string chooseResult;//result of choose
                bool chch = chooseCheck(factor,chooseResult); 
                //cout << factor << endl;
                if(!chch) throw std::runtime_error( "invalid choose");
                postfixStack.push(chooseResult);
                //cout << factor << endl;
            }
            else{// factor is an expression
                string expr = exprFunc(factor.substr(paranthesIndex+1,len),postfixStack);
                //cout << expr << endl;
            }
    }
    else{
        if(varCheck(factor)){//does factor have only one token
            // tokenize
            stringstream ss(factor); 
            string fact;
            ss >> fact;

            bool intCheck = instanceOfInt(fact);
            if(!intCheck){//variable or int ?
                //variable
                varList.insert(fact);     
                lines.push_back("%_" + to_string(tVar) + " = load i32* %" + fact);
                string loadName = "_" + to_string(tVar);
                tVar++;
                postfixStack.push(loadName);
            }
            else{ 
                //integer
                lines.push_back("%_" + to_string(tVar) + " = add i32 " + fact + ", 0");
                string loadName = "_" + to_string(tVar);
                tVar++;
                postfixStack.push(loadName);
            }
            //cout << factor << endl;
        }
        else {
            throw std::runtime_error("more than one token for factor"); //eklenti
            return;
        }
    }
    return;
}
/*
    Finds the next * or / operator, again separates the left and right hand sides, calculates and pushes the result of left hand side first,
    and then pops the top 2 elements from the operands stack and applies the current operator on them and pushes the result back.
    Also calls itself for the right hand side again for rest of the calculation
    morefactors -> * factor {print(*)} morefactors |
                   / factor {print(/)} morefactors |
                   NULL
*/
void morefactorsFunc(string term,stack<string> &postfixStack){
    if(term == "" ){
        throw runtime_error("empty operand");
    }
    char currentOpr = term[0];
    term = term.substr(1);
    string factor, morefactors;
    stack<string> parenthesesStack;
    bool insideParantheses = false;
    int tokenIndex = 0;
    for (int tokenIndex = 0; tokenIndex < term.length(); ++tokenIndex)
    {
        if(term[tokenIndex]== '('){
            parenthesesStack.push(term[tokenIndex]+"");
            insideParantheses = true;
        }
        else if(term[tokenIndex]== ')'){
            parenthesesStack.pop();
            if(parenthesesStack.empty()){
                insideParantheses = false;
            }
        }
        else if(!insideParantheses && (term[tokenIndex] == '*' ||  term[tokenIndex]== '/' )){
            factor = term.substr(0,tokenIndex);
            morefactors = term.substr(tokenIndex);
            factorFunc(factor,postfixStack);
            //cout << currentOpr << endl;
            calculator(currentOpr,postfixStack);
            morefactorsFunc(morefactors,postfixStack);
            return;
        }
    }
    if(factor == ""){
        factor = term;
        factorFunc(factor,postfixStack);
        //cout << currentOpr << endl;
        calculator(currentOpr,postfixStack);//pop
    }
}
//Checks whether the number of open parantheses and close parantheses matches or not
bool checkParanthesis(string expr){

    int open = 0;
    int close = 0;

    for(int i = 0; i < expr.length(); i++){

        if(expr[i] == '('){
            open++;
        }
        else if (expr[i] == ')'){
            close++;
        }
    }
    return open == close;
}

bool varCheck(string factor){// check the token number of a variable
    stringstream ss(factor);
    string word;
    int token = 0;
    while(ss >> word)
    {
        token++;
    }
    return token == 1;
}

bool chooseCheck(string factor, string &var){
    // cout << "--------------------------------"<< endl;
    // cout <<  factor <<endl;
    int openParanthesesIndex = factor.find('(');    
    int len = factor.find_last_of(')') - openParanthesesIndex-1;
    string fact = factor.substr(openParanthesesIndex+1,len); // a,b,expr,d
    //cout << fact << endl;
    // regex rNestedChoose("[\t ]*choose[\t ]*\\([ \ta-zA-Z0-9(),+-/*]+,[ \ta-zA-Z0-9(),+-/*]+,[ \ta-zA-Z0-9(),+-/*]+,[ \ta-zA-Z0-9(),+-/*]+\\)[\t ]*");
    // smatch m;
    // regex_search(fact, m, rNestedChoose);
    // while(!m.empty()){
    //     fact = regex_replace(fact,rNestedChoose,"expr");
    //     cout << fact << endl;
    //     regex_search(fact, m, rNestedChoose);
    // }
    //cout << "--------------------------------"<< endl;
    //cout <<  fact <<endl;
    int chooseNumber = 1;
    int chooseIndex = fact.find("choose"); 
    queue<string> ch;
    while(chooseIndex != -1){
        string temp = fact.substr(chooseIndex);
        stack<string> parenthesesStack;
        bool insideParantheses = false;
        int tokenIndex = temp.find('(');
        if(tokenIndex!=-1){
            tokenIndex +=chooseIndex;
            for (tokenIndex; tokenIndex < fact.length(); ++tokenIndex)
            {
                if(fact[tokenIndex]== '('){
                    parenthesesStack.push(fact[tokenIndex]+"");
                    insideParantheses = true;
                }
                if(fact[tokenIndex]== ')'){
                    parenthesesStack.pop();
                    if(parenthesesStack.empty()){
                        insideParantheses = false;
                    }
                }
                if(!insideParantheses ){
                    break;
                }
            }
            int len2 = tokenIndex +1- chooseIndex;
            ch.push(fact.substr(chooseIndex,len2));
            //cout << "-----------"<< endl;
            //cout << fact.substr(chooseIndex,len2) <<endl;
            fact.replace(chooseIndex,len2,"expr_");
        }
        chooseIndex = fact.find("choose");
    }
    vector <string> tokens;
    stringstream check(fact);
    string intermediate;
    // Tokenizing w.r.t. space ' '
    while(getline(check, intermediate, ',')) 
    {
        //cout << intermediate << endl;
        int ind = intermediate.find("expr_");
        //bool m = ind==-1;
        while(ind!= -1){
            //cout << "00000000000000000"  << ch.front()<< endl;
            intermediate.replace(ind,5,ch.front());
            ch.pop();
            ind = intermediate.find("expr_");
        }
        tokens.push_back(intermediate);
        // stringstream str;
        // str << intermediate;
        // string i;
        // str >> i;
        // if(i == "expr_"){
        //     //cout << ch.front() << "000" << endl;
        //    tokens.push_back(ch.front()); 
        //    ch.pop();
        // }
        // else{
        //     tokens.push_back(intermediate);
        // }
    }
    for (int i = 0; i < tokens.size(); ++i)
    {
        stringstream check(tokens[i]);
        string t;
        check >> t;
        if(t.empty()){
            tokens.erase(tokens.begin()+i-1);
        }
    }
    if(tokens.size() != 4){
        //cout << "000000000000000000" <<endl;
        return false;
    }
    else{//a,b,expr,d
        //cout << "----------------"<< endl;
        // cout << tokens[0]<< endl;
        // cout << tokens[1]<< endl;
        // cout << tokens[2]<< endl;
        // cout << tokens[3]<< endl;
        var = chooseCalculate(tokens[0],tokens[1],tokens[2],tokens[3]);
        lines.push_back("%_"+to_string(tVar)+" = load i32* %"+var);
        var = "_"+to_string(tVar);
        tVar++;
        return true;
    }
}
