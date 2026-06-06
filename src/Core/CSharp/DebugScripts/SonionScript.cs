using SharedInterface;
using System;

public class SonionScript : IScriptBehavior
{
    public IntPtr ScriptInstancePtr { get; set; }

    public void Update()
    {
        Console.WriteLine("SonionScript Update has run successfully. Ptr is " + ScriptInstancePtr);
    }

    public void Decimate()
    {
        Console.WriteLine("SonionScript Decimate has run successfully. Yipee.");
    }
}