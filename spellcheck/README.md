# Spelling Checker

This project was created by:

1. Rachit Gupta: rg1091
2. Vian Miranda: vsm35


# Using a trie to store the dictionary
We chose to use a trie as our data structure to store the words from the dictionary. Compared to arrays, tries provide us faster lookup times and is more memory efficient. Tries take O(m), where m is the length of the word, whereas an array with binary search, for example, can take O(nlogn), where n is the length of the array. This means for larger dictionary, an array may be significantly slower than a trie, which does not rely on the size of the dictionary as much.

We also stored the possible variations each word can have and still be marked as correct in the dictionary in the trie. By adding this in the trie itself, it became less of a problem in accessing these variations for later spell-checking comparisons.

We created helper functions to initialize, free, and add/check word in trie. These made it easier to debug any errors when using the trie in our spell check function.

# Spchk design

In our `spchk.c` file, we break the code down into many different helper functions that are ultimately used in some way in our main function. We also keep track of a SUCCESS integer variable, which is used to keep track of spell-checking mistakes. It is initially 1 because we assume there are no mistakes and is marked as 0 when we identify a mistake. Below are the helper functions, their purpose in creating this spell checker, and how SUCCESS is managed.


1. `cleanText* create_variations(char* word)`

    This helper function was made to get the possible variations of the word so it can be used when creating the dictionary (trie). This makes the code less confusing. The different variations are based on 4 cases:
        
    1. The word is all lowercase
    2. The word's first letter is upper case (propercase)
    3. The word is all uppercase
    4. The word is none of the above and matches exactly the word provided

    After identifying these cases for variations, we return a cleanText struct. The cleanText struct holds a character array called variations and the number of variations, numVariations.



1. `int create_dict(int fd)`

    Here, we take the dictionary's file descriptor as an input, iterate through each word in the dictionary, and add it to our trie. We also get the variations of each word using create_variations and store those in the trie as well. We return -1 anytime there is an error reading a file or failing to add a word to the trie. We return 0 if we successfully created the dictionary.

2. `cleanText* clean_text(char* word)`
  
    This is a helper function to simply handle any leading and trailing punctuation. 

    We return a cleanText struct containing the cleaned version of the word. We return NULL if there is an error allocating memory.

3. `int handle_hyphenated_word(char* word, int start_index)`
  
    This function handles the case where there is a hyphenated word. We check each word in the hyphenated word (ex. apple, pie in "apple-pie"). We iterate through the trie and if we reach the end of the function, that means that none of the valid variations match to the dictionary, resulting in a mistake. We set SUCCESS to 0 if this happens. If there is an error in cleaning the word using clean_text, we return -1; otherwise, we return 0.

4. `int check_text(int fd, char* file_name)`
  
    Here, we read the file, go word by word in each line, and check if the word is in the dictionary (trie). This means we have to check when we encounter a new word in a line (use isspace() and check the whitespace before current byte) and if we are at the end of the line. We use clean_text to clean the word, and then use that cleaned word (or handle_hyphenated_word if we encounter a hyphenated word) to see if the word is a hyphenated word. We also mark SUCCESS accordingly. Finally, we keep track of the line and column number and print the error message if SUCCESS = 0.

    We also handled the last word separately because there is no trailing white space, whereas all the other words before did have trailing white space.

    We return -1 on any memory allocation errors and 0 if everything went successfuly.

5. `int file_handler(const char* pathname, int (*func)())`
  
    This is a simple helper function to ensure we are able to open the file properly. We do this by utilizing the file descriptor based on the provided pathname. The 2nd parameter is used for create_dict to make sure we can open all text files, including that of the dictionary. We also make sure to close the file here.

    We return -1 on any error handling the file, and 0 if file_handler had no issues.

6. `int main(int argc, char* argv[])`
  
    Here is where we incorporate everything together.
    1. Initialize trie
    2. For each argument provided, check if they are valid
    3. Read through the file and compare with the dictionary using check_text
    4. Free the dictionary and make sure any files were closed properly using file_handler
    5. Return EXIT_SUCCESS if SUCCESS is true, EXIT_FAILURE if SUCCESS is false

# Testing

To test the implementation of our spell checker, we created different test text files to test the different test cases that we identified against the Ilab dictionary and our own test dictionary (`spellcheck/testcases/testdict.txt`)

1. Handling trailing punctuation/quotation marks and preceding brackets
    Example (located in `spellcheck/testcases/punctuation.txt`)
    ```
    hi
    how 'are' you
    pizza
    apple-pie'
    [hi
    hi]
    {hello}
    "what"
    "hello"
    "apple-pie"
    "hi"
    ```
    We check here if clean_text works properly. This means getting rid of any leading and trailing punctuation marks.

2. Handling multiple words in a line and correctly displaying line and column number
    Example (located in `spellcheck/testcases/multwords.txt`):
    ````
    hi how are you
    my name is
    i like 'Food'
    ````
    Here we check if our program can identify multiple words in each line and check them. We also make sure that if a word is not in the dictionary, the column number and line are being updated properly.

3. Extra whitespace

    Example (located in `spellcheck/testcases/extraspace.txt`):
    ```
    everything in this file should be             correct! 
    ```
    We check if we are able to successfully read past the spaces in bewtween the words "be" and "correct".


4. Checking if the last word in the text file is read properly

    Example (located in `spellcheck/testcases/sometext.txt`):
    ```
    apple
    banana
    pear
    hi
    pizza-is-nice
    pizza-is
    hello
    hi apple-pie-hi
    math
    science
    ```
    We make sure that science is being read here.

5. Handling hyphenated words with >= 1 hyphen
  
    Example (located in `spellcheck/testcases/hyphens.txt`):
    ```
    apple-pie
    how-are-you
    water-watchdog-wastrel
    i-like-food
    my-name-is-rachit
    my-name-is-vian
    my-favorite-food-is-pizza
    we-are-the-chfapmions-of-the-world
    "apple-"pie
    "apple-pie
    ```

    We want to make sure that our spell checker handles not only 1 hyphen, but more. This ensures that we are traversing through each word in the hyphenated word correctly. "apple-pie", "my-name-is-vian" and "my-favorite-food-is-pizza" are all in `spellcheck/testcases/testdict.txt`, but the rest should report an error. Note that " "apple-"pie " will also return an error because of the quote inside the hyphenated word, which is not allowed.

6. Having multiple text files as arguments

    Here is the command line argument:
    ```
    ./spchk /usr/share/dict/words testcases/extraspace.txt testcases/sometext.txt
    ```

    We check if we can iterate through multiple text files. This lets us know that our text file traversal is working properly, and not just for one text file.

7. Having multiple directories as arguments

    Here is the command line argument:
    ```
    ./spchk /usr/share/dict/words testcases/dir1/ testcases/dir2/
    ```

    We check if we can iterate through multiple directories. This lets us know that our text file traversal is working properly for multiple directories.
    
8. Having multiple text files and multiple directories as arguments

    Here is the command line argument:
    ```
    ./spchk /usr/share/dict/words testcases/dir1/ testcases/extraspace.txt testcases/sometext.txt
    ```

  We check if we can iterate through multiple directories and files. This lets us know that our text file traversal is working properly for multiple arguments, no matter their type.

9. Success: 
    When reading a file that has all words properly spelled, such as `spellcheck/testcases/extraspace.txt`, we get a success message: 
  
    "All words are spelled correctly"
  