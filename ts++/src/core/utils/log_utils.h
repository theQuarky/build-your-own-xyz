#pragma once
#include "tokens/tokens.h"
#include <vector>

/**
 * @brief Prints details of a single token to the console
 * @param token Token to print
 * @throws None
 */
void printToken(const tokens::Token &token);

/**
 * @brief Prints the entire token stream to the console
 * @param tokens Token vector to print
 * @note This function will consume the entire token stream
 * @throws None
 */
void printTokenStream(const std::vector<tokens::Token> &tokens);