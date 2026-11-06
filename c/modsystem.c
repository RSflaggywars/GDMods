/*
 * modsystem.c - implementation of the GD-style moderator system.
 * See modsystem.h for docs. C99, no dependencies beyond libc.
 */

#include "modsystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT_CAP 16
#define MAX_COMMENTS 10000
#define MAX_BANS 512

struct ms_system {
    ms_comment_t *comments;
    size_t        count;
    size_t        cap;
    int           next_id;

    ms_ban_t      bans[MAX_BANS];
    size_t        ban_count;
};

static void copy_str(char *dst, size_t size, const char *src) {
    if (!src) src = "";
    snprintf(dst, size, "%s", src);
}

ms_system_t *ms_create(void) {
    ms_system_t *ms = calloc(1, sizeof(*ms));
    if (!ms) return NULL;
    ms->cap = INIT_CAP;
    ms->comments = calloc(ms->cap, sizeof(ms_comment_t));
    if (!ms->comments) { free(ms); return NULL; }
    ms->next_id = 1;
    return ms;
}

void ms_destroy(ms_system_t *ms) {
    if (!ms) return;
    free(ms->comments);
    free(ms);
}

/* ---------------- comments ---------------- */

int ms_add_comment(ms_system_t *ms, const char *user, ms_role_t role,
                   const char *text) {
    if (!ms || !text || !*text || ms->count >= MAX_COMMENTS) return -1;
    if (ms->count == ms->cap) {
        size_t ncap = ms->cap * 2;
        ms_comment_t *nc = realloc(ms->comments, ncap * sizeof(*nc));
        if (!nc) return -1;
        ms->comments = nc;
        ms->cap = ncap;
    }
    ms_comment_t *c = &ms->comments[ms->count++];
    memset(c, 0, sizeof(*c));
    c->id = ms->next_id++;
    copy_str(c->user, sizeof c->user, user);
    c->role = role;
    copy_str(c->text, sizeof c->text, text);
    c->time = time(NULL);
    return c->id;
}

int ms_delete_comment(ms_system_t *ms, int id) {
    if (!ms) return 0;
    for (size_t i = 0; i < ms->count; i++) {
        if (ms->comments[i].id == id) {
            memmove(&ms->comments[i], &ms->comments[i + 1],
                    (ms->count - i - 1) * sizeof(ms_comment_t));
            ms->count--;
            return 1;
        }
    }
    return 0;
}

static ms_comment_t *find_mut(ms_system_t *ms, int id) {
    if (!ms) return NULL;
    for (size_t i = 0; i < ms->count; i++)
        if (ms->comments[i].id == id) return &ms->comments[i];
    return NULL;
}

int ms_report_comment(ms_system_t *ms, int id) {
    ms_comment_t *c = find_mut(ms, id);
    if (!c) return 0;
    c->reports++;
    return 1;
}

int ms_like_comment(ms_system_t *ms, int id) {
    ms_comment_t *c = find_mut(ms, id);
    if (!c) return 0;
    c->likes++;
    return 1;
}

int ms_pin_comment(ms_system_t *ms, int id, int pinned) {
    ms_comment_t *c = find_mut(ms, id);
    if (!c) return 0;
    c->pinned = pinned ? 1 : 0;
    return 1;
}

size_t ms_count(const ms_system_t *ms) { return ms ? ms->count : 0; }

const ms_comment_t *ms_get(const ms_system_t *ms, size_t index) {
    if (!ms || index >= ms->count) return NULL;
    return &ms->comments[index];
}

const ms_comment_t *ms_find(const ms_system_t *ms, int id) {
    if (!ms) return NULL;
    for (size_t i = 0; i < ms->count; i++)
        if (ms->comments[i].id == id) return &ms->comments[i];
    return NULL;
}

/* ---------------- moderation ---------------- */

int ms_can_moderate(ms_role_t role) {
    return role == MS_MOD || role == MS_LEAD || role == MS_DEV;
}

static ms_ban_t *find_ban(ms_system_t *ms, const char *user) {
    for (size_t i = 0; i < ms->ban_count; i++)
        if (strcmp(ms->bans[i].user, user) == 0) return &ms->bans[i];
    return NULL;
}

int ms_is_banned(ms_system_t *ms, const char *user) {
    ms_ban_t *b = ms ? find_ban(ms, user) : NULL;
    if (!b) return 0;
    if (b->until == (time_t)-1) return 1;          /* permanent */
    if (time(NULL) < b->until) return 1;
    return 0;                                       /* expired */
}

