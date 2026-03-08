#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "features.h"
#include "database.h"
#include "file_io.h"

extern SongNode *head;

void search_by_title(const char *title) {
    if (!head) {
        printf("No songs available.\n");
        return;
    }

    SongNode *temp = head;
    int found = 0;

    while (temp) {
        if (strcasecmp(temp->title, title) == 0) {
            printf("\n Song Found:\n");
            printf("Title: %s\nArtist: %s\nBPM: %d\nEnergy: %d\nDanceability: %d\nMood: %s\n",
                   temp->title, temp->artist, temp->bpm, temp->energy, temp->danceability, temp->mood);
            found = 1;
            break;
        }
        temp = temp->next;
    }

    if (!found)
        printf("No song found with title '%s'.\n", title);
}

void search_by_mood(const char *mood) {
    if (!head) {
        printf("No songs available.\n");
        return;
    }

    SongNode *temp = head;
    int found = 0;

    printf("\nSongs with mood '%s':\n", mood);
    printf("-------------------------------------------------------------\n");

    while (temp) {
        if (strcasecmp(temp->mood, mood) == 0) {
            printf("%-20s %-20s BPM: %-5d Energy: %-5d Danceability: %-5d\n",
                   temp->title, temp->artist, temp->bpm, temp->energy, temp->danceability);
            found = 1;
        }
        temp = temp->next;
    }

    if (!found)
        printf("No songs found with mood '%s'.\n", mood);
}

void search_by_artist(const char *artist) {
    if (!head) {
        printf("No songs available.\n");
        return;
    }

    SongNode *temp = head;
    int found = 0;

    printf("\nSongs by artist '%s':\n", artist);
    printf("-------------------------------------------------------------\n");

    while (temp) {
        if (strcasecmp(temp->artist, artist) == 0) {
            printf("%-20s %-20s BPM: %-5d Energy: %-5d Danceability: %-5d Mood: %-10s\n",
                   temp->title, temp->artist, temp->bpm, temp->energy, temp->danceability, temp->mood);
            found = 1;
        }
        temp = temp->next;
    }

    if (!found)
        printf("No songs found by artist '%s'.\n", artist);
}

// Sorting by BPM (Ascending)
void sort_by_bpm() {
    if (!head) {
        printf("No songs to sort.\n");
        return;
    }

    SongNode *i, *j;
    for (i = head; i != NULL; i = i->next) {
        for (j = i->next; j != NULL; j = j->next) {
            if (i->bpm > j->bpm) {
                SongNode temp = *i;
                strcpy(i->title, j->title);
                strcpy(i->artist, j->artist);
                i->bpm = j->bpm;
                i->energy = j->energy;
                i->danceability = j->danceability;
                strcpy(i->mood, j->mood);

                strcpy(j->title, temp.title);
                strcpy(j->artist, temp.artist);
                j->bpm = temp.bpm;
                j->energy = temp.energy;
                j->danceability = temp.danceability;
                strcpy(j->mood, temp.mood);
            }
        }
    }

    printf("Songs sorted by BPM (Ascending).\n");
    save_songs();
}

// Sorting by Energy (Descending)
void sort_by_energy() {
    if (!head) {
        printf("No songs to sort.\n");
        return;
    }

    SongNode *i, *j;
    for (i = head; i != NULL; i = i->next) {
        for (j = i->next; j != NULL; j = j->next) {
            if (i->energy < j->energy) {
                SongNode temp = *i;
                strcpy(i->title, j->title);
                strcpy(i->artist, j->artist);
                i->bpm = j->bpm;
                i->energy = j->energy;
                i->danceability = j->danceability;
                strcpy(i->mood, j->mood);

                strcpy(j->title, temp.title);
                strcpy(j->artist, temp.artist);
                j->bpm = temp.bpm;
                j->energy = temp.energy;
                j->danceability = temp.danceability;
                strcpy(j->mood, temp.mood);
            }
        }
    }

    printf("Songs sorted by Energy (Descending).\n");
    save_songs();
}
