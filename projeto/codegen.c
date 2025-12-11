#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 1024
#define MAX_CHILDREN 50

typedef enum {
    NODE_PROG,
    NODE_FUNC,
    NODE_VAR,
    NODE_TYPE,
    NODE_IF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_ASSIGN,
    NODE_CALL,
    NODE_OUTPUT,
    NODE_INPUT,
    NODE_OP,
    NODE_ID,
    NODE_CONST,
    NODE_UNKNOWN
} NodeKind;

typedef struct TreeNode {
    NodeKind kind;
    char name[256];
    char op[32];
    int value;
    int depth;
    struct TreeNode* children[MAX_CHILDREN];
    int child_count;
} TreeNode;

int temp_counter = 0;
int label_counter = 0;
FILE* output;

char* newTemp() {
    static char temp[32];
    sprintf(temp, "t%d", temp_counter++);
    return strdup(temp);
}

char* newLabel() {
    static char label[32];
    sprintf(label, "L%d", label_counter++);
    return strdup(label);
}

NodeKind getNodeKind(const char* line) {
    if (strstr(line, "Stmt: Prog")) return NODE_PROG;
    if (strstr(line, "Stmt: Func:")) return NODE_FUNC;
    if (strstr(line, "Stmt: Var:")) return NODE_VAR;
    if (strstr(line, "Exp: Type:")) return NODE_TYPE;
    if (strstr(line, "Stmt: If")) return NODE_IF;
    if (strstr(line, "Stmt: While")) return NODE_WHILE;
    if (strstr(line, "Stmt: Return:")) return NODE_RETURN;
    if (strstr(line, "Stmt: Assign")) return NODE_ASSIGN;
    if (strstr(line, "Stmt: Call")) return NODE_CALL;
    if (strstr(line, "Stmt: Output")) return NODE_OUTPUT;
    if (strstr(line, "Stmt: Input")) return NODE_INPUT;
    if (strstr(line, "Exp: Op:")) return NODE_OP;
    if (strstr(line, "Exp: Id:")) return NODE_ID;
    if (strstr(line, "Exp: Const:")) return NODE_CONST;
    return NODE_UNKNOWN;
}

int getDepth(const char* line) {
    int depth = 0;
    const char* p = line;
    while (strncmp(p, "----", 4) == 0) {
        depth++;
        p += 4;
    }
    return depth;
}

void extractName(const char* line, char* name) {
    const char* colon = strrchr(line, ':');
    if (colon) {
        colon++;
        while (*colon == ' ') colon++;
        strcpy(name, colon);
        // Remove newline
        char* nl = strchr(name, '\n');
        if (nl) *nl = '\0';
        // Remove trailing whitespace
        int len = strlen(name);
        while (len > 0 && isspace(name[len-1])) {
            name[len-1] = '\0';
            len--;
        }
    } else {
        name[0] = '\0';
    }
}

TreeNode* createTreeNode(NodeKind kind, int depth) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    node->kind = kind;
    node->depth = depth;
    node->child_count = 0;
    node->name[0] = '\0';
    node->op[0] = '\0';
    node->value = 0;
    return node;
}

TreeNode* parseAST(FILE* file) {
    char line[MAX_LINE];
    TreeNode* stack[100];
    int stack_top = -1;
    TreeNode* root = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        int depth = getDepth(line);
        NodeKind kind = getNodeKind(line);
        
        if (kind == NODE_UNKNOWN) continue;
        
        TreeNode* node = createTreeNode(kind, depth);
        
        char extracted[256];
        extractName(line, extracted);
        
        if (kind == NODE_FUNC || kind == NODE_VAR || kind == NODE_TYPE || 
            kind == NODE_RETURN || kind == NODE_ID) {
            strcpy(node->name, extracted);
        } else if (kind == NODE_OP) {
            strcpy(node->op, extracted);
        } else if (kind == NODE_CONST) {
            node->value = atoi(extracted);
        }
        
        // Pop stack until we find the parent
        while (stack_top >= 0 && stack[stack_top]->depth >= depth) {
            stack_top--;
        }
        
        if (stack_top >= 0) {
            // Add as child to parent
            TreeNode* parent = stack[stack_top];
            if (parent->child_count < MAX_CHILDREN) {
                parent->children[parent->child_count++] = node;
            }
        } else {
            // This is the root
            root = node;
        }
        
        stack[++stack_top] = node;
    }
    
    return root;
}

