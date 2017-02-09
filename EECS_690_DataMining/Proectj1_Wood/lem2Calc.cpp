// lem2Calc.cpp
#include "lem2Calc.h"

using namespace std;

// Constructor for the class; it should find all the final rules
lem2Calc::lem2Calc(avPairs inputValues, bool lower) {
	if (lower)
		determineTentativeRuleset(inputValues, true);
	else
		determineTentativeRuleset(inputValues, false);

}

//Destructor for the class
lem2Calc::~lem2Calc(){}

// Print out the give ruleset
void lem2Calc::printRuleSet (vector<vector<avPairs::singlePair>> inputRuleSet, ostream& stream) {
	int count = 0;

	avPairs::singlePair singleBlock;
	// Pull out a rule
	for (auto singleRule : inputRuleSet) {
		count ++;
		// Pull out each attribute for printing one at a time
		// Keep in mind that the last attribute is the decision value
		for (unsigned index = 0; index < (singleRule.size() - 1); index++) {
			singleBlock = singleRule[index];
			// AND together all rules
			if (index != 0) {
				stream << " & ";
			}
			stream << "(" + singleBlock.attribute + "," + singleBlock.value + ")";
		}

		// Add the decision onto the end
		singleBlock = singleRule[singleRule.size() - 1];
		stream << " -> (" << singleBlock.attribute + "," + singleBlock.value + ")" << endl;
	}
	stream << "Final RuleSet Rule Count: " + to_string(count) << endl;
}

// Check if the tentative rule already contains the av block in questions
bool lem2Calc::alreadyInUse (vector<avPairs::singlePair> tentativeRule, avPairs::singlePair inputPair) {
	// Check all values in the tentative rule
	for (auto single : tentativeRule) {
		if (single.attribute == inputPair.attribute && single.value == inputPair.value)
			return true;
	}

	// Not used yet
	return false;
}

