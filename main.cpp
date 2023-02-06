#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <string.h>
#include <sstream>
#include <stack>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <regex>

using namespace std;

int tVar = 1; //For temporary variables
int whLabel = 0; //For labelling while 'condition', 'end' and 'body'
int ifLabel = 0; //For labelling if 'condition', 'end' and 'body'
unordered_set<string> varList; //To hold variable names
int chVar = 0; //Labelling choose conditions
int chRes = 0; //To label the result of choose 
int numLine = -1;//hold the number of lines in case of occurence of error
vector<string> lines; //to store the lines of .ll file

extern string exprFunc(string expr,stack<string> &postfixStack);
void calculator(char opr,stack<string> &postfixStack);
string trim(string expr);
string calculate(string var1, string var2, char opr);
bool instanceOfInt(string var);
void errorPrint(ofstream &outfile);

int main(int argc, char* argv[]) {

	string file_name = argv[1];

	ifstream infile;
	infile.open(file_name);

	ofstream outfile;
	outfile.open(file_name.substr(0,file_name.length()-2)+"ll"); 

	string line;

	bool whInside = false; //to detect whether we are inside the while statement to avoid nested
	bool ifInside = false; //to detect whether we are inside the if statement to avoid nested

	outfile << "; ModuleID = 'mylang2ir'" << endl;
	outfile << "declare i32 @printf(i8*, ...)" << endl;
	outfile << "@print.str = constant [4 x i8] c\"%d\\0A\\00\"" << endl;
	outfile << "@.ln = constant [6 x i8] c\"Line \\00\"" << endl;
	outfile << "@.num = constant [3 x i8] c\"%d\\00\"" << endl;
	outfile << "@.snx = constant [16 x i8] c\": syntax error\\0A\\00\"" << endl;
	outfile << "define i32 @main() {" << endl;
	
	while(getline(infile, line)){

		numLine++;
			
		string tempLine = line;	
		stack <string> p;
	
		int commentIndex = tempLine.find("#");// looking for comment to ignore		
		
		if(commentIndex >= 0){ //to ignore comments	
        	tempLine = tempLine.substr(0, commentIndex);	
    	}

     	string lineCheck = tempLine;	
        lineCheck.erase(std::remove(lineCheck.begin(), lineCheck.end(), ' '),lineCheck.end());	
        lineCheck.erase(std::remove(lineCheck.begin(), lineCheck.end(), '\t'), lineCheck.end());
        lineCheck.erase(std::remove(lineCheck.begin(), lineCheck.end(), '\n'), lineCheck.end());	
        if(lineCheck.empty()){ //Empty lines dont hurt		
 			continue;	
		}	

    	string sstemp = tempLine;	
		stringstream ss(sstemp);
	
    	string word1;
   		ss >> word1;
   		 
   		if(word1.empty()){
   			continue;
   		}
    	
    	string word2;
   		ss >> word2;

		if(word1.substr(0,5) == "print" && (tempLine.find("=") == string::npos)){ //Print statement is found
			int begin = tempLine.find("(");
			int end = tempLine.find_last_of(')'); 

			regex rPrint("^[\\t \\r]*print[\\t \\r]*\\([a-zA-Z0-9 \\t+-/*\\(\\),]*\\)[\\t \\r\\n]*$");
			smatch m;
			if(!regex_match(tempLine,rPrint)){
				//cout <<"heree" <<endl;
				errorPrint(outfile);
				return 0;
			}
			string expression = trim(tempLine.substr(begin+1, end-begin-1)); 	
			string n;	
			
			try{	
				n = exprFunc(expression,p);	
		    }catch (const std::runtime_error& ex) {	
                //cerr << ex.what() << endl;	
                errorPrint(outfile);
	            return 0;
    		}	
				
			string written = "call i32 (i8*, ...)* @printf(i8* getelementptr ([4 x i8]* @print.str, i32 0, i32 0), i32 %" + n + ")";
			lines.push_back(written);	
		}

		else if(word1.substr(0,5) == "while" && (tempLine.find("=") == string::npos)){// while statment is found
			
			if(ifInside || whInside){ //To avoid nested while/if
				errorPrint(outfile);
	            return 0;
			}

			int end = tempLine.find("{");	
			int endOfExpression = tempLine.find_last_of(')');	
			int begin = tempLine.find("(");	
		
			regex rWhile("^[\\t \\r]*while[\\t \\r]*\\([a-zA-Z0-9 \\t+-/*\\(\\),]*\\)[\\t \\r]*\\{[\\t \\r\\n]*$");
			smatch m;
			if(!regex_match(tempLine,rWhile)){
				//cout <<"heree" <<endl;
				errorPrint(outfile);
				return 0;
			}	
			string expression = tempLine.substr(begin+1, endOfExpression - begin-1);	
			expression = trim(expression);		
			lines.push_back("br label %whcond_" + to_string(whLabel));
			lines.push_back("whcond_" + to_string(whLabel) + ":");	
	
			string n;	
			try{	
				n = exprFunc(expression,p);
		    }catch (const std::runtime_error& ex) {		
                //cerr << ex.what() << endl;	
                errorPrint(outfile);
	            return 0;	
    		}	
				
			whInside = true;	
			string written = "%_" + to_string(tVar) + " = icmp ne i32 %" + n + ", 0";
			lines.push_back(written);
			tVar++;
			written = "br i1 %_" + to_string(tVar - 1) + ", label %whbody_" + to_string(whLabel) + ", label %whend_" + to_string(whLabel);
			lines.push_back(written);
			written = "whbody_" + to_string(whLabel) + ":";
			lines.push_back(written);
		}
		else if(word1.substr(0,2) == "if" && (tempLine.find("=") == string::npos)){ // if statement is found
			if(ifInside || whInside){ //To avoid nested while/if
				errorPrint(outfile);
	            return 0;
			}
			int end = tempLine.find("{");
			int endOfExpression = tempLine.find_last_of(')');	
			int begin = tempLine.find("(");
			regex rIf("^[\\t \\r]*if[\\t \\r]*\\([a-zA-Z0-9 \\t+-/*\\(\\),]*\\)[\\t \\r]*\\{[\\t \\r\\n]*$");
			smatch m;
			if(!regex_match(tempLine,rIf)){
				//cout <<"heree" <<endl;
				errorPrint(outfile);
				return 0;
			}
			string expression = tempLine.substr(begin+1, endOfExpression - begin-1);
			expression = trim(expression);
			ifInside = true;
			string n;	
			
			try{	
				n = exprFunc(expression,p);
		    }catch (const std::runtime_error& ex) {	
                errorPrint(outfile);
	            return 0;
    		}	
	    	lines.push_back("br label %ifcond_" + to_string(ifLabel));
			lines.push_back("ifcond_" + to_string(ifLabel) + ":");
			lines.push_back("%_" + to_string(tVar) + " = icmp ne i32 %" + n + ", 0");
			tVar++;
			lines.push_back("br i1 %_" + to_string(tVar - 1) + ", label %ifbody_" + to_string(ifLabel) + ", label %ifend_" + to_string(ifLabel));
			lines.push_back("ifbody_" + to_string(ifLabel) + ":");
		}
		else if(word1.find("=") != string::npos || word2.find("=") != string::npos){ //assignment statement is found
			string varName;	
			string value;	
			if(word1.find("=") != string::npos){// valid assignment	
				int i = word1.find("=");	
				varName = word1.substr(0,i);	
			}	
			else{	
				if(word2[0] != '=') {
					//throw std::runtime_error( "more than one variable to assign");	
					errorPrint(outfile);
            		return 0;
				}
				else{//valid assingment	
					varName = word1;	
				}	
			}	
			int indexOfEquals = tempLine.find("=");	
			if(indexOfEquals != tempLine.find_last_of("=")) {
				//throw std::runtime_error( "cant have more than one equals sign");	
				errorPrint(outfile);
	            return 0;
			}
			value = tempLine.substr(indexOfEquals+1);	
			if(varName == "while" || varName == "if" || varName == "print" || varName=="choose"){	
				errorPrint(outfile);
	            return 0;
			}	
			if (varList.find(varName) == varList.end()){	
				varList.insert(varName);	
			} 	
			string tempVal; 	
			try{	
				tempVal = exprFunc(value,p); 	
			}catch (const std::runtime_error& ex) {	
                errorPrint(outfile);
	            return 0;
            }	
			lines.push_back("store i32 %" + tempVal + ", i32* %" + varName);
		}

		else if(word1 == "}"){
			if(!word2.empty()){ //there should not be other element than "}" in this statement
				errorPrint(outfile);
	            return 0;
			}
			if(ifInside){
				ifInside = false;
				lines.push_back("br label %ifend_" + to_string(ifLabel));
				lines.push_back("ifend_" + to_string(ifLabel) + ":\n");
				ifLabel++; 
			}
			else if(whInside){ 
				whInside = false;
				lines.push_back("br label %whcond_" + to_string(whLabel));
				lines.push_back("whend_" + to_string(whLabel) + ":\n");
				whLabel++; 
			}
			else { //Useless closing curly bracket
				errorPrint(outfile);
	            return 0;
			}
		}
		
		else { //weird sentence is found
			errorPrint(outfile);
	        return 0;
		}
	}

	lines.push_back("ret i32 0");
	lines.push_back("}");
	
	if(whInside || ifInside){ // Closing curly bracket for while or if is not founded
		errorPrint(outfile);
	    return 0;
	}

	for (auto& var : varList )
	{
		outfile << "%" << var << " = alloca i32" << endl;
		outfile << "store i32 0, i32* %" << var << endl;
	}
	for (int i = 0; i < lines.size(); i++) {
		outfile << lines[i] << endl;
	}
	return 0;
}

