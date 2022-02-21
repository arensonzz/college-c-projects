#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEN 50
#define MAX_COLUMN 20
#define BUF_LEN 500
#define FILE_ERR -2
#define MEM_ERR -3
#define NO_INSTRUCTOR -1
#define COLUMN_WIDTH 15

#define STUDENTS_FILE "students.txt"
#define INSTRUCTORS_FILE "instructors.txt"
#define COURSES_FILE "courses.txt"
#define STUDENT_TO_COURSE_FILE "studentToCourse.txt"
#define CLASS_LIST_FILE "_SINIFLISTESI.txt"

#define STUDENT_IDS_FILE "ID_student.txt"
#define INSTRUCTOR_IDS_FILE "ID_instructor.txt"
#define STUDENT_TO_COURSE_IDS_FILE "ID_studentToCourse.txt"

/* Helper functions */
char **allocCharMatrix(int nrows, int ncolumns);
void deallocCharMatrix(char **mat);
int getNextId(const char *fileName);
void pushId(int id, const char *fileName);
long getIdLocation(const char *fileName, const char *needle);
void inputString(const char *prompt, char *str);
void flushStdin(void);
void uppercaseStr(char *str);
long getFileLength(const char *fileName);
int getColumnCount(const char *fileName);
char *selectFromRow(const char *row, int columnIndex, char *resultStr);
void printFormattedRow(const char *row, FILE *outputStream);
void printColumnNames(FILE *outputStream, int argCount, ...);
void deleteFiles(void);

/* Generic functions */
int insertIntoTable(const char *fileName, void (*getStr)(char *));
int deleteFromTable(const char *fileName, const char *rowId);
int updateTable(const char *fileName, const char *rowId, int argCount, ...);
int updateTableWithArray(const char *fileName, const char *rowId, int len, int *columnIndices, char **columnValues);
void userUpdatesTable(const char *fileName, const char *rowId);
char *selectFromTable(const char *fileName, const char *rowId, int columnIndex, char *resultStr);
int listJoinTwoTables(const char *srcTableName,
    const char *destTableName,
    int srcIndex,
    int destIndex,
    const char *sourceId,
    FILE *outputStream);
int listJoinThreeTables(const char *srcTableName,
    const char *midTableName,
    const char *destTableName,
    int srcIndex,
    int midToSrcIndex,
    int midToDestIndex,
    int destIndex,
    const char *sourceId,
    FILE *outputStream);
void listStudentsInCourseOfInstructor(int instructorIdNum);

/* Specific functions */
// Student
void getStudentStr(char *str);
void deleteStudent(int idNum);
int updateStudentForCourse(int stdId, const char *courseCode, int sign);
int isStudentEnrolled(int id, const char *courseCode, long *rowLoc);
void enrollStudentInCourse(int studentId, const char *courseCode, int courseCountLimit, int totalCreditLimit);
void disenrollStudentFromCourse(int studentIdNum, const char *courseCode);
// Instructor
void getInstructorStr(char *str);
void deleteInstructor(int idNum);
void addInstructorToCourse(int instructorId, const char *courseCode);
void removeInstructorFromCourses(int idNum);
// Course
void getCourseStr(char *str);
void deleteCourse(const char *courseCode);
void removeCourseInstructor(const char *courseCode);

typedef struct Student {
    int id; // primary key
    int courseCount;
    int totalCredit;
    char name[LEN];
    char surname[LEN];
} Student;

typedef struct Instructor {
    int id; // primary key
    char name[LEN];
    char surname[LEN];
    char rank[LEN];
} Instructor;

typedef struct Course {
    char courseCode[LEN]; // primary key
    int instructorId;     // foreign key
    int credit;
    int capacity;
    char name[LEN];
} Course;

typedef struct StudentToCourse {
    int id;               // primary key
    int studentId;        // foreign key
    char courseCode[LEN]; // foreign key
    char dateCreated[LEN];
    int enrollState;
} StudentToCourse;

