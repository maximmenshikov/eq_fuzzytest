/*
 * FuzzyTest - simple random program generator.
 * (C) Maxim Menshikov 2019-2020
 */
#pragma once

namespace FuzzyTest
{
    enum class SyntaxKind
    {
        Root,
        Type,
        Identifier,
        Literal,
        Exact,
        Declaration,
        Function,
        FunctionProto,
        Block,
        IfGroup,
        If,
        Return,
        Assign,
        Binary,
        For,
        While,
        Switch,
        Case,
        Break,
        Assert,
        Nop,
    };
}
