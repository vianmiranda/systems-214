#include <stdio.h>
#include <glob.h>
#include <string.h>
// extern char* strdup(const char* s);
int main() {

    char* string1 = "bopity bopity boo";
    char* string2 = strdup(string1);
    printf("string1: %s\n", string1);
    printf("string2: %s\n", string2);

    // glob_t glob_result;

    // // Perform glob expansion
    // if (glob("*.txt", 0, NULL, &glob_result) == 0) {
    //     for (size_t i = 0; i < glob_result.gl_pathc; i++) {
    //         printf("%s\n", glob_result.gl_pathv[i]);
    //     }   
    // } else {
    //     printf("Error in globbing\n");
    //     return 1;
    // }

    // // Iterate over matched file names


    // // Free allocated memory
    // globfree(&glob_result);



    return 0;
}