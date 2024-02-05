namespace Foster.Audio;

public class SoundGroup: IDisposable
{
	public string Name { get; set; }

	public SoundGroup? Parent { get; }

	public float Volume
	{
		get => Platform.FosterSoundGroupGetVolume(Ptr);
		set => Platform.FosterSoundGroupSetVolume(Ptr, value);
	}

	public float Pitch
	{
		get => Platform.FosterSoundGroupGetPitch(Ptr);
		set => Platform.FosterSoundGroupSetPitch(Ptr, value);
	}

	internal IntPtr Ptr { get; private set; }

	public SoundGroup(string? name = null, SoundGroup? parent = null)
	{
		Name = name ?? string.Empty;
		Parent = parent;
		Ptr = Platform.FosterSoundGroupCreate(Parent?.Ptr ?? IntPtr.Zero);

		if (Ptr == IntPtr.Zero)
		{
			throw new Exception("Failed to create SoundGroup");
		}
	}

	public void PlayAll(bool recursive = true) => ApplyAll(m => m.Play(), recursive);

	public void PauseAll(bool recursive = true) => ApplyAll(m => m.Pause(), recursive);

	public void StopAll(bool recursive = true) => ApplyAll(m => m.Stop(), recursive);

	public void ReleaseAll(bool recursive = true) => ApplyAll(m => m.Release(), recursive);

	public void ApplyAll(Action<SoundInstance> action, bool recursive = true)
	{
		foreach (var instance in Audio.Instances)
		{
			if (instance.Active && (recursive ? IsChildOrSelf(instance.Group) : (instance.Group == this)))
			{
				action(instance);
			}
		}
	}

	public bool IsChildOrSelf(SoundGroup? other)
	{
		while(other != null)
		{
			if(other == this)
			{
				return true;
			}

			other = other.Parent;
		}

		return false;
	}

	~SoundGroup() => Dispose();

	public void Dispose()
	{
		ReleaseAll();

		if (Ptr != IntPtr.Zero)
		{
			Platform.FosterSoundGroupDestroy(Ptr);
			Ptr = IntPtr.Zero;
		}
	}
}
