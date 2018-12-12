using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;
using LexicalAnalysis;
using MASM;

namespace Commands
{
    public enum AllCommands
    {
        CountOfCmds = 8,
             Define = 0,
             Import = 1,
             Macros = 2,
             IfNDef = 3,
              IfDef = 4,
               Else = 5,
             Repeat = 6,
            ForEach = 7,
        // далее идут вспомагательные
              EndIF = 20,
           EndMacro = 21,
             EndFor = 22,
                Err = 23
    }

    public abstract class Command
    {
        public static void EraseComments (ref string text)
        {
            Regex regex = new Regex(@";.*");
            text = regex.Replace(text, "");
        }

        public static AllCommands DefineType(string cmd)
        {
            switch (cmd){
                case "#define":
                    return AllCommands.Define;
                case "#import":
                    return AllCommands.Import;
                case "#macros":
                    return AllCommands.Macros;
                case "#repeat":
                    return AllCommands.Repeat;
                case "#foreach":
                    return AllCommands.ForEach;
                case "#ifdef":
                    return AllCommands.IfDef;
                case "#ifndef":
                    return AllCommands.IfNDef;
                case "#else":
                    return AllCommands.Else;
                case "#endif":
                    return AllCommands.EndIF;
                case "#endmacro":
                    return AllCommands.EndMacro;
                case "#endcycle":
                    return AllCommands.EndFor;
            }
            return AllCommands.Err;
        }
        protected abstract string Parse(ref string text, ref int pos);
        public abstract void Run(MacroAsm masm, ref string text, ref int pos);
    }

    public class Define : Command
    {
        protected override string Parse(ref string text, ref int pos)
        {
            string res;
            int tmp = text.IndexOf("\n", pos);
            res = text.Substring(pos, tmp-pos);
            text = text.Remove(pos, tmp-pos);
            return res;
        }

        public override void Run(MacroAsm masm, ref string text, ref int pos)
        {
            MatchCollection res = Lexer.Run(Parse(ref text, ref pos));
            try
            {
                string value = "";
                switch (res.Count)
                {
                    case 1:
                        throw new Exception("Ожидалось имя и значение закрепленное за ним");
                    case 2:
                        value = MacroAsm.NULL;
                        masm.Defines.Add(res[1].Value, value);
                        break;
                    default:
                        for (int i = 2; i < res.Count; i++)
                            value += res[i].Value;
                        masm.Defines.Add(res[1].Value, value);
                        break;
                }
                Regex r = new Regex($"\\b{res[1].Value}\\b");
                if (value != MacroAsm.NULL)
                    text = r.Replace(text, value);
            }
            catch (ArgumentException)
            {
                /*кинуть исключение, такое имя уже объявлено*/
            };
        }

    }

    public abstract class Сonditions : Command
    {   // создам один раз regex, чтобы не создавать данный объект кучу раз для каждого наследника
        protected static Regex regex = new Regex(@"(#ifndef)|(#ifdef)|(#else)|(#endif)");
        // нахождение блока c ifdef, else и endif, начиная с позиции pos в тексте
        protected Match GetBlock(string text, int pos)
        {
            MatchCollection res = regex.Matches(text, pos);
            int count = 1, i;
            for (i = 0; i < res.Count && count > 0; i++)
            {
                var cmdType = DefineType(res[i].Value);
                if (cmdType == AllCommands.IfDef)
                {
                    count++;
                    continue;
                }
                if (i > 0)
                    if (cmdType == AllCommands.EndIF && res[i - 1].Value != "#else")
                    {
                        count++;
                        continue;
                    }
                count--;
            }
            // если недостаточно endif или else бросается исключение
            /*if (count > 0 ) throw Exception*/
            return res[--i];
        }

        protected void Skip(ref string text, int pos)
        {
            Match res = GetBlock(text, pos);
            text = text.Remove(pos, res.Index - pos + res.Length);
            if (DefineType(res.Value) == AllCommands.Else)
            {
                res = GetBlock(text, pos);
                pos = res.Index;
                text = text.Remove(res.Index, res.Length);
            }
        }
    }

    public class IfDef : Сonditions
    {
        protected override string Parse(ref string text, ref int pos)
        {
            string res;
            int tmp = text.IndexOf("\n", pos);
            res = text.Substring(pos, tmp - pos);
            text = text.Remove(pos, tmp-pos);
            return res;
        }

