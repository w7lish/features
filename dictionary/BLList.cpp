
#include "BLList.h"
#include <iostream>
using namespace std;

BlockList* glist = NULL;

Node* node_alloc(void)
{
    cout << "node_alloc: " << endl;
    
    // allocated from pfree first
    if (glist->pfree != DNULL) {
        // cout << "node_alloc:  rerue freed node" << endl;
        Node* node = glist->pool + glist->pfree;
        glist->pfree = glist->pool[glist->pfree].next;
        // cout << "node_alloc: allocated from freed node, new glist->pfree = " << glist->pfree << endl;
        return node;
    } else {
        if (glist->npool < MAX_DLEN) {
            // cout << "node_alloc: returning node at glist->npool = " << glist->npool+1 <<  endl;
            return &glist->pool[glist->npool++];
        }
    }

    cout << "node_alloc: no free space" << endl;
    return NULL;
}

void node_free(Node* node)
{
    cout << "node_free: " << endl;

    if ( node == NULL) {
        cout << "node_free: node pointer is null" << endl;
        return;
    }
    cout << "node_free: free node with value:" << node->data << endl;    
    if (node) {
        node->data[0] = '\0';
        node->next = glist->pfree;
        // cout << "node_free: node=" << node << ", glist->pool=" << glist->pool << ", glist->pfree=" << glist->pfree << endl;
        glist->pfree = node - glist->pool;
        // cout << "node_free: new glist->pfree=" << glist->pfree << endl;
    }
}

Node *node_get(size_t index)
{
    return (index >= DNULL) ? NULL : glist->pool + index;
}

Node *node_next(const Node *node)
{
    return node_get(node->next);
}

// Node *node_push(size_t *head, const char *val)
Node *node_push(const char *val)
{
    cout << "node_push: " << val << endl;
    
    Node* tmp = node_search(val); 
    if ( tmp != NULL ) {
        cout << "node_push: node with value: " << val << " already exists" << endl;
        return tmp;
    }
    
    // inserting new node
    Node *node = node_alloc();
    if (node) {
        strncpy(node->data, val, sizeof(node->data));
        // node->next = *head;
        node->next = glist->head;
        cout << "node_push: node next:" << node->next << endl;
        glist->head = node - glist->pool;
        cout << "node_push: list head: " << glist->head << endl;
        cout << "node_push: pushed data - " << node->data << endl;
    } else {
        cout << "node_push: no space\n";
    }

    return node;
}

void node_pop(size_t *head)
{
    if (*head != DNULL) {
        size_t next = glist->pool[*head].next;

        node_free(&glist->pool[*head]);
        *head = next;
    }
}

Node *node_search(const char *val)
{
    cout << "node_search: " << val << endl;
    Node *ret = NULL;

    // empty
    if (glist->head == DNULL) {
        cout << "node_search: dict empty" << endl;
        return ret;
    }
    
    Node* node = &(glist->pool[glist->head]);
    
    if ( strcmp(node->data, val) == 0 ) {
        cout << "node_search: found match " << node->data << ", val " << val  << endl;
        return node;
    }
    
    int i = 0;
    while ( node->next != DNULL) {
        cout << "node_search: " << i++ << ", node next= " << node->next << endl;
        node = node_next(node);
        if ( strcmp(node->data, val) == 0 ) {
            ret = node;
            cout << "node_search: found " << ret->data << endl;
            break;
        }
    }

    if ( ret == NULL) {
        cout << "not found" << endl;
    }
    
    return ret;
}

// delete
void node_delete(const char *val)
{
    cout << "node_delete: " << val << endl;

    // empty
    if (glist->head == DNULL) {
        cout << "node_delete: dict empty" << endl;
        return;
    }
    
    Node* node = &(glist->pool[glist->head]);
    
    // delete head
    if ( strcmp(node->data, val) == 0 ) {
        cout << "node_delete: found match " << node->data << ", val " << val  << endl;
        glist->head = node->next;
        node_free(node);
        return;
    }
    
    Node* prev = node;
    int i = 0;
    while ( node->next != DNULL) {
        cout << "node_delete: " << i++ << ", node next= " << node->next << endl;
        node = node_next(node);
        // found match , delete the node
        if ( strcmp(node->data, val) == 0 ) {
            cout << "node_delete: found " << node->data << endl;
            prev->next = node->next;
            node_free(node);
            return;
        }
    }

    cout << "node_delete: not found" << endl;
}


// display words
void node_list()
{
    cout << "node_list: " << endl;
    // empty
    if (glist->head == DNULL) {
        cout << "node_list: empty dict" << endl;
        return;
    }
    
    cout << "node_list: word list - ";
    Node* node = &(glist->pool[glist->head]);
    cout << node->data << ", ";
    int i = 1;
    while ( node->next != DNULL && i++) {
        node = node_next(node);
        cout << node->data << ", ";
    }
    cout << " ***num of words:" << i <<  endl;
    
}
