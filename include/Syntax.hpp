/*
 * FuzzyTest - simple random program generator.
 * (C) Maxim Menshikov 2019-2020
 */
#pragma once
#include <any>
#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include "SyntaxKind.hpp"

namespace FuzzyTest
{
class Syntax
{
public:
    Syntax() = default;

    Syntax(SyntaxKind newKind) : Syntax()
    {
        _kind = newKind;
    }

    Syntax(SyntaxKind                                     newKind,
           std::initializer_list<std::shared_ptr<Syntax>> l) :
      Syntax(newKind)
    {
        for (auto &obj : l)
        {
            _children.push_back(obj);
        }
    }

    Syntax(SyntaxKind newKind, std::string newValue) : Syntax(newKind)
    {
        _value = newValue;
    }

    virtual ~Syntax() = default;
    Syntax(const Syntax &rhs) = default;
    Syntax &operator=(const Syntax &rhs) = default;

    /**
     * @brief      Get the kind of the syntax node
     *
     * @return     The syntax node kind
     */
    SyntaxKind getKind() const
    {
        return _kind;
    }

    /**
     * @brief      Get the string value encoded in a syntax node
     *
     * @return     The string value
     */
    std::string getStringValue() const
    {
        return _value;
    }

    /**
     * @brief      Get children directly allowing for manipulations
     *
     * @return     The syntax node children
     */
    std::vector<std::shared_ptr<Syntax>> &children()
    {
        return _children;
    }

    /**
     * @brief      Add the child syntax node to the vector of children
     *
     * @param      node  The syntax node to add
     */
    void add(std::shared_ptr<Syntax> node)
    {
        _children.push_back(node);
    }

    /**
     * @brief      Ensure that end-of-line is present for the syntax node
     *             string representation in one or another way
     *
     * @param      str   Syntax node string representation
     *
     * @return     String with ensured end-of-line
     */
    static std::string ensureEOL(std::string str)
    {
        if (str.size() == 0)
            return ";";
        if (str[str.size() - 1] != ';' && str[str.size() - 1] != '}')
            return str + ";";
        return str;
    }

