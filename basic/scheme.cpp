#include "scheme.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <sstream>

// Interpreter func
std::string Interpreter::Run(const std::string& str) {
    std::stringstream ss{str};
    Tokenizer tokenizer{&ss};
    std::shared_ptr<Object> root = Read(&tokenizer);
    root = ToDo(root, this);
    return Show(root);
}

void Interpreter::AddProcessing(const std::string& s, MakeOp func) {
    proccessings_[s] = func;
}

// order func
std::shared_ptr<Object> IsCell(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto first = As<Cell>(node)->GetFirst();
    auto second = As<Cell>(node)->GetSecond();
    if (Is<Symbol>(first) && As<Symbol>(first)->GetName() != "#f" &&
        As<Symbol>(first)->GetName() != "#t") {
        if (!interpreter->proccessings_.count(As<Symbol>(first)->GetName())) {
            throw NameError("what?");
        }
        return interpreter->proccessings_[As<Symbol>(first)->GetName()](second, interpreter);
    }
    return node;
}

std::shared_ptr<Object> ToDo(std::shared_ptr<Object> node, Interpreter* interpreter) {
    if (node == nullptr) {
        return nullptr;
    }
    if (Is<Cell>(node)) {
        return IsCell(node, interpreter);
    }
    if (Is<Number>(node)) {
        return node;
    }
    std::string name = As<Symbol>(node)->GetName();
    if (name == "#t" || name == "#f") {
        return node;
    }
    /*if (defined.count(name)) {
        return ToDo(defined[name]); // замена определенных переменных (некст таска)
    }*/
    throw NameError("what?");
}

std::string ShowTree(std::shared_ptr<Object> node) {
    if (node == nullptr) {
        return "";
    }
    if (Is<Number>(node) || Is<Symbol>(node)) {
        throw RuntimeError("pair isn't correct for quote");
    }
    auto first = As<Cell>(node)->GetFirst();
    auto second = As<Cell>(node)->GetSecond();
    if (Is<Cell>(second)) {
        return ShowAll(first) + " " + ShowTree(second);
    }
    if (second != nullptr) {
        return ShowAll(first) + " . " + ShowAll(second);
    }
    return ShowAll(first);
}

std::string ShowAll(std::shared_ptr<Object> node) {
    if (Is<Number>(node)) {
        return std::to_string(As<Number>(node)->GetValue());
    }
    if (Is<Symbol>(node)) {
        return As<Symbol>(node)->GetName();
    }
    return "(" + ShowTree(node) + ")";
}

std::string Show(std::shared_ptr<Object> node) {
    if (Is<Number>(node)) {
        return std::to_string(As<Number>(node)->GetValue());
    }
    if (Is<Symbol>(node) &&
        (As<Symbol>(node)->GetName() == "#f" || As<Symbol>(node)->GetName() == "#t")) {
        return As<Symbol>(node)->GetName();
    }
    if (Is<Cell>(node)) {
        auto quote = As<Cell>(node)->GetFirst();
        if (Is<Symbol>(quote) && As<Symbol>(quote)->GetName() == "quote") {
            return ShowTree(As<Cell>(node)->GetSecond());
        }
    }
    throw RuntimeError("Out isn't correct");
}

// Tools
std::shared_ptr<Object> OneArg(
    std::shared_ptr<Object> node, Interpreter* interpreter,
    std::function<std::shared_ptr<Object>(std::shared_ptr<Object>)> func) {
    if (node == nullptr) {
        throw RuntimeError("there is arg?");
    }
    if (!Is<Cell>(node)) {
        throw RuntimeError("pair isn't correct for func");
    }
    if (As<Cell>(node)->GetSecond() != nullptr) {
        throw RuntimeError("more arg, than needed");
    }
    node = As<Cell>(node)->GetFirst();
    node = ToDo(node, interpreter);
    return func(node);
}