        public override void Run(MacroAsm masm, ref string text, ref int pos)
        {
            MatchCollection res = Lexer.Run(Parse(ref text, ref pos));
            bool check = masm.Defines.ContainsKey(res[1].Value);
            if ( (res[0].Value == "ifdef"  &&  check) ||
                 (res[0].Value == "ifndef" && !check)   )
            {
                var item = GetBlock(text, pos);
                pos = item.Index;
                text = text.Remove(item.Index, item.Length);
                if (DefineType(item.Value) == AllCommands.Else)
                {
                    Skip(ref text, pos);
                }
            }
            else
            {
                Skip(ref text, pos);
            }                
        }
    }

    public class Macros : Command
    {
        private      string  _name = MacroAsm.NULL;
        private List<string> _args;

        private List<string> GetArgs(MatchCollection mc, int pos = 1)
        {
            List<string> ls = new List<string>();
            for (; pos < mc.Count; pos++)
            {
                var tmp = mc[pos].CurGroup();
                switch (tmp)
                {
                    case (int)Lexer.Lexems.Ident:
                    case (int)Lexer.Lexems.Number:
                        ls.Add(mc[pos].Value);
                        break;
                    case (int)Lexer.Lexems.ErrorName:
                        throw new Exception("Аргумент содержит недопустимое имя: " + mc[pos].Value);
                    default:
                        break;
                }
            }
            return ls;
        }

        private void FindAndChangeMacros(ref string text, string body)
        {
            Regex r = new Regex($"{_name}\\s.*");
            MatchCollection res = r.Matches(text);
            for (int count = 0; count < res.Count; count++)
            {
                var tmp = Lexer.Run(res[count].Value);
                var localArgs = GetArgs(tmp);
                /* бросить исключение недостаточно аргументов */
                var localBody = body;
                for( int i = 0; i < _args.Count; i++)
                {
                    r = new Regex($"\\${_args[i]}");
                    localBody = r.Replace(localBody, localArgs[i]);
                }
                // можно указывать с помощью комментариев начало и конец макроподстановки
                //localBody = localBody.Trim('\r', '\n');
                //text = text.Replace(res[count].Value, "; Макроподстановка: " + _name + "\n" + localBody + " ;Конец макроподстановки: " + _name + "\n", StringComparison.Ordinal);
                text = text.Replace(res[count].Value, localBody, StringComparison.Ordinal);
            }
        }

        protected override string Parse(ref string text, ref int pos)
        {
            string res = "";
            int tmp = text.IndexOf("#endmacro", pos);
            if (tmp == -1)
                throw new Exception("Ожидалось оканчание макроопределения");
            res = text.Substring(pos, tmp - pos);
            text = text.Remove(pos, tmp + "#endmacro".Length - pos);
            return res;
        }

        public override void Run(MacroAsm masm, ref string text, ref int pos)
        {
            // получаем все макроопределение
            // разделяем все макроопределение на голову и тело
            // голова содержит имя и аргументы
            // тело содержит команды ассемблера
            var body = Parse(ref text, ref pos);
            var head = body.Substring(0, body.IndexOf("\n", 0));
            body = body.Substring(head.Length, body.Length - head.Length);
            MatchCollection res = Lexer.Run(head);
            if (res.Count < 2)
                throw new Exception("Ожидалось имя макроса");
            if (res[1].CurGroup() != (int)Lexer.Lexems.Ident)
                throw new Exception("Ожидалось имя макроса");
            // устанавливаем название макроопределения
            // а также аргументы
            _name = res[1].Value;
            _args = GetArgs(res, 2);
            FindAndChangeMacros(ref text, body);
        }
    }

    public abstract class Cycle : Command
    {   // создам один раз regex, чтобы не создавать данный объект кучу раз для каждого наследника
        protected static Regex regex = new Regex(@"(#repeat)|(#foreach)|(#endcycle)");

        // нахождение блока c repeat, foreach и endcycle, начиная с позиции pos в тексте
        protected Match GetBlock(string text, int pos)
        {
            MatchCollection res = regex.Matches(text, pos);
            int count = 0, i = 0;
            do
            {
                var cmdType = DefineType(res[i].Value);
                i++;
                if (cmdType == AllCommands.Repeat || cmdType == AllCommands.ForEach)
                {
                    count++;
                    continue;
                }
                count--;
            } while (i < res.Count && count > 0);
            // если недостаточно #endcycle бросается исключение
            /*if (count > 0 ) throw Exception*/
            return res[--i];
        }
    }

