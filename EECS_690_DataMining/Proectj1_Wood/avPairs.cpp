// avPairs.cpp
#include "avPairs.h"

using namespace std;

vector<string> tempValues;   // Set for passing symbolic values
set<double> numValues;    // Set to store numeric values before putting them through
// numericToSymbolic and placing the return value in tempValues

// Only insert if new and retain order
void avPairs::vectorUniqueInsert(vector<string> &inputVec, string inputVal) {
  for ( auto i : inputVec) {
    if (i == inputVal)
      return;
  }
  inputVec.push_back(inputVal);
}

// Returns a vector containing the first and second double from a range symbolic
vector<double> avPairs::splitSymbolicRange(string symbolicRep) {
  vector<double> retTemp;

  size_t firstLength = symbolicRep.find("..");
  size_t secondStart = firstLength + 2;
  retTemp.push_back(stod(symbolicRep.substr(0, firstLength)));
  retTemp.push_back(stod(symbolicRep.substr(secondStart)));

  return retTemp;
}

// Takes a range of values and checks if the value is in it
// This is used for check value inclusion for numeric to symbolic values
// Should check if numericRep is largest or smallest of symbolicRep
bool avPairs::valIncluded(string symbolicRep, double numericRep) {
  vector<double> parsedSymbolic = splitSymbolicRange(symbolicRep);
  return (parsedSymbolic[0] <= numericRep && numericRep <= parsedSymbolic[1]);
}

// This will create the list of all avPairs
avPairs::avPairs(vector<vector<caseList::entry*>> dataSet, vector<string> attrDecisionNames) {
  // Are we converting the given attribute values from numeric
  bool fromNumeric = false;

  // Create a universal set for use in establishing characteristic sets
  for (unsigned i = 0; i < dataSet.size(); i++) {
    universe.insert(i);
  }

  // TODO : NOT go crazy due to printout that you forgot to get rid of! ;)
  // Print the names of the attributes and decision
  // for (string i : attrDecisionNames) {
  //   cout << i + "\t";
  // }
  // cout << endl;

  // i i i i i i i i
  // j
  // j
  // j
  // j

  // For all attributes
  for (int i = (int) dataSet[0].size() - 1; i >= 0 ; i--) {
    tempValues.clear();
    numValues.clear();
    // For all cases
    for (unsigned j = 0; j < dataSet.size(); j++) {
      // The value we are dealing with is symbolic
      if (dataSet[j][i]->symbolic) {
        vectorUniqueInsert(tempValues, dataSet[j][i]->value);
      }
      // The value is numeric so store it inorder to later convert
      else {
        numValues.insert(atof(dataSet[j][i]->value.c_str()));
      }
    }

    fromNumeric = (numValues.size() != 0);

    // Convert the numeric values to symbolic and put them in tempValues
    if (fromNumeric)
      numericToSymbolic();

    for (auto t : tempValues) {
			// If the value isn't missing
			if (t != "*" && t != "?" && t != "-") {
				// Create a new single av block and add it to allPairs
				struct singlePair* newPair = new struct singlePair;
				newPair->attribute = attrDecisionNames[i];
				newPair->value = t;
				newPair->fromNumeric = fromNumeric;
				newPair->caseSet = makeAVBlock(dataSet, i, t, fromNumeric);

				// Either av pair of decision pair
				if (i == (int) attrDecisionNames.size() - 1)
					decisionSets.push_back(*newPair);
				else
					allPairs.push_back(*newPair);
			}
    } // end for : t
  }

  // This is for adding new sets
  set<int> tempKIntersect;    // Used to keep track of intersection as we do it;
  set<int> returnedSet;
  set<int> intersectionResult;
  string currentValue;
  set<int>::iterator it;
	int characteristicCount = 0;

	// TODO : FOR MORE INFO
	cout << "Characteristic sets formed: " + to_string(characteristicCount) << flush;

  // Here we will build up the characteristic sets now that we have the av sets; put everything in kSets
  for (unsigned i = 0; i < dataSet.size(); i++) {
    // TODO: compute for numeric and missing; this is just for symbolic
    tempKIntersect.clear();
    tempKIntersect = universe;

    // Make sure not to take the decision value from a case
    for (unsigned j = 0 ; j < dataSet[i].size() - 1; j++) {
      currentValue = dataSet[i][j]->value;
      returnedSet.clear();
      intersectionResult.clear();

      // For missing value cases
      if (currentValue == "*" || currentValue == "?") {/* Ignore since we just intersect with universe*/}
      else if (currentValue == "-") {
				set<string> conceptMissingVals;		// --- These are the defined value for that attribute in the decision ---
				string decisionValue;
        // Iterate over all values in the decisionSet to find all possible values
				// Make a set of distinct values then find those values union them together then intersect with the rest
				// cout << dataSet[i][dataSet[i].size() - 1]->value << endl; // TODO : REMOVE this is for making sure that we grab the right decision to look for values
				set<int> decisionSetResult = getDecisionSet(dataSet[i][dataSet[i].size() - 1]->value);
				for (auto currCase : decisionSetResult) {
					decisionValue = dataSet[currCase][j]->value;
					// If we have a definite value still within the concept then add it to conceptMissingVals
					if (decisionValue != "*" && decisionValue != "-" && decisionValue != "?") {
						conceptMissingVals.insert(decisionValue);

					}
				}

				// Now add our starting current case to the set with all definite cases
				set<int> tempRetrievedSet;
				returnedSet.insert(i);
				for (auto currDefValue : conceptMissingVals) {
					tempRetrievedSet.clear();
					tempRetrievedSet = retrieveAVSets(dataSet, attrDecisionNames[j], j, currDefValue, dataSet[i][j]->symbolic);
					returnedSet.insert(tempRetrievedSet.begin(), tempRetrievedSet.end());
				}
				// Now the returnedSet is ready for intersection
				set_intersection(tempKIntersect.begin(), tempKIntersect.end(), returnedSet.begin(), returnedSet.end(), inserter(intersectionResult, intersectionResult.begin()));
				tempKIntersect = intersectionResult;
      }
      // For known value cases; handles both symbolic and numeric; just treat numeric as if it were symbolic
      else {
        returnedSet = retrieveAVSets(dataSet, attrDecisionNames[j], j, currentValue, dataSet[i][j]->symbolic);
        set_intersection(tempKIntersect.begin(), tempKIntersect.end(), returnedSet.begin(), returnedSet.end(), inserter(intersectionResult, intersectionResult.begin()));
        // intersectionResult.resize(it - intersectionResult.begin());
        tempKIntersect = intersectionResult;
      }
    }

		// TODO : FOR MORE INFO
    // print the values before storing them
    // cout << "Ka(" + to_string(i) + ")= {";
    // for (set<int>::iterator it = tempKIntersect.begin(); it != tempKIntersect.end(); it++) {
    //   cout << to_string(*it);
		// 	if (it != prev(tempKIntersect.end(), 1)) {
		// 		cout << ",";
		// 	}
    // }
    // cout << "}" << endl;
    kSets.push_back(tempKIntersect);

		// TODO : FOR MORE SORTA INFO
		characteristicCount++;
		cout << '\r' << "Characteristic sets formed: " + to_string(characteristicCount) << flush;

  }

	// Make sure to add a newline
	cout << endl;
}

