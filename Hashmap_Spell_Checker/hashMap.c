#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink* link)
{
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink*) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map
 */
void hashMapCleanUp(HashMap* map)
{
    //Iterate through each bucket sequentially
    for (int i = 0; i < map->capacity; i++) {
        HashLink* curLink = map->table[i];
        //If the bucket has anything in it, traverse the linked list and free each link
        while (curLink != NULL) {
            HashLink* tempLink = curLink->next;
            hashLinkDelete(curLink);
            curLink = tempLink;
        }
    }
    free(map->table);
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
    HashMap* map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap* map)
{
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key  and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
    int idx = (HASH_FUNCTION(key) % map->capacity);
    HashLink* curLink = map->table[idx];
    while (curLink != NULL) {
        if (strcmp(curLink->key, key) == 0) {
            return &(curLink->value);
        }
        else {
            curLink = curLink->next;
        }
    }
    return NULL;
}

/**
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
    HashLink** newTable = malloc(sizeof(HashLink*) * capacity);
    //init new table to all NULL values
    for (int i = 0; i < capacity; i++) {
        newTable[i] = NULL;
    }
    //iterate through buckets of the original table 
    for (int i = 0; i < map->capacity; i++) {
        HashLink* curLink = map->table[i];
        //When we find a non-empty bucket in the original table, traverse the linked list there
        while (curLink != NULL) {
            //Rehash each link we encounter and reassign it to its new bucket in the new table
            int newIdx = (HASH_FUNCTION(curLink->key) % capacity);
            HashLink* tempLink = curLink->next;
            curLink->next = newTable[newIdx];
            newTable[newIdx] = curLink;
            curLink = tempLink;
        }
    }
    map->capacity = capacity;
    free(map->table);
    map->table = newTable;
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
    //check if our table needs resizing first
    if (hashMapTableLoad(map) >= MAX_TABLE_LOAD) {
        resizeTable(map, (map->capacity * 2));
    }
    int idx = (HASH_FUNCTION(key) % map->capacity);
    HashLink* curLink = map->table[idx];
    //traverse the list at the index until the key is found and the value is updated, or we reach the end of the list and a new link is created
    while (curLink != NULL) {
        if (strcmp(curLink->key, key) == 0) {
            curLink->value = value;
            return;
        }
        else {
            curLink = curLink->next;
        }
    }
    map->table[idx] = hashLinkNew(key, value, map->table[idx]);
    map->size++;
}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap* map, const char* key)
{
    int idx = (HASH_FUNCTION(key) % map->capacity);
    HashLink* curLink = map->table[idx];
    //Traverse the linked list at the hash index, searching for the key
    while (curLink != NULL) {
        //case where the head of the linked list is the target value and we must reassign the head
        if (strcmp(map->table[idx]->key, key) == 0) {
            HashLink* tempLink = map->table[idx]->next;
            hashLinkDelete(map->table[idx]);
            map->table[idx] = tempLink;
            map->size--;
            return;
        //case otherwise, if the link after the current link is the target link, remove it and reassign current link's next pointer to target's next pointer.
        } else {
            if (curLink->next != NULL) {
                if (strcmp(curLink->next->key, key) == 0) {
                    HashLink* tempLink = curLink->next->next;
                    hashLinkDelete(curLink->next);
                    curLink->next = tempLink;
                    map->size--;
                    return;
                }
            }
            curLink = curLink->next;
        }
    }
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
    int idx = (HASH_FUNCTION(key) % map->capacity);
    HashLink* curLink = map->table[idx];
        //Traverse the linked list at the bucket hash index until it finds the key
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

/**
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
    int emptyBuckets = 0;
    //Iterate through the hash table and increment a counter everytime an empty bucket is encountered
    for (int i = 0; i < map->capacity; i++) {
        if (map->table[i] == NULL) {
            emptyBuckets++;
        }
    }
    return emptyBuckets;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
    return ((float)(map->size) / (float)(map->capacity));
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap* map)
{
    //Prints all the links' key-value pairs separated by commas that are in a bucket on a single line per bucket
    printf("\n");
    for (int i = 0; i < map->capacity; i++) {
        HashLink* curLink = map->table[i];
        while (curLink != NULL) {
            printf("%s %d", curLink->key, curLink->value);
            if (curLink->next != NULL) {
                printf(", ");
            }
            curLink = curLink->next;
        }
        if (map->table[i] == NULL) {
            printf("Empty");
        }
        printf("\n");
    }
}
