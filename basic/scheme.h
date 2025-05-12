#pragma once

#include "object.h"
#include "parser.h"

#include <functional>
#include <map>
#include <memory>
#include <string>

class Interpreter;
using MakeOp = std::function<std::shared_ptr<Object>(std::shared_ptr<Object>, Interpreter*)>;

std::shared_ptr<Object> ToDo(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> IsCell(std::shared_ptr<Object> node, Interpreter* interpreter);
std::string ShowTree(std::shared_ptr<Object> node);
std::string Show(std::shared_ptr<Object> node);
std::string ShowAll(std::shared_ptr<Object> node);

std::shared_ptr<Object> IsBoolean(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> IsNumber(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> IsPair(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> IsNull(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> IsList(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Not(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Quote(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Equal(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> More(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> MoreOrEq(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Less(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> LessOrEq(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> And(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Or(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Add(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Sub(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Mult(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Div(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Max(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Min(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Abs(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Car(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Cdr(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> Cons(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> List(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> ListRef(std::shared_ptr<Object> node, Interpreter* interpreter);
std::shared_ptr<Object> ListTail(std::shared_ptr<Object> node, Interpreter* interpreter);

class Interpreter {
public:
    Interpreter() {
        AddProcessing("boolean?", IsBoolean);
        AddProcessing("number?", IsNumber);
        AddProcessing("pair?", IsPair);
        AddProcessing("null?", IsNull);
        AddProcessing("list?", IsList);
        AddProcessing("not", Not);
        AddProcessing("quote", Quote);
        AddProcessing("=", Equal);
        AddProcessing(">", More);
        AddProcessing(">=", MoreOrEq);
        AddProcessing("<", Less);
        AddProcessing("<=", LessOrEq);
        AddProcessing("and", And);
        AddProcessing("or", Or);
        AddProcessing("+", Add);
        AddProcessing("-", Sub);
        AddProcessing("*", Mult);
        AddProcessing("/", Div);
        AddProcessing("max", Max);
        AddProcessing("min", Min);
        AddProcessing("abs", Abs);
        AddProcessing("car", Car);
        AddProcessing("cdr", Cdr);
        AddProcessing("cons", Cons);
        AddProcessing("list", List);
        AddProcessing("list-ref", ListRef);
        AddProcessing("list-tail", ListTail);
    }

    std::string Run(const std::string& str);

private:
    friend std::shared_ptr<Object> ToDo(std::shared_ptr<Object> node, Interpreter* interpreter);
    friend std::shared_ptr<Object> IsCell(std::shared_ptr<Object> node, Interpreter* interpreter);

    void AddProcessing(const std::string& s, MakeOp func);
    std::map<std::string, MakeOp> proccessings_;
};
