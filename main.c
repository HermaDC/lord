#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define RED    "\x1b[31m"
#define GREEN  "\x1b[32m"
#define YELLOW "\x1b[33m"
#define RESET  "\x1b[0m"


/* ---------- Creación ---------- */


/* ---------- Lógica ---------- */

Track *get_next_track(Track *track) {

    if (!track) return NULL;

    /* Si es switch, mirar posición */
    if (track->type == SWITCH_TRACK) {

        Switch *sw = (Switch *)track;

        if (sw->pos == DIVERGING_POS && sw->branch)
            return sw->branch;
    }

    return track->next;
}


/* ---------- Print ---------- */

void print_tracks_with_switches(Track *head) {
    Track *current = head;

    while (current != NULL) {

        if (current->type == SWITCH_TRACK) {
            Switch *sw = (Switch *)current;

            // Contar tracks en la rama
            int branch_count = count_branch_tracks(sw->branch);

            if (sw->pos == STRAIGHT_POS)
                printf("SW[→](%d) ", branch_count);
            else
                printf("SW[~](%d) ", branch_count);

            // También podemos imprimir la rama recursivamente
            if (sw->branch){
                printf("{");
                print_tracks_with_switches(sw->branch);
                printf("} ");
            }
        } else {
            // Track normal
            switch (current->status) {
                case CLEAR:   printf(GREEN "------ " RESET); break;
                case OCCUPIED:printf(RED "------ " RESET); break;
                case WARNING: printf(YELLOW "------ " RESET); break;
            }
        }

        current = get_next_track(current);
    }

    //printf("\n");
}


/* ---------- Liberación ---------- */



/* ---------- MAIN ---------- */

int main() {
    // Initialize random seed once at program start
    srand(1234); //For testing it sould return
                 //0 2 0 0 1 0 1 2 2 1    
    
    Track *head = create_straight_line(5);
    if (!head){ 
        fprintf(stderr, "Error creating straight line\n"); 
        return EXIT_FAILURE; 
    } 
    // Create a switch with a branch of 3 tracks 
    Track *branch = create_straight_line(3); 
    if (!branch) { 
        fprintf(stderr, "Error creating branch\n"); 
        free_tracks(head, NULL); return EXIT_FAILURE; } 
    Switch *sw = insert_switch(head, head->next->next, NULL, branch); 
    if (!sw) { 
        fprintf(stderr, "Error inserting switch\n"); 
        free_tracks(head, NULL); 
        free_tracks(branch, NULL); 
        return EXIT_FAILURE; } 

    printf(get_last_track)
        // Print the system layout 
        printf("System Layout:\n"); 
        print_tracks_with_switches(head); 
        printf("\n");
    
    return 0;
}