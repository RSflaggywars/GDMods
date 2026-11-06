# GD-Mod-System / c

A Geometry Dash-style moderator/comment system as a **C library** you can
drop into any C game.

## Files

- `modsystem.h` - public API
- `modsystem.c` - implementation (pure C99, only libc)
- `demo.c`     - working example
- `build.bat`  - compiles demo.exe on Windows

## Use in your game

```c
#include "modsystem.h"

ms_system_t *ms = ms_create();
ms_add_comment(ms, "Player1", MS_USER, "first!");
ms_add_comment(ms, "RobTop",  MS_DEV,  "gg");

/* user tries to comment - check ban first */
if (!ms_is_banned(ms, "h8r_m3"))
    ms_add_comment(ms, "h8r_m3", MS_USER, "this sucks");

/* a moderator deletes it and bans the user */
if (ms_can_moderate(MS_MOD)) {
    ms_delete_comment(ms, 3);
    ms_ban_user(ms, "h8r_m3", 3600);   /* 1 hour; <0 = permanent */
}

/* iterate comments */
for (size_t i = 0; i < ms_count(ms); i++) {
    const ms_comment_t *c = ms_get(ms, i);
    printf("%s: %s\n", c->user, c->text);
}

/* save/load between sessions */
ms_save(ms, "comments.txt");
ms_destroy(ms);
```

## Roles (same as the real game)

| Role | Constant | Color | Powers |
|---|---|---|---|
| User | `MS_USER` | #E8E8F0 | comment |
| Rating Advisor | `MS_ADVISOR` | #C8FFC8 | highlighted comments |
| Moderator | `MS_MOD` | #4BFF4B | delete, ban/unban |
| Leaderboard Mod | `MS_LEAD` | #E8E8F0 | leaderboard tools |
| Developer | `MS_DEV` | #32FFFF | everything |

## API highlights

- `ms_add_comment / ms_delete_comment / ms_get / ms_find / ms_count`
- `ms_report_comment / ms_like_comment / ms_pin_comment`
- `ms_ban_user / ms_unban_user / ms_is_banned / ms_ban_remaining`
- `ms_can_moderate(role)` - true for MOD/LEAD/DEV
- `ms_save / ms_load` - plain-text persistence
- `ms_role_name / ms_role_color` - UI helpers

## Build

```
gcc -Wall -Wextra -O2 -o demo.exe demo.c modsystem.c
```

For your own game: compile `modsystem.c` alongside your sources and
`#include "modsystem.h"`.
