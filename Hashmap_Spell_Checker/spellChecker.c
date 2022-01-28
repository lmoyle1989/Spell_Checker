#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

int isLetter(char charIn) {
    if ((charIn >= 65) && (charIn <= 90)) {
        return 1;
    }
    else if ((charIn >= 97) && (charIn <= 122)) {
        return 1;
    }
    else {
        return 0;
    }
}

int dictHash(const char* key) 
{
    int hashIndex = 0;
    for (int i = 0; key[i] != '\0'; i++) 
    {
        hashIndex += (i + 1) * key[i];
    }
    return hashIndex;
}

//the memory for the key was allocated already in the nextword function, otherwise a copy from what i wrote in hashmap.c
HashLink* dictHashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = key;
    link->value = value;
    link->next = next;
    return link;
}

//Basically a copy of hashMapPut from hashMap.c
void dictHashMapPut(HashMap* map, const char* key, int value)
{
    int idx = (dictHash(key) % map->capacity);
    HashLink* curLink = map->table[idx];
    while (curLink != NULL) {
        if (strcmp(curLink->key, key) == 0) {
            curLink->value = value;
            return;
        }
        else {
            curLink = curLink->next;
        }
    }
    map->table[idx] = dictHashLinkNew(key, value, map->table[idx]);
    map->size++;
}

int dictHashMapContainsKey(HashMap* map, const char* key)
{
    int idx = (dictHash(key) % map->capacity);
    HashLink* curLink = map->table[idx];
    while (curLink != NULL) {
        if (strcmp(curLink->key, key) == 0) {
            return 1;
        }
        else {
            curLink = curLink->next;
        }
    }
    return 0;
}

int minimum(int a, int b, int c) {
    int min = a;
    if (b <= min) {
        min = b;
    }
    if (c <= min) {
        min = c;
    }
    return min;
}

//*CITATION*
//This Levenshtein distance algorithm is a modified version of the one found here: https://www.lemoda.net/c/levenshtein/
int levenshteinDistance(const char* word1, int len1, const char* word2, int len2)
{
    int i;
    int** arr = malloc((len1 + 1) * sizeof(int*));

    for (i = 0; i < len1 + 1; i++) {
        arr[i] = malloc((len2 + 1) * sizeof(int));
    }
    for (i = 0; i <= len1; i++) {
        arr[i][0] = i;
    }
    for (i = 0; i <= len2; i++) {
        arr[0][i] = i;
    }
    for (i = 1; i <= len1; i++) {
        int j;
        char c1;

        c1 = word1[i - 1];
        for (j = 1; j <= len2; j++) {
            char c2;

            c2 = word2[j - 1];
            if (c1 == c2) {
                arr[i][j] = arr[i - 1][j - 1];
            }
            else {
                arr[i][j] = minimum(arr[i - 1][j] + 1, arr[i][j - 1] + 1, arr[i - 1][j - 1] + 1);
            }
        }
    }
    int result = arr[len1][len2];
    for (i = 0; i < (len1 + 1); i++) {
        free(arr[i]);
    }
    free(arr);
    return result;
}

//assigns the levenshtein distance to all key/value pairs in the map compared to input bufer, then populates an array of suggestions with the lowest lev. distance
assignLevenshteinDistance(HashMap* map, char* inputString, HashLink** suggestions) {
    int sug_prelim_idx = 0;
    for (int i = 0; i < map->capacity; i++) {
        HashLink* curLink = map->table[i];
        while (curLink != NULL) {
            curLink->value = levenshteinDistance(curLink->key, strlen(curLink->key), inputString, strlen(inputString));
            //start by populating the suggestion array with the first 5 words in the dictionary map, and then compare to everything else
            if (sug_prelim_idx < 5) {
                suggestions[sug_prelim_idx] = curLink;
                sug_prelim_idx++;
            }
            else {
                //check each new key against what is already in the suggestion array and replace if lower
                for (int i = 0; i < 5; i++) {
                    if ((curLink->value < suggestions[i]->value) && (curLink->value > 0)) {
                        suggestions[i] = curLink;
                        break;
                    }
                }
            }
            curLink = curLink->next;
        }
    }
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    char* newWord = nextWord(file);
    while (1) {
        if (newWord == NULL) {
            return;
        }
        else {
            dictHashMapPut(map, newWord, 0);
        }
        newWord = nextWord(file);
    }
}

/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    HashMap* map = hashMapNew(1000);

    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    int quit = 0;
    int invalidString = 0;

    while (!quit)
    {
        invalidString = 0;
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%255[^\n]", inputBuffer);
        //this next line clears the actual input buffer
        while (getchar() != '\n');

        //check input buffer for non-letter characters and also convert to all lower-case
        for (int i = 0; inputBuffer[i] != '\0'; i++) {
            inputBuffer[i] = tolower(inputBuffer[i]);
            if (!isLetter(inputBuffer[i])) {
                invalidString = 1;
            }
        }

        if (strcmp(inputBuffer, "quit") == 0)
        {
            quit = 1;
        }

        if (invalidString) {
            printf("Invalid string. Please enter a single word containing only letters.\n");
        }
        else {
            //if the word is in the dictionary, no need to calculate lev. distance
            printf("The inputted word is ... ");
            if (dictHashMapContainsKey(map, inputBuffer)) {
                printf("spelled correctly.\n");
            }
            //if the word is not in the dictionary, then calculate the lev. distance for all words in the dictionary and create an array of 5 words with the lowest lev distance
            else {
                HashLink* suggestions[5];
                assignLevenshteinDistance(map, inputBuffer, suggestions);
                printf("spelled incorrectly.\n");
                printf("Did you mean...?\n");
                for (int i = 0; i < 5; i++) {
                    printf("%s\n", suggestions[i]->key);
                }
                printf("\n");
            }
        }
    }

    hashMapDelete(map);
    return 0;
}
