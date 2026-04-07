#include "ExpressionParser.h"
#include <cmath>
#include <cctype>
#include <stdexcept>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

ExpressionParser::ExpressionParser()
    : pos(0), varX(0.0), error(false) {}

void ExpressionParser::setVariable(double xValue) { varX = xValue; }

double ExpressionParser::evaluate(const std::string& expression) {
    expr = expression;
    pos = 0;
    error = false;
    errorMsg.clear();

    if (expr.empty()) {
        error = true;
        errorMsg = "Empty expression";
        return 0.0;
    }

    double result = parseExpression();

    skipWhitespace();
    if (!error && pos < expr.size()) {
        error = true;
        errorMsg = "Unexpected character: '";
        errorMsg += expr[pos];
        errorMsg += "'";
    }
    return result;
}

double ExpressionParser::evaluate(const std::string& expression, double x) {
    setVariable(x);
    return evaluate(expression);
}

bool ExpressionParser::hasError() const { return error; }
const std::string& ExpressionParser::getError() const { return errorMsg; }

char ExpressionParser::peek() {
    skipWhitespace();
    if (pos < expr.size()) return expr[pos];
    return '\0';
}

char ExpressionParser::get() {
    skipWhitespace();
    if (pos < expr.size()) return expr[pos++];
    return '\0';
}

void ExpressionParser::skipWhitespace() {
    while (pos < expr.size() && std::isspace(static_cast<unsigned char>(expr[pos])))
        ++pos;
}

// expression = term (('+' | '-') term)*
double ExpressionParser::parseExpression() {
    double left = parseTerm();
    while (!error) {
        char c = peek();
        if (c == '+') { get(); left += parseTerm(); }
        else if (c == '-') { get(); left -= parseTerm(); }
        else break;
    }
    return left;
}

// term = unary (('*' | '/') unary)*
double ExpressionParser::parseTerm() {
    double left = parseUnary();
    while (!error) {
        char c = peek();
        if (c == '*') { get(); left *= parseUnary(); }
        else if (c == '/') {
            get();
            double d = parseUnary();
            if (d == 0.0) { left = std::numeric_limits<double>::quiet_NaN(); }
            else { left /= d; }
        }
        else break;
    }
    return left;
}

// unary = ('-' | '+') unary | power
double ExpressionParser::parseUnary() {
    char c = peek();
    if (c == '-') { get(); return -parseUnary(); }
    if (c == '+') { get(); return parseUnary(); }
    return parsePower();
}

// power = atom ('^' unary)?
double ExpressionParser::parsePower() {
    double base = parseAtom();
    if (peek() == '^') {
        get();
        double exp = parseUnary();  // right-associative
        return std::pow(base, exp);
    }
    return base;
}

// atom = number | '(' expression ')' | function '(' expression ')' | variable | constant
double ExpressionParser::parseAtom() {
    if (error) return 0.0;

    char c = peek();

    // Parenthesized expression
    if (c == '(') {
        get();
        double val = parseExpression();
        if (peek() != ')') {
            error = true;
            errorMsg = "Missing closing parenthesis";
            return 0.0;
        }
        get();
        return val;
    }

    // Number literal
    if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
        skipWhitespace();
        size_t start = pos;
        while (pos < expr.size() && (std::isdigit(static_cast<unsigned char>(expr[pos])) || expr[pos] == '.'))
            ++pos;
        // Scientific notation
        if (pos < expr.size() && (expr[pos] == 'e' || expr[pos] == 'E')) {
            ++pos;
            if (pos < expr.size() && (expr[pos] == '+' || expr[pos] == '-')) ++pos;
            while (pos < expr.size() && std::isdigit(static_cast<unsigned char>(expr[pos]))) ++pos;
        }
        std::string numStr = expr.substr(start, pos - start);
        try {
            return std::stod(numStr);
        }
        catch (...) {
            error = true;
            errorMsg = "Invalid number: " + numStr;
            return 0.0;
        }
    }

    // Identifier (function, variable, or constant)
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        skipWhitespace();
        size_t start = pos;
        while (pos < expr.size() && (std::isalnum(static_cast<unsigned char>(expr[pos])) || expr[pos] == '_'))
            ++pos;
        std::string name = expr.substr(start, pos - start);

        // Constants
        if (name == "pi") return M_PI;
        if (name == "e" && peek() != '(') return M_E;

        // Variable
        if (name == "x") return varX;

        // Function call
        if (peek() == '(') {
            return parseFunction(name);
        }

        error = true;
        errorMsg = "Unknown identifier: " + name;
        return 0.0;
    }

    error = true;
    if (c == '\0')
        errorMsg = "Unexpected end of expression";
    else {
        errorMsg = "Unexpected character: '";
        errorMsg += c;
        errorMsg += "'";
    }
    return 0.0;
}

double ExpressionParser::parseFunction(const std::string& name) {
    get(); // consume '('
    double arg = parseExpression();
    if (peek() != ')') {
        error = true;
        errorMsg = "Missing closing parenthesis for function " + name;
        return 0.0;
    }
    get(); // consume ')'

    if (name == "sin")   return std::sin(arg);
    if (name == "cos")   return std::cos(arg);
    if (name == "tan")   return std::tan(arg);
    if (name == "asin")  return std::asin(arg);
    if (name == "acos")  return std::acos(arg);
    if (name == "atan")  return std::atan(arg);
    if (name == "exp")   return std::exp(arg);
    if (name == "log")   return std::log(arg);
    if (name == "ln")    return std::log(arg);
    if (name == "log10") return std::log10(arg);
    if (name == "sqrt")  return std::sqrt(arg);
    if (name == "abs")   return std::fabs(arg);
    if (name == "floor") return std::floor(arg);
    if (name == "ceil")  return std::ceil(arg);
    if (name == "sinh")  return std::sinh(arg);
    if (name == "cosh")  return std::cosh(arg);
    if (name == "tanh")  return std::tanh(arg);

    error = true;
    errorMsg = "Unknown function: " + name;
    return 0.0;
}
