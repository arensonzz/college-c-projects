#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 1000
#define NAME_LEN 50
#define STACK_LEN 1000
#define PATH_LIMIT 1000
#define CONNECTING_FLIGHT_DELAY_M 60

#define DASHES "------------------------------------------------------------------"

typedef struct HashNode {
    int key;             // calculated hash value for the name
    int graphIndex;      // in which cell this node is stored in the adjacency list
    char name[NAME_LEN]; // used to calculate hash
} HashNode;

typedef struct HashTable {
    HashNode **data; // hash table storing nodes
    int size;        // maximum capacity
    int itemCount;   // number of full cells
} HashTable;

typedef struct Flight {
    char dest[NAME_LEN];
    int durationMinutes;
    int price;
    struct Flight *next;
} Flight;

typedef struct Stack {
    Flight *arr[STACK_LEN];
    int top;         // top points to the last item in the stack, initialized to -1
    Flight ***paths; // matrix of Flight pointers, path history that is temporarily stored in stack is moved into next
                     // row of paths after reaching destination. Empty cells are denoted by NULL pointers.
    int pathsNext;   // next empty row in the paths matrix
} Stack;

// user-defined error codes are between 64 and 113
enum errCodes {
    E_MEM_ERR = 64,
    E_FILE_ERR,
    E_NODE_PRESENT,
    E_CITY_NOT_FOUND,
    E_INVALID_CRITERIA,
    E_INVALID_MENU_CHOICE,
    E_EMPTY_PATH,
    E_PATHS_NOT_SORTED,
    E_NO_PATH_FOUND,
};

// General functions
void inputString(const char *prompt, char *str);
void printError(enum errCodes errCode);
int getLineCount(const char *fileName);
void strToLower(char *str);
void flushStdin(void);

// Hash related functions
int findClosestPrime(int n);
unsigned long hash(unsigned char *str);
HashTable *createHashTable(int size);
int addToHashTable(HashTable *hashTable, const char *name);
int isNodePresent(HashTable *hashTable, const char *name, int *loc);
void *deleteHashTable(HashTable *hashTable);
HashNode *getFromHashTable(HashTable *hashTable, const char *name);

// Stack related functions
Stack *createStack(int vertexCount);
int isStackEmpty(Stack *stack);
int isStackFull(Stack *stack);
Flight *pop(Stack *stack);
int push(Stack *stack, Flight *data);

// Main functions
Flight *createFlight(const char *dest, int durationMinutes, int price);
int addNewCity(HashTable *cities, Flight **flightAdj, char *name);
int readFlights(const char *fileName, HashTable *cities, Flight **flightAdj);
void printAdjList(Flight **flightAdj, int vertexCount);
void addPath(Stack *history);
void findPathDfs(HashTable *cities,
    Flight **flightAdj,
    Flight *source,
    Stack *history,
    Flight *dest,
    int *visited,
    int depth,
    int depthLimit);

// Print paths sorted
int findPathGains(Stack *history, int *pathGains, char criteria);
void merge(Flight ***paths, int *pathGains, int l, int q, int r);
void sortPaths(Flight ***paths, int *pathGains, int l, int r);
void printPathsTable(Stack *history, int *pathMainGains, char criteria, FILE *out);