time_t ms_ban_remaining(ms_system_t *ms, const char *user) {
    ms_ban_t *b = ms ? find_ban(ms, user) : NULL;
    if (!b || !ms_is_banned(ms, user)) return 0;
    if (b->until == (time_t)-1) return (time_t)-1;
    return b->until - time(NULL);
}

int ms_ban_user(ms_system_t *ms, const char *user, long seconds) {
    if (!ms || !user || !*user || ms->ban_count >= MAX_BANS) return 0;
    ms_ban_t *b = find_ban(ms, user);
    if (!b) {
        b = &ms->bans[ms->ban_count++];
        copy_str(b->user, sizeof b->user, user);
        b->until = 0;
    }
    if (seconds < 0) b->until = (time_t)-1;         /* permanent */
    else b->until = time(NULL) + (time_t)seconds;
    return 1;
}

int ms_unban_user(ms_system_t *ms, const char *user) {
    if (!ms) return 0;
    for (size_t i = 0; i < ms->ban_count; i++) {
        if (strcmp(ms->bans[i].user, user) == 0) {
            memmove(&ms->bans[i], &ms->bans[i + 1],
                    (ms->ban_count - i - 1) * sizeof(ms_ban_t));
            ms->ban_count--;
            return 1;
        }
    }
    return 0;
}

/* ---------------- persistence ---------------- */

int ms_save(const ms_system_t *ms, const char *path) {
    if (!ms || !path) return 0;
    FILE *f = fopen(path, "w");
    if (!f) return 0;
    for (size_t i = 0; i < ms->count; i++) {
        const ms_comment_t *c = &ms->comments[i];
        /* text may contain '|' - replace with '/' on save */
        char safe[256];
        copy_str(safe, sizeof safe, c->text);
        for (char *p = safe; *p; p++) if (*p == '|' || *p == '\n') *p = ' ';
        fprintf(f, "%d|%s|%d|%lld|%d|%d|%d|%s\n",
                c->id, c->user, (int)c->role, (long long)c->time,
                c->likes, c->reports, c->pinned, safe);
    }
    fclose(f);
    return 1;
}

int ms_load(ms_system_t *ms, const char *path) {
    if (!ms || !path) return 0;
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[512];
    int max_id = 0;
    while (fgets(line, sizeof line, f)) {
        ms_comment_t tmp;
        memset(&tmp, 0, sizeof tmp);
        /* parse: id|user|role|time|likes|reports|pinned|text */
        char *fields[8] = {0};
        int nf = 0;
        char *p = line;
        while (nf < 8) {
            fields[nf++] = p;
            char *bar = strchr(p, '|');
            if (!bar) break;
            *bar = '\0';
            p = bar + 1;
        }
        if (nf < 8) continue;
        /* last field keeps rest of line (may contain spaces) */
        fields[7][strcspn(fields[7], "\r\n")] = '\0';

        tmp.id = atoi(fields[0]);
        copy_str(tmp.user, sizeof tmp.user, fields[1]);
        tmp.role = (ms_role_t)atoi(fields[2]);
        tmp.time = (time_t)strtoll(fields[3], NULL, 10);
        tmp.likes = atoi(fields[4]);
        tmp.reports = atoi(fields[5]);
        tmp.pinned = atoi(fields[6]);
        copy_str(tmp.text, sizeof tmp.text, fields[7]);

        if (tmp.id > max_id) max_id = tmp.id;
        if (ms->count == ms->cap) {
            size_t ncap = ms->cap * 2;
            ms_comment_t *nc = realloc(ms->comments,
                                      ncap * sizeof(*nc));
            if (!nc) break;
            ms->comments = nc;
            ms->cap = ncap;
        }
        ms->comments[ms->count++] = tmp;
    }
    fclose(f);
    ms->next_id = max_id + 1;
    return 1;
}

/* ---------------- helpers ---------------- */

const char *ms_role_name(ms_role_t role) {
    switch (role) {
    case MS_USER:    return "User";
    case MS_ADVISOR: return "Rating Advisor";
    case MS_MOD:     return "Moderator";
    case MS_LEAD:    return "Leaderboard Moderator";
    case MS_DEV:     return "Developer";
    default:         return "User";
    }
}

const char *ms_role_color(ms_role_t role) {
    switch (role) {
    case MS_ADVISOR: return "#C8FFC8";  /* light green */
    case MS_MOD:     return "#4BFF4B";  /* dark green */
    case MS_DEV:     return "#32FFFF";  /* cyan */
    default:         return "#E8E8F0";
    }
}
