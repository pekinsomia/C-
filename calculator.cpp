#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <cctype>
#include <stdexcept>
#include <iomanip>
#include <sstream>
using namespace std;

const double PI = 3.141592656dfdfdf;

class Calculator {
private:
    string text;
    size_t pos;
    double lastAnswer;
    bool useDegrees;
    vector<string> history;

    void skipSpaces() {
        while (pos < text.size() && isspace((unsigned char)text[pos])) pos++;
    }

    char peek() {
        skipSpaces();
        return pos < text.size() ? text[pos] : '\0';
    }

    bool match(char c) {
        if (peek() == c) { pos++; return true; }
        return false;
    }

    double parseExpr() {
        double result = parseTerm();
        while (true) {
            if (match('+')) result += parseTerm();
            else if (match('-')) result -= parseTerm();
            else break;
        }
        return result;
    }

    double parseTerm() {
        double result = parseUnary();
        while (true) {
            if (match('*')) {
                result *= parseUnary();
            } else if (match('/')) {
                double d = parseUnary();
                if (d == 0) throw runtime_error("Division by zero");
                result /= d;
            } else if (match('%')) {
                double d = parseUnary();
                if (d == 0) throw runtime_error("Modulus by zero");
                result = fmod(result, d);
            } else break;
        }
        return result;
    }

    double parseUnary() {
        if (match('-')) return -parseUnary();
        if (match('+')) return parseUnary();
        return parsePower();
    }

    double parsePower() {
        double base = parsePostfix();
        if (match('^')) {
            double exponent = parseUnary();
            double r = pow(base, exponent);
            if (std::isnan(r)) throw runtime_error("Invalid power operation");
            return r;
        }
        return base;
    }

    double parsePostfix() {
        double value = parsePrimary();
        while (match('!')) value = factorial(value);
        return value;
    }

    double parsePrimary() {
        char c = peek();

        if (isdigit((unsigned char)c) || c == '.') {
            const char* start = text.c_str() + pos;
            char* end = NULL;
            double num = strtod(start, &end);
            pos += (end - start);
            return num;
        }

        if (match('(')) {
            double v = parseExpr();
            if (!match(')')) throw runtime_error("Missing closing bracket ')'");
            return v;
        }

        if (isalpha((unsigned char)c)) {
            string name;
            while (pos < text.size() && isalpha((unsigned char)text[pos]))
                name += tolower(text[pos++]);

            if (name == "pi") return PI;
            if (name == "e") return exp(1.0);
            if (name == "ans") return lastAnswer;

            if (!match('(')) throw runtime_error("Unknown name '" + name + "'");
            double arg = parseExpr();
            if (!match(')')) throw runtime_error("Missing closing bracket ')'");
            return applyFunction(name, arg);
        }

        if (c == '\0') throw runtime_error("Expression ended unexpectedly");
        throw runtime_error(string("Unexpected character '") + c + "'");
    }

    double factorial(double n) {
        if (n < 0 || n != floor(n)) throw runtime_error("Factorial needs a non-negative whole number");
        if (n > 170) throw runtime_error("Factorial too large");
        double r = 1;
        for (int i = 2; i <= (int)n; i++) r *= i;
        return r;
    }

    double cleanZero(double v) { return fabs(v) < 1e-12 ? 0.0 : v; }

    double applyFunction(const string& f, double x) {
        double angle = useDegrees ? x * PI / 180.0 : x;
        if (f == "sqrt") {
            if (x < 0) throw runtime_error("Square root of a negative number");
            return sqrt(x);
        }
        if (f == "sin") return cleanZero(sin(angle));
        if (f == "cos") return cleanZero(cos(angle));
        if (f == "tan") {
            if (fabs(cos(angle)) < 1e-12) throw runtime_error("tan is undefined here");
            return cleanZero(tan(angle));
        }
        if (f == "log") {
            if (x <= 0) throw runtime_error("log needs a positive number");
            return log10(x);
        }
        if (f == "ln") {
            if (x <= 0) throw runtime_error("ln needs a positive number");
            return log(x);
        }
        if (f == "abs") return fabs(x);
        if (f == "exp") return exp(x);
        throw runtime_error("Unknown function '" + f + "'");
    }

public:
    Calculator() : pos(0), lastAnswer(0), useDegrees(true) {}

    double evaluate(const string& expression) {
        text = expression;
        pos = 0;
        double result = parseExpr();
        if (peek() != '\0')
            throw runtime_error(string("Unexpected character '") + text[pos] + "'");
        lastAnswer = result;
        history.push_back(expression + " = " + format(result));
        return result;
    }

    static string format(double v) {
        ostringstream out;
        out << setprecision(10) << v;
        return out.str();
    }

    void setDegrees(bool d) { useDegrees = d; }
    bool isDegrees() const { return useDegrees; }

    void showHistory() const {
        if (history.empty()) { cout << "  (no history yet)\n"; return; }
        for (size_t i = 0; i < history.size(); i++)
            cout << "  " << i + 1 << ". " << history[i] << "\n";
    }
};

void showHelp() {
    cout << "\n=== SMART CALCULATOR ===\n"
         << "Operators : + - * / % ^ ! ( )\n"
         << "Functions : sqrt sin cos tan log ln abs exp\n"
         << "Constants : pi  e  ans (last answer)\n"
         << "Commands  : help | history | deg | rad | quit\n"
         << "Examples  : 2+3*4   (2+3)^2   sqrt(16)+5!   sin(30)   ans*2\n\n";
}

int main() {
    Calculator calc;
    string line;
    showHelp();

    while (true) {
        cout << (calc.isDegrees() ? "[deg] " : "[rad] ") << "> ";
        if (!getline(cin, line)) break;

        string cmd;
        for (size_t i = 0; i < line.size(); i++)
            if (!isspace((unsigned char)line[i])) cmd += tolower(line[i]);
        if (cmd.empty()) continue;

        if (cmd == "quit" || cmd == "exit" || cmd == "q") break;
        if (cmd == "help")    { showHelp(); continue; }
        if (cmd == "history") { calc.showHistory(); continue; }
        if (cmd == "deg")     { calc.setDegrees(true);  cout << "  Degrees mode\n"; continue; }
        if (cmd == "rad")     { calc.setDegrees(false); cout << "  Radians mode\n"; continue; }

        try {
            double result = calc.evaluate(line);
            cout << "  = " << Calculator::format(result) << "\n";
        } catch (const exception& e) {
            cout << "  Error: " << e.what() << "\n";
        }
    }

    cout << "Goodbye!\n";
    return 0;
}
