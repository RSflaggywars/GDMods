/*
 * demo.c - example of using modsystem in your own C game.
 *
 * Build:  gcc -Wall -Wextra -O2 -o demo demo.c modsystem.c
 * Run:    ./demo
 */

#include <stdio.h>
#include <string.h>

#include "modsystem.h"

int main(void) {
    ms_system_t *ms = ms_create();
    if (!ms) { fprintf(stderr, "out of memory\n"); return 1; }

    /* seed a few comments like a live level page */
    ms_add_comment(ms, "RobTop", MS_DEV,
                   "thanks for playing :)");
    ms_add_comment(ms, "CubeGuy99", MS_USER,
                   "HOW IS THIS EVEN POSSIBLE");
    ms_add_comment(ms, "NeonCat", MS_ADVISOR,
                   "solid level! the wave section is clean.");
    ms_add_comment(ms, "h8r_m3", MS_USER,
                   "this level is garbage delete it");

    /* moderation demo */
    ms_pin_comment(ms, 1, 1);
    ms_report_comment(ms, 4);   /* users report the rude comment */
    ms_report_comment(ms, 4);
    ms_ban_user(ms, "h8r_m3", 3600);   /* 1 hour comment ban */

    /* a mod deletes the reported comment */
    if (ms_can_moderate(MS_MOD))
        ms_delete_comment(ms, 4);

    /* print the comment list, GD-style */
    for (size_t i = 0; i < ms_count(ms); i++) {
        const ms_comment_t *c = ms_get(ms, i);
        const char *badge = ms_can_moderate(c->role)
                            || c->role == MS_ADVISOR ? " [M]" : "";
        printf("%s%s: %s%s\n", c->user,
               badge, c->text,
               c->pinned ? "  (pinned)" : "");
        printf("    role=%s  likes=%d  reports=%d\n",
               ms_role_name(c->role), c->likes, c->reports);
    }

    /* ban status */
    printf("\nh8r_m3 banned? %s (remaining ~%ld min)\n",
           ms_is_banned(ms, "h8r_m3") ? "yes" : "no",
           (long)(ms_ban_remaining(ms, "h8r_m3") / 60));

    /* persistence demo */
    ms_save(ms, "comments.txt");
    ms_destroy(ms);

    ms = ms_create();
    ms_load(ms, "comments.txt");
    printf("reloaded %zu comments from comments.txt\n", ms_count(ms));

    ms_destroy(ms);
    return 0;
}