// Set either the upper or lower approximation; true for lower ; false for upper
void lem2Calc::determineTentativeRuleset(avPairs inputValues, bool lower) {
	// Extract key values
	vector<avPairs::conceptApproximation> lowerApproximations = inputValues.lowerApproximations;
	vector<avPairs::conceptApproximation> upperApproximations = inputValues.upperApproximations;
	vector<avPairs::singlePair> allPairs = inputValues.allPairs;
	vector<avPairs::singlePair> tempAllPairs = allPairs;		// This is used for eliminating unneeded values from allPairs for a certain rule

	unsigned avSize = 0;								// The size of the av pair that helps make the best intersect

	set<int> accumulatorSet;			// The set that we check is a subset of column top to see if we should continue
	set<int> tempDecisionSet;			// The value at the top of the column
	set<int> originaDecisionSet;	// DON'T MAKE THE SAME MISTAKE
	set<int> tempIntersect;				// The intersect that we calculate in the table
	set<int> tempIntersect2;			// The second intersect temporary
	set<int> tempRuleAddition;		// The current best intersect
	set<int> tempAVBlock;					// The is the set of the current AV Block
	avPairs::singlePair currentValue; 	// The block that we will add to our tentative rule (singleRuleVec)

	// Just for showing progress
	//int count = (lower ? lowerApproximations.size() : upperApproximations.size());
	int tentativeRuleCount = 0;

	cout << "Tentative Rules Generated: " + to_string(tentativeRuleCount) << flush;

	// --- Lower Approximations ---
	// Go through each lower approximation and
	for (auto currVal : (lower ? lowerApproximations : upperApproximations)) {
		// This will house the rule we are making for this lower approximation
		vector<avPairs::singlePair> singleRuleVec;														// This will be added to the ruleset when done
		tempDecisionSet = currVal.approximationCases;
		accumulatorSet.clear();

		// Get the original value here
		originaDecisionSet = tempDecisionSet;

		//TODO : remove this
		// cout << "Concepts left to make rules for: " + to_string(count) << endl;
		// count--;
		tempAllPairs = allPairs;
		// TODO : don't forget to set tempAllPairs to allPairs when we finish up a tentative rule;


		// inside the while
		while (tempDecisionSet.size() > 0) {
			// cout << to_string(tempDecisionSet.size()) << endl;
			// Zero out these temporary values
			tempRuleAddition.clear();
			avSize = 0;
			// currentValue.clear(); // may need something to zero the current value


			// For all attribute value blocks pick the best
			for (auto currAVBlock : tempAllPairs) {
				// Ignore this avblock if its already in the tentative rule
			//	if (!alreadyInUse(singleRuleVec, currAVBlock)) {
				// *** tempAllPairs should already be reduced ***
				tempIntersect.clear();
				tempAVBlock = currAVBlock.caseSet;
				// If this is the first intersect that we calculate then call it the best for now
				if (tempRuleAddition.size() == 0) {
					set_intersection(tempDecisionSet.begin(), tempDecisionSet.end(), tempAVBlock.begin(), tempAVBlock.end(), inserter(tempRuleAddition, tempRuleAddition.begin()));
					// Intersect with the accumulatorSet as well is we have one
					if (!accumulatorSet.empty()) {
						tempIntersect.clear();
						set_intersection(accumulatorSet.begin(), accumulatorSet.end(), tempRuleAddition.begin(), tempRuleAddition.end(), inserter(tempIntersect, tempIntersect.begin()));
						tempRuleAddition = tempIntersect;
					}
					avSize = tempAVBlock.size();
					currentValue.caseSet = tempAVBlock;
					currentValue.attribute = currAVBlock.attribute;
					currentValue.value = currAVBlock.value;
					currentValue.fromNumeric = currAVBlock.fromNumeric;
					// // TODO : REMOVE This
					// cout << "intersect: ";
					// for (auto singleVal : tempRuleAddition) {
					// 	cout << to_string(singleVal) + ",";
					// }
					// cout << endl;

				}
				// Then this isn't the first intersect that we've calculated
				else {
					// Calculate a temporary intersect to compare against the current best
					tempIntersect.clear();
					set_intersection(tempDecisionSet.begin(), tempDecisionSet.end(), tempAVBlock.begin(), tempAVBlock.end(), inserter(tempIntersect, tempIntersect.begin()));
					// Intersect with the accumulatorSet as well is we have one
					if (!accumulatorSet.empty()) {
						tempIntersect2.clear();
						set_intersection(accumulatorSet.begin(), accumulatorSet.end(), tempIntersect.begin(), tempIntersect.end(), inserter(tempIntersect2, tempIntersect2.begin()));
						tempIntersect = tempIntersect2;
					}

					// // TODO : REMOVE This
					// cout << "intersect: ";
					// for (auto singleVal : tempIntersect) {
					// 	cout << to_string(singleVal) + ",";
					// }
					// cout << endl;

					//	The new intersect that we calculated is better than our current best
					if (tempIntersect.size() > tempRuleAddition.size()) {
						tempRuleAddition = tempIntersect;
						avSize = tempAVBlock.size();
						currentValue.caseSet = tempAVBlock;
						currentValue.attribute = currAVBlock.attribute;
						currentValue.value = currAVBlock.value;
						currentValue.fromNumeric = currAVBlock.fromNumeric;
					}
					// If they are the same size then we must check the size of the av blocks
					else if (tempIntersect.size() == tempRuleAddition.size()) {
						if (tempAVBlock.size() < avSize) {
							tempRuleAddition = tempIntersect;
							avSize = tempAVBlock.size();
							currentValue.caseSet = tempAVBlock;
							currentValue.attribute = currAVBlock.attribute;
							currentValue.value = currAVBlock.value;
							currentValue.fromNumeric = currAVBlock.fromNumeric;
						}
					}
				}
			//	}	// end if : already in use
			} // end for : all pairs

			// Add this av block to the current tentative rule we are creating; fromNumeric doesn't matter at this point
			// TODO : CHECK IT OUT // currentValue.caseSet = tempRuleAddition;

			// TODO : REMOVE THIS lets print the av pair that I am adding to the rule
			//cout << endl << endl << currentValue.attribute + " " + currentValue.value << endl << endl << endl;
			// if (tempRuleAddition.empty()) {
			// 	cout << endl << "!!!!!!!!!!!!!!" << endl;
			// 	cout << "The av block chosen has an empty intersect" << endl;
			// }

			// Add this av block (currentValue) to our tentaive rule (singleRuleVec)
			singleRuleVec.push_back(currentValue);

			// Everytime we add an attribute value to a tentative rule remove unnecessary values
			removeUnnecessaryAVBlocks(tempAllPairs, currentValue);

			// First av block selected for this particular colum
			if (accumulatorSet.empty()) {
				accumulatorSet = currentValue.caseSet;
			}
			// Not the first av block selected
			else {
				tempIntersect.clear();
				set_intersection(accumulatorSet.begin(), accumulatorSet.end(), currentValue.caseSet.begin(), currentValue.caseSet.end(), inserter(tempIntersect, tempIntersect.begin()));
				accumulatorSet = tempIntersect;		// Use intersect as proxy
			}


			// // TODO: REMOVE
			// cout << "vals before removal top of col " + to_string(tempDecisionSet.size()) + ": ";
			// for (set<int>::iterator it = tempDecisionSet.begin() ; it != tempDecisionSet.end(); it++) {
			// 	cout << to_string(*it) +  ",";
			// }
			// cout << endl;
			//
			// // TODO: REMOVE
			// cout << "vals removed " + to_string(tempRuleAddition.size()) + ": ";
			// for (set<int>::iterator it = tempRuleAddition.begin() ; it != tempRuleAddition.end(); it++) {
			// 	cout << to_string(*it) +  ",";
			// }
			// cout << endl;
			//
			// // TODO: REMOVE
			// cout << "accumulator " + to_string(accumulatorSet.size()) + ": ";
			// for (set<int>::iterator it = accumulatorSet.begin() ; it != accumulatorSet.end(); it++) {
			// 	cout << to_string(*it) +  ",";
			// }
			// cout << endl;


			// Check if accumulator is subset of ORIGINAL column value
			tempIntersect.clear();
			set_difference(accumulatorSet.begin(), accumulatorSet.end(), originaDecisionSet.begin(), originaDecisionSet.end(), inserter(tempIntersect, tempIntersect.begin()));
			if (tempIntersect.empty()) {
				// TODO : remove this; but for now use it to display progress to the user
				tentativeRuleCount++;
				cout << '\r' << "Tentative Rules Generated: " + to_string(tentativeRuleCount) << flush;

				// Now remove values from the top of our column (tempDecisionSet) that were in our final intersect (tempRuleAddition)
				tempIntersect.clear();		// VERY FUCKING IMPORTANT
				set_difference(tempDecisionSet.begin(), tempDecisionSet.end(), tempRuleAddition.begin(), tempRuleAddition.end(), inserter(tempIntersect, tempIntersect.begin()));
				tempDecisionSet = tempIntersect;	// Use tempIntersect as a proxy

				// TODO : REMOVE
				//cout << "Size of decisionSet : " + to_string(tempDecisionSet.size()) << endl << endl;

				// Create the rule, by adding the decision to the end then adding the rule to the ruleset
				avPairs::singlePair decisionPair;
				decisionPair.attribute = inputValues.decisionSets[0].attribute;
				decisionPair.value = currVal.value;
				decisionPair.caseSet = currVal.approximationCases;
				singleRuleVec.push_back(decisionPair);			// *** The last value of each tentative rule is the decision

				// Now add our single tentative rule (singleRuleVec) to the lower/upper rule set (ruleSetUpper)
				if (lower)
					ruleSetLower.push_back(singleRuleVec);
				else
					ruleSetUpper.push_back(singleRuleVec);

				// Clear my single rule vector
				singleRuleVec.clear();

				// Clear the accumulator
				accumulatorSet.clear();

				// Reset the value of the tempAllPairs to allPairs since we finished a tentative rule
				tempAllPairs = allPairs;
			}
			// It isn't a subset and we are still working with the same column head value
			else {
				// We just need to make sure that we don't just select the same value again :)
				// DONE
			}

			// // TODO: REMOVE
			// cout << "current col top " + to_string(tempIntersect.size()) + ": ";
			// for (set<int>::iterator it = tempIntersect.begin() ; it != tempIntersect.end(); it++) {
			// 	cout << to_string(*it) +  ",";
			// }
			// cout << endl;

		}	//end while
	}	// end for : over all lower approximationCase

	// Now that we are done flushing move to the next line
	cout << endl;
}