char* generateExpression(TreeNode* node);

char* generateOp(TreeNode* node) {
    if (node->child_count < 2) return NULL;
    
    char* left = generateExpression(node->children[0]);
    char* right = generateExpression(node->children[1]);
    
    if (!left || !right) return NULL;
    
    char* result = newTemp();
    char op_symbol[10] = "";
    
    if (strcmp(node->op, "ADD") == 0) strcpy(op_symbol, "+");
    else if (strcmp(node->op, "SUB") == 0) strcpy(op_symbol, "-");
    else if (strcmp(node->op, "MULT") == 0) strcpy(op_symbol, "*");
    else if (strcmp(node->op, "DIV") == 0) strcpy(op_symbol, "/");
    else if (strcmp(node->op, "<") == 0) strcpy(op_symbol, "<");
    else if (strcmp(node->op, "<=") == 0) strcpy(op_symbol, "<=");
    else if (strcmp(node->op, ">") == 0) strcpy(op_symbol, ">");
    else if (strcmp(node->op, ">=") == 0) strcpy(op_symbol, ">=");
    else if (strcmp(node->op, "==") == 0) strcpy(op_symbol, "==");
    else if (strcmp(node->op, "!=") == 0) strcpy(op_symbol, "!=");
    
    fprintf(output, "  %s = %s %s %s\n", result, left, op_symbol, right);
    free(left);
    free(right);
    return result;
}

char* generateExpression(TreeNode* node) {
    if (!node) return NULL;
    
    switch (node->kind) {
        case NODE_CONST: {
            char* temp = (char*)malloc(32);
            sprintf(temp, "%d", node->value);
            return temp;
        }
        case NODE_ID: {
            return strdup(node->name);
        }
        case NODE_OP: {
            return generateOp(node);
        }
        case NODE_INPUT: {
            char* temp = newTemp();
            fprintf(output, "  %s = input()\n", temp);
            return temp;
        }
        case NODE_CALL: {
            char* temp = newTemp();
            if (node->child_count > 0 && node->children[0]->kind == NODE_ID) {
                TreeNode* funcId = node->children[0];
                fprintf(output, "  ");
                // Print arguments
                for (int i = 0; i < funcId->child_count; i++) {
                    char* arg = generateExpression(funcId->children[i]);
                    fprintf(output, "param %s\n  ", arg);
                    free(arg);
                }
                fprintf(output, "%s = call %s, %d\n", temp, funcId->name, funcId->child_count);
            }
            return temp;
        }
        default:
            return NULL;
    }
}

void generateStatement(TreeNode* node);

void generateAssign(TreeNode* node) {
    if (node->child_count < 2) return;
    
    TreeNode* lhs = node->children[0];
    TreeNode* rhs = node->children[1];
    
    char* rhsValue = generateExpression(rhs);
    if (!rhsValue) return;
    
    if (lhs->kind == NODE_ID) {
        fprintf(output, "  %s = %s\n", lhs->name, rhsValue);
    }
    free(rhsValue);
}

void generateIf(TreeNode* node) {
    if (node->child_count < 2) return;
    
    char* condition = generateExpression(node->children[0]);
    char* labelFalse = newLabel();
    char* labelEnd = newLabel();
    
    fprintf(output, "  ifFalse %s goto %s\n", condition, labelFalse);
    
    // Then branch
    generateStatement(node->children[1]);
    
    if (node->child_count > 2) {
        // Has else branch
        fprintf(output, "  goto %s\n", labelEnd);
        fprintf(output, "%s:\n", labelFalse);
        generateStatement(node->children[2]);
        fprintf(output, "%s:\n", labelEnd);
        free(labelEnd);
    } else {
        fprintf(output, "%s:\n", labelFalse);
    }
    free(condition);
    free(labelFalse);
}

