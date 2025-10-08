# Collabwrite-C
CollabWrite-C is a multifunctional real-time collaborative text editor built in pure C using core DSA and networking concepts. It features a linked-list text buffer, undo/redo stacks, versioning with snapshots (tree), and multi-client TCP communication with thread-safe synchronization for concurrent editing.
# CollabWrite-C  
Multifunctional Real-Time Collaborative Text Editor using Data Structures and Algorithms in C  

# Overview

**CollabWrite-C** is a multifunctional, real-time **collaborative text editor** built purely in **C** using core **Data Structures and Algorithms (DSA)** concepts and **TCP networking**.  
It allows multiple clients to connect simultaneously and edit a shared text document in real-time.  

The project demonstrates the practical use of:
- **Linked Lists** → for dynamic text buffer management  
- **Stacks** → for Undo/Redo functionality  
- **Trees** → for versioning and branching (snapshot storage)  
- **Threads & Mutexes** → for concurrent client handling  
- **Sockets (TCP/IP)** → for real-time networking and synchronization  

# Features

- Multi-user collaborative editing in real time  
- Linked-list-based text buffer for line-wise editing  
- Undo and Redo operations using stacks  
- Version control via snapshot tree (branching & restore)  
- Thread-safe synchronization using POSIX mutex locks  
- Multi-client communication with TCP sockets  
- Simple CLI command-based interface for users  

# Project Structure

CollabWrite-C/
│
├── include/
│ ├── text_buffer.h # Doubly linked list text buffer with undo/redo
│ ├── stack.h # Stack implementation for edit operations
│ ├── version.h # Snapshot & version tree (for branching)
│ ├── network.h # Networking utilities and constants
│
├── src/
│ ├── text_buffer.c # Implements text buffer, insert, delete, update
│ ├── stack.c # Stack operations (push, pop, free)
│ ├── version.c # Snapshot creation, restore, and version listing
│ ├── server.c # Handles clients, broadcasting, commands, threads
│ ├── client.c # CLI client to send commands & receive updates
│
├── Makefile # Build automation (compiles server & client)
├── README.md # Project documentation
└── LICENSE # Open-source license (MIT recommended)


# Supported Commands :-

1. INS <pos> <text>	Insert text at given position
2. DEL <pos> <len>	Delete a specific number of characters
3. UNDO	Undo the last operation
4. REDO	Redo the last undone operation
5. SNAP	Create a snapshot (version save)
6. LIST_VERSIONS	Show all saved versions in a tree structure
7. RESTORE <id>	Restore the document to a specific snapshot version
8. GET	Retrieve and print the current document
9. QUIT	Disconnect from the server

# Data Structures Used :-

Text Buffer	Doubly Linked List	Stores document line-by-line for efficient insertion/deletion
Undo/Redo	Stack	Stores previous operations for undo/redo actions
Versioning	Tree	Stores snapshots (each as a node), enabling branching and history
Networking	Threads + Mutex	Handles concurrent clients and synchronized edits

# Example Workflow :-
Client 1: INS 0 Hello

Client 2: INS 5 World

Both see → Hello World

Client 1: DEL 5 6 → Text becomes Hello

Client 2: UNDO → Restores Hello World

Client 1: SNAP → Saves version v1

Client 2: LIST_VERSIONS → Displays snapshot tree

Client 1: RESTORE 1 → Restores version v1

# Algorithmic Concepts :-
Linked List Traversal & Manipulation

Stack Push/Pop Operations

Tree Traversal for Version Listing

Mutex Locks for Critical Sections

Multi-threaded TCP Communication

DFS for Version Node Search

# Future Enhancements :-
🔹 Merge functionality between versions
🔹 File save/load (persistence)
🔹 Authentication system (user-level sessions)
🔹 GUI interface using ncurses or GTK
🔹 Diff/merge algorithm for branch merging
🔹 CRDT-based conflict resolution

# Contributors :-
Esheta
Nishandeep Singh
Parv Saini
Tejas Joshi

Guided by: Siddhant Thapliyal

# License
This project is licensed under the MIT License — you’re free to use, modify, and distribute with attribution.

# Acknowledgements
This project demonstrates how core DSA concepts can be applied to build a fully functional, real-time collaborative editor from scratch using only the C language.