// Return the set for the decision provided
set<int> avPairs::getDecisionSet(string decisionValue) {
	for (auto currSet : decisionSets) {
		if (currSet.value == decisionValue) {
			return currSet.caseSet;
		}
	}
	set<int> empty;
	return empty;
}

// Send back the set containing cases for characteristic sets
set<int> avPairs::retrieveAVSets(vector<vector<caseList::entry*>> dataSet, string attributeName, int attributeIndex, string value, bool symbolic) {
	set<int> retSet;

	// For symbolic values
	if (symbolic) {
		for (auto avSingle : allPairs) {
			if (avSingle.attribute == attributeName && avSingle.value == value)
				return avSingle.caseSet;
		}
	}
	// For Numeric values
	else {
		// For all cases check to see if the numeric value matches and if so then add it
		for (unsigned j = 0; j < dataSet.size(); j++) {
			if (dataSet[j][attributeIndex]->value == value) {
				retSet.insert(j);
			}
		}
	}
	return retSet;
}

avPairs::~avPairs(){};

// Used by the constructor to get symbolic values from numeric; place two values on tempValues
void avPairs::numericToSymbolic() {
  // These are the first and last values in the sorted set
  double smallest = (*numValues.begin());
  double largest = (*prev(numValues.end(), 1));

  double currVal, nextVal, cutVal;

  // Find the cut point for all values, and add range to beginning and range to end to tempValues
  for (set<double>::iterator it = numValues.begin() ; it != prev(numValues.end(), 1); it++) {
    currVal = *it;
    nextVal = *next(it, 1);
    cutVal = (currVal + nextVal) / 2;

    vectorUniqueInsert(tempValues, to_string(smallest) + ".." + to_string(cutVal));
    vectorUniqueInsert(tempValues, to_string(cutVal) + ".." + to_string(largest));
  }
}

