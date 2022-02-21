/* @author Erkan Vatan
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// user-defined error codes are between 64 and 113
enum errCodes {
    E_MEM_ERR = 64,
    E_FILE_ERR,
    E_MISSING_HEADER,
    E_EMPTY_PGM,
    E_BUF_OVERFLOW,
    E_PGM_CORRUPT,
    E_RLE_CORRUPT,
    E_PGM_COORDINATE,
};

#define H_LINE "-----------------------------------\n"
#define BUF_LEN 1000
#define ENCODE_PAIR_COUNT 10 // maximum number of (repetition, color) pairs in one line
#define PGM_LINE_LEN 70      // maximum number of characters in one PGM line
#define DECODED_PGM_FILE "test_decoded.pgm"
#define ENCODED_TXT_FILE "test_encoded.txt"
#define SPACE_DELIMITER " \f\n\r\t\v"

typedef struct PgmHeader {
    char magicNumber[BUF_LEN];
    int imgWidth;
    int imgHeight;
    int maxval;
} PgmHeader;

typedef struct RleNode {
    int repCount;
    int colorValue;
    struct RleNode *next;
    struct RleNode *prev;
} RleNode;

typedef struct RleList {
    PgmHeader header;
    RleNode *head;
    RleNode *tail;
} RleList;

int skipPgmHeaderBlanks(FILE *file);
int readPgmHeader(PgmHeader *header, FILE *file);
int encodePgm(PgmHeader *header, FILE *outputFile, FILE *inputFile);
int decodePgm(FILE *inputFile, FILE *outputFile);
RleList *loadEncodedPgm(FILE *inputFile);
void printRleList(RleList *list, FILE *outputFile);
void printRleListHistogram(RleList *list, FILE *outputFile);
RleNode *changeRleListNodeColor(RleNode *node, int swapWith, RleList *list);
int replaceRleListColor(int searchFor, int swapWith, RleList *list);
int changeRleListPixel(int row, int col, int swapWith, RleList *list);
RleNode *createRleNode(int repCount, int colorValue);
void freeRleList(RleNode *tail);
void printError(enum errCodes errCode);
void flushStdin(void);

int main(void)
{
    char inputFileName[BUF_LEN];
    FILE *inputFile, *outputFile;
    RleList *rleList;
    PgmHeader header;
    int errCode;
    int programContinues = 1;
    int searchFor, swapWith; // find and replace color
    int rowIndex, colIndex;  // coordinates of the pixel
    int affectedBlockCount;

    // MAIN MENU
    while(programContinues) {
        char choice;

        // main menu choices
        printf("-------------MAIN-MENU-------------\n");
        printf("[a] encode PGM file and write to '%s'\n", ENCODED_TXT_FILE);
        printf("[s] decode '%s' and write to '%s'\n", ENCODED_TXT_FILE, DECODED_PGM_FILE);
        printf("[d] find and replace all colors in '%s'\n", ENCODED_TXT_FILE);
        printf("[f] change a pixel's color in '%s'\n", ENCODED_TXT_FILE);
        printf("[g] calculate frequency of colors in '%s'\n", ENCODED_TXT_FILE);
        printf("[x] exit\n");
        printf(">>Choice: ");
        scanf("%c", &choice);
        choice = tolower(choice);
        flushStdin();
        printf("%s", H_LINE);
        switch(choice) {
        case 'a':
            // encode PGM file

            printf(">Relative path of the PGM file to open?: ");
            scanf("%s", inputFileName);
            flushStdin();

            if((inputFile = fopen(inputFileName, "r")) == NULL) {
                printError(E_FILE_ERR);
                break;
            }
            if((errCode = readPgmHeader(&header, inputFile))) {
                printError(errCode);
                fclose(inputFile);
                break;
            }
            // check the magic number
            if(strcmp(header.magicNumber, "P2") != 0 && strcmp(header.magicNumber, "P5") != 0) {
                printError(E_MISSING_HEADER);
                fclose(inputFile);
                break;
            }
            if((outputFile = fopen(ENCODED_TXT_FILE, "w")) == NULL) {
                printError(E_FILE_ERR);
                fclose(inputFile);
                break;
            }
            printf("%s", H_LINE);
            // run RLE on the input file and write results to output file
            if((errCode = encodePgm(&header, outputFile, inputFile))) {
                printError(errCode);
            }
            fclose(inputFile);
            fclose(outputFile);
            break;
        case 's':
            // decode encoded file and write to a new PGM file

            if((inputFile = fopen(ENCODED_TXT_FILE, "r")) == NULL) {
                printError(E_FILE_ERR);
                break;
            }

            if((outputFile = fopen(DECODED_PGM_FILE, "w")) == NULL) {
                printError(E_FILE_ERR);
                fclose(inputFile);
                break;
            }
            // decode the RLE encoded input file and write obtained PGM into output file
            if((errCode = decodePgm(inputFile, outputFile))) {
                printError(errCode);
            }
            fclose(inputFile);
            fclose(outputFile);
            break;
        case 'd':
            // find and replace all occurences of a color in encoded file

            if((inputFile = fopen(ENCODED_TXT_FILE, "r")) == NULL) {
                printError(E_FILE_ERR);
                break;
            }
            if(!(rleList = loadEncodedPgm(inputFile))) {
                fclose(inputFile);
                break;
            }
            fclose(inputFile);
            if((outputFile = fopen(ENCODED_TXT_FILE, "w")) == NULL) {
                printError(E_FILE_ERR);
                freeRleList(rleList->tail);
                free(rleList);
                break;
            }
            printf(">Search for?: ");
            scanf("%d", &searchFor);
            flushStdin();
            printf(">Replace with?: ");
            scanf("%d", &swapWith);
            flushStdin();

            affectedBlockCount = replaceRleListColor(searchFor, swapWith, rleList);
            printf("Affected block count: %d\n", affectedBlockCount);
            printRleList(rleList, outputFile);
            freeRleList(rleList->tail);
            free(rleList);
            fclose(outputFile);
            break;
        case 'f':
            // change color of the pixel with given coordinates to another color

            if((inputFile = fopen(ENCODED_TXT_FILE, "r")) == NULL) {
                printError(E_FILE_ERR);
                break;
            }
            if(!(rleList = loadEncodedPgm(inputFile))) {
                fclose(inputFile);
                break;
            }
            fclose(inputFile);
            if((outputFile = fopen(ENCODED_TXT_FILE, "w")) == NULL) {
                printError(E_FILE_ERR);
                freeRleList(rleList->tail);
                free(rleList);
                break;
            }
            printf(">Coordinates of the pixel (row column)?: ");
            scanf("%d %d", &rowIndex, &colIndex);
            flushStdin();
            printf(">Replace with?: ");
            scanf("%d", &swapWith);
            flushStdin();
            if(!(changeRleListPixel(rowIndex, colIndex, swapWith, rleList))) {
                printRleList(rleList, outputFile);
            }
            fclose(outputFile);
            freeRleList(rleList->tail);
            free(rleList);
            break;
        case 'g':
            // calculate frequency of each color in the encoded file

            if((inputFile = fopen(ENCODED_TXT_FILE, "r")) == NULL) {
                printError(E_FILE_ERR);
                break;
            }
            if(!(rleList = loadEncodedPgm(inputFile))) {
                fclose(inputFile);
                break;
            }
            fclose(inputFile);
            if((outputFile = fopen(ENCODED_TXT_FILE, "a")) == NULL) {
                printError(E_FILE_ERR);
                freeRleList(rleList->tail);
                free(rleList);
                break;
            }
            printRleListHistogram(rleList, outputFile);

            fclose(outputFile);
            freeRleList(rleList->tail);
            free(rleList);
            break;
        case 'x':
            // exit the program

            programContinues = 0;
            puts("Goodbye!");
            break;
        default:
            puts("error: Unknown menu option.");
            break;
        }
    }

    return 0;
}

/*
 * Skips whitespaces and whole comment lines until it comes across a digit or a char not in comment.
 *
 * @return 0 if all went well,
 *         E_MISSING_HEADER if encountered EOF
 */
