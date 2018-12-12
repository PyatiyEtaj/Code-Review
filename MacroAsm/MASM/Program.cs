using System;
using System.Diagnostics;
using System.Text.RegularExpressions;
using LexicalAnalysis;

namespace MASM
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine(args.Length);
            if (args.Length > 1)
            {
                MacroAsm masm = new MacroAsm();
                masm.Run(args[0], args[1]);
                Console.WriteLine("Завершение работы МакроАсма");
                Process cmd = Process.Start("./Assembler", $"{args[1]} OutputBIN");
                cmd.WaitForExit();

            }
            else
            {
                Console.WriteLine("Ожидалось имя входного и выходного файла!");
            }
        }
    }
}