// Remove any unneeded values from the tempAllPairs set that contains all av pair options
// avSelected is the value that we just selected to add to our tentative rule in question
void lem2Calc::removeUnnecessaryAVBlocks(vector<avPairs::singlePair> &tempAllPairs, avPairs::singlePair avSelected) {
	set<int> tempIntersect;		// The result of the set_intersection
	set<int> tempSetDifference;	// The result of the set_difference

	// Implementation to remove entries while iterating
	vector<avPairs::singlePair>::iterator it = tempAllPairs.begin();
	while (it != tempAllPairs.end()) {
		avPairs::singlePair currTempAVBlock = *it;

		// Check to make sure that the attributes match
		if (currTempAVBlock.attribute == avSelected.attribute) {
			// Check to see if the av block selected is already more specific
			tempSetDifference.clear();
			set_difference(avSelected.caseSet.begin(), avSelected.caseSet.end(), currTempAVBlock.caseSet.begin(), currTempAVBlock.caseSet.end(), inserter(tempSetDifference, tempSetDifference.begin()));

			// Check to see if the intersect is the empty set; if so then remove the values
			tempIntersect.clear();
			set_difference(avSelected.caseSet.begin(), avSelected.caseSet.end(), currTempAVBlock.caseSet.begin(), currTempAVBlock.caseSet.end(), inserter(tempIntersect, tempIntersect.begin()));

			if (tempSetDifference.empty() || tempIntersect.empty()) {
				tempAllPairs.erase(it);
			}
			// Iterate since we didn't remove an entry
			else {
				it++;
			}
		}	// end if : attribute check

		// Otherwise just Iterate
		else {
			it++;
		}

		// TODO: REMOVE OLD CHECK
		// // If it we added a symbolic value then remove any av blocks with the same attribute
		// if (!avSelected.fromNumeric && currTempAVBlock.attribute == avSelected.attribute) {
		// 	tempAllPairs.erase(it);
		// }
		//
		// // If the value is a numeric range then check and see if the value selected is entirely within the tempAllPairs value, if so then remove it
		// else if (avSelected.fromNumeric && currTempAVBlock.attribute == avSelected.attribute) {
		// 	// Check to see if the av block selected is already more specific
		// 	tempSetDifference.clear();
		// 	set_difference(avSelected.caseSet.begin(), avSelected.caseSet.end(), currTempAVBlock.caseSet.begin(), currTempAVBlock.caseSet.end(), inserter(tempSetDifference, tempSetDifference.begin()));
		//
		// 	// Check to see if the intersect is the empty set; if so then remove the values
		// 	tempIntersect.clear();
		// 	set_difference(avSelected.caseSet.begin(), avSelected.caseSet.end(), currTempAVBlock.caseSet.begin(), currTempAVBlock.caseSet.end(), inserter(tempIntersect, tempIntersect.begin()));
		//
		// 	if (tempSetDifference.empty() || tempIntersect.empty()) {
		// 		tempAllPairs.erase(it);
		// 	}
		// 	// Iterate since we didn't remove an entry
		// 	else {
		// 		it++;
		// 	}
		// }
		// // Iterate since we didn't remove an entry
		// else {
		// 	it++;
		// }
	}	// end while : we have gotten through the entire list;
}

