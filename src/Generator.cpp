/*
 * FuzzyTest - simple random program generator.
 * (C) Maxim Menshikov 2019-2020
 */
#include "Generator.hpp"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include "Syntax.hpp"

namespace FuzzyTest
{
std::string
Generator::generateString(size_t length)
{
    auto randchar = []() -> char {
        const char charset[] = "0123456789"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    auto randcharSymbol = []() -> char {
        const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[rand() % max_index];
    };
    std::string str(length, 0);
    std::generate_n(str.begin(), 1, randcharSymbol);
    std::generate_n(str.begin() + 1, length - 1, randchar);
    return str;
}

std::string
Generator::generateValue(std::string type)
{
    if (type == "uint32_t")
    {
        return std::to_string(std::rand());
    }
    return "";
}

std::shared_ptr<Syntax>
Generator::pickRandomVar(std::vector<std::vector<std::shared_ptr<Syntax>>> vars)
{
    int r;
    int retries = 0;

    if (vars.size() == 0)
        return nullptr;

    while (true)
    {
        r = (std::rand() % vars.size());

        if (vars[r].size() != 0 || retries == 10)
            break;

        retries++;
    }

    if (retries == 10)
        return nullptr;

    return vars[r][std::rand() % vars[r].size()];
}

std::shared_ptr<Syntax>
Generator::getExpressionEvaluatingToValue(std::string value)
{
    int  r;
    auto lit = Syntax::create(SyntaxKind::Literal, value);

    assert(value != "");
    r = std::rand() % 10;

    if (r <= 5)
    {
        return lit;
    }
    else if (r == 6)
    {
        /* Trivial minus */
        uint32_t val = stoull(value, NULL, 0);
        uint32_t r = rand();
        uint32_t target = r + val;

        return Syntax::create(
            SyntaxKind::Binary, "-",
            getExpressionEvaluatingToValue(std::to_string(target)),
            getExpressionEvaluatingToValue(std::to_string(r)));
    }
    else if (r == 7)
    {
        /* Trivial plus */
        uint32_t val = stoull(value, NULL, 0);
        uint32_t r = rand();
        uint32_t target = r - val;

        return Syntax::create(
            SyntaxKind::Binary, "+",
            getExpressionEvaluatingToValue(std::to_string(target)),
            getExpressionEvaluatingToValue(std::to_string(r)));
    }
    else if (r == 8)
    {
        /* Trivial & 0xFFFFFFFF */
        return Syntax::create(
            SyntaxKind::Binary, "&", lit,
            getExpressionEvaluatingToValue("0xFFFFFFFF"));
    }
    else if (r == 9)
    {
        /* Trivial ^ rand ^ rand */
        return Syntax::create(
            SyntaxKind::Binary, "^", lit,
            Syntax::create(SyntaxKind::Binary, "^",
                           getExpressionEvaluatingToValue(value),
                           getExpressionEvaluatingToValue(value)));
    }

    return nullptr;
}

std::shared_ptr<Syntax>
Generator::getAlwaysExpression(bool truth)
{
    int r;

    r = std::rand() % 2;

    if (r == 0)
    {
        /* Trivial */
        std::string ops[] = { "==", ">=", "<=" };
        std::string negops[] = { "!=", "<", ">" };
        int         r2 = std::rand() % (sizeof(ops) / sizeof(*ops));

        auto lit = getExpressionEvaluatingToValue(generateValue("uint32_t"));
        return Syntax::create(SyntaxKind::Binary, truth ? ops[r2] : negops[r2],
                              lit, lit);
    }
    else if (r == 1)
    {
        /* At least with different values */
        std::string val1 = generateValue("uint32_t");
        std::string val2;

        while ((val2 = generateValue("uint32_t")) == val1)
            ;

        auto lit1 = getExpressionEvaluatingToValue(val1);
        auto lit2 = getExpressionEvaluatingToValue(val2);
        return Syntax::create(SyntaxKind::Binary, truth ? "!=" : "==", lit1,
                              lit2);
    }

    return nullptr;
}

std::shared_ptr<Syntax>
Generator::createRandomObfuscatedBlock(
    std::vector<std::vector<std::shared_ptr<Syntax>>> &falseVars)
{
    auto falseVar = Syntax::create(SyntaxKind::Identifier, generateString(3));
    auto falseVarDecl =
        Syntax::create(SyntaxKind::Declaration,
                       Syntax::create(SyntaxKind::Type, "uint32_t"), falseVar);
    auto block = Syntax::create(SyntaxKind::Block, falseVarDecl);

    block->add(obfuscate(Syntax::create(SyntaxKind::Assign, falseVar,
                                        Syntax::create(SyntaxKind::Literal,
                                                       generateValue("uint32_t"))),
                         falseVars));
    return block;
}

std::shared_ptr<Syntax>
Generator::obfuscate(
    std::shared_ptr<Syntax>                            resultExpr,
    std::vector<std::vector<std::shared_ptr<Syntax>>> &falseVars)
{
    int                     r;
    std::shared_ptr<Syntax> tmpExpr = resultExpr;

    while ((r = std::rand() % 7) >= 1)
    {
        if (r == 1)
        {
            /* Pick one variable and assign */
            if (tmpExpr->getKind() != SyntaxKind::Block)
            {
                tmpExpr = Syntax::create(SyntaxKind::Block, tmpExpr);
            }

            auto randVar = pickRandomVar(falseVars);
            if (randVar != nullptr)
            {
                tmpExpr->add(Syntax::create(
                    SyntaxKind::Assign, randVar,
                    Syntax::create(SyntaxKind::Literal, generateValue("uint32_t"))));
            }
        }
        else if (r == 2)
        {
            /* Just declare */
            if (tmpExpr->getKind() != SyntaxKind::Block)
            {
                tmpExpr = Syntax::create(SyntaxKind::Block, tmpExpr);
            }

            auto falseVar = Syntax::create(
                SyntaxKind::Declaration,
                Syntax::create(SyntaxKind::Type, "uint32_t"),
                Syntax::create(SyntaxKind::Identifier, generateString(3)));
            tmpExpr->add(falseVar);
        }
        else if (r == 3)
        {
            int r2;
            /* If */
            tmpExpr = Syntax::create(SyntaxKind::IfGroup,
                                     Syntax::create(SyntaxKind::If,
                                                    getAlwaysExpression(true),
                                                    tmpExpr));
            while ((r2 = std::rand() % 10) >= 3)
            {
                std::shared_ptr<Syntax> elseGoal;

                if (resultExpr->getKind() == SyntaxKind::Return)
                {
                    elseGoal =
                        Syntax::create(SyntaxKind::Return,
                                       Syntax::create(SyntaxKind::Literal,
                                                      generateValue("uint32_t")));
                }

                if (elseGoal != nullptr)
                {
                    tmpExpr->add(Syntax::create(
                        SyntaxKind::If, getAlwaysExpression(false), elseGoal));
                }
            }
        }
        else if (r == 4)
        {
            int                     r2;
            auto                    randVar = pickRandomVar(falseVars);
            std::shared_ptr<Syntax> assuredValue = nullptr;
            std::shared_ptr<Syntax> switchBlock =
                Syntax::create(SyntaxKind::Block);
            std::shared_ptr<Syntax> switchClause;

            if (randVar == nullptr)
            {
                randVar =
                    Syntax::create(SyntaxKind::Identifier, generateString(3));
                assuredValue =
                    Syntax::create(SyntaxKind::Literal, generateValue("uint32_t"));
                auto randVarDecl =
                    Syntax::create(SyntaxKind::Declaration,
                                   Syntax::create(SyntaxKind::Type, "uint32_t"),
                                   randVar, assuredValue);
                switchBlock->add(randVarDecl);
            }

            switchClause = Syntax::create(SyntaxKind::Switch, randVar);
            switchBlock->add(switchClause);
            {
                auto mainCase = Syntax::create(
                    SyntaxKind::Case,
                    assuredValue != nullptr ? assuredValue
                                            : std::shared_ptr<Syntax>(),
                    tmpExpr, Syntax::create(SyntaxKind::Break));
                switchClause->add(mainCase);
            }

            if (assuredValue != nullptr)
            {
                int r2;
                while ((r2 = std::rand() % 10) >= 3)
                {
                    auto tmpValue = generateValue("uint32_t");
                    if (tmpValue != assuredValue->getStringValue())
                    {
                        auto secCase = Syntax::create(
                            SyntaxKind::Case,
                            Syntax::create(SyntaxKind::Literal, tmpValue));
                        if ((std::rand() % 10) >= 2)
                        {
                            if (resultExpr->getKind() == SyntaxKind::Return)
                            {
                                auto elseGoal = Syntax::create(
                                    SyntaxKind::Return,
                                    Syntax::create(SyntaxKind::Literal,
                                                   generateValue("uint32_t")));
                                secCase->add(elseGoal);
                            }
                            else if (resultExpr->getKind() ==
                                     SyntaxKind::Assign)
                            {
                                auto elseGoal = Syntax::create(
                                    SyntaxKind::Assign,
                                    resultExpr->children()[0],
                                    Syntax::create(SyntaxKind::Literal,
                                                   generateValue("uint32_t")));
                                secCase->add(elseGoal);
                            }
                            if (std::rand() % 2 == 1)
                            {
                                secCase->add(Syntax::create(SyntaxKind::Break));
                            }

                            if (secCase->children().size() == 1)
                                secCase->add(Syntax::create(SyntaxKind::Nop));
                        }
                        else
                        {
                            secCase->add(
                                createRandomObfuscatedBlock(falseVars));
                            if (std::rand() % 2 == 1)
                            {
                                secCase->children()[1]->add(
                                    Syntax::create(SyntaxKind::Break));
                            }

                            if (secCase->children()[1]->children().size() == 1)
                                secCase->children()[1]->add(
                                    Syntax::create(SyntaxKind::Nop));
                        }

                        switchClause->add(secCase);
                    }
                }
            }

            tmpExpr = switchBlock;
        }
        else if (r == 5)
        {
            /* For has constant expression - that's not entirely great */
            int r2;
            auto id = Syntax::create(SyntaxKind::Identifier, generateString(3));
            auto falseVar = Syntax::create(
                SyntaxKind::Declaration,
                Syntax::create(SyntaxKind::Type, "uint32_t"),
                id);
            int  minorVar = (rand() % 9) + 1;
            auto oldGoalExpr = tmpExpr;

            tmpExpr = Syntax::create(SyntaxKind::For,
                Syntax::create(SyntaxKind::Assign, falseVar,
                    Syntax::create(SyntaxKind::Literal, "0")),
                Syntax::create(SyntaxKind::Binary,
                    "<",
                    id,
                    Syntax::create(SyntaxKind::Literal,
                                   std::to_string(minorVar))),
                Syntax::create(SyntaxKind::Exact,
                    std::string("++") + id->getStringValue()));
            tmpExpr->add(Syntax::create(SyntaxKind::Block, oldGoalExpr));
        }
        else if (r == 6)
        {
            /* While has constant expressions - that's not entirely great */
            int r2;
            auto id = Syntax::create(SyntaxKind::Identifier, generateString(3));
            auto falseVar = Syntax::create(
                SyntaxKind::Declaration,
                Syntax::create(SyntaxKind::Type, "uint32_t"),
                id,
                Syntax::create(SyntaxKind::Literal, "0"));
            int  minorVar = (rand() % 9) + 1;
            auto oldGoalExpr = tmpExpr;
            auto outerBlock = Syntax::create(SyntaxKind::Block);
            auto innerBlock = Syntax::create(SyntaxKind::Block);

            outerBlock->add(falseVar);

            outerBlock->add(Syntax::create(SyntaxKind::While,
                Syntax::create(SyntaxKind::Binary,
                    "<",
                    id,
                    Syntax::create(SyntaxKind::Literal,
                                   std::to_string(minorVar))),
                innerBlock));
            innerBlock->add(Syntax::create(SyntaxKind::Block, oldGoalExpr));
            innerBlock->add(Syntax::create(SyntaxKind::Exact,
                std::string("++") + id->getStringValue() + ";"));

            tmpExpr = outerBlock;
        }
    }
    return tmpExpr;
}

int
Generator::permute(std::vector<std::shared_ptr<Syntax>> &children,
                   int                                   start,
                   int                                   end,
                   int                                   shift,
                   std::function<bool()>                 callback)
{
    std::map<std::shared_ptr<Syntax>, int> map;
    int                                    count = 0;
    int                                    sum = 0;
    int                                    r;

    if (end - start == 0)
    {
        return -1;
    }

    for (auto &ch : children)
    {
        /* This weird condition accelerates changes */
        map[ch] = ((std::rand() % 50) < 30)
            ? count
            : count++;
    }

    while (std::next_permutation(
        children.begin() + start, children.begin() + end,
        [&map](std::shared_ptr<Syntax> a, std::shared_ptr<Syntax> b) {
            return map[a] < map[b];
        }))
    {
        if (callback())
            return -1;
        for (int i = start; i < end; ++i)
        {
            r = permute(children[i], shift + 1, callback);
            if (r == -1)
                return -1;
            sum += r;
        }

        sum++;
    }

    return sum;
}

int
Generator::permute(std::shared_ptr<Syntax> root,
                   int                     shift,
                   std::function<bool()>   callback)
{
    int sum = 0, r;
    if (root->children().size() == 0)
        return 0;

    switch (root->getKind())
    {
        case SyntaxKind::IfGroup:
        case SyntaxKind::Switch:
        {
            r = permute(root->children(),
                        root->getKind() == SyntaxKind::IfGroup ? 0 : 1,
                        root->children().size(), shift + 1, callback);
            if (r == -1)
                return -1;
            sum += r;
            break;
        }
        case SyntaxKind::For:
        {
            r = permute(root->children(),
                        3,
                        root->children().size(), shift + 1, callback);
            if (r == -1)
                return -1;
            sum += r;
            break;
        }
        case SyntaxKind::While:
        {
            r = permute(root->children(),
                        1,
                        root->children().size(), shift + 1, callback);
            if (r == -1)
                return -1;
            sum += r;
            break;
        }
        case SyntaxKind::Type:
        case SyntaxKind::Identifier:
        case SyntaxKind::Literal:
        case SyntaxKind::Exact:
        case SyntaxKind::Declaration:
        case SyntaxKind::FunctionProto:
        case SyntaxKind::Return:
        case SyntaxKind::Assign:
        case SyntaxKind::Binary:
        case SyntaxKind::Break:
        case SyntaxKind::Assert:
        case SyntaxKind::Nop:
        {
            break;
        }
        default:
        {
            for (auto &ch : root->children())
            {
                r = permute(ch, shift + 1, callback);
                if (r == -1)
                    return -1;
                sum += r;
            }
        }
    }
    return sum;
}

void
Generator::generateTestScript(std::string path)
{
    std::ostream &os = std::cout;

    std::shared_ptr<Syntax> root = Syntax::create(SyntaxKind::Root);
    root->add(Syntax::create(SyntaxKind::Exact,
        "#include <assert.h>\n#include <stdint.h>\n"));

    auto goal = Syntax::create(SyntaxKind::Literal, generateValue("uint32_t"));
    auto block = Syntax::create(SyntaxKind::Block);
    root->add(Syntax::create(
        SyntaxKind::Function,
        Syntax::create(SyntaxKind::FunctionProto,
                       Syntax::create(SyntaxKind::Type, "uint32_t"),
                       Syntax::create(SyntaxKind::Identifier, "main")),
        block));

    {
        std::vector<std::vector<std::shared_ptr<Syntax>>> vars;
        auto                                              falseVar =
            Syntax::create(SyntaxKind::Identifier, generateString(3));
        auto falseVarDecl =
            Syntax::create(SyntaxKind::Declaration,
                           Syntax::create(SyntaxKind::Type, "uint32_t"),
                           falseVar);
        block->add(falseVarDecl);
        auto goalExpr = Syntax::create(SyntaxKind::Assign, falseVar, goal);
        auto goalCheckExpr = Syntax::create(
            SyntaxKind::Assert,
            Syntax::create(SyntaxKind::Binary, "==", falseVar, goal));
        block->add(obfuscate(goalExpr, vars));
        block->add(goalCheckExpr);
    }

    block->add(Syntax::create(SyntaxKind::Return,
                              Syntax::create(SyntaxKind::Literal, "0")));

    std::ofstream prim(path + "/_primary.c");
    prim << root->toString();
    prim.close();

    int i = 0;

    permute(root, 0, [root, &i, path]() {
        std::ofstream ofs(path + "/" + std::to_string(i) + ".c");

        ofs << root->toString();
        i++;
        return i == 100;
    });
}

}