int main(void)
{
    int courseCountLimit; // Maximum number of courses one student can enroll in
    int totalCreditLimit; // Maximum number of credits one student can enroll in
    int stayInMenu = 1;

    // Create files if they don't exist
    fclose(fopen(STUDENTS_FILE, "a"));
    fclose(fopen(COURSES_FILE, "a"));
    fclose(fopen(INSTRUCTORS_FILE, "a"));
    fclose(fopen(STUDENT_TO_COURSE_FILE, "a"));



    printf("Welcome to MyCollegeSQL...\n"
           "-----------------------\n");
    printf("Enter maximum number of courses one student can take: ");
    scanf("%d", &courseCountLimit);
    printf("Enter maximum credits one student can take          : ");
    scanf("%d", &totalCreditLimit);
    flushStdin();
    printf("-----------------------\n");
    while(stayInMenu) {
        char ch1;
        int stayInQuery = 1;

        printf("[a] Instructor queries\n"
               "[s] Course queries\n"
               "[d] Student queries\n"
               "[f] Delete all of the created files\n"
               "[x] Exit\n");
        printf("Choice: ");
        scanf("%c", &ch1);
        flushStdin();
        ch1 = tolower(ch1);
        printf("-----------------------\n");
        switch(ch1) {
        case 'a':
            while(stayInQuery) {
                char ch2;
                int instructorId;
                char instructorIdStr[LEN];
                char courseCode[LEN];

                printf("[a] Insert a new instructor\n"
                       "[s] Delete an existing instructor\n"
                       "[d] Assign an instructor to a course\n"
                       "[f] Remove an instructor from all courses\n"
                       "[g] Update an existing instructor\n"
                       "[h] List an instructor's courses\n"
                       "[j] Create log of all enrolled students of an instructor's one course\n"
                       "[x] Go to main menu\n");
                printf("Choice: ");
                scanf("%c", &ch2);
                flushStdin();
                ch2 = tolower(ch2);
                printf("-----------------------\n");

                if(ch2 == 's' || ch2 == 'd' || ch2 == 'f' || ch2 == 'g' || ch2 == 'h' || ch2 == 'j') {
                    printf("Enter the instructor's ID: ");
                    scanf("%d", &instructorId);
                    flushStdin();
                    sprintf(instructorIdStr, "%d", instructorId);
                }
                switch(ch2) {
                case 'a':
                    insertIntoTable(INSTRUCTORS_FILE, getInstructorStr);
                    break;
                case 's':
                    deleteInstructor(instructorId);
                    break;
                case 'd':
                    inputString("Enter the course code    : ", courseCode);
                    uppercaseStr(courseCode);
                    addInstructorToCourse(instructorId, courseCode);
                    break;
                case 'f':
                    removeInstructorFromCourses(instructorId);
                    break;
                case 'g':
                    userUpdatesTable(INSTRUCTORS_FILE, instructorIdStr);
                    break;
                case 'h':
                    printColumnNames(stdout, 5, "Course Code", "Instructor ID", "Credit", "Capacity", "Name");
                    listJoinTwoTables(INSTRUCTORS_FILE, COURSES_FILE, 1, 2, instructorIdStr, stdout);
                    break;
                case 'j':
                    listStudentsInCourseOfInstructor(instructorId);
                    break;
                case 'x':
                    stayInQuery = 0;
                    break;
                default:
                    printf("!! Unknown command, try again. !!\n");
                    break;
                }
                if(ch2 != 'x') {
                    printf("-----------------------\n");
                }
            }
            break;
        case 's':
            while(stayInQuery) {
                char ch2;
                char courseCode[LEN];

                printf("[a] Insert a new course\n"
                       "[s] Delete an existing course\n"
                       "[d] Remove a course's instructor\n"
                       "[f] Update an existing course\n"
                       "[g] List enrolled students of a course\n"
                       "[x] Go to main menu\n");
                printf("Choice: ");
                scanf("%c", &ch2);
                flushStdin();
                printf("-----------------------\n");
                if(ch2 == 's' || ch2 == 'd' || ch2 == 'f' || ch2 == 'g') {
                    inputString("Enter the course code: ", courseCode);
                    uppercaseStr(courseCode);
                }
                switch(ch2) {
                case 'a':
                    insertIntoTable(COURSES_FILE, getCourseStr);
                    break;
                case 's':
                    deleteCourse(courseCode);
                    break;
                case 'd':
                    removeCourseInstructor(courseCode);
                    break;
                case 'f':
                    userUpdatesTable(COURSES_FILE, courseCode);
                    break;
                case 'g':
                    printColumnNames(stdout, 5, "Id", "Course Count", "Total Credit", "Name", "Surname");
                    listJoinThreeTables(COURSES_FILE, STUDENT_TO_COURSE_FILE, STUDENTS_FILE, 1, 3, 2, 1, courseCode, stdout);
                    break;
                case 'x':
                    stayInQuery = 0;
                    break;
                default:
                    printf("!! Unknown command, try again. !!\n");
                    break;
                }
                if(ch2 != 'x') {
                    printf("-----------------------\n");
                }
            }
            break;
        case 'd':
            while(stayInQuery) {
                char ch2;
                int studentId;
                char studentIdStr[LEN];
                char courseCode[LEN];

                printf("[a] Insert new student\n"
                       "[s] Delete existing student\n"
                       "[d] Enroll student in course\n"
                       "[f] Disenroll student from course\n"
                       "[g] Update existing student\n"
                       "[h] List a student's courses\n"
                       "[x] Go to main menu\n");
                printf("Choice: ");
                scanf("%c", &ch2);
                flushStdin();
                printf("-----------------------\n");

                if(ch2 == 's' || ch2 == 'd' || ch2 == 'f' || ch2 == 'g' || ch2 == 'h') {
                    printf("Enter the student's ID: ");
                    scanf("%d", &studentId);
                    flushStdin();
                    sprintf(studentIdStr, "%d", studentId);
                }
                if(ch2 == 'd' || ch2 == 'f') {
                    inputString("Enter the course code : ", courseCode);
                    uppercaseStr(courseCode);
                }
                switch(ch2) {
                case 'a':
                    insertIntoTable(STUDENTS_FILE, getStudentStr);
                    break;
                case 's':
                    deleteStudent(studentId);
                    break;
                case 'd':
                    enrollStudentInCourse(studentId, courseCode, courseCountLimit, totalCreditLimit);
                    break;
                case 'f':
                    disenrollStudentFromCourse(studentId, courseCode);
                    break;
                case 'g':
                    userUpdatesTable(STUDENTS_FILE, studentIdStr);
                    break;
                case 'h':
                    printColumnNames(stdout, 5, "Course Code", "Instructor Id", "Credit", "Capacity", "Name");
                    listJoinThreeTables(STUDENTS_FILE, STUDENT_TO_COURSE_FILE, COURSES_FILE, 1, 2, 3, 1, studentIdStr, stdout);
                    break;
                case 'x':
                    stayInQuery = 0;
                    break;
                default:
                    printf("!! Unknown command, try again. !!\n");
                    break;
                }
                if(ch2 != 'x') {
                    printf("-----------------------\n");
                }
            }
            break;
        case 'f':
            deleteFiles();
            break;
        case 'x':
            stayInMenu = 0;
            break;
        default:
            printf("!! Unknown command, try again. !!\n");
            break;
        }
    }


    return 0;
}

/* Helper functions */
char **allocCharMatrix(int nrows, int ncolumns) {
    /* allocates continuous memory for 2d char array
     * On success returns the char **, otherwise frees all allocated blocks and returns NULL
     */
    int i;
    char **mat;
    char *t0;

    mat = malloc(nrows * sizeof(*mat));
    if(mat == NULL) return NULL;
    t0 = malloc(nrows * ncolumns * sizeof(*t0));
    if(t0 == NULL) {
        free(mat);
        return NULL;
    }
    for(i = 0; i < nrows; i++) {
        mat[i] = t0 + i * ncolumns;
    }
    return mat;
}

void deallocCharMatrix(char **mat)
{
    /* frees the memory allocated for 2d char array */
    free(mat[0]); // free the array holding data
    free(mat);
}

int getNextId(const char *fileName)
{
    /* id file format is like this:
     *  First line: next auto-increment id
     *  Second line: comma seperated list of numbers to use before using auto-increment
     * Everytime this function is called, it pops one number out of second line of idFile
     * until there are no numbers left.
     * If second line contains only comma then function returns next auto-increment id
     */
    FILE *file = fopen(fileName, "r+");
    if(file == NULL) { // id file doesn't exist
        file = fopen(fileName, "w");
        fprintf(file, "%s\n,", "2"); // write next available id to the file
        fclose(file);
        return 1;
    }
    char buf[BUF_LEN];
    int currentId;

    fgets(buf, BUF_LEN, file);
    sscanf(buf, "%d", &currentId);
    fgets(buf, BUF_LEN, file);
    if(strcmp(buf, ",") == 0) {
        rewind(file);
        fprintf(file, "%d", currentId + 1);
    } else {
        char *token;
        int prevId;

        token = strtok(buf, ",");
        prevId = atoi(token);
        fclose(file);
        file = fopen(fileName, "w");
        fprintf(file, "%d\n,", currentId);
        while((token = strtok(NULL, ",")) != NULL) {
            fprintf(file, "%s,", token);
        }
        currentId = prevId;
    }
    fclose(file);
    return currentId;
}

void pushId(int id, const char *fileName)
{
    /* Pushes unused ID's to corresponding ID files to reuse them later */
    FILE *file = fopen(fileName, "a");

    if(file == NULL) {
        printf("!! Couldn't open the '%s' file. Couldn't recover the unused ID. !!\n", fileName);
        return;
    }
    fprintf(file, ",%d", id); // append id
    fclose(file);
}

long getIdLocation(const char *fileName, const char *needle)
{
    /* On success returns starting location of the row with the given ID(needle) in given file,
     * or -1 if ID is not found.
     * Returns FILE_ERR if an error occurs.
     */
    char buf[BUF_LEN];
    char tmpBuf[BUF_LEN];
    FILE *file = fopen(fileName, "r");
    long loc;
    int i;

    if(file == NULL) return FILE_ERR;
    while(1) {
        loc = ftell(file);
        if(fgets(buf, BUF_LEN, file) == NULL) break;
        for(i = 0; buf[i] != ',' && buf[i] != '\n' && buf[i] != EOF; i++) { // only look at the first column
            tmpBuf[i] = buf[i];
        }
        tmpBuf[i] = '\0';
        if(strstr(tmpBuf, needle) != NULL) {
            // found the id
            fclose(file);
            return loc;
        }
    }
    fclose(file);
    return -1; // id not found
}