// Minimize all rules in the tentative ruleset
void lem2Calc::ruleMinimization(bool lower) {
	// Determine the ruleset that we are using
	vector<vector<avPairs::singlePair>> ruleSet = (lower ? ruleSetLower : ruleSetUpper);

	// For each rule in the ruleset, try to remove values
	for (unsigned i = 0; i < ruleSet.size(); i++) {
		ruleSet[i] = runRule(ruleSet[i]);
	}

	// Set the ruleSetLower or ruleSetUpper value to what we got
	if (lower)
		ruleSetLower = ruleSet;
	else
		ruleSetUpper = ruleSet;
}

// Recursive function for removing rules; rule minimization on a single rule
vector<avPairs::singlePair> lem2Calc::runRule(vector<avPairs::singlePair> inputVec) {
	// Some temp values
	set<int> accumulatorSet;		// For getting the intersect of each remaining rule
	set<int> tempIntersect;			// Used to store the result when getting an intersection
	set<int> tempSetDifference;	// Used to check is a set is a subset of another
	vector<avPairs::singlePair> newStandin;		// Make a new vector under then assumption that we can remove an av pair then test
	int index = 0;

	// If we only have one av pair going to a concept then the rule is already minimized
	if (inputVec.size() == 2)
		return inputVec;

	// If we have multiple av pairs then try removing each of them
	for (vector<avPairs::singlePair>::iterator it = inputVec.begin(); it != prev(inputVec.end(), 1); it++, index++) {
		// Remove the current singePair from inputVec and place in newStandin
		newStandin = inputVec;
		newStandin.erase(newStandin.begin()+index);

		// Sum all the values in the newStandin
		accumulatorSet.clear();
		for (unsigned i = 0; i < newStandin.size() - 1; i++) {
			// If we are looking at the first av pair then return the
			if (i == 0) {
				accumulatorSet = newStandin[0].caseSet;
			}
			// Not the first value so intersect it with accumulatorSet
			else {
				tempIntersect.clear();
				set_intersection(accumulatorSet.begin(), accumulatorSet.end(), newStandin[i].caseSet.begin(), newStandin[i].caseSet.end(), inserter(tempIntersect, tempIntersect.begin()));
				accumulatorSet = tempIntersect;
			}
		}		// end for : newStandin

		// // TODO: REMOVE
		// cout << "vals accumulatorSet " + to_string(accumulatorSet.size()) + ": ";
		// for (set<int>::iterator it = accumulatorSet.begin() ; it != accumulatorSet.end(); it++) {
		// 	cout << to_string(*it) +  ",";
		// }
		// cout << endl;
		//
		// // TODO: REMOVE
		// cout << "vals decisionSet " + to_string(newStandin[newStandin.size() - 1].caseSet.size()) + ": ";
		// for (set<int>::iterator it = newStandin[newStandin.size() - 1].caseSet.begin() ; it != newStandin[newStandin.size() - 1].caseSet.end(); it++) {
		// 	cout << to_string(*it) +  ",";
		// }
		// cout << endl;

		// Find the set difference; with the concept block for that rule
		tempSetDifference.clear();
		set_difference(accumulatorSet.begin(), accumulatorSet.end(), newStandin[newStandin.size() - 1].caseSet.begin(), newStandin[newStandin.size() - 1].caseSet.end(), inserter(tempSetDifference, tempSetDifference.begin()));
		// If it is a subset then the remove was successful so recurse with the new set; if now then do nothing
		if (tempSetDifference.empty()) {
			return runRule(newStandin);
		}
	} // end for : newStandin

	return inputVec;
}

