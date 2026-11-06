/*
 * modsystem.h - a Geometry Dash-style moderator/comment system for C games.
 *
 * Drop-in library: compile modsystem.c with your game, #include this header.
 * Pure C99, zero dependencies, in-memory storage with optional file save/load.
 *
 * Roles mirror the real game:
 *   MS_USER     normal player - can comment, can be banned
 *   MS_ADVISOR  Rating Advisor - highlighted comments
 *   MS_MOD      Moderator      - can delete comments, ban/unban users
 *   MS_LEAD     Leaderboard Moderator
 *   MS_DEV      Developer      - all powers, highlighted comments
 */

#ifndef MODSYSTEM_H
#define MODSYSTEM_H

#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MS_USER = 0,
    MS_ADVISOR = 1,
    MS_MOD = 2,
    MS_LEAD = 3,
    MS_DEV = 4
} ms_role_t;

typedef struct {
    int      id;
    char     user[32];
    ms_role_t role;
    char     text[256];
    time_t   time;
    int      likes;
    int      reports;
    int      pinned;
} ms_comment_t;

typedef struct {
    char   user[32];
    time_t until;      /* 0 = not banned, (time_t)-1 = permanent */
} ms_ban_t;

typedef struct ms_system ms_system_t;

/* lifecycle */
ms_system_t *ms_create(void);
void         ms_destroy(ms_system_t *ms);

/* comments */
int  ms_add_comment(ms_system_t *ms, const char *user, ms_role_t role,
                    const char *text);              /* returns comment id, -1 on fail */
int  ms_delete_comment(ms_system_t *ms, int id);    /* mods+ only enforced by caller */
int  ms_report_comment(ms_system_t *ms, int id);    /* +1 report count */
int  ms_like_comment(ms_system_t *ms, int id);
int  ms_pin_comment(ms_system_t *ms, int id, int pinned);

/* access */
size_t             ms_count(const ms_system_t *ms);
const ms_comment_t *ms_get(const ms_system_t *ms, size_t index);
const ms_comment_t *ms_find(const ms_system_t *ms, int id);

/* moderation */
int  ms_can_moderate(ms_role_t role);   /* MOD, LEAD, DEV */
int  ms_is_banned(ms_system_t *ms, const char *user);          /* 1 = banned now */
time_t ms_ban_remaining(ms_system_t *ms, const char *user);    /* secs left, -1 perm, 0 none */
int  ms_ban_user(ms_system_t *ms, const char *user, long seconds); /* seconds<0 = permanent */
int  ms_unban_user(ms_system_t *ms, const char *user);

/* persistence - plain text file, one comment per line:
 *   id|user|role|time|likes|reports|pinned|text            */
int  ms_save(const ms_system_t *ms, const char *path);
int  ms_load(ms_system_t *ms, const char *path);

/* role helpers */
const char *ms_role_name(ms_role_t role);
const char *ms_role_color(ms_role_t role);  /* hex string, GD-ish */

#ifdef __cplusplus
}
#endif

#endif /* MODSYSTEM_H */