std::shared_ptr<Object> BoolOp(
    std::shared_ptr<Object> node, Interpreter* interpreter, std::shared_ptr<Object> meta,
    std::function<bool(std::shared_ptr<Object>, std::shared_ptr<Object>, std::shared_ptr<Object>&)>
        func) {
    if (node == nullptr) {
        return meta;
    }
    if (!Is<Cell>(node)) {
        throw RuntimeError("pair isn't correct for func");
    }
    auto first = As<Cell>(node)->GetFirst();
    auto second = As<Cell>(node)->GetSecond();
    first = ToDo(first, interpreter);
    std::shared_ptr<Object> result;
    if (func(first, meta, result) || second == nullptr) {
        return result;
    }
    second = ToDo(second, interpreter);
    return BoolOp(second, interpreter, first, func);
}

void ToVec(std::shared_ptr<Object> node, Interpreter* interpreter,
           std::vector<std::shared_ptr<Object>>& vec, bool is_static = false) {
    if (!Is<Cell>(node)) {
        vec.push_back(node);
        return;
    }
    auto first = As<Cell>(node)->GetFirst();
    auto second = As<Cell>(node)->GetSecond();
    if (!is_static) {
        first = ToDo(first, interpreter);
        second = ToDo(second, interpreter);
    }
    vec.push_back(first);
    ToVec(second, interpreter, vec, is_static);
}

std::shared_ptr<Object> VecToAST(const std::vector<std::shared_ptr<Object>>& vec) {
    if (vec.empty()) {
        throw RuntimeError("Empty vector");
    }
    std::shared_ptr<Object> result = vec.back();
    for (int i = vec.size() - 2; i >= 0; i--) {
        result = std::make_shared<Cell>(vec[i], result);
    }
    return result;
}

std::shared_ptr<Object> BinaryOp(
    std::shared_ptr<Object> node, Interpreter* interpreter, std::shared_ptr<Object> meta,
    std::function<std::shared_ptr<Object>(std::shared_ptr<Object>, std::shared_ptr<Object>)> func) {
    std::vector<std::shared_ptr<Object>> vec;
    ToVec(node, interpreter, vec);
    if (vec.empty() || vec.back() != nullptr) {
        throw RuntimeError("pair isn't correct for func");
    }
    vec.pop_back();
    std::shared_ptr<Object> result = meta;
    if (!meta) {
        if (vec.empty()) {
            throw RuntimeError("Arg can't empry");
        }
        result = vec[0];
        vec.erase(vec.begin());
    }
    for (auto& num : vec) {
        result = func(result, num);
    }
    return result;
}

std::shared_ptr<Object> MakeQuote(std::shared_ptr<Object> node, Interpreter*) {
    return std::make_shared<Cell>(std::make_shared<Symbol>("quote"),
                                  std::make_shared<Cell>(node, nullptr));
}

std::shared_ptr<Object> Demining(std::shared_ptr<Object> node, Interpreter* interpreter) {
    if (!Is<Cell>(node)) {
        throw RuntimeError("incorrect demining");
    }
    node = ToDo(node, interpreter);
    auto first = As<Cell>(node)->GetFirst();
    auto second = As<Cell>(node)->GetSecond();
    if (!Is<Symbol>(first) || As<Symbol>(first)->GetName() != "quote") {
        throw RuntimeError("incorrect demining");
    }
    return second;
}

std::vector<std::shared_ptr<Object>> VecArg(std::shared_ptr<Object> node, Interpreter* interpreter,
                                            bool is_static = false) {

    if (!Is<Cell>(node)) {
        throw RuntimeError("incorrect quote");
    }
    auto first = As<Cell>(node)->GetFirst();
    auto second = As<Cell>(node)->GetSecond();
    if (second != nullptr) {
        throw RuntimeError("more arg, than needed");
    }
    try {
        node = Demining(first, interpreter);
    } catch (...) {
        return {};
    }
    if (!Is<Cell>(node)) {
        throw RuntimeError("incorrect quote");
    }
    first = As<Cell>(node)->GetFirst();
    second = As<Cell>(node)->GetSecond();
    if (second != nullptr) {
        throw RuntimeError("incorrect quote");
    }
    std::vector<std::shared_ptr<Object>> result;
    ToVec(first, interpreter, result, is_static);
    return result;
}

// other
std::shared_ptr<Object> Quote(std::shared_ptr<Object> node, Interpreter*) {
    return std::make_shared<Cell>(std::make_shared<Symbol>("quote"), node);
}

