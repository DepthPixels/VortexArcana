using SharedInterface;
using System;

public class DihScript : IScriptBehavior
{
    public IntPtr ScriptInstancePtr;

    public void Update()
    {
        Console.WriteLine("DihScript Update has run successfully. Ptr is " + ScriptInstancePtr);
    }

    public void Decimate()
    {
        Console.WriteLine("DihScript Decimate has run successfully. Yipee.");
    }
}