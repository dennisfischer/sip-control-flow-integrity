#include "StackAnalysis.hpp"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
typedef struct node {
    char *value;
    struct node *next;
} node_t;

node_t *stack = nullptr;
int stack_len = 0;

void registerFunction(char functionName[]);

void deregisterFunction(const char functionName[]);


using namespace std;

void registerFunction(char functionName[]) {
    //printf("In function: %s\n", functionName);

    node_t *next = stack;
    while (next != nullptr) {
        if (next->value == functionName) {
            return;
        }
        next = next->next;
    }

    stack_len++;
    auto *new_node = (node_t *) malloc(sizeof(node_t));
    new_node->value = functionName;
    new_node->next = stack;
    stack = new_node;

    printf("Stack: ");
    next = stack;
    while (next != nullptr) {
        printf("%s ; ", next->value);
        next = next->next;
    }
    printf("\n");
}

void deregisterFunction(const char functionName[]) {
    //printf("Exit function: %s\n", functionName);

    if (stack == nullptr) {
        fprintf(stderr, "Error: No function on shadow stack that can be poped!\n");
        return;
    }
    if (stack->value != functionName) {
        return;
    }

    stack_len--;
    node_t *next_node = stack->next;
    free(stack);
    stack = next_node;

    node_t *next = stack;
    printf("Stack: ");
    while (next != nullptr) {
        printf("%s ; ", next->value);
        next = next->next;
    }
    printf("\n");
}

/*
* Reads known edges from file 'X.txt' line by line.
* Returns pointer to array of known edges and number of edges read.
*/
void readEdges(map<string, int> *mapping, bool ***adj_mat, size_t *edges_count) {
    ifstream ifs;
    vector<string> tmp;

    string line, buffer;

    int i = 0;

    ifs.open("graph.txt");

    while (getline(ifs, line)) {
        if (line.back() == '\n') {
            line.erase(line.size() - 1);
        }
        stringstream ss(line);
        while (ss >> buffer) {
            tmp.push_back(buffer);
            //cout << "Buffer: " << buffer << endl;
            if (mapping->find(buffer) == mapping->end()) {
                mapping->insert(pair<string, int>(buffer, i));
                i++;
            }
        }
    }
    ifs.close();

    size_t edges = mapping->size();
    *edges_count = edges;
    cout << "Edges: " << edges << endl;

    // Malloc adjacency matrix
    *adj_mat = (bool **) malloc(edges * sizeof(bool *));
    if (*adj_mat == nullptr) {
        fprintf(stderr, "Failed to malloc adj_mat\n");
        exit(1);
    }

    for (int id = 0; id < edges; id++) {
        (*adj_mat)[id] = (bool *) malloc(edges * sizeof(bool));
        if ((*adj_mat)[id] == NULL) {
            fprintf(stderr, "Failed to malloc adj_mat[%d]\n", id);
            exit(1);
        }
        memset((*adj_mat)[id], 0, sizeof(bool) * edges);
    }

    // Build matrix
    for (int idx = 0; idx < tmp.size(); idx += 2) {
        if (mapping->find(tmp[idx]) == mapping->end()) {
            mapping->insert(pair<string, int>(tmp[idx], i));
            i++;
        }
        if (mapping->find(tmp[idx + 1]) == mapping->end()) {
            mapping->insert(pair<string, int>(tmp[idx + 1], i));
            i++;
        }
        // update adjacency matrix
        (*adj_mat)[(*mapping)[tmp[idx]]][(*mapping)[tmp[idx + 1]]] = 1;
    }
}

void response() {
    cout << "Response mechanism.\n";
    //exit(1);
}

void verify(map<string, int> *mapping, bool ***adj_mat) {
    node_t *curr = stack, *next = curr->next, *tmp;
    string curr_name, next_name;

    do {
        // The stack is calledFct, calleeFct, so we need to check if edge "next -> curr" exists
        curr_name = curr->value;
        next_name = next->value;
        cout << "Checking edge: " << next_name << " -> " << curr_name << "\n";

        if (mapping->find(curr_name) == mapping->end()) {
            cout << "Could not find function " << curr_name << endl;
            response();
            return;
        }
        if (mapping->find(next_name) == mapping->end()) {
            cout << "Could not find function " << next_name << endl;
            response();
            return;
        }

        int row = (*mapping)[curr_name];
        int column = (*mapping)[next_name];
        if (!((*adj_mat)[row][column])) {
            // This call is not legitimate -> response mechanism.
            cout << "Error row " << row << ", column " << column << endl;
            response();
            return;
        }

        tmp = next;
        curr = next;
        next = tmp->next;
    } while (next != nullptr);
}

void verifyStack() {
    //registerFunction("main");
    //registerFunction("bar");
    //registerFunction("foobar");
    // TODO: verify file? I suppose here that it is a regular file
    bool **adj_mat;
    size_t edges_count;
    map<string, int> mapping;
    readEdges(&mapping, &adj_mat, &edges_count);

    for (auto it = mapping.begin(); it != mapping.end(); it++) {
        cout << it->first << " at " << it->second << endl;
    }

    for (int j = 0; j < edges_count; j++) {
        for (int k = 0; k < edges_count; k++) {
            cout << adj_mat[j][k];
        }
        cout << endl;
    }
    verify(&mapping, &adj_mat);

    for (int i = 0; i < edges_count; i++) {
        free(adj_mat[i]);
    }
    free(adj_mat);
}

#pragma clang diagnostic pop