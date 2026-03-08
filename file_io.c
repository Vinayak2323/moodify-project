#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_io.h"
#include "database.h"

#define SONG_FILE "songs.txt" 
extern SongNode *head;

void load_songs() {
    FILE *fp = fopen(SONG_FILE, "r");
    if (!fp) {
        printf("Could not open file %s\n", SONG_FILE);
        return;  
    } 

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        SongNode *new_song = malloc(sizeof(SongNode));
        if (!new_song) {
            printf("Memory allocation failed!\n");
            fclose(fp);
            return;
        }

        // CSV format: title,artist,bpm,energy,danceability,mood
        sscanf(line, "%49[^,],%49[^,],%d,%d,%d,%24[^\n]",
               new_song->title,
               new_song->artist,
               &new_song->bpm,
               &new_song->energy,
               &new_song->danceability,
               new_song->mood);

        new_song->next = NULL;
        new_song->prev = NULL;

        // Insert at end of DLL
        if (!head) {
            head = new_song;
        } else {
            SongNode *temp = head;
            while (temp->next) temp = temp->next;
            temp->next = new_song;
            new_song->prev = temp;
        }
    }

    fclose(fp);
    printf("Songs loaded from %s successfully.\n", SONG_FILE);
}

// Save DLL songs to CSV file
void save_songs() {
    FILE *fp = fopen(SONG_FILE, "w");
    if (!fp) {
        printf("Could not write to file %s\n", SONG_FILE);
        return;
    }

    SongNode *temp = head;
    while (temp) {
        fprintf(fp, "%s,%s,%d,%d,%d,%s\n",
                temp->title,
                temp->artist,
                temp->bpm,
                temp->energy,
                temp->danceability,
                temp->mood);
        temp = temp->next;
    }

    fclose(fp);
}

// Print all songs from DLL
void print_songs() {
    if (!head) {
        printf("No songs available.\n");
        return;
    }

    printf("\n Title                Artist              BPM    Energy   Danceability   Mood\n");
    printf("-----------------------------------------------------------------------------------------------\n");

    SongNode *temp = head;
    while (temp) {
        printf("%-20s %-20s %-6d   %-7d  %-12d  %-15s\n",
               temp->title,
               temp->artist,
               temp->bpm,
               temp->energy,
               temp->danceability,
               temp->mood);
        temp = temp->next;
    }
}