int skipPgmHeaderBlanks(FILE *file)
{
    int c;

    // skip any number of comment lines
    c = fgetc(file);
    while(isspace(c) || c == '#') {
        if(c == '#') {
            while((c = fgetc(file)) != '\n' && c != EOF)
                ;
            if(c == EOF) {
                return E_MISSING_HEADER;
            }
        }
        c = fgetc(file);
    }
    if(c == EOF) {
        return E_MISSING_HEADER;
    }
    // unread the last char
    ungetc(c, file);
    return 0;
}

/*
 * Reads PGM file's header and fills the header struct.
 *
 * @return 0 if all went well,
 *         E_MISSING_HEADER if the header is missing some fields,
 *         E_EMPTY_PGM if after reading the header there are no more lines to read in the PGM,
 *         E_BUF_OVERFLOW if the magic number of the PGM is too long to fit into the string
 */
int readPgmHeader(PgmHeader *header, FILE *file)
{
    int c;
    char *buf = header->magicNumber;
    int i = 0;

    if(skipPgmHeaderBlanks(file)) {
        return E_MISSING_HEADER;
    }
    // read magic number until whitespace
    while(i < BUF_LEN && isalnum(c = fgetc(file))) {
        buf[i++] = c;
    }
    if(i == BUF_LEN) {
        return E_BUF_OVERFLOW;
    }
    buf[i] = '\0';
    if(c == EOF) {
        return E_MISSING_HEADER;
    }
    ungetc(c, file);
    strcpy(header->magicNumber, buf);

    if(skipPgmHeaderBlanks(file)) {
        return E_MISSING_HEADER;
    }
    // scan imgWidth and skip blanks
    if((fscanf(file, "%d", &header->imgWidth)) == EOF || skipPgmHeaderBlanks(file)) {
        return E_MISSING_HEADER;
    }
    // scan imgHeight and skip blanks
    if((fscanf(file, "%d", &header->imgHeight)) == EOF || skipPgmHeaderBlanks(file)) {
        return E_MISSING_HEADER;
    }
    // scan maxval and skip blanks
    if((fscanf(file, "%d", &header->maxval)) == EOF) {
        return E_MISSING_HEADER;
    }
    if(skipPgmHeaderBlanks(file)) {
        return E_EMPTY_PGM;
    }
    return 0;
}

