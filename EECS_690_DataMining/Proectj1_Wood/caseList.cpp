#include <iostream>
#include <vector>
#include <stdlib.h>
#include "caseList.h"

using namespace std;

caseList::caseList(){}

caseList::~caseList(){}

// Create a new entry and return a pointer to it
caseList::entry* caseList::generateEntry( string val, bool sym) {
  struct entry* temp = new struct entry();
  temp->value = val;
  temp->symbolic = sym;
  return temp;
}

// Add a new entry to our caseList; TODO: Remove this cause we don't use it
void caseList::addCase(vector<entry*> input, int caseNum) {
  // TODO: Fill this in
  struct singleCase* temp = new struct singleCase;
  cList.push_back(temp);
}
