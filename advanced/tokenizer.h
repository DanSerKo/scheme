#pragma once

#include "token_parsers.h"

#include <istream>
#include <vector>

class Tokenizer {
public:
    Tokenizer(std::istream* in);
    ~Tokenizer();

    bool IsEnd();

    void Next();

    Token GetToken();

private:
    bool is_end_;
    std::istream* input_;
    Token now_;
    std::vector<TokenParser*> parsers_;
};