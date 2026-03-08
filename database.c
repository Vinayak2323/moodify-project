#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "rules.h"
#include "features.h"
#include "file_io.h"  

SongNode *head = NULL; 

// Add a new song
void add_song() {
    SongNode *new_song = malloc(sizeof(SongNode));
    if (!new_song) {
        printf("Memory allocation failed!\n");
        return;
    }

    printf("Enter song title: ");
    scanf(" %[^\n]", new_song->title);
    printf("Enter artist: ");
    scanf(" %[^\n]", new_song->artist);
    printf("Enter BPM: ");
    scanf("%d", &new_song->bpm);
    printf("Enter Energy (0-100): ");
    scanf("%d", &new_song->energy);
    printf("Enter Danceability (0-1): ");
    scanf("%d", &new_song->danceability);
    strcpy(new_song->mood, "");  
    new_song->next = NULL;
    new_song->prev = NULL;

    // Insert at end
    if (!head) {
        head = new_song;
    } else {
        SongNode *temp = head;
        while (temp->next) temp = temp->next;
        temp->next = new_song;
        new_song->prev = temp;
    }

    printf("Song added successfully!\n");
    classify_songs(); 
    save_songs(); 
}

void classify_songs() {
    if (!head) {
        printf("No songs to classify.\n");
        return;
    }

    SongNode *temp = head;
    while (temp) {
        classify_mood(temp);
        temp = temp->next;
    }
    save_songs(); 
}
