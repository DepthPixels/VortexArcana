namespace VortexArcana
{
    public abstract class BaseEntity
    {
        public IntPtr ScriptInstancePtr { get; set; }
        public abstract void Update();

        public abstract void Decimate();

        public abstract void SaveState(Dictionary<string, object> SaveMap);
        public abstract void ReloadState(Dictionary<string, object> ReloadMap);
    }
}