    public class Repeat : Cycle
    {
        private int _countOfRepeat = 0;
        protected override string Parse(ref string text, ref int pos)
        {
            string res;
            var end = GetBlock(text, pos);
            res = text.Substring(pos, end.Index - pos);
            text = text.Remove(pos, end.Index - pos + end.Length);
            return res;
        }

        public override void Run(MacroAsm masm, ref string text, ref int pos)
        {
            var parsePart = Parse(ref text, ref pos);
            var endFirstLine = parsePart.IndexOf('\n');
            var headLexems = Lexer.Run(parsePart.Substring(0, endFirstLine));
            if (headLexems.Count < 2)
            {
                /* кинуть исключение не указано количество повторений*/
            }
            if (headLexems[1].CurGroup() != (int)Lexer.Lexems.Number)
            {
                /* кинуть исключение количество повторений должно быть целым числом*/
            }
            _countOfRepeat = Int32.Parse(headLexems[1].Value);
            var body = parsePart.Remove(0, endFirstLine + 1);
            string result = "";
            for (int i = 0; i < _countOfRepeat; i++)
                result += body;
            text = text.Insert(pos, result);
        }
    }

    public class ForEach : Cycle
    {
        private string _var = "$$$";
        private Regex _localRegex;
        private List<string> _args = new List<string>();
        protected override string Parse(ref string text, ref int pos)
        {
            string res;
            var end = GetBlock(text, pos);
            res = text.Substring(pos, end.Index - pos);
            text = text.Remove(pos, end.Index - pos + end.Length);
            return res;
        }

        public override void Run(MacroAsm masm, ref string text, ref int pos)
        {
            _args.Clear();
            var parsePart = Parse(ref text, ref pos);
            var endFirstLine = parsePart.IndexOf('\n');
            MatchCollection headLexems = Lexer.Run(parsePart.Substring(0, endFirstLine));
            var body = parsePart.Remove(0, endFirstLine + 1);
            if (headLexems.Count < 3)
            {
                /* кинуть исключение не указаны значения для подстановки в след. блок*/
            }
            _var = headLexems[1].Value;
            for (int i = 2; i < headLexems.Count; i++)
            {
                var tmp = headLexems[i].CurGroup();
                if (tmp == (int)Lexer.Lexems.Ident || tmp == (int)Lexer.Lexems.Number)
                    _args.Add(headLexems[i].Value);
            }
            _localRegex = new Regex($"\\${_var}");
            string result = "";
            foreach (var item in _args)
            {
                result += _localRegex.Replace(body, item);
            }
            text = text.Insert(pos, result);
        }
    }

    public class Import : Command
    {
        private string _filename = "";
        protected override string Parse(ref string text, ref int pos)
        {
            string res;
            int tmp = text.IndexOf("\n", pos);
            res = text.Substring(pos, tmp - pos);
            text = text.Remove(pos, tmp - pos);
            return res;
        }

        public override void Run(MacroAsm masm, ref string text, ref int pos)
        {
            _filename = ""; // очищаем имя файла
            MatchCollection res = Lexer.Run(Parse(ref text, ref pos));
            if (res.Count < 4)
            {
                /* кинуть иск. неверный формат директивы: #import <...> */
            }
            if (res[1].Value == "<")
            {
                /* ожидалось <*/
            }
            // путь до файла, который необходимо импортировать
            for (int i = 1; i < res.Count; i++)
            {
                switch (res[i].CurGroup())
                {
                    case (int)Lexer.Lexems.Sign:
                        if (res[i].Value == ">")
                            i = res.Count;
                        else if (res[i].Value == "<")
                        {
                            /* Ошибочный символ < */
                        }
                        else
                            _filename += res[i].Value;
                        break;
                    default:
                        _filename += res[i].Value;
                        break;
                }
            }
            using (StreamReader sr = new StreamReader(_filename))
            {
                text = text.Insert(pos, sr.ReadToEnd());
                EraseComments(ref text);
            }
        }
    }
}
