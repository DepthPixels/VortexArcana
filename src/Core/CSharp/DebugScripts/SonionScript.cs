using VortexArcana;
using System;

public class SonionScript : BaseEntity
{
    public override void Awake()
    {
        Console.WriteLine("SonionScript Awake has run successfully. Ptr is " + _entityID);
    }

    public override void Update()
    {
        Console.WriteLine("SonionScript Update has run successfully. Ptr is " + _entityID);
    }

    public override void PhysUpdate(float deltaTime)
    {
        Console.WriteLine("SonionScript PhysUpdate has run successfully. Ptr is " + _entityID);
    }

    public override void Decimate()
    {
        Console.WriteLine("SonionScript Decimate has run successfully. Yipee.");
    }
}