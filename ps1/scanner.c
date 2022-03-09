#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERROR -1
#define N_STATES 9
#define ACCEPT 8

// Buffer for accepted characters, to interpret when in accepting state
char lexical_buffer[256];

// Program internal state
int
    state = 0,                          // Current state in the automaton
    transition_table[N_STATES][256],    // Table form of the automaton
    lexical_position = 0,               // Track position in character buffer
    x = 421, y = 298,                   // Set starting point at page center
    dx = 0, dy = 0;                     // Horizontal/vertical shift values


/*
 * When a complete command has been scanned, this
 * function translates it into postscript
 */
void interpret_lexical_buffer ( void );


/*
 * Prepare the table form of the automaton
 */
void
initialize_transition_table ( void )
{
    memset ( transition_table, 0, N_STATES*256*sizeof(int) );
    for ( int s=0; s<N_STATES; s++ )
        for ( int i=0; i<256; i++ )
            transition_table[s][i] = ERROR;

    transition_table[0]['d'] = 1;
    transition_table[0]['g'] = 2;
    transition_table[1]['x'] = 4;
    transition_table[1]['y'] = 4;
    transition_table[2]['o'] = 3;
    transition_table[3]['\n'] = ACCEPT;
    transition_table[4]['='] = 5;
    transition_table[5]['-'] = 6;

    for ( int c='0'; c<='9'; c++ ) {
        for ( int s=5; s<=7; s++ ) {
            transition_table[s][c] = 7;
        }
    }
    transition_table[7]['\n'] = ACCEPT;


}


/* Reset the automaton after completing a command */
void
reset ( void )
{
    state = 0;
    lexical_position = 0;
    memset ( lexical_buffer, 0, 256*sizeof(char) );
}


/*
 * Main function:
 *  - Initialize the program to start drawing
 *  - Scan command
 *  - Translate accepted command string into postscript
 *  - Reset automaton
 *  - Repeat
 */
int
main ()
{
    // Prepare the table representation of the automaton
    initialize_transition_table();

    // Start drawing from the initial coordinates
    printf ( "<< /PageSize [842 595] >> setpagedevice\n" );
    printf ( "%d %d moveto\n", x, y );

    // Read characters until end-of-file or failure
    int read = 0;
    while ( read != EOF )
    {
        // Fetch the next character and store it in the lexical buffer
        read = getchar();
        lexical_buffer[lexical_position] = read;
        lexical_position += 1;

        // Change state according to the table
        state = transition_table[state][read];

        // Check for error or accepting states:
        //  * stop the program on error
        //  * interpret the contents of the lexical buffer when valid
        switch (state) 
        {
            case ERROR:
                fprintf ( stderr, "Invalid input\n" );
                exit ( EXIT_FAILURE );
                break;
            case ACCEPT:
                interpret_lexical_buffer();
                reset();
                break;
        }
    }
    printf ( "stroke\n" );
    printf ( "showpage\n" );
    exit ( EXIT_SUCCESS );
}


/* This function interprets correct statements after they have been accepted */
void
interpret_lexical_buffer ( void )
{
    if ( lexical_buffer[1] == 'x' )
        sscanf ( &lexical_buffer[3], "%d", &dx );
    else if ( lexical_buffer[1] == 'y' )
        sscanf ( &lexical_buffer[3], "%d", &dy );
    else if ( strncmp ( lexical_buffer, "go", 2 ) == 0 )
    {
        x = x + dx;
        y = y + dy;
        printf ( "%d %d lineto\n", x, y );
        printf ( "%d %d moveto\n", x, y );
    }
    else
        fprintf ( stderr, "Unknown command accepted by the scanner:\n%s\n",
            lexical_buffer
        );
}
