using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using Commands;

namespace MASM
{
    public class MacroAsm
    {
        // исп. в дефайне если не было задано значение т.е. #define FILE1
        // в таблицу попадет FILE1 со значением NULL
        public static readonly string NULL = "NULL";  
        // как-то по другому надо назвать
        // таблица макросов/дефайнов
        public Dictionary<string, string> Defines { get; set; }
        // команды МАСМА
        private Command[] _cmds = new Command[(int)AllCommands.CountOfCmds];

        public MacroAsm()
        {
            Defines = new Dictionary<string, string>();
            _cmds[(int)AllCommands .Define] = new Define ();
            _cmds[(int)AllCommands.  IfDef] = new IfDef  ();
            _cmds[(int)AllCommands. IfNDef] = new IfDef  ();
            _cmds[(int)AllCommands. Macros] = new Macros ();
            _cmds[(int)AllCommands. Repeat] = new Repeat ();
            _cmds[(int)AllCommands.ForEach] = new ForEach();
            _cmds[(int)AllCommands. Import] = new Import ();
        }

        public void Run(string inputFileName, string outputFileName)
        {
            string text;
            using (StreamReader sr = new StreamReader(inputFileName, Encoding.UTF8))
            {
                text = sr.ReadToEnd();
            }
            // удаление комментариев
            Command.EraseComments(ref text);
            // основной цикл
            int pos = text.IndexOf("#");
            /*добавить вывод ошибок*/
            while (pos != -1)
            {
                var cmdType = Command.DefineType(GetCmdName(text, pos)); // выделяю команду и опред. ее тип
                // вызов команды
                if (cmdType != AllCommands.Err)
                {
                    _cmds[(int)cmdType].Run(this, ref text, ref pos);
                    pos = text.IndexOf("#"); // нахожу команду
                }
                else
                {
                    Console.WriteLine("ОШИБОЧКА");
                    pos = -1;
                }
                //Console.WriteLine( text + "\n----------------------");
                //Console.ReadKey();
            }
            using (StreamWriter sw = new StreamWriter(outputFileName, false, Encoding.UTF8))
            {
                sw.WriteLine(text.Trim('\n', ' ', '\r'));
            }
        }

        private string GetCmdName(string text, int pos)
        {
            int tmp = text.IndexOf(" ", pos);
            string cmdName = text.Substring(pos, tmp-pos);
            return cmdName;
        }
    }
}