// Determine the attribute value blocks
set<int> avPairs::makeAVBlock(vector<vector<caseList::entry*>> dataSet, unsigned attributeIndex, string value, bool fromNumeric) {
  set<int> retval;
	string decisionValue;

  // Iterate through all a single attribute value for all cases
  for (unsigned j = 0; j < dataSet.size(); j++) {

		// Handle missing characters
		if (dataSet[j][attributeIndex]->value == "?") {
			/* Do nothing */
		}
		else if (dataSet[j][attributeIndex]->value == "*") {
			retval.insert((int) j);
		}
		else if (dataSet[j][attributeIndex]->value == "-") {
			// TODO :  logic for concept values

			// Iterate over all values in the decisionSet to find all possible values
			// Make a set of distinct values then find those values union them together then intersect with the rest
			// cout << dataSet[i][dataSet[i].size() - 1]->value << endl; // TODO : REMOVE this is for making sure that we grab the right decision to look for values
			set<int> decisionSetResult = getDecisionSet(dataSet[j][dataSet[j].size() - 1]->value);

			for (auto currCase : decisionSetResult) {
				decisionValue = dataSet[currCase][attributeIndex]->value;
				// If we have a definite value still within the concept then add it to conceptMissingVals
				if (decisionValue != "*" && decisionValue != "-" && decisionValue != "?") {
					// If the value is a concept-value then add it to the av block; insert j if one of the values is the specified value
					// // Check if the numeric value in the dataSet matches the range from parameter "value"
					if (fromNumeric) {
						if (valIncluded(value, stod(decisionValue))) {
							retval.insert((int) j);
						}
					}
					// Value is flat symbolic and thus just compare the string
					else {
						if (decisionValue == value) {
							retval.insert((int) j);
						}
					}

				} // end if definite value not *,-, or ?
			} // end for all value in the decisionset
		}
		// Regular values
		else {
			// // Check if the numeric value in the dataSet matches the range from parameter "value"
			if (fromNumeric) {
				if (valIncluded(value, stod(dataSet[j][attributeIndex]->value))) {
					retval.insert((int) j);
				}
			}
			// Value is flat symbolic and thus just compare the string
			else {
				if (dataSet[j][attributeIndex]->value == value) {
					retval.insert((int) j);
				}
			}
		}
  }   // end for

  return retval;
}

// Set the lower and upper approximation sets
void avPairs::setApproximationValues() {
	// Different decisionSets
	for (auto currDecisionSet : decisionSets) {
		// The values from each decision
		struct conceptApproximation* newLower = new struct conceptApproximation;
		struct conceptApproximation* newUpper = new struct conceptApproximation;

		newLower->value = currDecisionSet.value;
		newUpper->value = currDecisionSet.value;


		// --- Lower approximation ---
		for (auto decisionSetVal : currDecisionSet.caseSet) {
			// If all values in the decision set are within the characteristic set then include the characteristic set
			if (includes(currDecisionSet.caseSet.begin(), currDecisionSet.caseSet.end(), kSets[decisionSetVal].begin(), kSets[decisionSetVal].end())) {
				newLower->approximationCases.insert(kSets[decisionSetVal].begin(), kSets[decisionSetVal].end());
			}

			// -- Upper Approximation ---
			newUpper->approximationCases.insert(kSets[decisionSetVal].begin(), kSets[decisionSetVal].end());
		}

		// Add the newly created approxitmation to the correct lower and upper approximation sets
		if (newLower->approximationCases.size() != 0) {
			lowerApproximations.push_back(*newLower);
		}
		if (newUpper->approximationCases.size() != 0) {
			upperApproximations.push_back(*newUpper);
		}
	}

	// TODO : FOR MORE INFO
	// --- Print Lower ---
	// cout << "--- Lower Approximation: ---" << endl;
	// for (auto singleApproximation : lowerApproximations) {
	// 	// Print the values in the added Lower approcimation
	// 	if (singleApproximation.approximationCases.size() > 0) {
	// 		cout << singleApproximation.value + " : ";
	// 		for (auto caseVal : singleApproximation.approximationCases) {
	// 			cout << to_string(caseVal) + ",";
	// 		}
	// 		cout << endl;
	// 	}
	// }

	// TODO : FOR MORE INFO
	// --- Print Upper ---
	// cout << "--- Upper Approximation: ---" << endl;
	// for (auto singleApproximation : upperApproximations) {
	// 	// Print the values in the added Lower approcimation
	// 	if (singleApproximation.approximationCases.size() > 0) {
	// 		cout << singleApproximation.value + " : ";
	// 		for (auto caseVal : singleApproximation.approximationCases) {
	// 			cout << to_string(caseVal) + ",";
	// 		}
	// 		cout << endl;
	// 	}
	// }
}
