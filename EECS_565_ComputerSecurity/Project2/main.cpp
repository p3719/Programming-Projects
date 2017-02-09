// main.cpp

#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include "Timer.cpp"
using namespace std;

// Function used to encode using vigenere
string encode (string original, string key)
{
  for (int i = 0; i < original.size(); i++)
  {
    original[i] = (char)(((((int)original[i] + (int)key[i % key.size()]) - 194) % 26) + 97);
  }

  return original;
}

// Function used to decode using vigenere
string decode (string ciphertext, string key)
{
  int val;

  for (int i = 0; i < ciphertext.size(); i++)
  {
    // This is inorder to make a true modulo function since c++'s % is actually a remainder
    val = ((int)ciphertext[i] - 65) - ((int)key[i % key.size()] - 65);
    while (val < 0)
    {
      val += 26;
    }
    ciphertext[i] = (char)((val % 26) + 65);
  }

  return ciphertext;
}

// This was my original decode by brute forcing all possible keys
// // Decode the ciphertext using key length and first word length
// // Print all values where there is a matching word for the decode first word
// void completeDecode(string ciphertext, int wordLength, int keyLength, string* dict, int dictSize)
// {
//   int currentIndex = keyLength - 1;
//   string keys = "";
//   string decipheredFirst;
//
//   // This will probably be a problem
//   // Initialize the keys with a'ss
//   // make this "a" for lower case
//   for (int i = 0; i < keyLength; i++)
//   {
//     keys = keys + "A";
//   }
//
//   cout << "Keys val: " << keys << endl;
//   cout << "Dict: " << dictSize << endl;
//
//   cout << endl << "Here are the possible values:" << endl;
//   // Start the while
//   do
//   {
//     // Check decode first word against words in dict, for a particular key
//     for (int i = 0; i < dictSize; i++)
//     {
//       decipheredFirst = decode(ciphertext.substr(0, wordLength), keys);
//       if (decipheredFirst == dict[i])
//       {
//         cout << decode(ciphertext, keys) << endl;
//       }
//     }
//
//     // use 122 for lower
//     if (keys[currentIndex] == 90)
//     {
//       // Increment the key; == 122 is z
//       while (keys[currentIndex] == 90 && currentIndex != -1)
//       {
//         // make 'a' if lower case
//         keys[currentIndex] = 'A';
//         currentIndex--;
//
//         if (currentIndex != -1 && keys[currentIndex] != 90)
//         {
//           // cout << "hit" << endl;
//           keys[currentIndex] = keys[currentIndex] + 1;
//           currentIndex = keyLength - 1;
//         }
//       }
//     }
//     else
//     {
//       keys[currentIndex] = keys[currentIndex] + 1;
//     }
//   } while (currentIndex > -1);
//
//   cout << "Done Searching" << endl;
// }

// New decode function; approahes the problem from the dictionary standpoint
// Need to know the length of the first word, the length of the key, and that the first word is in the dictionary
void completeDecode(string ciphertext, int wordLength, int keyLength, string* dict, int dictSize)
{
  Timer tracker;      // Used to find time to decode
  double runtime;     // Store the amount of time needed
  string currentVal; // This is the current value we are testing from the dictionary
  string currentKey;
  bool success;     // true means working so far; false not means not right;
  int starting;       // Index to start the comp on; changes if the prev succeeds
  int offsetSent;        // Difference in the ascii values from the beginning; doesnt change on increment
  int offset;         // The difference in the ascii vals changes as it goes
  int currentIndex = 0; // Current dictionary index being tested
  int i;            // Need to declare here to use for like i want

  cout << endl << "Here are the possible values:" << endl
        << "*******************************************" << endl;


  // Start the Timer
  tracker.start();

  while (currentIndex < dictSize)
  {
    starting = 0;
    currentKey = "";
    currentVal = dict[currentIndex];

    // Checks a single dictionary word (currentVal),
    for (i = starting, success = true; (i < wordLength) && success; i += keyLength)
    {
      offset = ((ciphertext[i] - currentVal[i]) + 26) % 26;

      // If this is the first compare then set the offset sentinel
      if (i == starting)
      {
        offsetSent = offset;
      }
      // Not the first compare
      else
      {
        // If offsets aren't the same then success=false
        if (offset != offsetSent)
        {
          success = false;
        }
      }

      // If compares are successful then add letter to the key;
      if ((i + keyLength) >= wordLength)
      {
        // Assign a letter to the key
        currentKey = currentKey + (char)(offset + 65);

        // Increment to the next starting character if able
        if (starting < (keyLength - 1))
        {
          starting++;
          i = starting - keyLength;
        }
      }
    }

    // We found a successful match; run decode and print that bad boy
    if (success)
    {
      cout << "\nCipher: " << ciphertext.substr(0, wordLength) << endl;
      cout << "Dict: " << currentVal << endl;
      cout << "Key: " << currentKey << endl;
      cout << "Plaintext: " << decode(ciphertext, currentKey) << endl;
    }

    // Move to the next word in the dictionary
    currentIndex++;
  } // end while

  // Store the time it took to run
  runtime = tracker.stop();

  // Print the time it took to decode
  cout << "\nDecode runtime: ";
  tracker.printTime(runtime);
  cout << "*******************************************" << endl;
}


int main()
{
  string original;
  string answer;
  int keyLength;
  int wordLength;
  int count = 0;

  // Get the values in the dictionary file
  string** dict = new string*[16];
  int stringLength;
  int index[16] = {};
  int size_dict[16] = {};
  ifstream dictfile("dict.txt");
  ifstream commandfile("commanddata.txt");
  string x;

  // Initialize the array
  for (int i = 0; i < 16; i++)
  {
    dict[i] = new string[50000];
  }

  // Add values from dictionary
  while (dictfile >> x)
  {
    stringLength = x.length();
    dict[stringLength][index[stringLength]] = x;
    index[stringLength]++;
  }

  // Identify the size of each array
  for (int i = 0; i < 16; i++)
  {
    size_dict[i] = index[i];
  }

  cout << "Don't blink or you'll miss it!!!" << endl;

  while (commandfile >> original)
  {
    count++;

    // Take in the first word size
    commandfile >> wordLength;

    // Take in the key length
    commandfile >> keyLength;

    // Display the original text and the key
    cout << "\n\n\n<---------------------------------- Test " << count << " ---------------------------------->\n";
    cout << "This is the original message: " << original << endl
          << "First word length: " << wordLength << endl
          << "Key length: " << keyLength << endl;

    completeDecode(original, wordLength, keyLength, dict[wordLength], size_dict[wordLength]);
    cout << "<---------------------------------- End Test " << count << " ------------------------------>\n";
  }

  cout << "\nAuthor's Notes:  That was quick!  Simply take a different approach." << endl
        << "If the issue you have is with the time complexity due to your key length, " << endl
        << "then maybe try changing the variable determining time complexity. I altered" << endl
        << "my code in a way that made time complexity primarily based on the size of our " << endl
        << "dictionary file which remains constant.  Furthermore, I am able to parse " << endl
        << "the dictionary file by length which drastically reduces the applicable size " << endl
        << "of the dictionary.  Worth!" << endl;

  dictfile.close();
  commandfile.close();
  return 0;
}
