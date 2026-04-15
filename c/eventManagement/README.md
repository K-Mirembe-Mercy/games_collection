# Event Management System — C

A production-grade command-line application written in **pure C (C11)** that manages events using efficient data structures and persistent CSV storage.

---

## Features

| Feature | Detail |
|---|---|
| **Full CRUD** | Add, view, edit, cancel, delete events |
| **Rich event model** | Title, description, location, organiser, start/end time, priority, category, tags, attendees, recurrence |
| **Hash map (O(1) lookup)** | Events indexed by ID using Knuth multiplicative hashing with chaining |
| **Min-heap (priority queue)** | Events ordered by start time for instant "next event" query |
| **Doubly-linked list** | Ordered iteration and O(1) insertion/removal |
| **Persistent storage** | CSV serialisation with backslash-escaping for embedded commas |
| **Search & filter** | Case-insensitive title search; filter by category or priority |
| **Statistics dashboard** | Breakdowns by category, priority, active/cancelled/upcoming counts |
| **ANSI colour UI** | Colour-coded priorities and categories; interactive menus |

---

## Architecture

```
event_manager/
├── event_manager.h   # All types, constants, and function declarations
├── event_ds.c        # Data structures: list, heap, hash map; CRUD operations; queries
├── event_utils.c     # CSV persistence, datetime helpers, print utilities, statistics
├── event_ui.c        # Interactive terminal UI loop and input forms
├── main.c            # Entry point
└── Makefile
```

The codebase deliberately separates concerns:
- **Header** acts as the contract — all public API lives there.
- **event_ds.c** is the pure algorithmic core with zero I/O.
- **event_utils.c** handles serialisation and display formatting.
- **event_ui.c** handles all user interaction, keeping it isolated.

---

## Data Structures

### Hash Map (`HashMap`)
- Open chaining with `HASH_TABLE_SIZE = 256` buckets.
- Knuth multiplicative hash: `(id × 2654435761) mod 256`.
- O(1) average-case insert, lookup, delete.

### Min-Heap (`EventHeap`)
- Dynamic array; doubles capacity when full.
- Keyed on `start_time` — `em_next_event()` is O(n) scan for first non-cancelled future event, with O(log n) push/pop.
- Supports arbitrary removal via `heap_remove_by_id` (O(n) scan + O(log n) re-heapify).

### Doubly-Linked List (`EventList`)
- Intrusive links embedded in `Event` itself — no extra node allocation.
- O(1) append and removal given a pointer.
- Used for ordered full-list traversal and result sets.

---

## Building

Requires GCC (or Clang) with C11 support and POSIX (`strptime`).

```bash
make          # build
make run      # build and launch
make clean    # remove build artefacts
```

The binary accepts an optional path argument for the database file:

```bash
./event_manager                  # uses events.csv in current directory
./event_manager ~/my_events.csv  # custom path
```

---

## Usage

```
MAIN MENU
──────────────────────────────────────────
1  Add event
2  List all events
3  View event details
4  Edit event
5  Cancel / delete event
6  Search by title
7  Filter events
8  Upcoming events
9  Next event
S  Statistics
W  Save to disk
Q  Quit (auto-saves)
```

### Event fields
- **Title** (required)
- **Description**, **Location**, **Organiser** (optional)
- **Start / End time** — format `YYYY-MM-DD HH:MM`
- **Priority** — Low · Medium · High · Urgent
- **Category** — Conference · Meeting · Workshop · Social · Webinar · Other
- **Recurring** — yes/no; if yes, repeat interval in days
- **Tags** — up to 8 free-text tags
- **Attendees** — up to 64 names

---

## CSV Format

One event per line; fields separated by commas; literal commas and backslashes within a field are backslash-escaped.

```
#next_id=3
#id,title,description,...
1,Team Kickoff,Q3 planning session,Conference Room A,...
```

---

## Key Design Decisions

1. **Intrusive linked list** — avoids a separate node allocation per event and improves cache locality.
2. **ID-keyed hash map** — makes `em_find_by_id` and `em_remove_event` independent of list size.
3. **Result lists return clones** — callers own their result sets and can free them independently without invalidating the live data.
4. **Single header** — keeps the public API visible in one place; simplifies inclusion in larger projects.
5. **CSV persistence** — human-readable, portable, zero external dependencies.

---

## Compiler Flags

```
-Wall -Wextra -Wpedantic -std=c11 -D_POSIX_C_SOURCE=200809L -O2 -g
```

Builds **zero warnings** under GCC 13 / Clang 17.
