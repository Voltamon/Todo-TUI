#include "todo.c"

sqlite3* init_db(void);
void init_table(sqlite3 *DB);

int get_id();
void error(void);

sqlite3* init_db(void) {
    sqlite3 *DB;
    int exit_code;

    exit_code = sqlite3_open("todo.db", &DB);
    if(exit_code != SQLITE_OK) {
        fprintf(stderr, "Failed to open todo.db : %S\n", sqlite3_errmsg(DB));
        fflush(stderr);

        return NULL;
    }

    return DB;
}

void init_table(sqlite3 *DB) {
    FILE *file;
    char *query, *errorMessage;

    file = fopen("init.sql", "r");
    if (!file) {
        fprintf(stderr, "failed to open init.sql\n");
        fflush(stderr);

        return;
    }

    fseek(file, 0, SEEK_END);
    long query_size = ftell(file);
    rewind(file);

    query = malloc(query_size + 1);
    if (!query) {
        fprintf(stderr, "Failed to allocate memory\n");
        fflush(stderr);

        return;
    }

    fread(query, 1, query_size, file);
    query[query_size] = '\0';
    fclose(file);

    int exit_code = sqlite3_exec(DB, query, NULL, 0, &errorMessage);
    if (exit_code != SQLITE_OK) {
        fprintf(stderr, "Failed to initialise TODO table : %s\n", errorMessage);
        fflush(stderr);

    }

    free(query);
    sqlite3_free(errorMessage);
    return;
}

int get_id() {
    int id;

    if (scanf("%d", &id) != 1 || id == 0) {
        fprintf(stderr, "Invalid ID, Try Again\n");
        fflush(stderr);

        while (getchar() != '\n');
        return 0;
    }

    return id;
}

void error(void) {
    fprintf(stderr, " Try Again\n");
    fflush(stderr);

    return;
}

int main(int argc, char** argv) {
    char input;
    int id;
    sqlite3 *DB;
    enum Result status;

    DB = init_db();
    if (!DB) {
        fprintf(stderr, "Failed to initialise database\n");
        fflush(stderr);

        return 1;
    }

    init_table(DB);

    while (1) {
        fprintf(stdout, "---------- TERMINAL TODO ----------\n");
        fprintf(stdout, "[A]dd [L]ist [M]ark [R]emove [Q]uit\n\n");
        fprintf(stdout, "> ");
        fflush(stdout);

        input = getchar();
        while(getchar() != '\n');

        switch (input) {
            case 'A':   case 'a':
                char task[25];
                fprintf(stdout, "Enter the task : ");
                fflush(stdout);

                if (fgets(task, sizeof(task) - 1, stdin) == NULL) {
                    fprintf(stderr, "Failed to read input. Try again.\n");
                    fflush(stderr);

                    break;
                }

                if (!strlen(task)) {
                    fprintf(stderr, "Task cannot be empty.\n");
                    fflush(stderr);

                    break;
                }
                if (task[strlen(task) - 1] != '\n') {
                    int c;
                    while ((c = getchar()) != '\n' && c != EOF);
                }

                task[strcspn(task, "\n")] = '\0';
                status = addTodo(DB, task);

                if (status != OK) error();
                break;

            case 'L':   case 'l':
                status = listTodo(DB);
                if (status != OK) error();

                break;
            case 'M':   case 'm':
                fprintf(stdout, "Enter task id : ");
                fflush(stdout);

                id = get_id();
                if (!id) break;

                status = markTodo(DB, id);
                while (getchar() != '\n');

                if (status != OK) error();
                break;

            case 'R':   case 'r':
                fprintf(stdout, "Enter task id : ");
                fflush(stdout);

                id = get_id();
                if (!id) break;

                status = removeTodo(DB, id);
                while (getchar() != '\n');

                if (status != OK) error();
                break;

            case 'Q':   case 'q':
                exit(0);
                break;

            default:
                fprintf(stderr, "Invalid Choice, Try Again\n");
                fflush(stderr);
                break;
        }

        fprintf(stdout, "\n");
        fflush(stdout);
    }

    return 0;
}