/*
 * Encodes PGM file with Run Length Encoding method. Writes results to the output file and also to stdout.
 *
 * @return 0 if all went well,
 *         E_PGM_CORRUPT if PGM file is corrupt
 */
int encodePgm(PgmHeader *header, FILE *outputFile, FILE *inputFile)
{
    char line[BUF_LEN];
    int pixelIndex = 0;
    int pixelCount = header->imgHeight * header->imgWidth;
    int repCount = 0;
    int wordCount = 0;
    int repeatedColor = -1; // initialize with invalid color

    puts("Encoding started...");
    puts("Encoded file contents:");
    // write the header
    fprintf(outputFile, "%s\n%d %d\n%d\n", header->magicNumber, header->imgWidth, header->imgHeight, header->maxval);
    printf("%s\n%d %d\n%d\n", header->magicNumber, header->imgWidth, header->imgHeight, header->maxval);
    while(!feof(inputFile) && pixelIndex < pixelCount) {
        char *token;

        // read the pixels field line by line and parse
        fgets(line, BUF_LEN, inputFile);
        token = strtok(line, SPACE_DELIMITER);
        while(token != NULL) {
            int i, len;

            // check for non-digits
            for(i = 0, len = strlen(token); i < len; i++) {
                if(!isdigit(token[i])) { // found a non-digit in the pixels field
                    // exit normally if the image is fully encoded
                    if(pixelIndex == pixelCount - 1) {
                        return 0;
                    } else {
                        return E_PGM_CORRUPT;
                    }
                }
            }
            int value = atoi(token);
            if(value == repeatedColor) {
                repCount++;
            } else {
                if(repeatedColor != -1) {
                    wordCount++;
                    fprintf(outputFile, "%d %d%c", repCount, repeatedColor,
                        ((wordCount % ENCODE_PAIR_COUNT) == 0) ? '\n' : ' ');
                    printf("%d %d%c", repCount, repeatedColor, ((wordCount % ENCODE_PAIR_COUNT) == 0) ? '\n' : ' ');
                }
                repeatedColor = value;
                repCount = 1;
            }
            pixelIndex++;
            token = strtok(NULL, SPACE_DELIMITER);
        }
    }
    fprintf(outputFile, "%d %d ", repCount, repeatedColor); // write the last values
    printf("%d %d \n", repCount, repeatedColor);            // write the last values
    if(pixelIndex != pixelCount) {
        return E_PGM_CORRUPT;
    } else {
        puts("\nEncoding completed.");
        return 0;
    }
}