void flushStdin(void)
{
    /* Removes unwanted characters from the input buffer stdin */
    int c;
    while((c = getchar()) != '\n' && c != EOF)
        ;
}

void inputString(const char *prompt, char *str)
{
    /* Prompts user with the given prompt string, gets a string from user and stores it in str */
    printf("%s", prompt);
    fgets(str, LEN, stdin);
    str[strcspn(str, "\n")] = 0; // remove trailing newline
}

void uppercaseStr(char *str)
{
    /* Turns given string to all uppercase letters, skips non-letter parts */
    int i;
    for(i = 0; str[i] != '\0'; i++) {
        str[i] = isalpha(str[i]) ? toupper(str[i]) : str[i];
    }
}

long getFileLength(const char *fileName)
{
    /* On success returns total length of the given file, returns FILE_ERR if an error occurs */
    FILE *file = fopen(fileName, "r");
    long len;

    if(file == NULL) return FILE_ERR;
    fseek(file, 0L, SEEK_END);
    len = ftell(file);
    fclose(file);
    return len;
}

int getColumnCount(const char *fileName)
{
    /* On success returns number of columns of the given table. Returns FILE_ERR if an error occurs. */
    FILE *file = fopen(fileName, "r");
    char *token;
    char line[BUF_LEN];
    int count = 0;

    if(file == NULL) return FILE_ERR;

    if(fgets(line, BUF_LEN, file) == NULL) {
        fclose(file);
        return 0; // return 0 as column count if file is empty
    }
    token = strtok(line, ",");
    // if file is empty, return 0 as column count
    while(token != NULL) {
        count++;
        token = strtok(NULL, ",");
    }
    fclose(file);
    return count;
}

char *selectFromRow(const char *row, int columnIndex, char *resultStr)
{
    /* Takes a string that contains one row of any table as argument. Then it parses this string and
     * stores the columnIndex'th column at resultStr.
     * If an error occurs, function empties resultStr by assigning '\0' to its first character.
     */
    char *token;
    int curColumnIndex = 0;
    char line[BUF_LEN];

    strcpy(line, row);
    token = strtok(line, ",");
    while(token != NULL) {
        curColumnIndex++;
        if(curColumnIndex == columnIndex) {
            strcpy(resultStr, token);
            return resultStr;
        }
        token = strtok(NULL, ",");
    }
    resultStr[0] = '\0'; // empty result string to indicate failure
    return resultStr;
}

void printFormattedRow(const char *row, FILE *outputStream)
{
    /* Takes a string that contains one row of any table and prints it to the
     * stream pointed by FILE *outputStream with tabs between every column
     * outputStream must be opened in write or append mode.
     */
    char *token;
    char line[BUF_LEN];
    char str[MAX_COLUMN][LEN];
    int count = 0, i;

    strcpy(line, row);             // copy string to keep original unchanged
    line[strcspn(line, "\n")] = 0; // remove trailing newline

    token = strtok(line, ",");
    i = 0;
    while(token != NULL) {
        count++;
        strcpy(str[i], token);
        token = strtok(NULL, ",");
        i++;
    }
    for(i = 0; i < count; i++) {
        fprintf(outputStream, "%-*s\t", COLUMN_WIDTH, str[i]);
    }
    fprintf(outputStream, "\n");
}

void printColumnNames(FILE *outputStream, int argCount, ...)
{
    /* Takes variable number of arguments as input. Parses given arguments as strings
     * and prints them to the stream outputStream formatted as column headers.
     * It's used before listing any table's rows.
     */
    int i, j;
    va_list argList;
    char **argVector; // array of column names to print

    if((argVector = allocCharMatrix(argCount, LEN)) == NULL) {
        printf("!! Not enough memory (f printColumnNames) !!\n");
        return;
    }
    va_start(argList, argCount);
    // Get the values from argList
    for(i = 0; i < argCount; i++) {
        char *nextArg = va_arg(argList, char *);

        strcpy(argVector[i], nextArg);
    }
    va_end(argList);

    for(i = 0; i < argCount; i++) {
        fprintf(outputStream, "%-*s\t", COLUMN_WIDTH, argVector[i]);
    }
    fprintf(outputStream, "\n");
    for(i = 0; i < argCount; i++) {
        int dashCount = strlen(argVector[i]);
        char dashes[LEN];

        for(j = 0; j < dashCount; j++) {
            dashes[j] = '-';
        }
        dashes[j] = '\0';
        fprintf(outputStream, "%-*s\t", COLUMN_WIDTH, dashes);
    }
    fprintf(outputStream, "\n");
    // free the 2d array
    deallocCharMatrix(argVector);
}

void deleteFiles(void)
{
    int flag = 0;

    flag = remove(STUDENTS_FILE) ? 1 : flag;
    flag = remove(INSTRUCTORS_FILE) ? 1 : flag;
    flag = remove(COURSES_FILE) ? 1 : flag;
    flag = remove(STUDENT_TO_COURSE_FILE) ? 1 : flag;

    flag = remove(STUDENT_IDS_FILE) ? 1 : flag;
    flag = remove(INSTRUCTOR_IDS_FILE) ? 1 : flag;
    flag = remove(STUDENT_TO_COURSE_IDS_FILE) ? 1 : flag;
    if(flag) {
        printf("!! Couldn't delete some of the files. !!\n");
    }
}

/* Generic functions */
int insertIntoTable(const char *fileName, void (*getStr)(char *))
{
    /* On success inserts a row into its related entity table and returns 0.
     * Otherwise returns FILE_ERR, MEM_ERR or -1
     */
    char *buf;
    FILE *file = fopen(fileName, "a");
    int returnStatus = 0; // 0 means successful insert

    if(file == NULL) return FILE_ERR;
    if((buf = malloc(BUF_LEN * sizeof(*buf))) == NULL) {
        fclose(file);
        return MEM_ERR;
    }
    (*getStr)(buf);
    if(buf[0] != '\0') { // if getStr function didn't empty the buf then append
        fputs(buf, file);
    } else {
        returnStatus = -1;
    }
    free(buf);
    fclose(file);
    return returnStatus;
}

int deleteFromTable(const char *fileName, const char *rowId)
{
    /* On success deletes a row from its related entity table and returns 0.
     * Otherwise returns FILE_ERR, MEM_ERR or -1
     */
    char *text;
    long fileLen = getFileLength(fileName);
    long rowLoc;
    int returnStatus = 0; // 0 means successful delete

    FILE *file = fopen(fileName, "r");

    if(file == NULL) {
        printf("!! Couldn't open the '%s' file. !!\n", fileName);
        return FILE_ERR;
    }
    if((text = malloc((fileLen + 10) * sizeof(*text))) == NULL) {
        fclose(file);
        printf("!! Not enough memory (f deleteFromTable) !!\n");
        return MEM_ERR;
    }
    rowLoc = getIdLocation(fileName, rowId);
    if(rowLoc < 0) {
        printf("!! ID '%s' is not in the table '%s'. !!\n", rowId, fileName);
        returnStatus = -1;
    } else {
        long curLoc;
        char line[BUF_LEN];

        text[0] = '\0'; // initialize empty string
        curLoc = ftell(file);
        while(fgets(line, BUF_LEN, file) != NULL) {
            if(curLoc != rowLoc) { // skip the matching row
                strcat(text, line);
            }
            curLoc = ftell(file);
        }
        fclose(file);
        file = fopen(fileName, "w");
        fputs(text, file);
    }
    free(text);
    fclose(file);
    return returnStatus;
}

