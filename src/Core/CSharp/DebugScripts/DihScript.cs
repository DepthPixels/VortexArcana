using VortexArcana;
using System;
using System.Numerics;

public class DihScript : BaseEntity
{
    public string digga = "diggadiggadigga";
    public Physics2D phys;

    public override void Awake()
    {
        Console.WriteLine("DihScript Awake has run successfully. Ptr is " + _entityID);
        phys = GetComponent<Physics2D>();
        phys.Velocity = new Vector2(25, 25);
        Console.WriteLine("Physics component found and velocity set to " + phys.Velocity);
    }

    public override void Update()
    {
        Console.WriteLine("DihScript Update has run successfully. Ptr is " + _entityID);
        phys.UpdatePhysics(1);
        Console.WriteLine("Physics!");
    }

    public override void Decimate()
    {
        Console.WriteLine("DihScript Decimate has run successfully. Yipee.");
    }
}