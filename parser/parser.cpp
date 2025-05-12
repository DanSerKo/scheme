#include <parser.h>

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("program hasn't end");
    }
    Token c = tokenizer->GetToken();
    if (BracketToken* x = std::get_if<BracketToken>(&c)) {
        if (*x == BracketToken::OPEN) {
            tokenizer->Next();
            return ReadList(tokenizer);
        }
    }
    if (ConstantToken* x = std::get_if<ConstantToken>(&c)) {
        tokenizer->Next();
        return std::make_shared<Number>(x->value);
    }
    if (SymbolToken* x = std::get_if<SymbolToken>(&c)) {
        tokenizer->Next();
        return std::make_shared<Symbol>(x->name);
    }

    throw SyntaxError("It isn't Symbol or Constant");
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("program hasn't end");
    }
    Token c = tokenizer->GetToken();
    if (BracketToken* x = std::get_if<BracketToken>(&c)) {
        if (*x == BracketToken::CLOSE) {
            tokenizer->Next();
            return nullptr;
        }
    }

    std::shared_ptr<Object> first;
    first = Read(tokenizer);
    if (tokenizer->IsEnd()) {
        throw SyntaxError("program hasn't end");
    }
    c = tokenizer->GetToken();
    if (std::get_if<DotToken>(&c)) {
        tokenizer->Next();
        std::shared_ptr<Object> result = std::make_shared<Cell>(first, Read(tokenizer));
        if (tokenizer->IsEnd()) {
            throw SyntaxError("program hasn't end");
        }
        c = tokenizer->GetToken();
        if (BracketToken* x = std::get_if<BracketToken>(&c)) {
            if (*x != BracketToken::CLOSE) {
                throw SyntaxError("list hasn't end");
            }
        } else {
            throw SyntaxError("list hasn't end");
        }
        tokenizer->Next();
        return result;
    }
    return std::make_shared<Cell>(first, ReadList(tokenizer));
}
