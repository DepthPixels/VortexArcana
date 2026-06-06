using VortexArcana;
using System;

public class SonionScript : BaseEntity
{
    public override void Update()
    {
        Console.WriteLine("SonionScript Update has run successfully. Ptr is " + ScriptInstancePtr);
    }

    public override void Decimate()
    {
        Console.WriteLine("SonionScript Decimate has run successfully. Yipee.");
    }
}