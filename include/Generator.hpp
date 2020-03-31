/*
 * FuzzyTest - random program generator.
 * (C) Maxim Menshikov 2019-2020
 */
#pragma once
#include <functional>
#include <unordered_map>
#include "Syntax.hpp"

namespace FuzzyTest
{
class Generator
{
public:
    Generator() = default;
    virtual ~Generator() = default;
    Generator(const Generator &rhs) = default;
    Generator& operator=(const Generator &rhs) = default;

    /**
     * @brief      Generate a random string of given @p length
     *
     * @param      length  The length of the string
     *
     * @return     Random string
     */
    std::string             generateString(size_t length);

    /**
     * @brief      Generate a random value of given @p type
     *
     * @param      type  The type of the value
     *
     * @return     Random value
     */
    std::string             generateValue(std::string type);

    /**
     * @brief      Pick one random variable
     *
     * @param      vars  The variables to pick from
     *
     * @return     Syntax node pointing to one of variables
     */
    std::shared_ptr<Syntax> pickRandomVar(
        std::vector<std::vector<std::shared_ptr<Syntax>>> vars);

    /**
     * @brief      Create a random obfuscated block
     *
     * @param      falseVars  The false variables
     *
     * @return     The obfuscated block
     */
    std::shared_ptr<Syntax> createRandomObfuscatedBlock(
        std::vector<std::vector<std::shared_ptr<Syntax>>> &falseVars);

    /**
     * @brief      Permute children of the selected node
     *
     * @param      root      The root
     * @param      shift     The shift (indicates the recursion level)
     * @param      callback  The callback which is triggered in order to save
     *                       the syntax tree
     *
     * @return     { description_of_the_return_value }
     */
    int permute(std::shared_ptr<Syntax> root,
                int                     shift,
                std::function<bool()>   callback);

    /**
     * @brief      Permute syntax node children
     *
     * @param      children  The syntax node children
     * @param      start     The start index
     * @param      end       The end index
     * @param      shift     The shift (indicates the recursion level)
     * @param      callback  The callback which is triggered in order to save
     *                       the syntax tree
     *
     * @return     @c -1 indicates that process should be stopped,
     *             otherwise it is a number of processed nodes
     */
    int permute(std::vector<std::shared_ptr<Syntax>> &children,
                int                                   start,
                int                                   end,
                int                                   shift,
                std::function<bool()>                 callback);

    /**
     * @brief      Obfuscate the goal with several false expressions
     *
     * @param      goalExpr   The goal expression
     * @param      falseVars  The false variables
     *
     * @return     Obfuscated node
     */
    std::shared_ptr<Syntax> obfuscate(
        std::shared_ptr<Syntax>                            goalExpr,
        std::vector<std::vector<std::shared_ptr<Syntax>>> &falseVars);

    /**
     * @brief      Get the always true or false expression.
     *
     * @param      truth  The truth - @c true to create true expression,
     *                    @c false otherwise
     *
     * @return     The always true or false expression.
     */
    std::shared_ptr<Syntax> getAlwaysExpression(bool truth);

    /**
     * @brief      Get the expression evaluating to a specified value.
     *
     * @param      value  The value
     *
     * @return     The expression evaluating to @p value.
     */
    std::shared_ptr<Syntax> getExpressionEvaluatingToValue(std::string value);

    /**
     * @brief      Generate a test script
     *
     * @param      path  The path to the folder where to put results
     */
    void generateTestScript(std::string path);
};
}