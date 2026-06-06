using VortexArcana;
using System;

public class DihScript : BaseEntity
{
    public override void Update()
    {
        Console.WriteLine("DihScript Update has run successfully. Ptr is " + ScriptInstancePtr);
    }

    public override void Decimate()
    {
        Console.WriteLine("DihScript Decimate has run successfully. Yipee.");
    }
}