int updateTable(const char *fileName, const char *rowId, int argCount, ...)
{
    /* Arguments: name of the file to update, ID of the row to update,
     * number of variable length arguments, indices of the columns to be changed (ascending order),
     *  corresponding values to be inserted into columns in the same order with indices.
     * Usage: updateTable("students.txt", "4", 4, 3, 4, "20", "Sarah")
     *      update row with the ID 4 in students.txt, place "20" into 3rd column, place "Sarah" into 4th column
     *
     * This function only parses arguments and turns them into two arrays. Then calls updateTableWithArray.
     * Returns 0 if successful, otherwise returns MEM_ERR, -1 or returnStatus
     */
    va_list argList;
    int *columnIndices;
    char **columnValues;
    int i, j;
    int returnStatus = 0;
    int updateCount = argCount / 2;

    if(argCount % 2 != 0) return -1; // argCount must be an even number

    if((columnIndices = malloc(updateCount * sizeof(*columnIndices))) == NULL) {
        printf("!! Not enough memory (f updateTable) !!\n");
        return MEM_ERR;
    }
    if((columnValues = allocCharMatrix(updateCount, LEN)) == NULL) {
        printf("!! Not enough memory (f updateTable) !!\n");
        free(columnIndices);
        return MEM_ERR;
    }

    va_start(argList, argCount);
    // Get the indices from argList
    for(i = 0; i < argCount / 2; i++) {
        columnIndices[i] = va_arg(argList, int);
    }
    // Get the values from argList
    for(j = 0; i < argCount; i++, j++) {
        char *nextArg = va_arg(argList, char *);

        strcpy(columnValues[j], nextArg);
    }
    va_end(argList);

    // Pass parsed argument arrays to other function
    returnStatus = updateTableWithArray(fileName, rowId, updateCount, columnIndices, columnValues);

    free(columnIndices);
    deallocCharMatrix(columnValues);
    return returnStatus;
}

int updateTableWithArray(const char *fileName, const char *rowId, int len, int *columnIndices, char **columnValues)
{
    /* Takes as arguments, columnIndices array containing indices of the columns to update and columnValues
     * array containing new values to insert in the same order as indices. Then applies the changes.
     * Returns 0 if successful, otherwise returns FILE_ERR, MEM_ERR or -1
     */

    int i, p, columnCount;
    FILE *file;
    char replacementLine[BUF_LEN]; // one line read from file
    long idLoc, curLoc;
    char buf[BUF_LEN];
    char *text; // string containing file's whole lines
    char *token;
    long fileLen = getFileLength(fileName);

    idLoc = getIdLocation(fileName, rowId);
    if((text = malloc((fileLen + 10) * sizeof(*text))) == NULL) {
        printf("!! Not enough memory (f updateTableWithArray) !!\n");
        return MEM_ERR;
    }
    if(idLoc < 0) {
        printf("!! There isn't any row with the ID '%s' in the table '%s'. !!\n", rowId, fileName);
        free(text);
        return -1;
    }
    columnCount = getColumnCount(fileName);
    if(columnCount == 0) {
        printf("!! Table '%s' is empty. !!\n", fileName);
    } else if(columnCount < len) {
        printf("!! There are %d columns in the table '%s', you tried to update %d columns. !!\n",
            columnCount, fileName, len);
        free(text);
        return -1;
    }
    file = fopen(fileName, "r");
    if(file == NULL) {
        printf("!! Couldn't open the file '%s' !!\n", fileName);
        free(text);
        return FILE_ERR;
    }
    fseek(file, idLoc, SEEK_SET);
    fgets(buf, BUF_LEN, file);
    token = strtok(buf, ",");

    p = 0;
    i = 1;
    replacementLine[0] = '\0'; // empty the line
    while(token != NULL) {
        if(p < len && i == columnIndices[p]) {
            strcat(replacementLine, columnValues[p]);
            p++;
        } else {
            strcat(replacementLine, token);
        }
        token = strtok(NULL, ",");
        if(token != NULL) {
            strcat(replacementLine, ",");
        } else { // if we overwrote last item in the row
            p--;
            if(p < len && p >= 0 && i == columnIndices[p]) { // Check if array is in bounds
                strcat(replacementLine, "\n");
            }
        }
        i++;
    }
    fseek(file, 0L, SEEK_SET);

    text[0] = '\0'; // initialize empty string
    curLoc = ftell(file);
    while(fgets(buf, BUF_LEN, file) != NULL) {
        if(curLoc != idLoc) {
            strcat(text, buf);
        } else { // line to replace
            strcat(text, replacementLine);
        }
        curLoc = ftell(file);
    }
    fclose(file);
    file = fopen(fileName, "w");
    fputs(text, file);
    free(text);
    fclose(file);
    return 0;
}

void userUpdatesTable(const char *fileName, const char *rowId)
{
    /* Gets index-'new value' pairs from user and by calling updateTableWith array function
     * updates given table.
     */
    int updateCount, i, p;
    int columnCount;
    int len;
    int *columnIndices;
    char **columnValues;

    if(getIdLocation(fileName, rowId) < 0) {
        printf("!! There isn't any row with the ID '%s' in the table '%s'. !!\n", rowId, fileName);
        return;
    }
    printf("Enter how many columns do you want to update: ");
    scanf("%d", &updateCount);
    columnCount = getColumnCount(fileName);
    if(columnCount < updateCount) {
        printf("!! There are %d columns in the table '%s', you tried to update %d columns. !!\n",
            columnCount, fileName, updateCount);
        return;
    }
    len = updateCount; // length for loop
    if(updateCount <= 0) return;

    if((columnIndices = malloc(updateCount * sizeof(*columnIndices))) == NULL) {
        printf("!! Not enough memory (f userUpdatesTable) !!\n");
        return;
    }
    if((columnValues = allocCharMatrix(updateCount, LEN)) == NULL) {
        printf("!! Not enough memory (f userUpdatesTable) !!\n");
        free(columnIndices);
        return;
    }

    printf("Enter 'column index' - 'new value' pairs (indices start at 1)\n");
    for(i = 0, p = 0; i < len; i++) {
        int ix;

        printf("Index    : ");
        scanf("%d", &columnIndices[p]);
        flushStdin();
        ix = columnIndices[p];
        // Check for the illegal columns, if given column is illegal than exclude it from update
        if((strcmp(fileName, STUDENTS_FILE) == 0 && (ix == 1 || ix == 2 || ix == 3)) ||
            (strcmp(fileName, COURSES_FILE) == 0 && (ix == 1 || ix == 2 || ix == 3)) ||
            (strcmp(fileName, INSTRUCTORS_FILE) == 0 && (ix == 1))) {
            printf("!! You can't update column %d of the table '%s' using this menu. !!\n", ix, fileName);
            updateCount--; // decrease column count because user entered one column index wrong
        } else {
            inputString("New value: ", columnValues[p]);
            p++;
        }
    }
    updateTableWithArray(fileName, rowId, updateCount, columnIndices, columnValues);
    deallocCharMatrix(columnValues);
    free(columnIndices);
}

