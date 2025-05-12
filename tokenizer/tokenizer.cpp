#include <tokenizer.h>

#include <cctype>
#include <cstdio>

Tokenizer::Tokenizer(std::istream* in) : input_(in) {
    parsers_.push_back(new ConstantParser(input_));
    parsers_.push_back(new BracketParser(input_));
    parsers_.push_back(new SymbolParser(input_));
    parsers_.push_back(new QuoteParser(input_));
    parsers_.push_back(new DotParser(input_));
    parsers_.push_back(new OperatorParser(input_));
    Next();
}

Tokenizer::~Tokenizer() {
    for (auto& parser : parsers_) {
        delete parser;
        parser = nullptr;
    }
}

bool Tokenizer::IsEnd() {
    return is_end_;
}

void Tokenizer::Next() {
    int c = input_->peek();
    while (isspace(c)) {
        input_->get();
        c = input_->peek();
    }
    if (input_->eof()) {
        is_end_ = true;
        return;
    }
    for (auto& parser : parsers_) {
        if (parser->Match()) {
            now_ = parser->GetToken();
            return;
        }
    }
}

Token Tokenizer::GetToken() {
    return now_;
}