int main(void)
{
    char inputFileName[BUF_LEN]; // name of the file to read inputs
    char srcCity[BUF_LEN];       // used in query, user's city to take off from
    char destCity[BUF_LEN];      // used in query, user's city to arrive at
    char criteria;               // criteria to sort resulting paths
    int maxConnectingFlight;     // maximum limit of connecting flights when searching for a path
    HashTable *cities;           // unique city names read from input file
    Flight **flightAdj;          // adjacency list for storing each city's adjacent cities to fly
    Stack *history;              // temporary location for paths
    int *visited;                // array to mark vertices visited when doing graph search
    int *pathGains;              // calculated gain values for each found paths
    int lineCount;               // line count of the input file
    int returnVal;               // used to print errors, catches return values from functions
    int isPathsSorted = 1;       // flag for print to file menu option
    int stayInMenu = 1;
    int i;
    FILE *outFile;
    char outFileName[BUF_LEN];

    inputString("> Enter the name of the file containing flight details: ", inputFileName);
    printf("\n");
    if((lineCount = getLineCount(inputFileName)) == -1) {
        printError(E_FILE_ERR);
        return 0;
    }
    if((cities = createHashTable(findClosestPrime(lineCount * 3))) == NULL ||
        (flightAdj = (Flight **)malloc((lineCount * 2) * sizeof(Flight *))) == NULL) {
        printError(E_MEM_ERR);
        return 0;
    }
    if((returnVal = readFlights(inputFileName, cities, flightAdj)) != 0) {
        printError(returnVal);
        return 0;
    }
    if((visited = (int *)calloc((cities->itemCount), sizeof(int))) == NULL ||
        (history = createStack(cities->itemCount)) == NULL) {
        printError(E_MEM_ERR);
        return 0;
    }

    while(stayInMenu) {
        char ch1;

        printf("\n");
        printf("\t[a] Enter new query\n"
               "\t[s] Sort paths and list\n"
               "\t[f] Save last table to file\n"
               "\t[x] Exit main menu\n");
        printf("\t> Choice: ");
        scanf("%c", &ch1);
        flushStdin();
        ch1 = tolower(ch1);
        printf("\n");

        switch(ch1) {
        case 'a':
            inputString("> Enter the city to take off from: ", srcCity);
            inputString("> Enter the city to arrive at: ", destCity);
            printf("> Enter the maximum number of connecting flights: ");
            scanf("%d", &maxConnectingFlight);
            flushStdin();
            printf("\n");

            if(getFromHashTable(cities, srcCity) == NULL || getFromHashTable(cities, destCity) == NULL) {
                printError(E_CITY_NOT_FOUND);
                return 0;
            }
            for(i = 0; i < history->pathsNext; i++) {
                history->paths[i][0] = NULL;
            }
            history->pathsNext = 0;
            isPathsSorted = 0;
            findPathDfs(cities, flightAdj, flightAdj[getFromHashTable(cities, srcCity)->graphIndex], history,
                flightAdj[getFromHashTable(cities, destCity)->graphIndex], visited, 0, maxConnectingFlight + 1);
            printAdjList(flightAdj, cities->itemCount);
            break;
        case 's':
            if(history->pathsNext == 0) {
                printError(E_EMPTY_PATH);
            } else {
                printf("> How to sort flights [d(uration)/p(rice)]: ");
                scanf("%c", &criteria);
                flushStdin();
                printf("\n");
                if((pathGains = (int *)malloc(history->pathsNext * sizeof(int))) == NULL) {
                    printError(E_MEM_ERR);
                    return 0;
                }
                if((returnVal = findPathGains(history, pathGains, criteria)) != 0) {
                    printError(returnVal);
                    return 0;
                }
                sortPaths(history->paths, pathGains, 0, history->pathsNext - 1);
                isPathsSorted = 1;
                printPathsTable(history, pathGains, criteria, stdout);
            }
            break;
        case 'f':
            if(history->pathsNext == 0) {
                printError(E_EMPTY_PATH);
            } else if(isPathsSorted == 0) {
                printError(E_PATHS_NOT_SORTED);

            } else {
                inputString("> Where to save flights?: ", outFileName);
                if((outFile = fopen(outFileName, "w")) == NULL) {
                    printError(E_FILE_ERR);
                } else {
                    printPathsTable(history, pathGains, criteria, outFile);
                    fclose(outFile);
                }
            }
            break;
        case 'x':
            printf("Goodbye!\n");
            stayInMenu = 0;
            break;
        case 'h':
            break;
        default:
            printError(E_INVALID_MENU_CHOICE);
            break;
        }
    }

    deleteHashTable(cities);
    free(visited);
    free(pathGains);
    free(flightAdj);
    free(history);
    return 0;
}

