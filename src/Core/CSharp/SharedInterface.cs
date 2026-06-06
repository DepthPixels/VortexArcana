namespace SharedInterface
{
    public interface IScriptBehavior
    {
        IntPtr ScriptInstancePtr { get; set; }
        void Update();

        void Decimate();
    }
}