std::shared_ptr<Object> IsBoolean(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node) {
        if (Is<Symbol>(node) &&
            (As<Symbol>(node)->GetName() == "#f" || As<Symbol>(node)->GetName() == "#t")) {
            return std::make_shared<Symbol>("#t");
        }
        return std::make_shared<Symbol>("#f");
    };
    return OneArg(node, interpreter, f);
}

std::shared_ptr<Object> IsNumber(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node) {
        if (Is<Number>(node)) {
            return std::make_shared<Symbol>("#t");
        }
        return std::make_shared<Symbol>("#f");
    };
    return OneArg(node, interpreter, f);
}

std::shared_ptr<Object> IsNull(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto vec = VecArg(node, interpreter);
    if (vec.size() == 1 && vec[0] == nullptr) {
        return std::make_shared<Symbol>("#t");
    }
    return std::make_shared<Symbol>("#f");
}

std::shared_ptr<Object> IsList(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto vec = VecArg(node, interpreter);
    if (!vec.empty() && vec.back() == nullptr) {
        return std::make_shared<Symbol>("#t");
    }
    return std::make_shared<Symbol>("#f");
}

std::shared_ptr<Object> IsPair(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto vec = VecArg(node, interpreter);
    while (!vec.empty() && vec.back() == nullptr) {
        vec.pop_back();
    }
    if (vec.size() == 2) {
        return std::make_shared<Symbol>("#t");
    }
    return std::make_shared<Symbol>("#f");
}

std::shared_ptr<Object> Not(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node) {
        if (Is<Symbol>(node) && As<Symbol>(node)->GetName() == "#f") {
            return std::make_shared<Symbol>("#t");
        }
        return std::make_shared<Symbol>("#f");
    };
    return OneArg(node, interpreter, f);
}

std::shared_ptr<Object> Abs(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node) {
        if (!Is<Number>(node)) {
            throw RuntimeError("this isn't correct arg");
        }
        return std::make_shared<Number>(std::abs(As<Number>(node)->GetValue()));
    };
    return OneArg(node, interpreter, f);
}

std::shared_ptr<Object> Equal(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node, std::shared_ptr<Object> meta,
                std::shared_ptr<Object>& result) {
        if (Is<Number>(node)) {
            result = std::make_shared<Symbol>("#t");
            if (!Is<Symbol>(meta) && As<Number>(meta)->GetValue() != As<Number>(node)->GetValue()) {
                result = std::make_shared<Symbol>("#f");
                return 1;
            }
            return 0;
        }
        throw RuntimeError("this isn't correct arg");
    };
    return BoolOp(node, interpreter, std::make_shared<Symbol>("#t"), f);
}

std::shared_ptr<Object> More(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node, std::shared_ptr<Object> meta,
                std::shared_ptr<Object>& result) {
        if (Is<Number>(node)) {
            result = std::make_shared<Symbol>("#t");
            if (!Is<Symbol>(meta) && As<Number>(meta)->GetValue() <= As<Number>(node)->GetValue()) {
                result = std::make_shared<Symbol>("#f");
                return 1;
            }
            return 0;
        }
        throw RuntimeError("this isn't correct arg");
    };
    return BoolOp(node, interpreter, std::make_shared<Symbol>("#t"), f);
}

std::shared_ptr<Object> MoreOrEq(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node, std::shared_ptr<Object> meta,
                std::shared_ptr<Object>& result) {
        if (Is<Number>(node)) {
            result = std::make_shared<Symbol>("#t");
            if (!Is<Symbol>(meta) && As<Number>(meta)->GetValue() < As<Number>(node)->GetValue()) {
                result = std::make_shared<Symbol>("#f");
                return 1;
            }
            return 0;
        }
        throw RuntimeError("this isn't correct arg");
    };
    return BoolOp(node, interpreter, std::make_shared<Symbol>("#t"), f);
}

std::shared_ptr<Object> Less(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node, std::shared_ptr<Object> meta,
                std::shared_ptr<Object>& result) {
        if (Is<Number>(node)) {
            result = std::make_shared<Symbol>("#t");
            if (!Is<Symbol>(meta) && As<Number>(meta)->GetValue() >= As<Number>(node)->GetValue()) {
                result = std::make_shared<Symbol>("#f");
                return 1;
            }
            return 0;
        }
        throw RuntimeError("this isn't correct arg");
    };
    return BoolOp(node, interpreter, std::make_shared<Symbol>("#t"), f);
}

