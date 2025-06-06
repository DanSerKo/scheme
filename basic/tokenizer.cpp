#include <tokenizer.h>

#include <cctype>
#include <cstdint>
#include <cstdio>

Tokenizer::Tokenizer(std::istream* in) : input_(in), now_() {
    parsers_.push_back(new ConstantParser(input_));
    parsers_.push_back(new BracketParser(input_));
    parsers_.push_back(new SymbolParser(input_));
    parsers_.push_back(new QuoteParser(input_));
    parsers_.push_back(new DotParser(input_));
    parsers_.push_back(new OperatorParser(input_));
    is_end_ = 0;
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
    int64_t c = input_->peek();
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