/*
 * Decodes given RLE encoded input file and writes the obtained PGM into output file.
 * It checks header integrity, color value range, encoding errors and total pixel count of the input file.
 *
 * @return 0 if all went well,
 *         E_MISSING_HEADER if header is missing some fields or they are wrong,
 *         E_RLE_CORRUPT if the encoded file does not obey the RLE encoding rules,
 *         E_EMPTY_PGM if after reading the header there are no more lines to read in the file,
 *         E_BUF_OVERFLOW if the magic number of the PGM is too long to fit into the string
 */
int decodePgm(FILE *inputFile, FILE *outputFile)
{
    PgmHeader header;
    int errCode;
    int pixelIndex = 0, pixelCount;
    int prevColorValue = -1; // initialize with a invalid color
    int prevRepCount = 0;
    int lineCharCount = 0;
    char line[BUF_LEN];

    puts("Decoding the file...");
    // fill the header and check for missing header fields
    if((errCode = readPgmHeader(&header, inputFile))) {
        puts("Check#1 FAILED");
        return errCode;
    }
    // check magic number
    if((strcmp(header.magicNumber, "P2") && strcmp(header.magicNumber, "P5"))) {
        puts("Check#1 FAILED");
        return E_MISSING_HEADER;
    }
    printf("PGM Headers:\n");
    printf(" Magic Number: %s\n", header.magicNumber);
    printf(" Img Width: %d\n", header.imgWidth);
    printf(" Img Height: %d\n", header.imgHeight);
    printf(" Maxval: %d\n", header.maxval);
    puts("Check#1 PASSED: All header fields are correct.");

    pixelCount = header.imgHeight * header.imgWidth;
    // write the header
    fprintf(outputFile, "%s\n%d %d\n%d\n", header.magicNumber, header.imgWidth, header.imgHeight, header.maxval);
    while(!feof(inputFile) && pixelIndex < pixelCount) {
        char *token;

        // read the pixels field line by line and parse
        fgets(line, BUF_LEN, inputFile);

        token = strtok(line, SPACE_DELIMITER);
        while(token != NULL) {
            int repCount, colorValue;

            repCount = atoi(token);
            pixelIndex += repCount;
            token = strtok(NULL, SPACE_DELIMITER);
            if(token != NULL) {
                colorValue = atoi(token);
                int colorCharWidth = strlen(token) + 1; // add the color char width + a space
                int i;

                // check range of the color value
                if(colorValue < 0 || colorValue > header.maxval) {
                    printf("Check#2 FAILED: Color values are not in the range 0-%d.\n", header.maxval);
                    printf("                Found a color with value=%d.\n", colorValue);
                    return E_RLE_CORRUPT;
                }
                // check for consecutive same value
                if(colorValue == prevColorValue) {
                    printf("Check#3 FAILED: Encoded file has consecutive same value.\n");
                    printf("                Found (%d, %d) and (%d, %d) in a row.\n", prevRepCount, prevColorValue,
                        repCount, colorValue);
                    return E_RLE_CORRUPT;
                }

                for(i = 0; i < repCount; i++) {
                    lineCharCount += colorCharWidth;
                    if(lineCharCount >= PGM_LINE_LEN) {
                        fprintf(outputFile, "\n");
                        lineCharCount = colorCharWidth;
                    }
                    fprintf(outputFile, "%d ", colorValue);
                }
                prevRepCount = repCount;
                prevColorValue = colorValue;
                token = strtok(NULL, SPACE_DELIMITER);
            } else {
                return E_RLE_CORRUPT;
            }
        }
    }
    printf("Check#2 PASSED: Color values are in the range 0-%d.\n", header.maxval);
    printf("Check#3 PASSED: Encoded file does not contain consecutive same value.\n");
    if(pixelIndex != pixelCount) {
        printf("Check#4 FAILED: Pixel count in the encoded file does not match the header.\n");
        printf("                Header: %d * %d = %d\n", header.imgHeight, header.imgWidth, pixelCount);
        printf("                Encoded File: %d total pixels\n", pixelIndex);
        return E_RLE_CORRUPT;
    } else {
        printf("Check#4 PASSED: Pixel count in the encoded file matches the header.\n");
        printf("                Header: %d * %d = %d total pixels\n", header.imgHeight, header.imgWidth, pixelCount);
        printf("                Encoded File: %d total pixels\n", pixelIndex);
        puts("Decoding completed, PGM file is ready.");
        return 0;
    }
}

