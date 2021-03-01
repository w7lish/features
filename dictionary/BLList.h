#pragma once

#include <stdio.h>
#include <string.h>
#include <map>
#include <vector>

typedef struct BNode BNode;
typedef struct BList BList;

// # define MAX_DNODE 1     // Fixed number of words supported by this memory block
# define MAX_DLEN  5       // Max. number of list nodes

# define DNULL (MAX_DLEN+1) // NULL value

// extra defines
# define SERVER_MTYPE 27L
# define CLIENT_MTYPE 42L
# define MAX_TEXT_LEN 64    // Max of word size : 64-1

using namespace std;

const std::vector<string> commands = { "insert", "delete", "search" , "list", "exit" }; 
const std::map<string, int> msg_ids = {{ "insert", 0}, {"delete", 1}, {"search", 2}, {"list", 3}, {"exit", 4}}; 

// structure for message queue 
struct Message { 
    int id = 0;
    char text[MAX_TEXT_LEN] = {'\0'}; 
}; 

struct MsgBuffer { 
    long type;
    Message msg; 
}; 

// Dictionary Node
struct Node {
    char data[MAX_TEXT_LEN];
    size_t next;
};

struct BlockList {
    Node pool[MAX_DLEN];    // fixed size space for nodes
    size_t npool;           // used space in pool
    size_t pfree;           // pointer to re-use freed nodes
    size_t head;            // global list head
    // size_t npool = 0;           // used space in pool
    // size_t pfree = DNULL;           // pointer to re-use freed nodes
    // size_t head = DNULL;            // global list head
};

// allocate a new instance of Node
Node* node_alloc(void);

// deallocate the node
void node_free(Node* node);

// retrive instance at index
Node *node_get(size_t index);

// get next node
Node *node_next(const Node *node);

// push new node with value val in front of the list
// Node *node_push(size_t *head, const char *val);
Node *node_push(const char *val);

// pop head node
void node_pop(size_t *head);

// search the value from the list
Node *node_search(const char *val);

// delete
void node_delete(const char *val);

// display words
void node_list();
