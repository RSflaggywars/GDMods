# GD-Mod-System

A demo of Geometry Dash's moderator system — the comment moderation
setup from the game, recreated in a single HTML file.

## Open it

Double-click `index.html` — no server, no install, works offline.
Everything is stored in the browser's localStorage.

## What's inside

- **Roles** (pick from the dropdown):
  - **User** — normal player
  - **Rating Advisor** — light-green (`#C8FFC8`) comments
  - **Moderator** — dark-green (`#4BFF4B`) comments, can **delete**
    comments and **ban** users
  - **Leaderboard Moderator** — leaderboard management role
  - **Developer** — cyan (`#32FFFF`) comments, all powers
- **Moderator tools** (visible when you're Mod/Lead/Dev):
  - `delete` — remove any comment
  - `ban...` — comment-ban a user for 1 min / 10 min / 1 hr / 24 hr / permanent
  - `unban` — lift a ban
- **User tools**: `report` button on every comment (adds a ⚑ counter)
- Banned users can't post until the ban expires; banned comments are
  greyed out but stay visible
- Spam protection: posting the same text twice in a row is rejected
- 200-char comment limit, like the real game
- Pinned comments sort to the top (seeded example: a "RobTop" comment)

## Try it

1. Open `index.html`
2. Post a comment as **User**
3. Switch role to **Moderator** — delete/ban buttons appear
4. Ban "Player1", switch back to User — you can't comment anymore
5. Click **reset demo** to wipe everything

## Importing in Games/projects
Importing in games is supported under the C file.
Ur game has to be written in C !