/*********************
 *  General functions
 *********************/

/* Prompts user with the given prompt string, gets a string from the user and stores it in str */
void inputString(const char *prompt, char *str)
{
    printf("%s", prompt);
    fgets(str, BUF_LEN, stdin);
    str[strcspn(str, "\n")] = 0; // remove trailing newline
}

/* Prints error messages when given an error code   */
void printError(enum errCodes errCode)
{
    // Print error only if errCode is non-zero, 0 means all went well
    printf("%s", (errCode) ? "error: " : "");

    switch(errCode) {
    case E_MEM_ERR:
        puts("Memory is full.");
        break;
    case E_FILE_ERR:
        puts("Could not open the specified file.");
        break;
    case E_NODE_PRESENT:
        puts("Node is already in the hash table.");
        break;
    case E_CITY_NOT_FOUND:
        puts("City with the given name is not in the list of cities.");
        break;
    case E_INVALID_CRITERIA:
        puts("There is no criteria corresponding to the entered character.");
        break;
    case E_INVALID_MENU_CHOICE:
        puts("Entered invalid menu choice.");
        break;
    case E_EMPTY_PATH:
        puts("No previous query available.");
        break;
    case E_PATHS_NOT_SORTED:
        puts("You did not sort the flight paths.");
        break;
    case E_NO_PATH_FOUND:
        puts("Could no find a path between given two cities.");
        break;
    }
}

/* Find line count of the given file.
 *
 * @return line count if no errors,
 *         -1 otherwise
 */
int getLineCount(const char *fileName)
{
    FILE *file;
    int i = 0;

    if((file = fopen(fileName, "r")) == NULL) {
        return -1;
    }
    while(fscanf(file, "%*s %*s %*d %*d %*d") != EOF) {
        i++;
    }

    fclose(file);
    return i;
}

/* Converts given string to all lowercase letters
 */
void strToLower(char *str)
{
    int i;

    for(i = 0; str[i] != '\0'; i++) {
        str[i] = tolower(str[i]);
    }
}

/* Removes unwanted characters from the input buffer.
 */
void flushStdin(void)
{
    int c;
    while((c = getchar()) != '\n' && c != EOF)
        ;
}

/*******************************
 *  Hash table related functions
 *******************************/

/* Find largest prime number smaller than n.
 * This function is particularly useful in hash tables
 * because table size with a prime number decreases collusions.
 *
 * @return largest prime number smaller than n
 */
int findClosestPrime(int n)
{
    int i, j;

    // All prime numbers are odd except two
    if(!(n & 1)) {
        n--;
    }

    for(i = n; i >= 2; i -= 2) {
        if(i % 2 != 0) {
            j = 3;
            while(j <= sqrt(i) && i % j != 0) {
                j += 2;
            }
            if(j > sqrt(i))
                return i;
        }
    }

    // It will only be executed when n is 3
    return 2;
}

/* Hashes given string to obtain integer key.
 * This hash function is created by Dan Bernstein.
 *
 * @return calculated integer hash value for the string
 */
unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c

    return hash;
}

/* Creates an hash table with the given size.
 *
 * @return pointer to the hash table on success,
 *         NULL pointer on memory allocation errors
 */
HashTable *createHashTable(int size)
{
    HashTable *table;
    int i;

    if((table = (HashTable *)malloc(sizeof(HashTable))) == NULL) {
        return NULL;
    }
    if((table->data = (HashNode **)malloc(size * sizeof(HashNode *))) == NULL) {
        free(table);
        return NULL;
    }
    // initialize hash table with NULL
    for(i = 0; i < size; i++) {
        table->data[i] = NULL;
    }
    table->size = size;
    table->itemCount = 0;

    return table;
}

/* Adds new node with the given properties to the hash table.
 * Name is turned into all lowercase before hashing.
 *
 * @param name - name of the node, name is used for hashing
 *
 * @return 0 if empty space found and node is not already in the table,
 *         E_NODE_PRESENT if node is already in the table
 */