/*
 * Loads encoded PGM file into a doubly linked list. Each node contains repetition count and color value.
 *
 * @return pointer to the malloc'd list or returns NULL if an error occurs.
 */
RleList *loadEncodedPgm(FILE *inputFile)
{
    PgmHeader *header;
    RleList *list;
    int errCode;
    int pixelCount;
    int pixelIndex = 0;
    char line[BUF_LEN];

    if(!(list = (RleList *)malloc(sizeof(RleList)))) {
        printError(E_MEM_ERR);
        return NULL;
    }
    // initialize the list
    list->head = list->tail = NULL;
    header = &(list->header);
    // fill the header and check for missing header fields
    if((errCode = readPgmHeader(header, inputFile))) {
        printError(errCode);
        free(list);
        return NULL;
    }
    // check magic number
    if((strcmp(header->magicNumber, "P2") && strcmp(header->magicNumber, "P5"))) {
        printError(E_MISSING_HEADER);
        free(list);
        return NULL;
    }
    pixelCount = header->imgHeight * header->imgWidth;
    while(!feof(inputFile) && pixelIndex < pixelCount) {
        char *token;

        // read the pixels field line by line and parse
        fgets(line, BUF_LEN, inputFile);

        token = strtok(line, SPACE_DELIMITER);
        while(token != NULL) {
            int repCount, colorValue;

            repCount = atoi(token);
            pixelIndex += repCount;
            token = strtok(NULL, SPACE_DELIMITER);
            if(token != NULL) {
                RleNode *newNode;
                colorValue = atoi(token);

                // check range of the color value
                if(colorValue < 0 || colorValue > header->maxval) {
                    printError(E_RLE_CORRUPT);
                    free(list->tail);
                    free(list);
                    return NULL;
                }
                if(!(newNode = createRleNode(repCount, colorValue))) {
                    printError(E_MEM_ERR);
                    free(list->tail);
                    free(list);
                    return NULL;
                }
                if(list->head != NULL) {
                    list->tail->next = newNode;
                    newNode->prev = list->tail;
                    list->tail = newNode;
                } else {
                    list->head = list->tail = newNode;
                }

                token = strtok(NULL, SPACE_DELIMITER);
            } else {
                printError(E_RLE_CORRUPT);
                free(list->tail);
                free(list);
                return NULL;
            }
        }
    }
    if(pixelIndex != pixelCount) {
        printError(E_RLE_CORRUPT);
        free(list->tail);
        free(list);
        return NULL;
    } else {
        return list;
    }
}