/*	Prints the error lines to output file

*/

void errorPrint(ofstream &outfile){

	outfile << "call i32 (i8*, ...)* @printf(i8* getelementptr ([6 x i8]* @.ln, i32 0, i32 0))" << endl;;
    outfile <<  "call i32 (i8*, ...)* @printf(i8* getelementptr ([3 x i8]* @.num, i32 0, i32 0), i32 " << numLine << ")"<< endl;;
    outfile << "call i32 (i8*, ...)* @printf(i8* getelementptr ([16 x i8]* @.snx, i32 0, i32 0))" << endl;
    
    outfile << "ret i32 0" << endl;
    outfile << "}" ;
}

/*
	Calls the "exprFunc" method for corresponding parameters 
	and Prints the choose branches to output file
*/

string chooseCalculate(string exp1, string exp2, string exp3, string exp4){
    stack <string> chooseStack;
    string expr1,expr2,expr3,expr4;
    try{
        expr1 = exprFunc(exp1,chooseStack);
        chooseStack.pop();
        expr2 = exprFunc(exp2,chooseStack);
        chooseStack.pop();
        expr3 = exprFunc(exp3,chooseStack);
        chooseStack.pop();
        expr4 = exprFunc(exp4,chooseStack);
        chooseStack.pop();
    } catch (const std::runtime_error& ex) {    
        throw std::runtime_error( "cinvalid choose expressions");   
    }   
    
    varList.insert("chooseRes_"+to_string(chRes));
    lines.push_back("br label %choose_" + to_string(chVar));
    lines.push_back("choose_" + to_string(chVar) + ":");
    lines.push_back("%_" + to_string(tVar) + " = icmp eq i32 %" + expr1 + ", 0");
    lines.push_back("br i1 %_" + to_string(tVar) + ", label %return_" + to_string(chVar) + ", label %choose_" + to_string(chVar + 1));
    tVar++;
    chVar++;
    lines.push_back("return_" + to_string(chVar-1) + ":");
    lines.push_back("store i32 %"+expr2+", i32* %chooseRes_"+to_string(chRes));
    lines.push_back("br label %chooseend_"+to_string(chRes));
    lines.push_back("choose_" + to_string(chVar) + ":");
    lines.push_back("%_" + to_string(tVar) + " = icmp slt i32 %" + expr1 +", 0");
    lines.push_back("br i1 %_" + to_string(tVar) + ", label %return_" + to_string(chVar) + ", label %choose_" + to_string(chVar + 1));
    chVar++;
    tVar++;
    lines.push_back("return_" + to_string(chVar-1) + ":");
    lines.push_back("store i32 %"+expr4+", i32* %chooseRes_"+to_string(chRes));
    lines.push_back("br label %chooseend_"+to_string(chRes));
    lines.push_back("choose_" + to_string(chVar) + ":");
    lines.push_back("store i32 %"+expr3+", i32* %chooseRes_"+to_string(chRes));
    lines.push_back("br label %chooseend_"+to_string(chRes));
    chVar++;
    lines.push_back("chooseend_"+to_string(chRes)+":");

    chRes++;
    return "chooseRes_"+to_string(chRes-1);

}