void generateWhile(TreeNode* node) {
    if (node->child_count < 2) return;
    
    char* labelStart = newLabel();
    char* labelEnd = newLabel();
    
    fprintf(output, "%s:\n", labelStart);
    
    char* condition = generateExpression(node->children[0]);
    fprintf(output, "  ifFalse %s goto %s\n", condition, labelEnd);
    
    // Loop body
    generateStatement(node->children[1]);
    
    fprintf(output, "  goto %s\n", labelStart);
    fprintf(output, "%s:\n", labelEnd);
    free(condition);
    free(labelStart);
    free(labelEnd);
}

void generateReturn(TreeNode* node) {
    if (node->child_count > 0) {
        char* retValue = generateExpression(node->children[0]);
        fprintf(output, "  return %s\n", retValue);
        free(retValue);
    } else {
        fprintf(output, "  return\n");
    }
}

void generateOutput(TreeNode* node) {
    if (node->child_count > 0) {
        char* value = generateExpression(node->children[0]);
        fprintf(output, "  output %s\n", value);
        free(value);
    }
}

void generateCall(TreeNode* node) {
    if (node->child_count > 0 && node->children[0]->kind == NODE_ID) {
        TreeNode* funcId = node->children[0];
        fprintf(output, "  ");
        // Print arguments
        for (int i = 0; i < funcId->child_count; i++) {
            char* arg = generateExpression(funcId->children[i]);
            fprintf(output, "param %s\n  ", arg);
            free(arg);
        }
        fprintf(output, "call %s, %d\n", funcId->name, funcId->child_count);
    }
}

void generateStatement(TreeNode* node) {
    if (!node) return;
    
    switch (node->kind) {
        case NODE_PROG:
            for (int i = 0; i < node->child_count; i++) {
                generateStatement(node->children[i]);
            }
            break;
        case NODE_ASSIGN:
            generateAssign(node);
            break;
        case NODE_IF:
            generateIf(node);
            break;
        case NODE_WHILE:
            generateWhile(node);
            break;
        case NODE_RETURN:
            generateReturn(node);
            break;
        case NODE_OUTPUT:
            generateOutput(node);
            break;
        case NODE_CALL:
            generateCall(node);
            break;
        case NODE_TYPE:
            // Process function or variable declaration
            if (node->child_count > 0) {
                TreeNode* child = node->children[0];
                if (child->kind == NODE_FUNC) {
                    fprintf(output, "\nfunc %s:\n", child->name);
                    // Process function body
                    for (int i = 0; i < child->child_count; i++) {
                        generateStatement(child->children[i]);
                    }
                    fprintf(output, "endfunc\n");
                } else if (child->kind == NODE_VAR) {
                    // Variable declaration
                    if (child->child_count > 0 && child->children[0]->kind == NODE_CONST) {
                        // Array declaration
                        fprintf(output, "  var %s %s[%d]\n", node->name, child->name, child->children[0]->value);
                    } else {
                        // Simple variable declaration
                        fprintf(output, "  var %s %s\n", node->name, child->name);
                    }
                }
            }
            break;
        default:
            break;
    }
}

void generateCode(TreeNode* root) {
    fprintf(output, "# Intermediate Code (Three-Address Code)\n");
    fprintf(output, "# Generated from AST\n\n");
    
    generateStatement(root);
}

void freeTree(TreeNode* node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) {
        freeTree(node->children[i]);
    }
    free(node);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <arvore.txt> <codigo_intermediario.txt>\n", argv[0]);
        return 1;
    }
    
    FILE* astFile = fopen(argv[1], "r");
    if (!astFile) {
        perror("Erro ao abrir arquivo de AST");
        return 1;
    }
    
    output = fopen(argv[2], "w");
    if (!output) {
        perror("Erro ao abrir arquivo de saída");
        fclose(astFile);
        return 1;
    }
    
    TreeNode* root = parseAST(astFile);
    fclose(astFile);
    
    if (root) {
        generateCode(root);
        freeTree(root);
        printf("Codigo intermediario gerado com sucesso em %s\n", argv[2]);
    } else {
        fprintf(stderr, "Erro ao parsear AST\n");
    }
    
    fclose(output);
    return 0;
}
