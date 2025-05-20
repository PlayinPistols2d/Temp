double MathParserModel::squeezeFunctionResultWithTwoArgs(
    const list<lexeme>::iterator function,
    const list<lexeme>::iterator arg2,
    const list<lexeme>::iterator arg1)
{
    arg1->value = pow(arg1->value, arg2->value);
    arg1->type = number;
    arg1->priority = 0;

    readyStack.erase(function);
    readyStack.erase(arg2);
    return calculateFullExpression();
}