char *selectFromTable(const char *fileName, const char *rowId, int columnIndex, char *resultStr)
{
    /* On success, selects given column from the table and stores the result in resultStr.
     * At the same time returns this resultStr for other functions to test return value.
     */
    char buf[BUF_LEN];
    long rowLoc;
    char *token;
    int found = 0;
    int i;
    FILE *file = fopen(fileName, "r");

    resultStr[0] = '\0'; // empty resultStr to indicate failure
    if(file == NULL) {
        printf("!! Couldn't open the file '%s'. !!\n", fileName);
        return resultStr;
    }
    rowLoc = getIdLocation(fileName, rowId);
    if(rowLoc < 0) {
        printf("!! There isn't any row with ID '%s' in the file '%s'. !!\n", rowId, fileName);
        fclose(file);
        return resultStr;
    }
    fseek(file, rowLoc, SEEK_SET);
    fgets(buf, BUF_LEN, file);
    token = strtok(buf, ",");

    i = 1;
    while(token != NULL && !found) {
        if(i == columnIndex) {
            strncpy(resultStr, token, LEN - 1);
            found = 1;
        }
        token = strtok(NULL, ",");
        i++;
    }
    fclose(file);
    return resultStr;
}

int listJoinTwoTables(const char *srcTableName,
    const char *destTableName,
    int srcIndex,
    int destIndex,
    const char *sourceId,
    FILE *outputStream)
{
    /* This function first joins srcTable with destTable on srcColumn = destColumn
     * Prints the resulting rows to the outputStream
     *
     * Arguments:
     *  srcIndex: index of the srcTable's column that has "1st" key
     *  destIndex: index of the destTable's column that has "1st" key as foreign key
     *  sourceId: ID of the src row that we want to find connections of
     * Returns 0 on success, FILE_ERR or -1 otherwise
     */
    FILE *srcTable;
    FILE *destTable;
    char srcLine[BUF_LEN];

    // Join list is empty
    if(srcIndex <= 0 || getColumnCount(srcTableName) < srcIndex) return 0;
    if(destIndex <= 0 || getColumnCount(destTableName) < destIndex) return 0;

    srcTable = fopen(srcTableName, "r");
    if(srcTable == NULL) {
        printf("!! Couldn't open the '%s' file. !!\n", srcTableName);
        return FILE_ERR;
    }
    destTable = fopen(destTableName, "r");
    if(destTable == NULL) {
        printf("!! Couldn't open the '%s' file. !!\n", destTableName);
        fclose(srcTable);
        return FILE_ERR;
    }
    // traverse the lines of the srcTable
    while(fgets(srcLine, BUF_LEN, srcTable) != NULL) {
        char srcRowId[LEN];

        sscanf(srcLine, "%49[^,]", srcRowId); // get srcRowId from first column
        // Join only the rows that we want to find connections of
        if(strcmp(srcRowId, sourceId) == 0) {
            char destLine[BUF_LEN];
            char srcColumn[LEN];

            selectFromRow(srcLine, srcIndex, srcColumn);
            //traverse the lines of the destTable
            while(fgets(destLine, BUF_LEN, destTable) != NULL) {
                char destColumn[LEN];

                selectFromRow(destLine, destIndex, destColumn);
                if(strcmp(srcColumn, destColumn) == 0) {
                    printFormattedRow(destLine, outputStream);
                }
            }
            rewind(destTable); // we have to rewind destTable every iteration of outer loop
        }
    }
    fclose(srcTable), fclose(destTable);
    return 0;
}

int listJoinThreeTables(const char *srcTableName,
    const char *midTableName,
    const char *destTableName,
    int srcIndex,
    int midToSrcIndex,
    int midToDestIndex,
    int destIndex,
    const char *sourceId,
    FILE *outputStream)
{
    /* This function first joins srcTable with midTable on srcColumn = srcToMidColumn
     * then takes the rows of the first join and joins them with destTable
     * on midToDestColumn = destColumn
     * Prints the resulting rows to the outputStream
     * Arguments:
     *  srcIndex: index of the srcTable's column that has "1st" key
     *  midToSrcIndex: index of the midTable's column that has "1st" key as foreign key
     *  midToDestIndex: index of the midTable's column that has "2nd" key
     *  destIndex: index of the destTable's column that has "2nd" key as foreign key
     *  sourceId: ID of the src row that we want to find connections of
     *
     * Returns 0 on success, FILE_ERR or -1 otherwise
     */
    FILE *srcTable;
    FILE *destTable;
    FILE *midTable;
    char srcLine[BUF_LEN];

    if(srcIndex <= 0 || getColumnCount(srcTableName) < srcIndex) return 0;
    if(midToSrcIndex <= 0 || getColumnCount(midTableName) < midToSrcIndex) return 0;
    if(midToDestIndex <= 0 || getColumnCount(midTableName) < midToDestIndex) return 0;
    if(destIndex <= 0 || getColumnCount(destTableName) < destIndex) return 0;

    srcTable = fopen(srcTableName, "r");
    if(srcTable == NULL) {
        printf("!! Couldn't open the '%s' file. !!\n", srcTableName);
        return -1;
    }
    midTable = fopen(midTableName, "r");
    if(midTable == NULL) {
        fclose(srcTable);
        printf("!! Couldn't open the '%s' file. !!\n", midTableName);
        return FILE_ERR;
    }
    destTable = fopen(destTableName, "r");
    if(destTable == NULL) {
        printf("!! Couldn't open the '%s' file. !!\n", destTableName);
        fclose(srcTable), fclose(midTable);
        return FILE_ERR;
    }
    // traverse rows of the srcTable
    while(fgets(srcLine, BUF_LEN, srcTable) != NULL) {
        char srcColumn[LEN];
        char srcRowId[LEN];

        sscanf(srcLine, "%49[^,]", srcRowId); // get srcRowId from first column
        // join only the rows that we want to find connections of
        if(strcmp(srcRowId, sourceId) == 0) {
            char midLine[BUF_LEN];

            selectFromRow(srcLine, srcIndex, srcColumn);
            // traverse rows of the midTable
            while(fgets(midLine, BUF_LEN, midTable) != NULL) {
                char srcToMidColumn[LEN];

                selectFromRow(midLine, midToSrcIndex, srcToMidColumn);
                if(strcmp(srcColumn, srcToMidColumn) == 0) {
                    char destLine[BUF_LEN];
                    char midToDestColumn[LEN];

                    selectFromRow(midLine, midToDestIndex, midToDestColumn);
                    //traverse rows of the destTable
                    while(fgets(destLine, BUF_LEN, destTable)) {
                        char destColumn[LEN];

                        selectFromRow(destLine, destIndex, destColumn);
                        if(strcmp(midToDestColumn, destColumn) == 0) {
                            printFormattedRow(destLine, outputStream);
                        }
                    }
                    rewind(destTable); // we have to rewind destTable every iteration of middle loop
                }
            }
            rewind(midTable); // we have to rewind midTable every iteration of outer loop
        }
    }
    fclose(srcTable), fclose(destTable), fclose(midTable);
    return 0;
}

