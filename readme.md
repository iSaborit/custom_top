# Custom Top

A basic process monitoring tool for macOS written in C. Displays running processes with basic information like memory usage and CPU time. This is a hobby/learning project, **not** a production-ready tool.

## What It Actually Does

- Displays a list of running processes with PID, user, CPU%, memory, state, and process name
- Sorts processes by CPU%, memory, or PID (user-selectable)
- Filters processes by name using an interactive search bar (`/`)
- Automatically groups child processes under parent processes
- Refreshes every ~50ms (UI thread) with data updated every ~1s (data thread)
- Can exit with `q`

## Project Structure

```
custom_top/
├── src/                    # Source files
│   ├── main.c             # Application entry point (spawns threads)
│   ├── app_state.c        # Shared application state implementation
│   ├── process.c          # Process management
│   ├── sysinfo.c          # System information gathering
│   ├── ui.c               # User interface rendering (UI thread)
│   ├── state.c            # State management
│   └── utils.c            # Utility functions
├── include/               # Header files
│   ├── app_state.h        # Shared application state (thread-safe)
│   ├── process.h          # Process structures and functions
│   ├── sysinfo.h          # System info interface
│   ├── ui.h               # UI interface
│   ├── state.h            # State definitions
│   └── utils.h            # Utility functions
├── CMakeLists.txt         # CMake build configuration
└── Makefile              # Alternative build system
```

## Requirements

- **macOS** - Requires macOS system (uses libproc and Mach APIs)
- **C17** or later
- **ncurses** library (included with macOS)
- **CMake 4.1+** (for building with CMake)

## Building

### Using Make (Recommended - Actually Works)

```bash
make
```

This will compile all source files and create the executable at `bin/mi_programa`.

To clean up build artifacts:

```bash
make clean
```

### Using CMake

CMake support exists in the project (see `CMakeLists.txt`), but CMake is **not installed** on the development machine. It should work on systems with CMake 4.1+ installed:

```bash
mkdir build
cd build
cmake ..
make
```

The compiled executable will be at `build/custom_top`.

**Note:** The CMake build is untested. Use Make instead.

## Usage

```bash
./bin/mi_programa
```

The program will display a table of running processes with their resource usage. Processes are sorted by CPU% by default. Child processes are aggregated under their parent processes.

### Controls

| Key | Action |
|-----|--------|
| `q` / `Q` | Exit the application |
| `c` / `C` | Sort by CPU% (default) |
| `r` / `R` | Sort by RAM |
| `p` / `P` | Sort by PID |
| `/` | Enter search mode (filter by process name) |
| `Enter` | Confirm search |
| `Esc` | Clear search / exit search mode |

## Implementation Details
### Core Components
#### Process Management (`process.h/c`)
- **Opaque Types:** `Process` and `ProcessArray` structures are opaque (implementation hidden from client code)
- `Process` structure internally contains:
  - Process ID (PID) and Parent PID (PPID)
  - Username and process title
  - CPU time snapshots for differential calculation
  - RAM usage in MB
  - Process state (Running, Sleeping, Idle, Stopped, Zombie)
  - Hierarchy flags (is_parent, is_collapsed)

- **Accessor Functions:**
  - Getters: `proc_get_pid()`, `proc_get_ppid()`, `proc_get_gid()`, `proc_get_is_parent()`, `proc_get_user()`, `proc_get_cpu()`, `proc_get_ram()`, `proc_get_state()`, `proc_get_title()`, `proc_get_collapsed()`, `proc_get_cpu_snapshot()`
  - Setters: `proc_set_pid()`, `proc_set_ppid()`, `proc_set_gid()`, `proc_set_is_parent()`, `proc_set_user()`, `proc_set_title_from_pid()`, `proc_set_ram()`, `proc_set_cpu()`, `proc_set_state()`, `proc_set_collapsed()`, `proc_set_cpu_snapshot()`, `proc_add_cpu()`, `proc_add_ram()`

- **ProcessArray Management:**
  - Opaque container for dynamic process collection
  - `proc_array_create()` - Allocates array with given capacity
  - `proc_array_get()` / `proc_array_length()` / `proc_array_set_length()` - Element access
  - `proc_array_order()` - Sorts processes by RAM, CPU, or PID (descending for resource fields)
  - `proc_array_delete()` - Memory cleanup

- **ProcessIterator Pattern:**
  - `proc_iter_create()` - Creates an iterator for a ProcessArray
  - `proc_iter_next()` - Returns next non-collapsed process
  - `proc_iter_destroy()` - Frees iterator resources
  - Automatically skips collapsed child processes during iteration

- **Debug Utilities:**
  - `process_printn()` - Prints first N processes to console

#### System Information (`sysinfo.h/c`)
- `get_process_list()` - Main function that:
  1. Fetches all process IDs using `proc_listpids()`
  2. Collects static process information via `proc_pidinfo()` and `proc_name()`
  3. Takes initial CPU time snapshot
  4. Waits 1 second for CPU time delta measurement
  5. Computes CPU usage percentage
  6. Performs process grouping/collapsing under parent processes

- Uses macOS libproc and Mach APIs:
  - `proc_listpids()` - Get list of all process IDs
  - `proc_pidinfo()` with `PROC_PIDTASKINFO` - Get CPU, memory, thread info
  - `proc_pidinfo()` with `PROC_PIDTBSDINFO` - Get BSD process info (user, state, ppid)
  - `proc_name()` - Get process executable name
  - `mach_absolute_time()` - High-resolution timing

