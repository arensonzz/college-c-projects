#include <stdio.h>

#define MAX 30 // Maximum number of candidates

// preferences[i][j] : number of voters who prefer candidate i over candidate j
int preferences[MAX][MAX];

// locked[i][j] : if it's 1, there's an arrow in the graph from i to j
int locked[MAX][MAX];

int pointed[MAX]; // candidates with at least 1 arrow pointing towards them

typedef struct pair {
    int winner;
    int loser;
} pair;

pair pairs[MAX * (MAX - 1) / 2];

int pair_count;
int candidate_count;

void f_mtrx_print(int mtrx[][MAX], int nrows, int ncolumns);
void record_preferences(int ranks[]);
void add_pairs(void);
void sort_pairs(void);
void lock_pairs(void);
void print_winner(void);
int is_cycle(int start, int end);

int main()
{
    int voter_count;

    printf("Number of candidates: ");
    scanf("%d", &candidate_count);
    if(candidate_count > MAX) {
        printf("Maximum number of candidates is %d\n", MAX);
        return 2;
    }

    pair_count = 0;
    printf("Number of voters: ");
    scanf("%d", &voter_count);

    // getting each voter's votes
    for(int i = 0; i < voter_count; i++) {
        // ranks[i] : i'th preference of the voter (ranks[0] first, ranks[1] second, etc.)
        int ranks[candidate_count];

        printf("Ranks: ");
        for(int j = 0; j < candidate_count; j++) {
            int choice;

            scanf("%d", &choice);
            if(choice < 0 || choice >= candidate_count) {
                printf("Invalid vote.\n");
                return 3;
            }
            ranks[j] = choice;
        }
        record_preferences(ranks);
    }
    printf("\npreferences[][]:\n");
    f_mtrx_print(preferences, candidate_count, candidate_count);
    add_pairs();
    sort_pairs();
    lock_pairs();
    printf("\nlocked[][]:\n");
    f_mtrx_print(locked, candidate_count, candidate_count);
    printf("\npointed[][]:\n");
    for(int i = 0; i < candidate_count; i++)
        printf("%3d ", i);
    printf("\n");
    for(int i = 0; i < candidate_count; i++)
        printf(" %c%c ", 196, 196);
    printf("\n");
    for(int i = 0; i < candidate_count; i++)
        printf("%3d ", pointed[i]);

    printf("\n\nWinner: ");
    print_winner();

    return 0;
}

// Print a matrix formatted
void f_mtrx_print(int mtrx[][MAX], int nrows, int ncolumns)
{
    printf("     ");
    for(int i = 0; i < ncolumns; i++) {
        printf("%3d ", i);
    }
    printf("\n     ");
    for(int i = 0; i < ncolumns; i++) {
        printf(" %c%c ", 196, 196);
    }
    printf("\n");
    for(int i = 0; i < nrows; i++) {
        printf("%3d| ", i);
        for(int j = 0; j < ncolumns; j++) {
            printf("%3d ", mtrx[i][j]);
        }
        printf("\n");
    }
}

// Given one voter's votes, updates preferences according to that
void record_preferences(int ranks[])
{
    for(int i = candidate_count - 2; i >= 0; i--) {
        for(int j = i + 1; j < candidate_count; j++) {
            preferences[ranks[i]][ranks[j]]++;
        }
    }
    return;
}

// Makes pairs from the candidates which one is preffered over other
void add_pairs(void)
{
    for(int i = 0; i < candidate_count - 1; i++) {
        for(int j = i + 1; j < candidate_count; j++) {
            int winner, loser;

            if(preferences[i][j] > preferences[j][i]) {
                winner = i;
                loser = j;
            } else if(preferences[i][j] < preferences[j][i]) {
                winner = j;
                loser = i;
            } else {
                continue; // skip pair if there's a tie
            }

            pairs[pair_count].winner = winner;
            pairs[pair_count++].loser = loser;
        }
    }
    return;
}

// Sort pairs according to strength of victory in the decreasing order
void sort_pairs(void)
{
    for(int i = 0; i < pair_count - 1; i++) {
        int max = i;
        for(int j = i + 1; j < pair_count; j++) {
            // Get the strength of victory from the preferences matrix
            int j_score = preferences[pairs[j].winner][pairs[j].loser];
            int max_score = preferences[pairs[max].winner][pairs[max].loser];

            if(j_score > max_score) {
                max = j;
            }
        }
        pair tmp = pairs[i];
        pairs[i] = pairs[max];
        pairs[max] = tmp;
    }
    return;
}

// Checks if there's a path from the pair's loser to winner
int is_cycle(int start, int end)
{
    if(start == end) { // there's a path
        return 1;
    }
    int cycle = 0;

    for(int i = 0; i < candidate_count; i++) {
        if(locked[start][i]) {
            cycle = is_cycle(i, end); // call the same function for the nodes connected to the current node
        }
        if(cycle) {
            return 1;
        }
    }

    return 0; // no cycle
}

// Locks pairs without causing a cycle in the graph
void lock_pairs(void)
{
    if(pair_count > 0) {
        locked[pairs[0].winner][pairs[0].loser] = 1;
        pointed[pairs[0].loser] = 1;
    }

    for(int i = 1; i < pair_count; i++) {
        int winner = pairs[i].winner;
        int loser = pairs[i].loser;

        // Check if locking will cause a cycle
        if(!is_cycle(loser, winner)) {
            locked[winner][loser] = 1;
            pointed[loser] = 1;
        }
    }

    return;
}

// Prints the winner of the election
void print_winner(void)
{
    for(int i = 0; i < candidate_count; i++) {
        if(!pointed[i]) // winner is the candidate with no arrow pointing towards
        {
            printf("%d\n", i);
        }
    }

    return;
}