/*
 * Takes a filled RleList and writes it in encoded format to the output file.
 * Results are also written to stdout.
 * Caller must ensure the integrity of RleList.
 */
void printRleList(RleList *list, FILE *outputFile)
{
    RleNode *trav;
    int wordCount = 0;

    // write the header
    fprintf(outputFile, "%s\n%d %d\n%d\n", list->header.magicNumber, list->header.imgWidth, list->header.imgHeight,
        list->header.maxval);
    printf("%s\n%d %d\n%d\n", list->header.magicNumber, list->header.imgWidth, list->header.imgHeight,
        list->header.maxval);

    for(trav = list->head; trav != NULL; trav = trav->next) {
        wordCount++;
        // write the node's contents
        fprintf(outputFile, "%d %d%c", trav->repCount, trav->colorValue,
            ((wordCount % ENCODE_PAIR_COUNT) == 0) ? '\n' : ' ');
        printf("%d %d%c", trav->repCount, trav->colorValue, ((wordCount % ENCODE_PAIR_COUNT) == 0) ? '\n' : ' ');
    }
    printf("\n");
}

/*
 * Takes a filled RleList and calculates frequency of every color in it. Writes results to the output file.
 * Results are also written to stdout.
 * Caller must ensure the integrity of RleList.
 */
void printRleListHistogram(RleList *list, FILE *outputFile)
{
    RleNode *trav;
    RleNode *hist;
    int i, j;
    int len = list->header.maxval + 1;

    if(!(hist = (RleNode *)malloc(len * sizeof(RleNode)))) {
        printError(E_MEM_ERR);
        return;
    }
    // initialize the frequency list
    for(i = 0; i < len; i++) {
        hist[i].colorValue = i;
        hist[i].repCount = 0;
    }
    // fill the frequency values
    for(trav = list->head; trav != NULL; trav = trav->next) {
        // write the node's contents
        hist[trav->colorValue].repCount += trav->repCount;
    }
    // sort frequencies in descending order
    for(i = 0; i < len; i++) {
        int maxIndex = i;
        RleNode *maxVal = &(hist[i]);

        for(j = i; j < len; j++) {
            if(hist[j].repCount > maxVal->repCount) {
                maxIndex = j;
                maxVal = &(hist[j]);
            }
        }
        RleNode tmp;
        tmp.colorValue = hist[maxIndex].colorValue;
        tmp.repCount = hist[maxIndex].repCount;
        hist[maxIndex].colorValue = hist[i].colorValue;
        hist[maxIndex].repCount = hist[i].repCount;
        hist[i].colorValue = tmp.colorValue;
        hist[i].repCount = tmp.repCount;
    }
    // print non-zero values
    fprintf(outputFile,
        "\n\nFrequency of Colors\n"
        "-------------------\n");
    fprintf(outputFile, "Color Value\tFrequency\n");
    printf("\nFrequency of Colors\n"
           "-------------------\n");
    printf("Color Value\tFrequency\n");
    i = 0;
    while(i < len && hist[i].repCount != 0) {
        fprintf(outputFile, "%-10d\t%-10d\n", hist[i].colorValue, hist[i].repCount);
        printf("%-10d\t%-10d\n", hist[i].colorValue, hist[i].repCount);
        i++;
    }
    free(hist);
}

/*
 * Changes color of the given node to swapWith.
 * Does all of the needed merge operations to keep the file's RLE integrity.
 * Caller of this function must verify the integrity of the RleList prior to the call.
 *
 * @return pointer to the next node after the modified nodes
 */
