using System;
using System.Text.RegularExpressions;

namespace LexicalAnalysis
{
    // расширение класса Match т.к. метод Name - который должен возвращать группу
    // всегда возвращает 0, т.к. св-во класса априори = 0 и данное св-во не меняется
    // но при этом группы, к которым может принадлежать строка, дополняются
    public static class MyExtensions
    {
        public static int CurGroup(this Match m)
        {
            for (int i = 1; i < m.Groups.Count; i++)
                if (m.Groups[i].Value.Length != 0)
                    return i;
            return -1;
        }
    }

    public static class Lexer
    {
        public enum Lexems
        {
            ErrorName = 1,
                Ident = 2,
               Number = 3,
                 Sign = 4
        }

        private static string _pattern = @"(?<ERROR>\d+[a-z|A-Z|_]+)|" +  // шаблон ошибки 
                                         @"(?<IDENT>[a-z|A-Z]{1}[a-z|A-Z|_|0-9]*)|" + // идентификатор
                                         @"(?<NUMBER>[0-9|.]+)|" +  // числа
                                         @"(?<SIGN>[,|(|)|!|\+|\-|\*|\/|@|$|<|>])";  // знаки

        private static Regex _regex = new Regex(_pattern);
        public static MatchCollection Run (string input)
        {
           return _regex.Matches(input);
        }
    }
}
