using System.Collections.ObjectModel;
using System.Runtime.InteropServices;

namespace Foster.Audio;

public static class Audio
{
	/// <summary>
	/// Master volume
	/// </summary>
	public static float Volume
	{
		get => Platform.FosterAudioGetVolume();
		set => Platform.FosterAudioSetVolume(value);
	}

	/// <summary>
	/// Audio engine channel count
	/// </summary>
	public static int Channels { get; private set; }

	/// <summary>
	/// Audio engine sample rate
	/// </summary>
	public static int SampleRate { get; private set; }

	/// <summary>
	/// Audio engine clock in PCM frames (see <seealso cref="SampleRate"/>)
	/// </summary>
	public static ulong TimePcmFrames
	{
		get => Platform.FosterAudioGetTimePcmFrames();
		set => Platform.FosterAudioSetTimePcmFrames(value);
	}

	/// <summary>
	/// Audio engine clock
	/// </summary>
	public static TimeSpan Time
	{
		get => TimeSpan.FromSeconds(1.0 * TimePcmFrames / SampleRate);
		set => TimePcmFrames = (ulong)Math.Floor(value.TotalSeconds * SampleRate);
	}

	/// <summary>
	/// The maximum number of simultaneously active <see cref="SoundInstance"/>s allowed.
	/// Instances created after this threshold are inactive and do not play any audio.
	/// </summary>
	public static int MaxActiveInstances { get; set; } = 100;

	/// <summary>
	/// The number of currently active <see cref="SoundInstance"/>s
	/// </summary>
	public static int ActiveInstances { get; internal set; }

	/// <summary>
	/// The primary (index 0) <see cref="AudioListener"/>
	/// </summary>
	public static AudioListener Listener { get; private set; } = null!;

	/// <summary>
	/// All available <see cref="AudioListener"/>s
	/// </summary>
	public static ReadOnlyCollection<AudioListener> Listeners { get; private set; } = null!;

	/// <summary>
	/// All instances currently being tracked, some may be inactive
	/// </summary>
	public static ReadOnlySpan<SoundInstance> Instances => CollectionsMarshal.AsSpan(instances);

	private static readonly List<SoundInstance> instances = new();

	public static void PlayAll() => ApplyAll(m => m.Play());

	public static void PauseAll() => ApplyAll(m => m.Pause());

	public static void StopAll() => ApplyAll(m => m.Stop());

	public static void ReleaseAll() => ApplyAll(m => m.Release());

	/// <summary>
	/// Applies an action to all active instances.
	/// </summary>
	/// <param name="action"></param>
	public static void ApplyAll(Action<SoundInstance> action)
	{
		foreach (var instance in Instances)
		{
			if(instance.Active)
			{
				action(instance);
			}
		}
	}

	/// <summary>
	/// Initializes Audio. Call once on startup.
	/// </summary>
	public static void Startup()
	{
		Platform.FosterAudioStartup(new());
		Channels = Platform.FosterAudioGetChannels();
		SampleRate = Platform.FosterAudioGetSampleRate();
		Listeners = Enumerable.Range(0, Platform.FosterAudioGetListenerCount())
			.Select(i => new AudioListener(i))
			.ToList()
			.AsReadOnly();
		Listener = Listeners[0];
	}

	/// <summary>
	/// Runs sound instance management. Call once per frame/update.
	/// </summary>
	public static void Update()
	{
		// Go through all instances and destroy all that are non-protected, finished
		for (int i = 0;i<instances.Count;i++)
		{
			var instance = instances[i];
			if (instance.Protected)
			{
				continue;
			}

			if (instance.Finished)
			{
				instance.Release();
			}
		}

		// Remove inactive instances
		lock (instances)
		{
			instances.RemoveAll(i => !i.Active);
		}
	}

	/// <summary>
	/// Deinitializes and cleans up Audio. Call once on shutdown.
	/// </summary>
	public static void Shutdown()
	{
		ReleaseAll();
		lock (instances)
		{
			instances.Clear();
		}
		Platform.FosterAudioShutdown();
	}

	internal static void OnSoundInstanceCreate(SoundInstance instance)
	{
		lock(instances)
		{
			instances.Add(instance);
		}
	}
}
