#include <stdio.h>
#include <string.h>
#include "file_io.h"
#include "features.h"
#include "rules.h"
#include "database.h"
#include "search_sort.h"
#include "auth.h"
#include "http_server.h"

char logUser[50];

static void read_line(const char *prompt, char *buf, size_t size) {
    if (prompt) printf("%s", prompt);
    if (!fgets(buf, size, stdin)) { buf[0] = '\0'; return; }
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
}

int main(int argc, char *argv[]) {
    // If started with --http, run the embedded HTTP server on port 3000
    if (argc > 1 && strcmp(argv[1], "--http") == 0) {
        start_http_server(3000);
        return 0;
    }

    load_songs();
    printf("\n\t\t Welcome to MOODIFY !!\n");

    while (1) {
        char line[128];
        read_line("1. Register\n2. Login\n3. Exit\nChoose an option: ", line, sizeof(line));
        int opt = atoi(line);
        switch (opt) {
            case 1:
                registerUser();
                break;
            case 2: {
                char username[50];
                if (login(username)) {
                    int choice = 0;
                    do {
                        read_line("****MENU*****\n1. Add New Song\n2. View All Songs\n3. Search Song by Title\n4. Search Songs by Artist\n5. Search Songs by Mood\n6. Sort Songs by BPM\n7. Sort Songs by Energy\n8. Exit\nEnter your choice: ", line, sizeof(line));
                        choice = atoi(line);
                        switch (choice) {
                            case 1: add_song(); break;
                            case 2: print_songs(); break;
                            case 3: {
                                char title[50];
                                read_line("Enter title to search: ", title, sizeof(title));
                                search_by_title(title);
                                break;
                            }
                            case 4: {
                                char artist[50];
                                read_line("Enter artist to search: ", artist, sizeof(artist));
                                search_by_artist(artist);
                                break;
                            }
                            case 5: {
                                char mood[25];
                                read_line("Enter mood to search: ", mood, sizeof(mood));
                                search_by_mood(mood);
                                break;
                            }
                            case 6: sort_by_bpm(); break;
                            case 7: sort_by_energy(); break;
                            case 8:
                                printf("\nThank you, come again...\n");
                                return 0;
                            default:
                                printf("Invalid choice.\n");
                        }
                    } while (choice != 8);
                }
                break;
            }
            case 3:
                return 0;
            default:
                break;
        }
    }

    return 0;
}