/* 	
	Returns the trimmed string

*/
string trim(string expr){
	for (int i = 0; i < expr.length(); i++)
	{
		if(expr[i] == ' '){
			expr.replace(i, 1, "");
		}
		else {
			break;
		}
	}
	for (int i = expr.length() - 1 ; i > 0; i--)
	{
		if(expr[i] == ' '){
			expr.replace(i, 1, "");
		}
		else {
			break;
		}
	}
	return expr;
}

/*	
	Prints the corresponding lines to output file to
	calculate equations and returns a temporary variable
*/
string calculate(string varR, string varL, char opr){

	switch(opr){
		case '+': {
			lines.push_back("%_" + to_string(tVar)  + " = add i32 %" + varL + ", %" + varR);
			break;
		}
		case '-':{
			lines.push_back("%_" + to_string(tVar)  + " = sub i32 %" + varL + ", %" + varR);
			break;
		}
		case '/':{
			lines.push_back("%_" + to_string(tVar)  + " = udiv i32 %" + varL + ", %" + varR);
			break;
		}
		case '*':{
			lines.push_back("%_" + to_string(tVar)  + " = mul i32 %" + varL + ", %" + varR);
			break;
		}
	}
	tVar++;
	return "_" + to_string(tVar-1);
}

/*	
	Calculates the equations in postfix form by calling
	"calculate" method
*/
void calculator(char opr,stack<string> &postfixStack){		
	    string varR = postfixStack.top();	 //Right variable	
	    postfixStack.pop();	
	    string varL = postfixStack.top();	//Left variable
	    postfixStack.pop();	
	    string tempVar = calculate(varR,varL,opr);//prints the operation and returns a temp varName	
	    postfixStack.push(tempVar);		
	}

/*	
	Returns true if the string parameter is an integer
*/
bool instanceOfInt(string var){
	string nums = "0123456789";
	if (nums.find(var.substr(0,1)) != string::npos){
		return true;
	}
	return false;
}