std::shared_ptr<Object> LessOrEq(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node, std::shared_ptr<Object> meta,
                std::shared_ptr<Object>& result) {
        if (Is<Number>(node)) {
            result = std::make_shared<Symbol>("#t");
            if (!Is<Symbol>(meta) && As<Number>(meta)->GetValue() > As<Number>(node)->GetValue()) {
                result = std::make_shared<Symbol>("#f");
                return 1;
            }
            return 0;
        }
        throw RuntimeError("this isn't correct arg");
    };
    return BoolOp(node, interpreter, std::make_shared<Symbol>("#t"), f);
}

std::shared_ptr<Object> And(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node, std::shared_ptr<Object>,
                std::shared_ptr<Object>& result) {
        result = node;
        if (Is<Symbol>(node) && As<Symbol>(node)->GetName() == "#f") {
            return 1;
        }
        return 0;
    };
    return BoolOp(node, interpreter, std::make_shared<Symbol>("#t"), f);
}

std::shared_ptr<Object> Or(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> node, std::shared_ptr<Object>,
                std::shared_ptr<Object>& result) {
        result = node;
        if (Is<Symbol>(node) && As<Symbol>(node)->GetName() != "#f") {
            return 1;
        }
        return 0;
    };
    return BoolOp(node, interpreter, std::make_shared<Symbol>("#f"), f);
}

std::shared_ptr<Object> Add(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> first, std::shared_ptr<Object> second) {
        if (!Is<Number>(first) || !Is<Number>(second)) {
            throw RuntimeError("this isn't correct arg");
        }
        return std::make_shared<Number>(As<Number>(first)->GetValue() +
                                        As<Number>(second)->GetValue());
    };
    return BinaryOp(node, interpreter, std::make_shared<Number>(0), f);
}

std::shared_ptr<Object> Sub(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> first, std::shared_ptr<Object> second) {
        if (!Is<Number>(first) || !Is<Number>(second)) {
            throw RuntimeError("this isn't correct arg");
        }
        return std::make_shared<Number>(As<Number>(first)->GetValue() -
                                        As<Number>(second)->GetValue());
    };
    return BinaryOp(node, interpreter, nullptr, f);
}

std::shared_ptr<Object> Mult(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> first, std::shared_ptr<Object> second) {
        if (!Is<Number>(first) || !Is<Number>(second)) {
            throw RuntimeError("this isn't correct arg");
        }
        return std::make_shared<Number>(As<Number>(first)->GetValue() *
                                        As<Number>(second)->GetValue());
    };
    return BinaryOp(node, interpreter, std::make_shared<Number>(1), f);
}

std::shared_ptr<Object> Div(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> first, std::shared_ptr<Object> second) {
        if (!Is<Number>(first) || !Is<Number>(second)) {
            throw RuntimeError("this isn't correct arg");
        }
        if (As<Number>(second)->GetValue() == 0) {
            throw RuntimeError("div on zero");
        }
        return std::make_shared<Number>(As<Number>(first)->GetValue() /
                                        As<Number>(second)->GetValue());
    };
    return BinaryOp(node, interpreter, nullptr, f);
}

std::shared_ptr<Object> Max(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> first, std::shared_ptr<Object> second) {
        if (!Is<Number>(first) || !Is<Number>(second)) {
            throw RuntimeError("this isn't correct arg");
        }
        return std::make_shared<Number>(
            std::max(As<Number>(first)->GetValue(), As<Number>(second)->GetValue()));
    };
    if (node == nullptr) {
        throw RuntimeError("Isn't correct for empty");
    }
    return BinaryOp(node, interpreter,
                    std::make_shared<Number>(std::numeric_limits<int64_t>::min()), f);
}