    /**
     * @brief      Get a string representation of the syntax node
     *
     * @return     String representation of the node
     */
    virtual std::string toString() const
    {
        switch (_kind)
        {
            case SyntaxKind::Type:
            case SyntaxKind::Identifier:
            case SyntaxKind::Literal:
            case SyntaxKind::Exact:
            {
                return _value;
            }
            case SyntaxKind::Declaration:
            {
                std::string result;

                assert(_children.size() >= 2);
                result =
                    _children[0]->toString() + " " + _children[1]->toString();
                if (_children.size() == 3)
                    result += " = " + _children[2]->toString();
                return result;
            }
            case SyntaxKind::Assign:
            {
                std::string result;

                assert(_children.size() == 2);
                result =
                    _children[0]->toString() + " = " + _children[1]->toString();
                return result;
            }
            case SyntaxKind::Function:
            {
                std::string result;

                assert(_children.size() <= 2);
                result = _children[0]->toString();
                if (_children[1]->_kind != SyntaxKind::Block)
                    result += "{";
                result += ensureEOL(_children[1]->toString());
                if (_children[1]->_kind != SyntaxKind::Block)
                    result += "}";
                result = ensureEOL(result);
                return result;
            }
            case SyntaxKind::FunctionProto:
            {
                std::string result;

                assert(_children.size() >= 2);
                result = _children[0]->toString() + " " +
                    _children[1]->toString() + "(";
                for (int i = 2; i < _children.size(); ++i)
                {
                    if (i != 2)
                        result += ", ";
                    result += _children[i]->toString();
                }
                result += ")";
                return result;
            }
            case SyntaxKind::Root:
            case SyntaxKind::Block:
            {
                std::string result;

                if (_kind == SyntaxKind::Block)
                    result += "{";
                for (auto &ch : _children)
                {
                    result += ch->toString();

                    if (ch->_kind != SyntaxKind::Function &&
                        ch->_kind != SyntaxKind::Exact)
                        result = ensureEOL(result);
                }
                if (_kind == SyntaxKind::Block)
                    result += "}";
                return result;
            }
            case SyntaxKind::IfGroup:
            {
                std::string result;

                for (int i = 0; i < _children.size(); ++i)
                {
                    if (i == 0)
                    {
                        result += "if ";
                    }
                    else
                    {
                        if (_children[i] != nullptr)
                            result += "else if ";
                        else
                            result += "else ";
                    }

                    result += _children[i]->toString();
                }

                return result;
            }
            case SyntaxKind::If:
            {
                std::string result;

                assert(_children.size() == 2);

                result += "(" +
                    (_children[0] != nullptr ? _children[0]->toString() : "") +
                    ")";
                std::string body;
                if (_children[1]->_kind != SyntaxKind::Block)
                {
                    body = "{" + ensureEOL(_children[1]->toString()) + "}";
                }
                else
                {
                    body = ensureEOL(_children[1]->toString());
                }
                result += body;
                return result;
            }
            case SyntaxKind::Return:
            {
                assert(_children.size() == 1);
                return std::string("return ") + _children[0]->toString();
            }
            case SyntaxKind::Binary:
            {
                std::string result;

                assert(_children.size() == 2);

                result += std::string("(") + _children[0]->toString() + ") " +
                    _value + " (" + _children[1]->toString() + ")";
                return result;
            }
            case SyntaxKind::For:
            {
                std::string result;

                assert(_children.size() == 4);

                result += "for (" + _children[0]->toString() + "; " +
                    _children[1]->toString() + "; " + _children[2]->toString() +
                    ")" + ensureEOL(_children[3]->toString());
                return result;
            }
            case SyntaxKind::While:
            {
                std::string result;

                assert(_children.size() == 2);

                result += "while (" + _children[0]->toString() + ")" +
                    ensureEOL(_children[1]->toString());
                return result;
            }
            case SyntaxKind::Switch:
            {
                std::string result;

                assert(_children.size() >= 1);

                result +=
                    std::string("switch (") + _children[0]->toString() + ") {";

                for (int i = 1; i < _children.size(); i++)
                {
                    result += _children[i]->toString();
                }
                result += std::string("}");
                return result;
            }
            case SyntaxKind::Case:
            {
                std::string result;

                assert(_children.size() >= 1);
                result += std::string("case ") + _children[0]->toString() + ":";
                for (int i = 1; i < _children.size(); ++i)
                {
                    result += ensureEOL(_children[i]->toString());
                }
                return result;
            }
            case SyntaxKind::Break:
            {
                return std::string("break");
            }
            case SyntaxKind::Assert:
            {
                assert(_children.size() == 1);
                return std::string("assert(") + _children[0]->toString() + ")";
            }
            case SyntaxKind::Nop:
            {
                return std::string(";");
            }
        }
        return "";
    }

    static std::shared_ptr<Syntax> create(SyntaxKind kind)
    {
        return std::make_shared<Syntax>(kind);
    }


    static std::shared_ptr<Syntax> create(SyntaxKind kind, const char *value)
    {
        return std::make_shared<Syntax>(kind, std::string(value));
    }

    static std::shared_ptr<Syntax> create(SyntaxKind kind, std::string value)
    {
        return std::make_shared<Syntax>(kind, std::string(value));
    }

    template <typename Type>
    static void toVector(std::vector<Type> &out, const Type &elem)
    {
        out.push_back(elem);
    };

    template <typename... Type>
    static std::shared_ptr<Syntax> create(SyntaxKind kind, Type... args)
    {
        auto result = std::make_shared<Syntax>(kind);
        auto vec = { args... };

        for (auto &obj : vec)
        {
            result->_children.push_back(obj);
        }

        return result;
    }

    template <typename... Type>
    static std::shared_ptr<Syntax> create(SyntaxKind  kind,
                                          const char *value,
                                          Type... args)
    {
        auto result = std::make_shared<Syntax>(kind);
        auto vec = { args... };

        result->_value = std::string(value);
        for (auto &obj : vec)
        {
            result->_children.push_back(obj);
        }

        return result;
    }

    template <typename... Type>
    static std::shared_ptr<Syntax> create(SyntaxKind  kind,
                                          std::string value,
                                          Type... args)
    {
        auto result = std::make_shared<Syntax>(kind);
        auto vec = { args... };

        result->_value = value;
        for (auto &obj : vec)
        {
            result->_children.push_back(obj);
        }

        return result;
    }

private:
    SyntaxKind                           _kind;
    std::vector<std::shared_ptr<Syntax>> _children;
    std::string                          _value;
};
}