int addToHashTable(HashTable *hashTable, const char *name)
{
    char nameLower[BUF_LEN];
    int index;
    int i;
    HashNode *node;

    // turn string to all lowercase before hashing
    strcpy(nameLower, name);
    strToLower(nameLower);
    index = hash((unsigned char *)nameLower) % hashTable->size;

    if((node = (HashNode *)malloc(sizeof(HashNode))) == NULL) {
        return E_MEM_ERR;
    }
    if(!isNodePresent(hashTable, name, &i)) {
        node->key = index;
        node->graphIndex = (hashTable->itemCount)++;
        strcpy(node->name, nameLower);
        hashTable->data[i] = node;

        return 0;
    } else {
        free(node);
        return E_NODE_PRESENT;
    }
}

/* Checks if node with the given name is in the hash table
 * Name comparison ignores case.
 *
 * @return 0 if node is not found,
 *         1 if node is found
 */
int isNodePresent(HashTable *hashTable, const char *name, int *loc)
{
    int index;
    char nameLower[BUF_LEN];
    int i;

    // turn string to all lowercase before finding index
    strcpy(nameLower, name);
    strToLower(nameLower);
    index = hash((unsigned char *)nameLower) % hashTable->size;
    i = index;

    // check hash table with linear probing until you see either a NULL node or the node you're searching for
    // NULL node means node is not present in the table
    while(hashTable->data[i] != NULL && strcasecmp(hashTable->data[i]->name, nameLower) != 0) {
        i = (i + 1) % hashTable->size;
    }
    *loc = i;
    if(hashTable->data[i] == NULL) {
        return 0;
    } else {
        return 1;
    }
}

/* Deallocs each node of the table and then the table itself.
 *
 * @return NULL to set the pointer in the calling function to NULL
 */
void *deleteHashTable(HashTable *hashTable)
{
    int i;

    for(i = 0; i < hashTable->size; i++) {
        free(hashTable->data[i]); // free node in the cell if available, otherwise no operation
    }
    free(hashTable);
    return NULL;
}

/* Searches the node with the given name in the table
 * Name comparison ignores case.
 *
 * @return pointer to the node if found,
 *         NULL pointer otherwise
 */
HashNode *getFromHashTable(HashTable *hashTable, const char *name)
{
    int i;

    if(isNodePresent(hashTable, name, &i)) {
        return hashTable->data[i];
    } else {
        return NULL;
    }
}

/*******************************
 *  Stack related functions
 *******************************/

// stack of Flight pointers
Stack *createStack(int vertexCount)
{
    Stack *tmp;
    Flight ***paths;
    int i;

    if((tmp = (Stack *)malloc(sizeof(Stack))) == NULL)
        return NULL;
    tmp->top = -1;
    if((paths = (Flight ***)malloc(PATH_LIMIT * sizeof(Flight **))) == NULL) {
        free(tmp);
        return NULL;
    }
    for(i = 0; i < PATH_LIMIT; i++) {
        if((paths[i] = (Flight **)malloc(vertexCount * sizeof(Flight *))) == NULL) {
            free(paths);
            free(tmp);
            return NULL;
        }
    }
    tmp->paths = paths;
    tmp->pathsNext = 0;

    return tmp;
}

int isStackEmpty(Stack *stack)
{
    return (stack->top == -1) ? 1 : 0;
}

int isStackFull(Stack *stack)
{
    return (stack->top == STACK_LEN - 1) ? 1 : 0;
}

Flight *pop(Stack *stack)
{
    if(isStackEmpty(stack)) {
        puts("Stack is empty, cannot pop.");
        return NULL;
    } else {
        (stack->top)--;
        return stack->arr[stack->top + 1];
    }
}

int push(Stack *stack, Flight *data)
{
    if(isStackFull(stack)) {
        puts("Stack is full, cannot push.");
        return -1;
    } else {
        (stack->top)++;
        stack->arr[stack->top] = data;
        return 0;
    }
}

