bool MathParserModel::isOperator() {
    if (isSign()) return true;
    if (input[currentIndex] == '*' || input[currentIndex] == '/' || input[currentIndex] == '%') {
        currentIndex++;
        return true;
    }
    return false;
}


void MathParserModel::addOperatorToList(bool &unaryFlag, bool &firstSign) {
    if ((input[currentIndex] == '+' || input[currentIndex] == '-') && (unaryFlag || firstSign)) {
        unaryFlag = false;
        firstSign = false;
        lexemesList.emplace_back(0, 0, number);
    }

    if (input[currentIndex] == '+') {
        lexemesList.emplace_back(0, 1, plus);
    } else if (input[currentIndex] == '-') {
        lexemesList.emplace_back(0, 1, minus);
    } else if (input[currentIndex] == '*') {
        lexemesList.emplace_back(0, 2, mult);
    } else if (input[currentIndex] == '/') {
        lexemesList.emplace_back(0, 2, division);
    } else if (input[currentIndex] == '%') {
        lexemesList.emplace_back(0, 2, mod_t);
    }
    currentIndex++;
}



bool MathParserModel::isFunction() {
    bool did_caught_function = false;

    if (!strncmp(&input[currentIndex], "pow", 3)) {
        currentIndex += 3;
        did_caught_function = true;
    } else if (!strncmp(&input[currentIndex], "cos", 3) || !strncmp(&input[currentIndex], "sin", 3)
            || !strncmp(&input[currentIndex], "tan", 3) || !strncmp(&input[currentIndex], "log", 3)
            || !strncmp(&input[currentIndex], "abs", 3)) {
        currentIndex += 3;
        did_caught_function = true;
    } else if (!strncmp(&input[currentIndex], "sqrt", 4)) {
        currentIndex += 4;
        did_caught_function = true;
    } else if (!strncmp(&input[currentIndex], "sqr", 3)) {
        currentIndex += 3;
        did_caught_function = true;
    } else if (!strncmp(&input[currentIndex], "ln", 2)) {
        currentIndex += 2;
        did_caught_function = true;
    }

    if (did_caught_function && input[currentIndex++] == '(') {
        did_caught_function = checkOperatorAndOperandsOrder(true);
        if (did_caught_function && input[currentIndex] == ')') {
            currentIndex++;
            return true;
        }
    }

    return false;
}



void MathParserModel::addFunctionToList() {
    if (!strncmp(&(input[currentIndex]), "pow", 3)) {
        lexemesList.emplace_back(0, 4, pow_t);
        currentIndex += 3;
    } else if (!strncmp(&(input[currentIndex]), "cos", 3)) {
        lexemesList.emplace_back(0, 4, cos_t);
        currentIndex += 3;
    } else if (!strncmp(&(input[currentIndex]), "sin", 3)) {
        lexemesList.emplace_back(0, 4, sin_t);
        currentIndex += 3;
    } else if (!strncmp(&(input[currentIndex]), "tan", 3)) {
        lexemesList.emplace_back(0, 4, tan_t);
        currentIndex += 3;
    } else if (!strncmp(&(input[currentIndex]), "log", 3)) {
        lexemesList.emplace_back(0, 4, log_t);
        currentIndex += 3;
    } else if (!strncmp(&(input[currentIndex]), "abs", 3)) {
        lexemesList.emplace_back(0, 4, abs_t);
        currentIndex += 3;
    } else if (!strncmp(&(input[currentIndex]), "sqrt", 4)) {
        lexemesList.emplace_back(0, 4, sqrt_t);
        currentIndex += 4;
    } else if (!strncmp(&(input[currentIndex]), "sqr", 3)) {
        lexemesList.emplace_back(0, 4, sqr_t);
        currentIndex += 3;
    } else if (!strncmp(&(input[currentIndex]), "ln", 2)) {
        lexemesList.emplace_back(0, 4, ln_t);
        currentIndex += 2;
    }
}






double MathParserModel::calculateFunction(const lexeme function, lexeme value) {
    switch (function.type) {
        case cos_t: return cos(value.value);
        case sin_t: return sin(value.value);
        case tan_t: return tan(value.value);
        case log_t: return log(value.value);
        case ln_t:  return log(value.value);
        case sqrt_t: return sqrt(value.value);
        case abs_t: return fabs(value.value);
        case sqr_t: return pow(value.value, 2);
        case pow_t: return pow(value.value, getNextArg());
        default: return 0;
    }
}





double MathParserModel::getNextArg() {
    if (readyStack.empty()) {
        *errorString = "Error: pow(x, y) missing exponent!";
        return 0;
    }
    double val = readyStack.begin()->value;
    readyStack.erase(readyStack.begin());
    return val;
}