// Minimize the ruleset by removing unneeded rules
void lem2Calc::rulesetMinimization(bool lower) {
	// Determine the ruleset that we are using
	vector<vector<avPairs::singlePair>> ruleSet = (lower ? ruleSetLower : ruleSetUpper);

	// For each rule in the ruleset, try to remove values
	ruleSet = runRuleset(ruleSet);

	// Set the ruleSetLower or ruleSetUpper value to what we got
	if (lower)
		ruleSetLower = ruleSet;
	else
		ruleSetUpper = ruleSet;
}

// Recursive function for removing rules; rule minimization on a single rule
vector<vector<avPairs::singlePair>> lem2Calc::runRuleset(vector<vector<avPairs::singlePair>> inputRuleSet) {
	// Some temp values
	set<int> accumulatorSet;		// For getting the intersect of each remaining rule
	set<int> accumulatorSetInner;		// For getting the intersect of each remaining rule
	set<int> tempIntersect;			// Used to store the result when getting an intersection
	set<int> tempSetDifference;	// Used to check is a set is a subset of another
	vector<vector<avPairs::singlePair>> newStandin;		// Make a new vector under then assumption that we can remove an av pair then test
	int index = 0;
	int lastIndex;
	string conceptAttribute;
	set<int> finalCompare;

	// If we only have one av pair going to a concept then the rule is already minimized
	if (inputRuleSet.size() == 1)
		return inputRuleSet;

	// If we have multiple av pairs then try removing each of them
	for (unsigned it = 0; it < inputRuleSet.size() - 1; it++, index++) {
		// Remove the current singePair from inputVec and place in newStandin
		newStandin = inputRuleSet;
		newStandin.erase(newStandin.begin()+index);

		// Need to know the concept value we are looking at cause those are the only values that we should check
		lastIndex = inputRuleSet[it].size() - 1;
		conceptAttribute = inputRuleSet[it][lastIndex].value;
		finalCompare = inputRuleSet[it][lastIndex].caseSet;

		// Sum all the values in the newStandin
		accumulatorSet.clear();
		for (unsigned k = 0 ; k < newStandin.size() -1; k++) {
			// Get he last index of the current rule
			lastIndex = inputRuleSet[k].size() -1;

			accumulatorSetInner.clear();
			if (conceptAttribute == inputRuleSet[k][lastIndex].value) {
				// For each av block
				for (unsigned i = 0; i < newStandin[k].size() - 1; i++) {
					// If we are looking at the first av pair then return the
					if (i == 0) {
						accumulatorSetInner = newStandin[k][0].caseSet;
					}
					// Not the first value so intersect it with accumulatorSet
					else {
						tempIntersect.clear();
						set_intersection(accumulatorSetInner.begin(), accumulatorSetInner.end(), newStandin[k][i].caseSet.begin(), newStandin[k][i].caseSet.end(), inserter(tempIntersect, tempIntersect.begin()));
						accumulatorSetInner = tempIntersect;
					}
				}		// end for : indiv rule

				// Union all values together
				accumulatorSet.insert(accumulatorSetInner.begin(), accumulatorSetInner.end());
			}	// end if : decision value check
		} // end for : newStandin



		// // TODO: REMOVE
		// cout << "vals accumulatorSet " + to_string(accumulatorSet.size()) + ": ";
		// for (set<int>::iterator it = accumulatorSet.begin() ; it != accumulatorSet.end(); it++) {
		// 	cout << to_string(*it) +  ",";
		// }
		// cout << endl;
		//
		// // TODO: REMOVE
		// cout << "vals decisionSet " + to_string(newStandin[newStandin.size() - 1].caseSet.size()) + ": ";
		// for (set<int>::iterator it = newStandin[newStandin.size() - 1].caseSet.begin() ; it != newStandin[newStandin.size() - 1].caseSet.end(); it++) {
		// 	cout << to_string(*it) +  ",";
		// }
		// cout << endl;

		// Check to see if we cover all cases that we set out to cover
		if (accumulatorSet == finalCompare) {
			return runRuleset(newStandin);
		}
	} // end for : each rule

	return inputRuleSet;
}
