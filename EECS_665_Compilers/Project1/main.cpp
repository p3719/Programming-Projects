/*
*   Name: Philip Wood
*   Class: EECS 665
*   Assignment: Project 1
*   Description; Convert given NFA to DFA
*/

#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>
#include <regex>
#include <sstream>
#include <queue>

using namespace std;
vector<vector<vector<int>>> stateMatrix;
vector<vector<int>> backTrace;
queue<vector<int>> que;
int e_index = 0;


// Remove the space and vonvert the string to lower case; This allows us to better handle slight variation in the input file
string happyConversion(string s) {
  s = regex_replace(s, regex("\\s+"), "");
  transform(s.begin(), s.end(), s.begin(), ::tolower);
  return s;
}

// Check backTrace to make sure that we haven't already seen the set
// if it is then return the number; else return -1
int inBackTrace(vector<int> input) {
  bool check = true;
  for (unsigned i = 0; i < backTrace.size(); i++ ) {
    check = true;
    if (input.size() == backTrace[i].size()) {
      // Check make sure that all values of the vectors are the same
      for (unsigned j = 0; j < input.size(); j++ ) {
        if (input[j] != backTrace[i][j])
          check = false;
      }
      // If all values are the same then return where we are in the backTrace
      if (check) {
        return i + 1;
      }
    }
  }
  return -1;
}

// Given a sets of initial NFA states, return a set with all the new DFA states
// This is used to find the new initial and final sets
vector<int> findNewSet(vector<int> input) {
  vector<int> ans;
  bool flag = true;
  for (unsigned i = 0; i < backTrace.size(); i++ ) {
    flag = true;
    // Check make sure that all values of the vectors are the same
    for (unsigned j = 0; j < backTrace[i].size() && flag; j++ ) {
      // Check each input value to see if it is in the backTrace
      for (unsigned k = 0 ; k < input.size() && flag; k++) {
        if (input[k] == backTrace[i][j]) {
          ans.push_back(i+1);
          flag = false;
        }
      }
    }
  }

  return ans;
}

// Print the provided vector as a comma seperated list inside brackets
string printVec(vector<int> input) {
  string ans = "{";
  for (unsigned i = 0 ; i < input.size(); i ++) {
    ans += to_string(input[i]);
    if (i != input.size()-1) {
      ans += ",";
    }
  }
  ans += "}";
  return ans;
}

// Complete the E-closure of the provided vector and returns the result; Passed by value
vector<int> e_closure(vector<int> input) {
  bool first = true;
  vector<int> ans, pre;
  int currState;

  // Add the current state and any e-transition states
  do {
    pre = first ? input : ans;
    first = false;
    ans.clear();
    for(unsigned i = 0; i < pre.size(); i++) {
      currState = pre[i];
      ans.push_back(currState);
      for(unsigned j = 0; j < stateMatrix[currState][e_index].size(); j++) {
        ans.push_back(stateMatrix[currState][e_index][j]);
      }
    }
    sort( ans.begin(), ans.end() );                               // Sort the vector
    ans.erase( unique( ans.begin(), ans.end() ), ans.end() );     // Remove duplicates
    // cout << "Comparison: pre-" << printVec(pre) << " and post-" << printVec(ans) << endl;
  }
  while (pre != ans);   // While is to ensure that we get e transitions more than 1 in length
  return ans;
}

// Complete a transition
vector<int> transition(vector<int> input, int tran_val) {
  vector<int> ans;
  int currState;

  // Add the current state and any transition states for the given kind of transition
  for(unsigned i = 0; i < input.size(); i++) {
    currState = input[i];
    for(unsigned j = 0; j < stateMatrix[currState][tran_val].size(); j++) {
      ans.push_back(stateMatrix[currState][tran_val][j]);
    }
  }
  sort( ans.begin(), ans.end() );                               // Sort the vector
  ans.erase( unique( ans.begin(), ans.end() ), ans.end() );     // Remove duplicates
  return ans;
}

