#pragma once

#include <tokens.h>

#include <istream>
#include <set>

class TokenParser {
public:
    TokenParser(std::istream* input) {
        input_ = input;
    }
    virtual ~TokenParser() = default;
    virtual bool Match() = 0;
    virtual Token GetToken() = 0;

protected:
    std::istream* input_;
};

class ConstantParser : public TokenParser {
public:
    ConstantParser(std::istream* input, int flag = 1) : TokenParser(input) {
        flag_ = flag;
    }
    bool Match() override {
        char c = input_->peek();
        return std::isdigit(c);
    }
    Token GetToken() override {
        ConstantToken result;
        int x;
        *input_ >> x;
        result.value = x * flag_;
        return result;
    }

private:
    int flag_;
};

class BracketParser : public TokenParser {
public:
    BracketParser(std::istream* input) : TokenParser(input) {
    }
    bool Match() override {
        char c = input_->peek();
        return c == '(' || c == ')';
    }
    Token GetToken() override {
        char c = input_->get();
        if (c == '(') {
            return BracketToken::OPEN;
        }
        return BracketToken::CLOSE;
    }
};

class SymbolParser : public TokenParser {
public:
    SymbolParser(std::istream* input) : TokenParser(input) {
        starts_ = {'<', '=', '>', '*', '/', '#'};
        for (char i = 'a'; i <= 'z'; i++) {
            starts_.insert(i);
        }
        for (char i = 'A'; i <= 'Z'; i++) {
            starts_.insert(i);
        }
        chars_ = starts_;
        for (char i = '0'; i <= '9'; i++) {
            chars_.insert(i);
        }
        chars_.insert('?');
        chars_.insert('!');
        chars_.insert('-');
    }
    bool Match() override {
        char c = input_->peek();
        return starts_.count(c);
    }
    Token GetToken() override {
        SymbolToken result;
        char c = input_->peek();
        while (chars_.count(c)) {
            result.name.push_back(c);
            input_->get();
            c = input_->peek();
        }
        return result;
    }

private:
    std::set<char> starts_;
    std::set<char> chars_;
};

class QuoteParser : public TokenParser {
public:
    QuoteParser(std::istream* input) : TokenParser(input) {
    }
    bool Match() override {
        char c = input_->peek();
        return c == '\'';
    }
    Token GetToken() override {
        input_->get();
        return QuoteToken();
    }
};

class DotParser : public TokenParser {
public:
    DotParser(std::istream* input) : TokenParser(input) {
    }
    bool Match() override {
        char c = input_->peek();
        return c == '.';
    }
    Token GetToken() override {
        input_->get();
        return DotToken();
    }
};

class OperatorParser : public TokenParser {
public:
    OperatorParser(std::istream* input) : TokenParser(input) {
    }
    bool Match() override {
        char c = input_->peek();
        return c == '+' || c == '-';
    }
    Token GetToken() override {
        char c = input_->get();
        ConstantParser check(input_, c == '-' ? -1 : 1);
        if (!input_->eof() && check.Match()) {
            return check.GetToken();
        }
        SymbolToken result;
        result.name.push_back(c);
        return result;
    }
};