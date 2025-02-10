#pragma once


struct AstNode {
    char data = '\0';
    AstNode* left = nullptr;
    AstNode* right = nullptr;
};