RleNode *changeRleListNodeColor(RleNode *node, int swapWith, RleList *list)
{
    if(node->colorValue == swapWith) {
        return node->next;
    }
    RleNode *nextNode = node->next;
    if(node->prev != NULL && node->next != NULL && node->prev->colorValue == swapWith &&
        node->next->colorValue == swapWith) {
        // both the node before and the node after are the same with swapWith
        // join three nodes

        node->prev->next = node->next->next;
        // if at the end of the list
        if(node->next->next != NULL) {
            node->next->next->prev = node->prev;
        } else {
            list->tail = node->prev;
        }
        node->prev->repCount += node->repCount + node->next->repCount;
        // only keep previous node
        nextNode = node->prev->next;
        free(node->next);
        free(node);
    } else if(node->prev != NULL && node->prev->colorValue == swapWith) {
        // only node before is the same with swapWith
        // join two nodes
        node->prev->next = node->next;
        // if at the end of the list
        if(node->next != NULL) {
            node->next->prev = node->prev;
        } else {
            list->tail = node->prev;
        }
        node->prev->repCount += node->repCount;
        // only keep previous node
        nextNode = node->prev->next;
        free(node);
    } else if(node->next != NULL && node->next->colorValue == swapWith) {
        // only node after is the same with swapWith
        // join two nodes
        if(node->prev != NULL) {
            node->prev->next = node->next;
        } else {
            list->head = node->next;
        }
        node->next->prev = node->prev;
        node->next->repCount += node->repCount;
        // only keep next node
        nextNode = node->next->next;
        free(node);
    } else {
        // only change node's color
        node->colorValue = swapWith;
        nextNode = node->next;
    }
    return nextNode;
}

/*
 * Finds and replaces all occurences of a color with another color in the decoded file.
 * Caller of this function must verify the integrity of the RleList prior to the call.
 *
 * @return how many occurences found and replaced
 */
int replaceRleListColor(int searchFor, int swapWith, RleList *list)
{
    RleNode *trav = list->head;
    int counter = 0;

    if(searchFor < 0 || searchFor > list->header.maxval) {
        return counter;
    }
    if(swapWith < 0 || swapWith > list->header.maxval) {
        printf("error: Cannot swap with the value %d.\n", swapWith);
        printf("       Values must be in the range 0-%d.\n", list->header.maxval);
        return counter;
    }
    // no operation needed if these are equal
    if(searchFor == swapWith) {
        return counter;
    }
    while(trav != NULL) {
        if(trav->colorValue == searchFor) {
            counter++;
            trav = changeRleListNodeColor(trav, swapWith, list);
        } else {
            trav = trav->next;
        }
    }
    return counter;
}

/*
 * Finds the color with the given coordinates, and changes it to another value.
 * Caller of this function must verify the integrity of the RleList prior to the call.
 *
 * @return 0 if all went well,
 *         E_PGM_COORDINATE if given coordinates are out of the PGM's range,
 *         E_MEM_ERR if malloc failed
 */