std::shared_ptr<Object> Min(std::shared_ptr<Object> node, Interpreter* interpreter) {
    auto f = [](std::shared_ptr<Object> first, std::shared_ptr<Object> second) {
        if (!Is<Number>(first) || !Is<Number>(second)) {
            throw RuntimeError("this isn't correct arg");
        }
        return std::make_shared<Number>(
            std::min(As<Number>(first)->GetValue(), As<Number>(second)->GetValue()));
    };
    if (node == nullptr) {
        throw RuntimeError("Isn't correct for empty");
    }
    return BinaryOp(node, interpreter,
                    std::make_shared<Number>(std::numeric_limits<int64_t>::max()), f);
}

std::shared_ptr<Object> Car(std::shared_ptr<Object> node, Interpreter* interpreter) {
    std::vector<std::shared_ptr<Object>> vec = VecArg(node, interpreter, true);
    if (vec.empty() || (vec.size() == 1 && vec[0] == nullptr)) {
        throw RuntimeError("EmptyList");
    }
    return MakeQuote(vec[0], interpreter);
}

std::shared_ptr<Object> Cdr(std::shared_ptr<Object> node, Interpreter* interpreter) {
    std::vector<std::shared_ptr<Object>> vec = VecArg(node, interpreter, true);
    if (vec.empty() || (vec.size() == 1 && vec[0] == nullptr)) {
        throw RuntimeError("EmptyList");
    }
    vec.erase(vec.begin());

    return MakeQuote(VecToAST(vec), interpreter);
}

std::shared_ptr<Object> Cons(std::shared_ptr<Object> node, Interpreter* interpreter) {
    std::vector<std::shared_ptr<Object>> vec;
    ToVec(node, interpreter, vec);
    if (vec.size() != 3) {
        throw RuntimeError("Need two arg");
    }
    return MakeQuote(std::make_shared<Cell>(vec[0], vec[1]), interpreter);
}

std::shared_ptr<Object> List(std::shared_ptr<Object> node, Interpreter* interpreter) {
    std::vector<std::shared_ptr<Object>> vec;
    ToVec(node, interpreter, vec);
    return MakeQuote(VecToAST(vec), interpreter);
}

std::shared_ptr<Object> ListRef(std::shared_ptr<Object> node, Interpreter* interpreter) {
    if (!Is<Cell>(node)) {
        throw RuntimeError("Need two arg");
    }
    auto first = make_shared<Cell>(As<Cell>(node)->GetFirst(), nullptr);
    auto second = As<Cell>(node)->GetSecond();
    if (!Is<Cell>(second)) {
        throw RuntimeError("Need two arg");
    }
    second = As<Cell>(second)->GetFirst();
    if (!Is<Number>(second)) {
        throw RuntimeError("index isn't correct");
    }
    int64_t index = As<Number>(second)->GetValue();
    std::vector<std::shared_ptr<Object>> vec = VecArg(first, interpreter, true);
    if (vec.empty() || vec.back() != nullptr) {
        throw RuntimeError("it isn't list");
    }
    vec.pop_back();
    if (index < 0 || index >= static_cast<int64_t>(vec.size())) {
        throw RuntimeError("index isn't correct");
    }
    return MakeQuote(vec[index], interpreter);
}

std::shared_ptr<Object> ListTail(std::shared_ptr<Object> node, Interpreter* interpreter) {
    if (!Is<Cell>(node)) {
        throw RuntimeError("Need two arg");
    }
    auto first = make_shared<Cell>(As<Cell>(node)->GetFirst(), nullptr);
    auto second = As<Cell>(node)->GetSecond();
    if (!Is<Cell>(second)) {
        throw RuntimeError("Need two arg");
    }
    second = As<Cell>(second)->GetFirst();
    if (!Is<Number>(second)) {
        throw RuntimeError("index isn't correct");
    }
    int64_t index = As<Number>(second)->GetValue();
    std::vector<std::shared_ptr<Object>> vec = VecArg(first, interpreter, true);
    if (vec.empty() || vec.back() != nullptr) {
        throw RuntimeError("it isn't list");
    }
    if (index < 0 || index >= static_cast<int64_t>(vec.size())) {
        throw RuntimeError("index isn't correct");
    }
    std::reverse(vec.begin(), vec.end());
    for (size_t i = 0; i < static_cast<size_t>(index); i++) {
        vec.pop_back();
    }
    std::reverse(vec.begin(), vec.end());

    return MakeQuote(VecToAST(vec), interpreter);
}