// Main logic of our code
int main(int argc, char* argv[]) {
  vector<string> transitions;
  vector<int> finStates;
  vector<int> startStates;

  int startState, endState, temp_i;
  string initLine, finLine, totLine;

  int queWriteCount = 0;
  int queReadCount = 0;

  // Get the Initial State
  getline(cin, initLine);
  initLine = happyConversion(initLine);
  startState = stoi(initLine.substr(initLine.find('{') + 1, initLine.find('}') - initLine.find('{') - 1));
  startStates.push_back(startState);

  // Get the final states
  getline(cin, finLine);
  finLine = happyConversion(finLine);
  finLine = finLine.substr(finLine.find('{') + 1, finLine.find('}') - finLine.find('{') - 1);
  stringstream ss (finLine);
  while (ss >> temp_i) {
    finStates.push_back(temp_i);

    if (ss.peek() == ',')
      ss.ignore();
  }

  //Get the total number of states
  getline(cin, totLine);
  endState = stoi(totLine.substr(totLine.find(':') + 1));

  // Grab the state transitions
  string temp_s;
  cin >> temp_s >> temp_s;
  while (temp_s != "E") {
    transitions.push_back(temp_s);
    cin >> temp_s;
  }
  transitions.push_back(temp_s);
  e_index = transitions.size() -1;

  // Declare the two dimensional vector that will contain all of the transition values
  vector<int> temp, temp_result, temp_result2;
  stateMatrix.resize(endState + 1);
  for (int state = 0 ; state <= endState; state++) {
    stateMatrix[state].resize(transitions.size());

    // Have an empty zero state inorder to more easily call states later on
    if (state != 0) {
      cin >> temp_s;  // ignore the state
      for (unsigned col = 0; col < transitions.size(); col++ ) {
        temp.clear();
        cin >> temp_s;
        ss.clear();
        ss.str(temp_s.substr(1, temp_s.length()-2));
        while (ss >> temp_i) {
          stateMatrix[state][col].push_back(temp_i);
          if (ss.peek() == ',')
            ss.ignore();
        }
      }
    }
  }

  // Test
  temp = e_closure(startStates);
  cout << "E-closure(I0) = " << printVec(temp).c_str();
  que.push(temp);
  backTrace.push_back(temp);
  queWriteCount++;
  cout << " = " << queWriteCount << endl;

  // Now loop until the que is empty
  while (!que.empty()) {
    temp = que.front();
    que.pop();
    queReadCount++;
    cout << endl << "Mark " << queReadCount << endl;

    // Run through each transition and display the output if the size isn't zero
    for (unsigned i = 0; i < transitions.size()-1; i++ ) {
      temp_result = transition(temp, i);
      if (temp_result.size() != 0) {
        printf("%s --%s--> %s\n", printVec(temp).c_str(), transitions[i].c_str(), printVec(temp_result).c_str());
        temp_result2 = e_closure(temp_result);

        // Check to see if we have already found the resulting state
        if ((temp_i = inBackTrace(temp_result2)) == -1) {
          que.push(temp_result2);
          backTrace.push_back(temp_result2);
          queWriteCount++;
          printf("E-closure%s = %s = %d\n", printVec(temp_result).c_str(), printVec(temp_result2).c_str(), queWriteCount );
        } else {
          printf("E-closure%s = %s = %d\n", printVec(temp_result).c_str(), printVec(temp_result2).c_str(), temp_i );
        }
      }
    }
  }

  /* This is just to check the value that end up in the backTrace
  for (unsigned i = 0; i < backTrace.size(); i ++ ) {
    cout << printVec(backTrace[i]).c_str() << endl;
  }*/

  // Print the top of the new table
  cout << endl;
  printf("Initial State: {1}\n");
  printf("Final States: %s\n", printVec(findNewSet(finStates)).c_str());
  cout << "State" << "\t";
  for (int i = 0; i < e_index ; i++) {
    cout << transitions[i] << "\t";
  }
  cout << endl;

  // Print the new state table
  for (unsigned i = 0; i < backTrace.size(); i++ ) {
    cout << i+1 << "\t";
    // Find and print the transition
    for (int j = 0; j < e_index; j++ ) {
      temp_i = inBackTrace(e_closure(transition(backTrace[i], j)));
      if (temp_i == -1) {
        cout << "{}";
      } else {
        cout << "{" << temp_i << "}";
      }
      if (i != backTrace.size()-1 || j != e_index-1)
        cout << "\t";
    }
    if (i != backTrace.size()-1)
      cout << endl;
  }

  return 0;
}
