// avPairs.h

#ifndef AVPAIRS_H
#define AVPAIRS_H

#include "caseList.h"
#include "stdlib.h"
#include <set>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

class avPairs {
  public:
    struct singlePair {
      std::string attribute;
      std::string value;
      std::set<int> caseSet;
      bool fromNumeric;
    };

		struct conceptApproximation {
			std::string value;
			std::set<int> approximationCases;
		};

    avPairs(std::vector<std::vector<caseList::entry*>> dataSet, std::vector<std::string> attrDecisionNames);   // Needs to make all the pairs that it can
    ~avPairs();
    void numericToSymbolic();
    std::vector<double> splitSymbolicRange(std::string symbolicRep);
    void vectorUniqueInsert(std::vector<std::string> &inputVec, std::string inputVal);

    // Return: Set of cases with that attribute value combo
    std::set<int> makeAVBlock(std::vector<std::vector<caseList::entry*>> dataSet, unsigned attributeIndex, std::string value, bool fromNumeric);
    bool valIncluded(std::string symbolicRep, double numericRep);
    std::set<int> retrieveAVSets(std::vector<std::vector<caseList::entry*>> dataSet, std::string attributeName, int attributeIndex, std::string value, bool symbolic);
		std::set<int> getDecisionSet(std::string decisionValue);

		// Set the values for lower and upper approximationCases
		// This should just need the characteristic sets (kSets) and decisionSets
		void setApproximationValues();

    std::vector<singlePair> allPairs;   //Should only be attribute value set
    std::vector<singlePair> decisionSets;   // a more advanced d*; desision sets
    std::set<int> universe;             // This is a set containing all cases... used in characteristic set
    std::vector<std::set<int>> kSets;

		std::vector<conceptApproximation> lowerApproximations;
		std::vector<conceptApproximation> upperApproximations;

};

#endif
