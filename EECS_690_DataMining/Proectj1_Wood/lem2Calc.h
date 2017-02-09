// lem2Calc.h

#ifndef LEM2CALC_H
#define LEM2CALC_H

#include "avPairs.h"
#include <fstream>

class lem2Calc {
  public:
		lem2Calc(avPairs inputValues, bool lower);
		~lem2Calc();
		void printRuleSet(std::vector<std::vector<avPairs::singlePair>> inputRuleSet, std::ostream& stream);
		bool alreadyInUse (std::vector<avPairs::singlePair> tentativeRule, avPairs::singlePair inputPair);
		void determineTentativeRuleset(avPairs inputValues, bool lower);
		void ruleMinimization(bool lower);
		std::vector<avPairs::singlePair> runRule(std::vector<avPairs::singlePair> inputVec);
		void rulesetMinimization(bool lower);
		std::vector<std::vector<avPairs::singlePair>> runRuleset(std::vector<std::vector<avPairs::singlePair>> inputVec);
		void removeUnnecessaryAVBlocks(std::vector<avPairs::singlePair> &tempAllPairs, avPairs::singlePair avSelected);

		std::vector<std::vector<avPairs::singlePair>> ruleSetLower;
		std::vector<std::vector<avPairs::singlePair>> ruleSetUpper;
};

#endif