void listStudentsInCourseOfInstructor(int instructorIdNum)
{
    /* Lists all courses given by the instructorIdNum. Then user picks one of the courses, and list of enrolled
     * students of that course is printed to the file with the name CLASS_LIST_FILE (macro) concatenated to courseCode
     *  e.g. BLM4510_STUDENTS.txt
     */
    char courseCode[LEN];
    char instructorId[LEN];
    FILE *outputFile;
    char outputFileName[LEN];

    sprintf(instructorId, "%d", instructorIdNum);
    if(getIdLocation(INSTRUCTORS_FILE, instructorId) < 0) {
        printf("!! There isn't any instructor with the ID '%s' in table '%s'. !!\n", instructorId, INSTRUCTORS_FILE);
        return;
    }
    printColumnNames(stdout, 5, "Course Code", "Instructor Id", "Credit", "Capacity", "Name");
    if(listJoinTwoTables(INSTRUCTORS_FILE, COURSES_FILE, 1, 2, instructorId, stdout) < 0) return;
    inputString("Enter one course's course code from the list above: ", courseCode);
    uppercaseStr(courseCode);

    outputFileName[0] = '\0'; // initialize with empty string
    strcat(outputFileName, courseCode);
    strcat(outputFileName, CLASS_LIST_FILE);
    outputFile = fopen(outputFileName, "w");
    if(outputFile == NULL) {
        printf("!! Couldn't open the file '%s'. !!\n", outputFileName);
        return;
    }

    printColumnNames(outputFile, 5, "Id", "Course Count", "Total Credit", "Name", "Surname");
    if(listJoinThreeTables(COURSES_FILE, STUDENT_TO_COURSE_FILE, STUDENTS_FILE, 1, 3, 2, 1, courseCode, outputFile) < 0) {
        fclose(outputFile);
        return;
    }

    fclose(outputFile);
}

/* Specific functions */
// Student
void getStudentStr(char *str)
{
    /* Creates a string that holds informations of a new Student.
     * This string can be used in insertIntoTable function */
    Student s;

    s.id = getNextId(STUDENT_IDS_FILE);
    s.courseCount = 0;
    s.totalCredit = 0;
    inputString("Enter the name: ", s.name);
    inputString("Enter the surname: ", s.surname);

    sprintf(str, "%d,%d,%d,%s,%s\n", s.id, s.courseCount, s.totalCredit, s.name, s.surname);
}

void deleteStudent(int idNum)
{
    /* Deletes a student from students table. Also deletes the rows that contain this student
     * from studentToCourse table. Pushes the ID of the removed student to the studentIds file.
     */
    char id[LEN];
    char line[BUF_LEN];

    sprintf(id, "%d", idNum);                           // convert idNum to string
    if(deleteFromTable(STUDENTS_FILE, id) != 0) return; // function prompts user

    FILE *file = fopen(STUDENT_TO_COURSE_FILE, "r");
    if(file == NULL) {
        printf("!! Couldn't open the file '%s'. !!\n", STUDENT_TO_COURSE_FILE);
        return;
    }

    while(fgets(line, BUF_LEN, file) != NULL) { // delete matching studentId rows from studentToCourse
        // LEN - 1 limit
        StudentToCourse s;
        s.studentId = -1; // initialize with invalid ID
        sscanf(line, "%d,%d,%49[^,],%49[^,],%d", &s.id, &s.studentId, s.courseCode, s.dateCreated, &s.enrollState);

        if(idNum == s.studentId) {
            char idStr[LEN];

            sprintf(idStr, "%d", s.id);
            deleteFromTable(STUDENT_TO_COURSE_FILE, idStr);
        }
    }
    pushId(idNum, STUDENT_IDS_FILE);
    fclose(file);
}

int updateStudentForCourse(int stdId, const char *courseCode, int sign)
{
    /* Changes student's courseCount and totalCredit columns according to the given course.
     * This functions can be used for both enroll in course and disenroll from course. If sign is positive,
     * it adds to columns, otherwise it subtracts from columns.
     * Returns 0 if successful, returns -1, FILE_ERR or MEM_ERR otherwise.
     */
    int courseCount, totalCredit;
    int credit;
    char courseCountStr[LEN], totalCreditStr[LEN], creditStr[LEN];
    char stdIdStr[LEN];

    sprintf(stdIdStr, "%d", stdId);
    selectFromTable(STUDENTS_FILE, stdIdStr, 2, courseCountStr);
    selectFromTable(STUDENTS_FILE, stdIdStr, 3, totalCreditStr);
    selectFromTable(COURSES_FILE, courseCode, 3, creditStr);

    if(sign > 0)
        sign = 1;
    else if(sign < 0)
        sign = -1;
    else {
        printf("!! Sign can't be equal to 0. (f updateStudentForCourse) !!\n");
        return -1;
    }

    courseCount = atoi(courseCountStr);
    totalCredit = atoi(totalCreditStr);
    credit = atoi(creditStr);
    courseCount += sign;
    totalCredit += credit * sign;
    sprintf(courseCountStr, "%d", courseCount);
    sprintf(totalCreditStr, "%d", totalCredit);
    return updateTable(STUDENTS_FILE, stdIdStr, 4, 2, 3, courseCountStr, totalCreditStr);
}

int isStudentEnrolled(int idNum, const char *courseCode, long *rowLoc)
{
    /* Returns 0 if id-courseCode pair is in the file and enrollState is 0
     * Returns 1 if id-courseCode pair is in the file and enrollState is 1
     * Returns -1 if not in the table */
    char buf[BUF_LEN];
    char id[LEN];
    FILE *file = fopen(STUDENT_TO_COURSE_FILE, "r");
    int registered = 0;
    int enrollState;

    if(file == NULL) return FILE_ERR;
    sprintf(id, "%d", idNum);

    while(fgets(buf, BUF_LEN, file) != NULL && !registered) {
        char lineId[LEN];
        char lineCourseCode[LEN];

        // Discard first column of the line, read 2nd and 3rd columns into corresponding strings
        sscanf(buf, "%*d%*c%49[^,]%*c%49[^,]", lineId, lineCourseCode); // LEN - 1 limit
        // get the last column's ascii value, convert to decimal
        enrollState = buf[strlen(buf) - 2] - '0';
        if(!strcmp(id, lineId) && !strcmp(courseCode, lineCourseCode)) {
            registered = 1;
            *rowLoc = feof(file) ? ftell(file) - strlen(buf) : ftell(file) - strlen(buf) - 1;
        }
    }
    fclose(file);
    return (registered) ? enrollState : -1;
}

