using VortexArcana;
using System;
using System.Numerics;

public class FireflyScript : BaseEntity
{
    public Physics2D phys;
    public PointLight light;

    private Vector2 targetVelocity;
    private float changeTimer = 0.0f;
    private float lightChangeTimer = (float)(Random.Shared.NextDouble());
    private float timeAlive = 0.0f;

    // fkying values
    private float changeInterval = 2.0f;
    private float maxSpeed = 60.0f;
    private float turnSpeed = 2.0f;
    private float flutterSpeed = 4.0f;
    private float flutterStrength = 15.0f;

    public override void Awake()
    {
        Console.WriteLine("FireflyScript Awake has run successfully. Ptr is " + _entityID);
        phys = GetComponent<Physics2D>();
        light = GetComponent<PointLight>();

        PickNewDirection();
    }

    public override void Update()
    {   
        // Oscillate brightness in the range 1.1 +- 0.3;
        light.Brightness = 1.1f + (MathF.Sin(lightChangeTimer * 2.0f) * 0.3f);
    }

    public override void PhysUpdate(float deltaTime)
    {
        timeAlive += deltaTime;
        changeTimer += deltaTime;
        lightChangeTimer += deltaTime;

        // Change velocity after the timer ends.
        if (changeTimer > changeInterval)
        {
            PickNewDirection();
            changeTimer = 0.0f;

            // Randomize the interval slightly it doesn't look weird them all turning together.
            changeInterval = (float)(Random.Shared.NextDouble() * 1.5 + 1.5);
        }

        // Linear interpolation based on speed of turning.
        phys.Velocity = Vector2.Lerp(phys.Velocity, targetVelocity, turnSpeed * deltaTime);

        // Move it side-to-side along with the new velocity change like fireflies do
        Vector2 drift = new Vector2(-phys.Velocity.Y, phys.Velocity.X);

        if (drift.LengthSquared() > 0.001f) // Prevent divide by zero and skip if drift too low.
        {
            drift = Vector2.Normalize(drift);

            // Push the firefly side-to-side using a sine wave based on its lifespan
            phys.Velocity += drift * MathF.Sin(timeAlive * flutterSpeed) * flutterStrength * deltaTime;
        }
    }

    public override void Decimate()
    {
        Console.WriteLine("DihScript Decimate has run successfully. Yipee.");
    }

    private void PickNewDirection()
    {
        // Pick a random angle in radians
        float angle = (float)(Random.Shared.NextDouble() * Math.PI * 2);

        targetVelocity = new Vector2(MathF.Cos(angle), MathF.Sin(angle)) * maxSpeed;
    }
}