int changeRleListPixel(int row, int col, int swapWith, RleList *list)
{
    int pixelIndex;
    int i;
    RleNode *trav;

    if(row < 0 || row >= list->header.imgHeight || col < 0 || col >= list->header.imgWidth) {
        printf("error: Coordinates (%d, %d) are out of the PGM's range.\n", row, col);
        printf("       PGM Height: %d\n", list->header.imgHeight);
        printf("       PGM Width: %d\n", list->header.imgWidth);
        return E_PGM_COORDINATE;
    }
    pixelIndex = row * list->header.imgWidth + col;

    trav = list->head;
    i = trav->repCount;
    while(i <= pixelIndex) {
        trav = trav->next;
        i += trav->repCount;
    }
    i -= trav->repCount;
    // (pixelIndex - i) gives us the position of the searched pixel in the Node

    if(trav->colorValue == swapWith) {
        // no operation needed if these are equal
        return 0;
    } else if(trav->repCount == 1) {
        changeRleListNodeColor(trav, swapWith, list);
    } else {
        // node has more than 1 rep count
        int pixelPosition = pixelIndex - i;

        if(pixelPosition == 0) {
            // at the start of the node
            if(trav->prev != NULL && trav->prev->colorValue == swapWith) {
                // color of the node before is the same with the swapWith
                (trav->prev->repCount)++;
                (trav->repCount)--;
            } else {
                RleNode *node;

                if(!(node = createRleNode(1, swapWith))) {
                    printError(E_MEM_ERR);
                    return E_MEM_ERR;
                }
                (trav->repCount)--;
                node->next = trav;
                node->prev = trav->prev;
                if(trav->prev != NULL) {
                    trav->prev->next = node;
                } else {
                    list->head = node;
                }
                trav->prev = node;
            }
        } else if(pixelPosition == trav->repCount - 1) {
            // at the end of the node

            if(trav->next != NULL && trav->next->colorValue == swapWith) {
                // color of the node after is the same with the swapWith
                (trav->next->repCount)++;
                (trav->repCount)--;
            } else {
                RleNode *node;

                if(!(node = createRleNode(1, swapWith))) {
                    printError(E_MEM_ERR);
                    return E_MEM_ERR;
                }
                (trav->repCount)--;
                node->next = trav->next;
                node->prev = trav;
                if(trav->next != NULL) {
                    trav->next->prev = node;
                } else {
                    list->tail = node;
                }
                trav->next = node;
            }
        } else {
            // at the middle of the node
            RleNode *node1, *node2;

            if(!(node1 = createRleNode(pixelPosition, trav->colorValue))) {
                printError(E_MEM_ERR);
                return E_MEM_ERR;
            }
            if(!(node2 = createRleNode(trav->repCount - pixelPosition - 1, trav->colorValue))) {
                printError(E_MEM_ERR);
                return E_MEM_ERR;
            }
            node1->prev = trav->prev;
            node1->next = trav;
            if(trav->prev != NULL) {
                trav->prev->next = node1;
            } else {
                list->head = node1;
            }
            node2->next = trav->next;
            node2->prev = trav;
            if(trav->next != NULL) {
                trav->next->prev = node2;
            } else {
                list->tail = node2;
            }
            trav->colorValue = swapWith;
            trav->repCount = 1;
            trav->prev = node1;
            trav->next = node2;
        }
    }

    return 0;
}

/*
 * Creates a new RleNode and initializes it to given values and defaults
 *
 * @return pointer to the malloc'd RleNode or returns NULL if an error occurs.
 */
RleNode *createRleNode(int repCount, int colorValue)
{
    RleNode *tmp;

    if(!(tmp = (RleNode *)malloc(sizeof(RleNode)))) {
        return NULL;
    }
    tmp->repCount = repCount;
    tmp->colorValue = colorValue;
    tmp->next = NULL;
    tmp->prev = NULL;
    return tmp;
}

/*
 * Frees linked list inside RleList
 */
void freeRleList(RleNode *tail)
{
    while(tail != NULL) {
        RleNode *tmp = tail;
        tail = tail->prev;
        free(tmp);
    }
}

/* Utility function to print error messages when given an error code   */
void printError(enum errCodes errCode)
{
    printf("%s", (errCode) ? "error: " : "");

    switch(errCode) {
    case E_MEM_ERR:
        puts("Memory is full.");
        break;
    case E_FILE_ERR:
        puts("Could not open the specified file.");
        break;
    case E_MISSING_HEADER:
        puts("File is missing some header fields.");
        break;
    case E_EMPTY_PGM:
        puts("File has no image data in it.");
        break;
    case E_BUF_OVERFLOW:
        puts("Encountered a string with unexpectedly long length.");
        break;
    case E_PGM_CORRUPT:
        puts("PGM file is corrupt, cannot read.");
        break;
    case E_RLE_CORRUPT:
        puts("RLE encoded file is corrupt, cannot read.");
        break;
    case E_PGM_COORDINATE:
        puts("error: Given coordinates are out of the PGM's range.");
        break;
    }
}

/* Utility function to remove the unwanted characters left in the stdin */
void flushStdin(void)
{
    int c;
    while((c = getchar()) != '\n' && c != EOF)
        ;
}
