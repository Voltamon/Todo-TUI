#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>

typedef enum Result {
    OK,
    ERR
} Result;

Result addTodo(sqlite3* DB, char task[]);
Result listTodo(sqlite3* DB);
Result markTodo(sqlite3* DB, int id);
Result removeTodo(sqlite3* DB, int id);

Result addTodo(sqlite3* DB, char task[]) {
    char *query, *errorMessage;
    int exit_code;

    query = malloc(60);
    if (!query) {
        fprintf(stderr, "Failed to allocate memory.");
        fflush(stderr);

        free(query);
        free(errorMessage);
        return ERR;
    }

    snprintf(query, 60, "INSERT INTO TODO VALUES(NULL, '%s', 0);", task);

    exit_code = sqlite3_exec(DB, query, NULL, 0, &errorMessage);
    if (exit_code != SQLITE_OK) {
        fprintf(stderr, "Failed to insert the task : %s.", errorMessage);
        fflush(stderr);

        free(query);
        sqlite3_free(errorMessage);
        return ERR;
    }

    fprintf(stdout, "Added Task [%s] successfully\n", task);
    fflush(stdout);

    free(query);
    sqlite3_free(errorMessage);
    return OK;
}

Result listTodo(sqlite3* DB) {
    char query[50];
    sqlite3_stmt *stmt;
    int exit_code;

    strncpy(query, "SELECT * FROM TODO ORDER BY ID;", 50);

    exit_code = sqlite3_prepare_v2(DB, query, -1, &stmt, NULL);
    if (exit_code != SQLITE_OK) {
        fprintf(stderr, "Failed to fetch the todo list : %s.", sqlite3_errmsg(DB));
        fflush(stdout);

        return ERR;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        fprintf(stdout, "No tasks in TODO\n");
        fflush(stdout);

        sqlite3_finalize(stmt);
        return OK;
    }

    fprintf(stdout, "%-3s | %-25s | %s\n", "ID", "TASK", "STATUS");
    do {
        char *task, *status;
        int id;

        id = sqlite3_column_int(stmt, 0);
        task = (char *)sqlite3_column_text(stmt, 1);
        status = sqlite3_column_int(stmt, 2) ? "done" : "pending";

        fprintf(stdout, "%-3d | %-25s | [%s]\n", id, task, status);
        fflush(stdout);
    } while (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return OK;
}

Result markTodo(sqlite3* DB, int id) {
    char query[70], *errorMessage;
    sqlite3_stmt *stmt;
    int exit_code, status;

    snprintf(query, 70, "SELECT STATUS FROM TODO WHERE ID = %d;", id);

    exit_code = sqlite3_prepare_v2(DB, query, -1, &stmt, NULL);
    if (exit_code != SQLITE_OK) {
        fprintf(stderr, "Failed to update task : %s.", sqlite3_errmsg(DB));
        fflush(stdout);

        return ERR;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        status = sqlite3_column_int(stmt, 0);
    } else {
        fprintf(stderr, "Task with ID : %d not found.", id);
        fflush(stderr);

        sqlite3_finalize(stmt);
        return ERR;
    }

    status = status ? 0 : 1;
    snprintf(query, 70, "UPDATE TODO SET STATUS = %d WHERE ID = %d;", status, id);

    exit_code = sqlite3_exec(DB, query, NULL, 0, &errorMessage);
    if (exit_code != SQLITE_OK) {
        fprintf(stderr, "Failed to update task : %s", errorMessage);
        fflush(stderr);

        free(errorMessage);
        sqlite3_finalize(stmt);
        return ERR;
    }

    fprintf(stdout, "Marked task with ID : %d successfully\n", id);
    fflush(stdout);

    free(errorMessage);
    sqlite3_finalize(stmt);
    return OK;
}
Result removeTodo(sqlite3* DB, int id) {
    char query[70], *errorMessage;
    sqlite3_stmt *stmt;
    int exit_code;

    snprintf(query, 70, "SELECT ID FROM TODO WHERE ID = %d;", id);

    exit_code = sqlite3_prepare_v2(DB, query, -1, &stmt, NULL);
    if (exit_code != SQLITE_OK) {
        fprintf(stderr, "Failed to delete task : %s", sqlite3_errmsg(DB));
        fflush(stderr);

        return ERR;
    }

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        fprintf(stderr, "Task with ID: %d does not exist.", id);
        fflush(stderr);

        sqlite3_finalize(stmt);
        return ERR;
    }

    sqlite3_finalize(stmt);
    snprintf(query, 70, "DELETE FROM TODO WHERE ID = %d;", id);

    exit_code = sqlite3_exec(DB, query, NULL, 0, &errorMessage);
    if (exit_code != SQLITE_OK) {
        fprintf(stderr, "Failed to delete task : %s", errorMessage);
        fflush(stderr);

        sqlite3_free(errorMessage);
        return ERR;
    }

    fprintf(stdout, "Deleted task with ID: %d successfully\n", id);
    fflush(stdout);

    sqlite3_free(errorMessage);
    return OK;
}