/*******************************
 *  Main functions
 *******************************/

/* Creates a new flight with the given info.
 *
 * @return pointer to created Flight if no errors,
 *         NULL pointer in case of memory error
 */
Flight *createFlight(const char *dest, int durationMinutes, int price)
{
    Flight *flight;

    if((flight = (Flight *)malloc(sizeof(Flight))) == NULL) {
        return NULL;
    }
    strcpy(flight->dest, dest);
    flight->durationMinutes = durationMinutes;
    flight->price = price;
    flight->next = NULL;

    return flight;
}

/* Adds the city with the given name to hash table and to first node of flightAdj list if not already available.
 *
 * @return 0 if no errors,
 *         E_MEM_ERR in case of an error
 */
int addNewCity(HashTable *cities, Flight **flightAdj, char *name)
{
    Flight *flight;
    int loc;

    if(!isNodePresent(cities, name, &loc)) {
        if((flight = createFlight(name, 0, 0)) == NULL) {
            return E_MEM_ERR;
        }
        addToHashTable(cities, name);
        // first nodes of the flightAdj list contains info about that index's city
        // assign created city node to that index
        flightAdj[getFromHashTable(cities, name)->graphIndex] = flight;
    }
    return 0;
}

/* Adds flight with the given info to adjacency list of the city at insertIndex
 *
 * @return 0 if no errors,
 *         E_MEM_ERR in case of an error
 */
int addFlightToAdj(Flight **flightAdj, int insertIndex, char *dest, int durationMinutes, int price)
{
    Flight *flight;

    if((flight = createFlight(dest, durationMinutes, price)) == NULL) {
        return E_MEM_ERR;
    }
    flight->next = flightAdj[insertIndex]->next;
    flightAdj[insertIndex]->next = flight;

    return 0;
}

/* Reads flight infos from file and fills adjacency list with them.
 * This function also creates cities hash table from unique cities.
 *
 * @return 0 if no errors,
 *         E_FILE_ERR or E_MEM_ERR in case of an error
 */
int readFlights(const char *fileName, HashTable *cities, Flight **flightAdj)
{
    FILE *file;
    int isEOF = 0;

    if((file = fopen(fileName, "r")) == NULL) {
        return E_FILE_ERR;
    }

    while(!isEOF) {
        char src[NAME_LEN];
        char dest[NAME_LEN];
        int hours;
        int minutes;
        int price;

        if((isEOF = fscanf(file, "%49s %49s %d %d %d", src, dest, &hours, &minutes, &price)) != EOF) {
            int returnVal;
            int insertIndex;

            // create cities if they are not in the hash table
            if((returnVal = addNewCity(cities, flightAdj, src)) != 0) {
                fclose(file);
                return returnVal;
            }
            if((returnVal = addNewCity(cities, flightAdj, dest)) != 0) {
                fclose(file);
                return returnVal;
            }

            insertIndex = getFromHashTable(cities, dest)->graphIndex;
            if((returnVal = addFlightToAdj(flightAdj, insertIndex, src, hours * 60 + minutes, price)) != 0) {
                fclose(file);
                return returnVal;
            }
            insertIndex = getFromHashTable(cities, src)->graphIndex;
            if((returnVal = addFlightToAdj(flightAdj, insertIndex, dest, hours * 60 + minutes, price)) != 0) {
                fclose(file);
                return returnVal;
            }
            isEOF = 0;
        } else {
            isEOF = 1;
        }
    }

    fclose(file);
    return 0;
}

void printAdjList(Flight **flightAdj, int vertexCount)
{
    int i;

    printf("Format: [%-10s, %-5s, %-5s]\n", "city_name", "total_duration_minutes", "total_price");
    for(i = 0; i < vertexCount; i++) {
        Flight *trav;

        for(trav = flightAdj[i]; trav != NULL; trav = trav->next) {
            printf("\t[%-10s, %-5d, %-5d] -> ", trav->dest, trav->durationMinutes, trav->price);
        }
        printf("\n");
    }
}

