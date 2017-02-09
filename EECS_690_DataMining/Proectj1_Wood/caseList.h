//caseList.h

#ifndef CASELIST_H
#define CASELIST_H

#include <string>
#include <vector>

class caseList {

  public:
    struct entry {
      std::string value;
      bool symbolic;
    };

    struct singleCase {
      int caseNumber;
      std::vector<entry*> avPairs;
    };

    caseList();
    ~caseList();
    struct entry* generateEntry(std::string val, bool sym);
    void addCase(std::vector<entry*> input, int caseNum);
    std::vector<singleCase*> cList;
};


#endif