#### Application State (`app_state.h/c`)
- `AppState` - Opaque struct holding the shared `ProcessArray`, sort order, search state, and a `pthread_mutex_t`
- `app_state_create()` / `app_state_destroy()` - Lifecycle management
- Thread-safe accessors:
  - `app_state_set_data()` / `app_state_get_data()` - Update/read current process list
  - `app_state_lock()` / `app_state_unlock()` - Manual mutex control for composite operations
  - `app_state_set_sort()` / `app_state_get_sort()` - Update/read current sort order
  - `app_state_set_search()` / `app_state_get_search()` - Update/read current search query
  - `app_state_set_searching_mode()` / `app_state_get_searching_mode()` - Toggle interactive search input
  - `app_state_should_run()` / `app_state_stop()` - Clean shutdown signalling

#### User Interface (`ui.h/c`)
- `setup_colors()` - Initializes ncurses color pairs (DEFAULT, ALERT, INFO)
- `ui_thread_func()` - UI thread entry point: polls for key input, redraws at ~20 fps
- `draw_layout()` (internal) - Renders the display with:
  - Bordered box around content with title and key hints
  - Header with column labels
  - Process list sorted by the current sort order
  - Column format: PID | USER | CPU% | MEM(MB) | STATE | NAME
  - Skips collapsed child processes from display
  - Bottom bar shows active search query or key legend

#### State Management (`state.h/c`)
- `State` enum: SLEEPING, RUNNING, IDLE, STOP, ZOMBIE
- `get_state_string()` - Converts state enum to readable string
- `convert_run_state()` - Converts macOS BSD process status to State enum

### Process Grouping/Collapsing

Child processes are automatically grouped under their parents:
1. `collapse_on_parent()` - Marks child processes as collapsed and adds their resource usage to the parent
2. `process_grouping_pass()` - Iterates through all processes and collapses those with ppid > 1
3. Special handling: kernel_task (0) and launchd (1) processes are not collapsed
4. Aggregated resources: Child CPU usage and RAM are added to parent totals

### Multi-threaded Architecture

The application uses two POSIX threads managed via `pthread`:

- **Data thread** (`data_thread_func` in `process.c`) — continuously fetches the process list (including the 1-second CPU delta measurement) and updates `AppState` under mutex lock.
- **UI thread** (`ui_thread_func` in `ui.c`) — redraws the screen at ~10 fps, reads sort order and acquires the mutex separately to read the latest `ProcessArray`, and handles keyboard input.

`AppState` (`app_state.h/c`) is the shared, mutex-protected state passed to both threads. `main()` simply spawns both threads and joins them on exit.

### Memory Management

The application:
- Uses **opaque types** (Process, ProcessArray, AppState) for encapsulation and memory safety
- Allocates process array with `calloc()`
- `AppState` owns the current `ProcessArray` and frees the previous one on each data update
- Implements `proc_delete()` and `proc_array_delete()` for cleanup
- Uses **iterator pattern** (`ProcessIterator`) for safe process traversal
- Properly destroys iterators with `proc_iter_destroy()` to prevent leaks

## macOS Specifics

This project is specifically designed for macOS and uses:
- `<libproc.h>` - Process enumeration and information APIs
- `<sys/proc.h>` - Process structure definitions
- `<mach/mach_time.h>` - High-resolution timing for CPU calculations
- `<pwd.h>` - User name resolution from UID
- macOS process APIs for accurate real-time system monitoring

The CPU usage calculation is based on elapsed time between two measurements:
1. Initial snapshot: `pti_total_user + pti_total_system`
2. After 1 second delay: Measure again
3. CPU% = (delta / elapsed_ticks) × 100%

---
## Future Enhancements
This roadmap outlines the critical functionalities and structural improvements planned for upcoming versions to transition from a basic monitor to a robust system utility.
### Core Functionality & UI

1. **Interactive Navigation:** Implementation of arrow-key scrolling to explore the process list.
2. **Process Management:** * **Expand/Collapse:** Ability to toggle child processes.
   * **Signal Control:** Integrated capability to kill or send signals to specific PIDs.
3. ~~**Search & Filtering:** Dynamic filtering by process name and custom sorting (CPU, Memory, PID, State).~~ ✅ **Done** — sorting by CPU, RAM, and PID is implemented via keyboard shortcuts; live process-name search via `/` key.
4. **Enhanced UX:**
   * **Color Coding:** Visual distinction based on process state (Running, Sleeping, Zombie).
   * **Help Menu:** Instant access to keyboard shortcuts via the 'h' key.
### Performance & Stability
1. ~~**Multi-threaded Architecture:** Separation of the CPU measurement logic (which requires a 1-second delay) from the UI thread to eliminate the current interface freezing.~~ ✅ **Done** — data and UI run in separate threads.
2. **Data Optimization:**
   * **Lazy Loading:** Efficient rendering for large process lists.
   * **Information Caching:** Reduction of redundant system calls between refresh cycles.
3. **Error Handling:** Graceful management of permission errors when accessing system-level or protected processes.

### Quality & Reliability

1. **Validation:** Implementation of unit tests and performance benchmarks.
2. **Memory Management:** Systematic testing for memory leaks and optimization of data structures.
3. **Technical Logging:** Exporting internal events and errors to a dedicated log file.
### Platform Support
1. **Compatibility:** Full support for Linux and Unix-based environments.
   * **Build System:** Refined Makefile for cross-platform targets.



## Known Issues

* None currently known.

## License

Created by Iker Saborit López

---

**Last Updated**: March 2026