void enrollStudentInCourse(int studentId, const char *courseCode, int courseCountLimit, int totalCreditLimit)
{
    /* Enrolls a student in a course. It checks if course is on full capacity before proceeding.
     * If given studentId-courseCode pair is available in the StudentToCourse table
     * then check the enrollState value. If enrollState is 0 then makes it 1, otherwise prints warning message.
     *
     * This function also updates courseCount and totalCredit columns of the student
     */
    StudentToCourse s;
    char str[BUF_LEN];
    int enrollState;
    char stdIdStr[LEN];
    long rowLoc;     // location of the row found in the table
    time_t t;        // seconds elapsed since Epoch
    struct tm *date; // string that holds parsed date

    s.studentId = studentId;
    strcpy(s.courseCode, courseCode);
    sprintf(stdIdStr, "%d", studentId);
    if(getIdLocation(STUDENTS_FILE, stdIdStr) < 0) {
        printf("!! Student '%d' doesn't exist. Check the student ID. !!\n", s.studentId);
        return;
    }
    if(getIdLocation(COURSES_FILE, s.courseCode) < 0) {
        printf("!! Course '%s' doesn't exist. Check the course code. !!\n", s.courseCode);
        return;
    }

    t = time(NULL);
    date = localtime(&t);
    sprintf(s.dateCreated, "%02d.%02d.%d %02d:%02d",
        date->tm_mday, date->tm_mon + 1, date->tm_year + 1900, date->tm_hour, date->tm_min);

    enrollState = isStudentEnrolled(s.studentId, s.courseCode, &rowLoc);

    // We will change student's enrollState, so we should also update student's columns
    if(enrollState == 0 || enrollState == -1) {
        int courseCount, totalCredit;
        int credit;
        char courseCountStr[LEN], totalCreditStr[LEN], creditStr[LEN];

        //Check course's capacity
        FILE *file = fopen(STUDENT_TO_COURSE_FILE, "r");
        char line[BUF_LEN];
        char capacityStr[LEN];
        int capacity;
        int enrollCount = 0;

        if(file == NULL) {
            printf("!! Couldn't open the '%s' file. !!\n", COURSES_FILE);
            return;
        }
        selectFromTable(COURSES_FILE, s.courseCode, 4, capacityStr); // get the capacity
        capacity = atoi(capacityStr);

        // Get number of enrolled students of the course
        while(fgets(line, BUF_LEN, file) != NULL) {
            StudentToCourse c;
            strcpy(c.courseCode, "-1"); // initialize with invalid ID

            sscanf(line, "%d,%d,%49[^,],%49[^,],%d", &c.id, &c.studentId, c.courseCode, c.dateCreated, &c.enrollState);
            if(strcmp(c.courseCode, courseCode) == 0 && c.enrollState == 1) {
                enrollCount++;
            }
        }
        fclose(file);
        if(enrollCount >= capacity) {
            printf("!! Course '%s's capacity is full. !!\n", s.courseCode);
            return;
        }

        selectFromTable(STUDENTS_FILE, stdIdStr, 2, courseCountStr);
        selectFromTable(STUDENTS_FILE, stdIdStr, 3, totalCreditStr);
        selectFromTable(COURSES_FILE, s.courseCode, 3, creditStr);

        courseCount = atoi(courseCountStr);
        totalCredit = atoi(totalCreditStr);
        credit = atoi(creditStr);
        courseCount++;
        totalCredit += credit;
        if(courseCount > courseCountLimit) {
            printf("!! Student '%d' can't enroll in more courses. Total course limit is %d. !!\n",
                s.studentId, courseCountLimit);
            return;
        } else if(totalCredit > totalCreditLimit) {
            printf("!! Student '%d' can't take more credits. Total credit limit is %d. !!\n",
                s.studentId, totalCreditLimit);
            return;
        }
        sprintf(courseCountStr, "%d", courseCount);
        sprintf(totalCreditStr, "%d", totalCredit);
        if(updateTable(STUDENTS_FILE, stdIdStr, 4, 2, 3, courseCountStr, totalCreditStr) < 0) return;
    }
    if(enrollState == 1) { // if studentId-courseCode is present in the table
        printf("!! Student number '%d' has already enrolled in '%s'. !!\n", s.studentId, s.courseCode);
        return;
    } else if(enrollState == 0) { // enrollState is 0, so we have to set enrollState to 1 and update the date
        FILE *file = fopen(STUDENT_TO_COURSE_FILE, "r");
        char rowIdStr[LEN];
        if(file == NULL) {
            printf("!! Couldn't open the file '%s'. !!\n", STUDENT_TO_COURSE_FILE);
            return;
        }
        // get the row's ID using location of row
        fseek(file, rowLoc, SEEK_SET);
        fscanf(file, "%d", &s.id);
        sprintf(rowIdStr, "%d", s.id);
        fclose(file);
        if(updateTable(STUDENT_TO_COURSE_FILE, rowIdStr, 4, 4, 5, s.dateCreated, "1") < 0) return;
    } else if(enrollState == -1) { // studentId-courseCode pair is not present in the table so we should create it
        FILE *file = fopen(STUDENT_TO_COURSE_FILE, "a");

        if(file == NULL) {
            printf("!! Couldn't open the file '%s'. !!\n", STUDENT_TO_COURSE_FILE);
            return;
        }
        s.id = getNextId(STUDENT_TO_COURSE_IDS_FILE);
        s.enrollState = 1;
        sprintf(str, "%d,%d,%s,%s,%d\n", s.id, s.studentId, s.courseCode, s.dateCreated, s.enrollState);
        fputs(str, file);
        fclose(file);
    }
}

void disenrollStudentFromCourse(int studentIdNum, const char *courseCode)
{
    /* Disenrolls a student from a course. Prints error message if an error occurs. */
    char line[BUF_LEN];
    FILE *file = fopen(STUDENT_TO_COURSE_FILE, "r");

    if(file == NULL) {
        printf("!! Couldn't open the file '%s'. !!\n", STUDENT_TO_COURSE_FILE);
        return;
    }
    while(fgets(line, BUF_LEN, file) != NULL) {
        StudentToCourse s;

        strcpy(s.courseCode, "-1"); // initialize with invalid ID
        sscanf(line, "%d,%d,%49[^,],%49[^,],%d", &s.id, &s.studentId, s.courseCode, s.dateCreated, &s.enrollState);
        if(studentIdNum == s.studentId && strcmp(courseCode, s.courseCode) == 0) {
            if(s.enrollState == 1) {
                char idStr[LEN];

                if(updateStudentForCourse(studentIdNum, courseCode, -1) < 0) {
                    fclose(file);
                    return;
                }
                sprintf(idStr, "%d", s.id);
                if(updateTable(STUDENT_TO_COURSE_FILE, idStr, 2, 5, "0") < 0) { // set enrollState to 0
                    fclose(file);
                    return;
                }
            } else {
                printf("!! Student '%d' is not enrolled in '%s'. !!\n", studentIdNum, courseCode);
            }
            fclose(file);
            return;
        }
    }
    printf("!! Student '%d' is not enrolled in '%s'. !!\n", studentIdNum, courseCode);
    fclose(file);
}

// Instructor
void getInstructorStr(char *str)
{
    /* Creates a string that holds informations of a new Instructor.
     * This string can be used in insertIntoTable function */
    Instructor s;

    s.id = getNextId(INSTRUCTOR_IDS_FILE);
    inputString("Enter the name: ", s.name);
    inputString("Enter the surname: ", s.surname);
    inputString("Enter the rank: ", s.rank);

    sprintf(str, "%d,%s,%s,%s\n", s.id, s.name, s.surname, s.rank);
}

void deleteInstructor(int idNum)
{
    /* Deletes an instructor from instructors table. Also sets instructor columns of matching courses to NO_INSTRUCTOR.
     * Pushes the ID of the removed instructor to the instructorIds file.
     */
    char id[LEN];
    char line[BUF_LEN];

    sprintf(id, "%d", idNum);
    if(deleteFromTable(INSTRUCTORS_FILE, id) != 0) return; // function prompts user
    FILE *file = fopen(COURSES_FILE, "r");

    if(file == NULL) {
        printf("!! Couldn't open the file '%s'. !!\n", COURSES_FILE);
        return;
    }
    while(fgets(line, BUF_LEN, file) != NULL) { // set matching rows' instructor column to NO_INSTRUCTOR
        Course s;
        s.instructorId = -1; // initialize with invalid ID
        // LEN - 1 limit
        sscanf(line, "%49[^,],%d,%d,%d,%49[^\n]", s.courseCode, &s.instructorId, &s.credit, &s.capacity, s.name);

        if(idNum == s.instructorId) {
            char noInstructor[LEN];

            sprintf(noInstructor, "%d", NO_INSTRUCTOR);
            if(updateTable(COURSES_FILE, s.courseCode, 2, 2, noInstructor) < 0) {
                fclose(file);
                return;
            }
        }
    }
    pushId(idNum, INSTRUCTOR_IDS_FILE);
    fclose(file);
}