/* Save found path to the paths matrix in the history stack.
 */
void addPath(Stack *history)
{
    int i;

    for(i = 0; i <= history->top; i++) {
        history->paths[history->pathsNext][i] = history->arr[i];
    }
    // set NULL the first empty cell after the last item
    history->paths[history->pathsNext][i] = NULL;
    (history->pathsNext)++;
}

/* Searches adjacency list for paths from source to dest.
 * Saves pointers to paths to paths matrix inside history stack.
 * Paths are limited by depthLimit.
 * Depth of root node is 0. When depth reaches depthLimit, dfs goes back to parent node.
 */
void findPathDfs(HashTable *cities,
    Flight **flightAdj,
    Flight *source,
    Stack *history,
    Flight *dest,
    int *visited,
    int depth,
    int depthLimit)
{
    if(source != NULL) {
        int graphIndex = getFromHashTable(cities, source->dest)->graphIndex;

        if(!visited[graphIndex]) {
            Flight *curr = source;

            // if current node is the root node, add it to the stack
            if(isStackEmpty(history)) {
                push(history, curr);
            }
            // if reached the destination, save path to stack go back to parent node
            if(curr == dest) {
                // append NULL to the history before adding to the path, like string termination '\0'
                push(history, NULL);
                addPath(history);
                pop(history);
                pop(history);
                return;
            }
            // return if current node is not destination node and depth limit is reached
            if(depth == depthLimit) {
                pop(history);
                return;
            }
            visited[graphIndex] = 1;

            // search unvisited adjacent nodes to the current node, visit them
            curr = curr->next;
            while(curr != NULL) {
                graphIndex = getFromHashTable(cities, curr->dest)->graphIndex;
                if(!visited[graphIndex]) {
                    push(history, curr);
                    findPathDfs(
                        cities, flightAdj, flightAdj[graphIndex], history, dest, visited, depth + 1, depthLimit);
                }
                curr = curr->next;
            }
            // remove current node from history before returning to parent node
            curr = pop(history);
            visited[getFromHashTable(cities, curr->dest)->graphIndex] = 0;
        }
    }
}

/*******************************
 *  Print paths sorted
 *******************************/

/* Calculates gain of each path according to the given criteria.
 * Lower gain is better when comparing two gains.
 * Each connecting flight adds extra minutes to the total gain (d criteria).
 *
 * @return 0 if all went well,
 *         E_INVALID_CRITERIA if given criteria is not valid
 */
int findPathGains(Stack *history, int *pathGains, char criteria)
{
    int i, j;

    // initialize pathGains
    for(i = 0; i < history->pathsNext; i++) {
        pathGains[i] = 0;
    }
    for(i = 0; i < history->pathsNext; i++) {
        // first column has info about source city, skip it
        for(j = 1; history->paths[i][j] != NULL; j++) {
            switch(criteria) {
            case 'd':
                pathGains[i] += history->paths[i][j]->durationMinutes + CONNECTING_FLIGHT_DELAY_M;
                break;
            case 'p':
                pathGains[i] += history->paths[i][j]->price;
                break;
            default:
                return E_INVALID_CRITERIA;
            }
        }
        if(criteria == 'd') {
            // subtract last added delay, because last node is the dest
            pathGains[i] -= CONNECTING_FLIGHT_DELAY_M;
        }
    }

    return 0;
}

/* Merges two sorted subarrays of paths by using additional pathGains array.
 *
 * @param l - starting index of the left subarray
 * @param q - ending index of the left subarray
 * @param r - starting index of the right subarray
 */
