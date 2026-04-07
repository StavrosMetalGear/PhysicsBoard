#pragma once

#include <string>
#include <complex>
#include <vector>
#include <functional>

// ── ExpressionParser ────────────────────────────────────────────────────────
// Recursive-descent parser for mathematical expressions.
// Supports:  +, -, *, /, ^, unary minus, parentheses
// Functions: sin, cos, tan, asin, acos, atan, exp, log, ln, sqrt, abs, floor, ceil
// Constants: pi, e
// Variable:  x  (set via setVariable before evaluate)
// ─────────────────────────────────────────────────────────────────────────────

class ExpressionParser {
public:
    ExpressionParser();

    // Set the value of x used during evaluation
    void setVariable(double xValue);

    // Parse and evaluate an expression string.  Returns the result.
    // On error, sets errorMsg and returns 0.
    double evaluate(const std::string& expression);

    // Evaluate for a specific x value (convenience)
    double evaluate(const std::string& expression, double x);

    // After evaluate(), true if an error occurred
    bool hasError() const;
    const std::string& getError() const;

private:
    std::string expr;
    size_t      pos;
    double      varX;
    bool        error;
    std::string errorMsg;

    char peek();
    char get();
    void skipWhitespace();

    double parseExpression();
    double parseTerm();
    double parseUnary();
    double parsePower();
    double parseAtom();
    double parseFunction(const std::string& name);
};
