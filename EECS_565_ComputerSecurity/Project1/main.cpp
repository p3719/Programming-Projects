// main.cpp

#include <string>
#include <iostream> 
#include <algorithm>
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
    val = ((int)ciphertext[i] - 97) - ((int)key[i % key.size()] - 97);
    while (val < 0)
    {
      val += 26;
    }
    ciphertext[i] = (char)((val % 26) + 97);
  }

  return ciphertext;
}

int main()
{
  string original;
  string key;
  string answer;
  string dein;

  // User chooses which action to perform
  cout << "Would you like to encrypt or decrypt (0 for encrypt, 1 for decrypt): ";
  getline(cin, dein);

  // Enter the original message
  cout << "Please input a message to be " << (dein == "1" ? "decrypted" : "encrypted") << ": ";
  getline(cin, original);
  transform(original.begin(), original.end(), original.begin(), (int (*)(int))tolower);
  original.erase(remove(original.begin(), original.end(), ' '), original.end());

  // Enter the key that you are using
  cout << "Input the key that you will be using: ";
  getline(cin, key);
  transform(key.begin(), key.end(), key.begin(), (int (*)(int))tolower);
  key.erase(remove(key.begin(), key.end(), ' '), key.end());

  // Display the original text and the key
  cout << endl << (dein == "1" ? "C" : "P") << ": " << original << endl;
  cout << "K: " << key << endl;

  // Print the answer
  answer = dein == "1" ? decode(original, key) : encode(original, key);
  cout << (dein == "1" ? "\nDecrypting..." : "\nEncrypting...") << endl << endl;
  cout << (dein == "1" ? "P" : "C") << ": " << answer << endl;

  return 0;
}