void merge(Flight ***paths, int *pathGains, int l, int q, int r)
{
    int i = l;
    int j = q + 1;
    int k = 0;
    int len = r - l + 1;

    Flight ***pathsSorted = (Flight ***)malloc(len * sizeof(Flight **));
    int *gainsSorted = (int *)malloc(len * sizeof(int));

    // Fill the both arrays by merging two arrays in sorted order
    while(i <= q && j <= r) {
        if(pathGains[i] < pathGains[j]) {
            pathsSorted[k] = paths[i];
            gainsSorted[k++] = pathGains[i++];
        } else {
            pathsSorted[k] = paths[j];
            gainsSorted[k++] = pathGains[j++];
        }
    }
    // Add any remaining values
    while(i <= q) {
        pathsSorted[k] = paths[i];
        gainsSorted[k++] = pathGains[i++];
    }
    while(j <= r) {
        pathsSorted[k] = paths[j];
        gainsSorted[k++] = pathGains[j++];
    }
    // Copy the data from tmp back to adverts array
    for(i = 0, k = l; i < len; i++, k++) {
        paths[k] = pathsSorted[i];
        pathGains[k] = gainsSorted[i];
    }

    free(pathsSorted);
    free(gainsSorted);
}

/* Sorts an array of paths in ascending order using merge sort.
 * Paths are compared by their gains. Gains are given in the pathGains array.
 * Check findPathGains function for more info on how to calculate gains.
 *
 * @param l - starting index of the subarray
 * @param r - ending index of the subarray
 */
void sortPaths(Flight ***paths, int *pathGains, int l, int r)
{
    if(l < r) {
        int q = l / 2 + r / 2;
        sortPaths(paths, pathGains, l, q);
        sortPaths(paths, pathGains, q + 1, r);
        merge(paths, pathGains, l, q, r);
    }
}

/* Prints pre-sorted all found paths to the screen in a table view.
 */
void printPathsTable(Stack *history, int *pathMainGains, char criteria, FILE *out)
{
    int *pathSideGains;
    int returnVal;
    int i, j;
    int hours, minutes, price;

    if(out == NULL) {
        printError(E_FILE_ERR);
        return;
    }
    // no flights found
    if(history->pathsNext == 0) {
        printError(E_NO_PATH_FOUND);
        return;
    }
    if((pathSideGains = malloc(history->pathsNext * sizeof(int))) == NULL) {
        printError(E_MEM_ERR);
        return;
    }
    
    // calculate gain values for side gain
    // if paths are sorted by duration, calculate price
    // if paths are sorted by price, calculate duration as side gains to print to the screen
    if((returnVal = findPathGains(history, pathSideGains, (criteria == 'd') ? 'p' : 'd')) != 0) {
        printError(returnVal);
        free(pathSideGains);
        return;
    }

    // print table header
    fprintf(out, "\t%-10s | %-15s | %-30s | %-10s | %-10s | %-10s\n", "Source", "Destination", "Stops", "Hours",
        "Minutes", "Price");
    fprintf(out, "\t%-.11s|%-.17s|%-.32s|%-.12s|%-.12s|%-.11s\n", DASHES, DASHES, DASHES, DASHES, DASHES, DASHES);
    for(i = 0; i < history->pathsNext; i++) {
        Flight **row = history->paths[i];
        char stops[BUF_LEN] = "\0";

        // print source
        fprintf(out, "\t%-10s |", row[0]->dest);

        // print stops
        for(j = 1; row[j + 1] != NULL; j++) {
            strcat(stops, row[j]->dest);
            // add the last stop without trailing comma
            if(row[j + 2] != NULL) {
                strcat(stops, ", ");
            }
        }
        // print destination
        fprintf(out, " %-15s |", row[j]->dest);
        fprintf(out, " %-30s |", stops);

        if(criteria == 'd') {
            hours = pathMainGains[i] / 60;
            minutes = pathMainGains[i] - 60 * hours;
            price = pathSideGains[i];
        } else if(criteria == 'p') {
            hours = pathSideGains[i] / 60;
            minutes = pathSideGains[i] - 60 * hours;
            price = pathMainGains[i];
        }
        fprintf(out, " %-10d |", hours);
        fprintf(out, " %-10d |", minutes);
        fprintf(out, " %-10d\n", price);
    }
    free(pathSideGains);
}