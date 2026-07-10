using ScriptEngine;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.Marshalling;

namespace VortexArcana {
    public abstract class BaseEntity
    {
        public IntPtr _entityID { get; set; }
        public TransformProxy? Transform;

        public void EngineInit(IntPtr EntityID)
        {
            _entityID = EntityID;
            Transform = new TransformProxy(EntityID);
            Awake();
        }

        // Abstract Functions
        public abstract void Awake();
        public abstract void Update();
        public abstract void PhysUpdate(float deltaTime);
        public abstract void Decimate();
        public abstract void SaveState(Dictionary<string, object> SaveMap);
        public abstract void ReloadState(Dictionary<string, object> ReloadMap);

        // Component Related
        public T? GetComponent<T>() where T : BaseComponent
        {
            Console.WriteLine($"Getting component of type {typeof(T).Name} for entity {_entityID}");
            int componentID = ComponentIdCache<T>.Id;
            Console.WriteLine($"Component ID for type {typeof(T).Name} is {componentID.ToString("X")}");
            IntPtr componentPtr = BridgeAPI.Entity_GetComponentBridge(_entityID, componentID);
            Console.WriteLine($"Component pointer for type {typeof(T).Name} is {componentPtr.ToString("X")}");

            // Component was not found.
            if (componentPtr == IntPtr.Zero)
            {
                return null;
            }

            return (T?)Activator.CreateInstance(typeof(T), componentPtr, _entityID);

        }

        public T[] GetComponents<T>() where T : BaseComponent
        {
            int componentID = ComponentIdCache<T>.Id;
            IntPtr componentsPtr = BridgeAPI.Entity_GetComponentsBridge(_entityID, componentID, out int length);
            // No components were found.
            if (componentsPtr == IntPtr.Zero || length <= 0)
            {
                return Array.Empty<T>();
            }
            T[] components = new T[length];
            for (int i = 0; i < length; i++)
            {
                IntPtr componentPtr = Marshal.ReadIntPtr(componentsPtr, i * IntPtr.Size);
                components[i] = (T)Activator.CreateInstance(typeof(T), componentPtr);
            }
            return components;
        }
    }

    public class TransformProxy
    {
        private IntPtr _transformPtr;

        public TransformProxy(IntPtr entityPtr)
        {
            _transformPtr = entityPtr;
        }

        public float X
        {
            get
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                return component.position.X;
            }
            set
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                component.position.X = value;
                Marshal.StructureToPtr(component, _transformPtr, false);
            }
        }

        public float Y
        {
            get
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                return component.position.Y;
            }
            set
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                component.position.Y = value;
                Marshal.StructureToPtr(component, _transformPtr, false);
            }
        }

        public Vector2 Position
        {
            get
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                return component.position;
            }
            set
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                component.position = value;
                Marshal.StructureToPtr(component, _transformPtr, false);
            }
        }

        public float Width
        {
            get
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                return component.Width;
            }
            set
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                component.Width = value;
                Marshal.StructureToPtr(component, _transformPtr, false);
            }
        }

        public float Height
        {
            get
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                return component.Height;
            }
            set
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                component.Height = value;
                Marshal.StructureToPtr(component, _transformPtr, false);
            }
        }

        public float Rotation
        {
            get
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                return component.Rotation;
            }
            set
            {
                TransformComponent component = Marshal.PtrToStructure<TransformComponent>(_transformPtr);
                component.Rotation = value;
                Marshal.StructureToPtr(component, _transformPtr, false);
            }
        }
    }

    public class BaseComponent
    {
        private IntPtr _entityID;

        public BaseComponent(IntPtr EntityID)
        {
            _entityID = EntityID;
        }
    }

    public partial class SpriteRenderer2D : BaseComponent
    {
        private IntPtr _componentPtr;
        public SpriteRenderer2D(IntPtr ComponentPtr, IntPtr EntityID) : base(EntityID)
        {
            _componentPtr = ComponentPtr;
        }

        public void LoadSprite(string location, bool alpha)
        {
            BridgeAPI.SpriteRenderer2D_LoadSpriteBridge(_componentPtr, location, alpha);
        }
    }

    public partial class Physics2D: BaseComponent
    {
        private IntPtr _componentPtr;
        private IntPtr _physPropPtr;

        public Physics2D(IntPtr ComponentPtr, IntPtr EntityID) : base(EntityID)
        {
            _componentPtr = ComponentPtr;
            _physPropPtr = ComponentPtr + BridgeAPI._Engine_Physics2DOffset;
        }

        public Vector2 Velocity
        {
            get
            {
                PhysicsProperties properties = Marshal.PtrToStructure<PhysicsProperties>(_physPropPtr);
                return properties.Velocity;
            }
            set
            {
                PhysicsProperties properties = Marshal.PtrToStructure<PhysicsProperties>(_physPropPtr);
                properties.Velocity = value;
                Marshal.StructureToPtr(properties, _physPropPtr, false);
            }
        }

        public Vector2 Acceleration
        {
            get
            {
                PhysicsProperties properties = Marshal.PtrToStructure<PhysicsProperties>(_physPropPtr);
                return properties.Acceleration;
            }
            set
            {
                PhysicsProperties properties = Marshal.PtrToStructure<PhysicsProperties>(_physPropPtr);
                properties.Acceleration = value;
                Marshal.StructureToPtr(properties, _physPropPtr, false);
            }
        }

        public float Mass
        {
            get
            {
                PhysicsProperties properties = Marshal.PtrToStructure<PhysicsProperties>(_physPropPtr);
                return properties.mass;
            }
            set
            {
                PhysicsProperties properties = Marshal.PtrToStructure<PhysicsProperties>(_physPropPtr);
                properties.mass = value;
                Marshal.StructureToPtr(properties, _physPropPtr, false);
            }
        }

        public void UpdatePhysics(float deltaTime)
        {
            BridgeAPI.Physics2D_UpdateBridge(_componentPtr, deltaTime);
        }
    }

    [StructLayout(LayoutKind.Explicit)]
    internal struct TransformComponent
    {
        [FieldOffset(0)]
        public Vector2 position;
        [FieldOffset(8)]
        public float Width, Height, Rotation;
    }

    [StructLayout(LayoutKind.Explicit)]
    internal struct PhysicsProperties
    {
        [FieldOffset(0)]
        public Vector2 Velocity;
        [FieldOffset(8)]
        public Vector2 Acceleration;
        [FieldOffset(24)]
        public float mass;
    }

    [StructLayout(LayoutKind.Explicit)]
    internal struct Properties
    {
        [FieldOffset(0)]
        public string Name;
    }
}