void addInstructorToCourse(int instructorId, const char *courseCode)
{
    /* Assigns an instructor to a currently NO_INSTRUCTOR course. Prints warning messages if an error occurs. */
    char courseInstructorStr[LEN];
    char instructorIdStr[LEN];
    int courseInstructor;

    sprintf(instructorIdStr, "%d", instructorId);
    if(getIdLocation(COURSES_FILE, courseCode) < 0) {
        printf("!! There isn't any course with the ID '%s' in table '%s'. !!\n", courseCode, COURSES_FILE);
        return;
    } else if(getIdLocation(INSTRUCTORS_FILE, instructorIdStr) < 0) {
        printf("!! There isn't any instructor with the ID '%d' in table '%s'. !!\n", instructorId, INSTRUCTORS_FILE);
        return;
    }
    selectFromTable(COURSES_FILE, courseCode, 2, courseInstructorStr);
    courseInstructor = atoi(courseInstructorStr);
    if(courseInstructor != NO_INSTRUCTOR) { // some instructor assigned before, prompt user
        if(courseInstructor == instructorId) {
            printf("!! Instructor '%d' is already assigned to course '%s'. !!\n", instructorId, courseCode);
        } else {
            printf("!! Currently instructor '%d' is assigned to course '%s'. !!\n", courseInstructor, courseCode);
            printf("!! Remove course's instructor before adding new one. !!\n");
        }
    } else { // assign the instructor to course
        sprintf(instructorIdStr, "%d", instructorId);
        if(updateTable(COURSES_FILE, courseCode, 2, 2, instructorIdStr) < 0) return;
    }
}

void removeInstructorFromCourses(int idNum)
{
    /* Sets all courses of given instructor to NO_INSTRUCTOR  */
    char id[LEN];
    char line[BUF_LEN];

    sprintf(id, "%d", idNum);
    FILE *file = fopen(COURSES_FILE, "r");

    if(file == NULL) {
        printf("!! Couldn't open the file '%s'. !!\n", COURSES_FILE);
        return;
    }
    while(fgets(line, BUF_LEN, file) != NULL) { // set matching rows' instructor column to NO_INSTRUCTOR
        Course s;
        s.instructorId = -1; // initialize with invalid ID
        // LEN - 1 limit
        sscanf(line, "%49[^,],%d,%d,%d,%49[^\n]", s.courseCode, &s.instructorId, &s.credit, &s.capacity, s.name);

        if(idNum == s.instructorId) {
            char noInstructor[LEN];

            sprintf(noInstructor, "%d", NO_INSTRUCTOR);
            if(updateTable(COURSES_FILE, s.courseCode, 2, 2, noInstructor) < 0) {
                fclose(file);
                return;
            }
        }
    }
    fclose(file);
}

// Course
void getCourseStr(char *str)
{
    /* Creates a string that holds informations of a new Course.
     * This string can be used in insertIntoTable function */
    Course s;

    inputString("Enter the course code (e.g. BLM1012): ", s.courseCode);
    uppercaseStr(s.courseCode);

    if(getIdLocation(COURSES_FILE, s.courseCode) >= 0) {
        printf("!! Course '%s' is already in the courses. Try different course code. !!\n", s.courseCode);
        str[0] = '\0'; // empty the string before returning to insertIntoTable
        return;
    }
    s.instructorId = NO_INSTRUCTOR; // no instructor assigned for now
    printf("Enter the credit: ");
    scanf("%d", &s.credit);
    printf("Enter the capacity: ");
    scanf("%d", &s.capacity);
    flushStdin();
    inputString("Enter the name: ", s.name);

    sprintf(str, "%s,%d,%d,%d,%s\n", s.courseCode, s.instructorId, s.credit, s.capacity, s.name);
}

void deleteCourse(const char *courseCode)
{
    /* Deletes a course from courses table. Also deletes matching rows from studentToCourse table and updates
     * columns of the course's enrolled students.
     */
    char creditStr[LEN];
    char line[BUF_LEN];

    selectFromTable(COURSES_FILE, courseCode, 3, creditStr);
    if(deleteFromTable(COURSES_FILE, courseCode) != 0) return; // function prompts user
    FILE *file = fopen(STUDENT_TO_COURSE_FILE, "r");

    if(file == NULL) {
        printf("!! Couldn't open the file '%s'. !!\n", STUDENT_TO_COURSE_FILE);
        return;
    }
    while(fgets(line, BUF_LEN, file) != NULL) { // delete matching courseCode rows from studentToCourse
        // LEN - 1 limit
        StudentToCourse s;
        strcpy(s.courseCode, "-1"); // initialize with invalid ID
        sscanf(line, "%d,%d,%49[^,],%49[^,],%d", &s.id, &s.studentId, s.courseCode, s.dateCreated, &s.enrollState);

        if(strcmp(courseCode, s.courseCode) == 0) {
            char idStr[LEN];

            if(s.enrollState == 1) { // Only update student's columns if student is still enrolled
                int courseCount, totalCredit;
                char courseCountStr[LEN], totalCreditStr[LEN];
                char studentIdStr[LEN];

                sprintf(studentIdStr, "%d", s.studentId);
                selectFromTable(STUDENTS_FILE, studentIdStr, 2, courseCountStr);
                selectFromTable(STUDENTS_FILE, studentIdStr, 3, totalCreditStr);

                // decrement student's courseCount
                // subtract removed course's credit from student's total credit
                courseCount = atoi(courseCountStr) - 1;
                totalCredit = atoi(totalCreditStr) - atoi(creditStr);
                sprintf(courseCountStr, "%d", courseCount);
                sprintf(totalCreditStr, "%d", totalCredit);
                if(updateTable(STUDENTS_FILE, studentIdStr, 4, 2, 3, courseCountStr, totalCreditStr) < 0) {
                    fclose(file);
                    return;
                }
            }

            sprintf(idStr, "%d", s.id);
            deleteFromTable(STUDENT_TO_COURSE_FILE, idStr);
        }
    }
    fclose(file);
}

void removeCourseInstructor(const char *courseCode)
{
    /* Sets given course to NO_INSTRUCTOR */
    char courseInstructor[LEN];
    char noInstructor[LEN];

    sprintf(noInstructor, "%d", NO_INSTRUCTOR);
    if(getIdLocation(COURSES_FILE, courseCode) < 0) {
        printf("!! There isn't any course with the ID '%s' in file '%s'. !!\n", courseCode, COURSES_FILE);
        return;
    }
    selectFromTable(COURSES_FILE, courseCode, 2, courseInstructor);
    if(strcmp(courseInstructor, noInstructor) == 0) {
        printf("!! Currently no instructor is assigned to course '%s'. !!\n", courseCode);
    } else {
        updateTable(COURSES_FILE, courseCode, 2, 2, noInstructor);
    }
}
