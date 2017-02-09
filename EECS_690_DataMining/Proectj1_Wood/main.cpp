//main.cpp

#include <iostream>
#include <vector>
#include <regex>
#include <string>
#include <fstream>
#include "caseList.h"
#include "avPairs.h"
#include "lem2Calc.h"

using namespace std;

vector<vector<caseList::entry*>> dataSet; // This contains all the values that we read in
vector<string> attrDecisionNames;         // The names of our attributes and decision

vector<caseList::entry*>* singleCase;

// Remove the space and vonvert the string to lower case; This allows us to better handle slight variation in the input file
string removeBrackets(string s) {
  s = regex_replace(s, regex("\\[|\\]"), "");
  return s;
}

// Print the current state of the table
void printDataSet() {
  // Print the names of the attributes and decision
  for (string i : attrDecisionNames) {
    cout << i + "\t";
  }
  cout << endl;

  // Print the values in the dataset
  for (vector<caseList::entry*> i : dataSet) {
    for (caseList::entry* j : i) {
      cout << j->value + "\t";
      // cout << j->value + " "; cout << (j->symbolic ? 1 : 0); cout <<  "\t";  // can check for symbolic with this print
    }
    cout << endl;
  }
}

// Check if a string is a double
// Reference: http://stackoverflow.com/questions/5932391/determining-if-a-string-is-a-double
bool isOnlyDouble(const char* str)
{
    char* endptr = 0;
    strtod(str, &endptr);

    if(*endptr != '\0' || endptr == str)
        return false;
    return true;
}

// Read the file in
void readFile(istream& stream) {
  caseList initCList;
  string finLine;
  char comptemp;
  int first = true;
  string temp;


  // Get the final states
  stringstream ss;
  while (stream >> temp) {
    comptemp = temp[0];
    // comptemp = (stream >> ws).peek();
    getline(stream, finLine);
    finLine = temp + finLine;
    // cout << "firstvale: "; cout << comptemp; cout << " " + finLine << endl;

    // Only read content that we care about; this ignores the first line and any comments
    if (comptemp != '<' && comptemp != '!') {
      finLine = removeBrackets(finLine);
      ss.clear();
      ss.str(finLine);

      // Then read in the attribute and decision names
      if (first) {
        while (ss >> temp) {
          attrDecisionNames.push_back(temp);
        }
        first = !first;
      }

      // Then read in the values from the table and store them in "dataSet"
      else {
        singleCase =  new vector<caseList::entry*>();
        while (ss >> temp) {
          singleCase->push_back(initCList.generateEntry(temp, !isOnlyDouble(temp.c_str())));
        }
        dataSet.push_back(*singleCase);
      }
    }
  }

	// Make all dataSet symbolic values for decisions symbolic
	int lastIndex = dataSet[0].size() - 1;
	for (unsigned i = 0; i < dataSet.size(); i++) {
		dataSet[i][lastIndex]->symbolic = true;
	}

	// TODO : FOR MORE INFO
  // Lets see what the dataset looks like
  //printDataSet();
}

// Prints the provided singlepair set
void printSinglePairs(vector<avPairs::singlePair> inputVec) {
  string tempCases = "";

  // Display all the contents of the av.allPairs
  for (auto avBlock : inputVec) {
    tempCases = "";
    cout << "[(" + avBlock.attribute + ", " + avBlock.value + ")]= {";

    // Print all the cases that are part of this av set
    for (auto caseNum : avBlock.caseSet) {
      tempCases += to_string(caseNum) + ",";
    }
    if (tempCases != "")
      tempCases = tempCases.substr(0, tempCases.length()-1);

    cout << tempCases + "}" << endl;
    // printf("[(%s, %.2f)]= {%s}\n", avBlock.attribute.c_str(), atof(avBlock.value.c_str()), tempCases.c_str());
  }
}

// Check if the string provided contains spaces
bool containsSpaces(string inputString) {
	size_t found = inputString.find(" ");
  if (found!=string::npos)
    return true;

	return false;
}

// Check if a file exists for us to take input from
bool fileCheck (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

// Main function
int main(int argc, char* argv[]) {
	// Bool for possible vs certain rules : true -> certain, false -> possible
	bool certPossible;
	string inputFilename;
	string outputFilename;

	// Ask the user for the necessary information
	cout << "What is the name of the input file (ex. input.txt): ";
	string response;
	getline(cin, response);
	// Keep asking until they give a valid file
	while (!fileCheck(response)) {
		cout << "That file cannot be found." << endl;
		cout << "Please give the name of an actual input file (ex. input.txt): ";
		getline(cin, response);
	}
	inputFilename = response;

	// Ask the User if they would like certain or possible rules
	cout << "Would you like to compute certain or possible rules."  << endl;
	cout << "Type 1 for certain and 0 for possible: ";
	getline(cin, response);
	while(response != "0" && response != "1") {
		cout << "You did not provide a valid response." << endl;
		cout <<  "Type 1 for certain and 0 for possible: ";
		getline(cin, response);
	}
	if (response == "0")
		certPossible = false;
	else
		certPossible = true;

	// Ask the user for the name of the output file
	cout << "Enter the name of the output text file.  It may not contain spaces and the \".txt\" extension will be appended for you (ex. output): ";
	getline(cin, response);
	while (containsSpaces(response)) {
		cout << "The response you gave contained spaces." << endl;
		cout << "Enter the name of the output text file.  It may not contain spaces and the \".txt\" extension will be appended for you (ex. output): ";
		getline(cin, response);
	}
	outputFilename = response;

	// Open the file
	ifstream myfile (inputFilename);

	// Read the input
  readFile(myfile);
	myfile.close();
  avPairs av(dataSet, attrDecisionNames);

	// TODO : FOR MORE INFO
	// Print all the attribute value pairs
	// cout << "--- Attrubute-Value Blocks ---" << endl;
  // printSinglePairs(av.allPairs);
	// cout << "--- Concept Blocks ---" << endl;
  // printSinglePairs(av.decisionSets);
	av.setApproximationValues();

	// Execute lem2 with the attribute value blocks and upper and lower Approximations
	lem2Calc lem2(av, certPossible);

	// Only show either the certain or possible ruleset
	// --- Certain ---
	if (certPossible) {
		// TODO : FOR MORE INFO
		// cout << "--- Tentative Certain Ruleset ---" << endl;
		// lem2.printRuleSet(lem2.ruleSetLower, cout);
		lem2.ruleMinimization(true);
		lem2.rulesetMinimization(true);
		cout << "--- Final Certain Ruleset ---" << endl;
		lem2.printRuleSet(lem2.ruleSetLower, cout);

		ofstream outputFileStream (outputFilename + ".txt");
		lem2.printRuleSet(lem2.ruleSetLower, outputFileStream);
	}
	// --- Possible ---
	else {
		// TODO : FOR MORE INFO
		// cout << "--- Tentative Possible Ruleset ---" << endl;
		// lem2.printRuleSet(lem2.ruleSetUpper, cout);
		lem2.ruleMinimization(false);
		lem2.rulesetMinimization(false);
		cout << "--- Final Possible Ruleset ---" << endl;
		lem2.printRuleSet(lem2.ruleSetUpper, cout);

		ofstream outputFileStream (outputFilename + ".txt");
		lem2.printRuleSet(lem2.ruleSetUpper, outputFileStream);
	}

	cout << "The final ruleset was also written to the output file specified!" << endl;
	cout << "Cheers!!!" << endl;

  return 0;
}
