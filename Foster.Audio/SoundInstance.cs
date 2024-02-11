using System.Numerics;

namespace Foster.Audio;

/// <summary>
/// A lightweight handle for interacting with sound instances. <br/>
/// Will be released automatically if <see cref="Finished"/> is true and <see cref="Protected"/> is false, <br/>
/// or if <see cref="Stop"/> is called and <see cref="Protected"/> is false. <br/>
/// Can be explicitly released via <see cref="Release"/>. <br/>
/// Released instances are marked as inactive. <br/>
/// <see cref="Active"/> will return false if this sound instance is inactive. <br/>
/// Additionally, instances will be created as inactive if <see cref="Audio.MaxActiveInstances"/> or <see cref="Sound.MaxActiveInstances"/> has been reached. <br/>
/// Attempting to access or modify an inactive instance will not throw any exceptions and instead silently fail.
/// </summary>
public readonly struct SoundInstance
{
	/// <summary>
	/// Instance volume.
	/// </summary>
	public float Volume
	{
		get => GetPlatform(Platform.FosterSoundGetVolume);
		set => SetPlatform(value, Platform.FosterSoundSetVolume);
	}

	/// <summary>
	/// Instance pitch.
	/// </summary>
	public float Pitch
	{
		get => GetPlatform(Platform.FosterSoundGetPitch);
		set => SetPlatform(value, Platform.FosterSoundSetPitch);
	}

	/// <summary>
	/// Instance pan.
	/// </summary>
	public float Pan
	{
		get => GetPlatform(Platform.FosterSoundGetPan);
		set => SetPlatform(value, Platform.FosterSoundSetPan);
	}

	/// <summary>
	/// Whether instance is playing.
	/// </summary>
	public bool Playing
	{
		get => GetPlatform(Platform.FosterSoundGetPlaying);
	}

	/// <summary>
	/// Whether instance is finished playing. <br/>
	/// If <see cref="Finished"/> is true and <see cref="Protected"/> is false, the instance will be automatically released.
	/// </summary>
	public bool Finished
	{
		get => GetPlatform(Platform.FosterSoundGetFinished);
	}

	public AudioFormat Format
	{
		get
		{
			GetDataFormat();
			return ActiveState?.Format ?? default;
		}
	}

	public int Channels
	{
		get
		{
			GetDataFormat();
			return ActiveState?.Channels ?? default;
		}
	}

	public int SampleRate
	{
		get
		{
			GetDataFormat();
			return ActiveState?.SampleRate ?? default;
		}
	}

	/// <summary>
	/// Instance length in PCM frames. <br/>
	/// This can be <i>extremely</i> slow for certain codecs (mp3). <br/>
	/// Limitation: This will always return 0 for an ogg loaded with <see cref="SoundLoadingMethod.Stream"/>.
	/// </summary>
	public ulong LengthPcmFrames
	{
		get => GetPlatform(Platform.FosterSoundGetLengthPcmFrames);
	}

	/// <summary>
	/// Instance length. <br/>
	/// This can be <i>extremely</i> slow for certain codecs (mp3). <br/>
	/// Limitation: This will always return <see cref="TimeSpan.Zero"/> for an ogg loaded with <see cref="SoundLoadingMethod.Stream"/>.
	/// </summary>
	public TimeSpan Length => TimeSpan.FromSeconds(1.0 * LengthPcmFrames / SampleRate);

	/// <summary>
	/// Instance cursor in PCM frames. <br/>
	/// Setting does not have an immediate effect and must be processed by the audio thread. <br/>
	/// Limitation: This will return total frames played for an ogg loaded with <see cref="SoundLoadingMethod.Stream"/> that is looped.
	/// </summary>
	public ulong CursorPcmFrames
	{
		get => GetPlatform(Platform.FosterSoundGetCursorPcmFrames);
		set => SetPlatform(value, Platform.FosterSoundSetCursorPcmFrames);
	}

	/// <summary>
	/// Instance cursor. <br/>
	/// Setting does not have an immediate effect and must be processed by the audio thread. <br/>
	/// Limitation: This will return total time played for an ogg loaded with <see cref="SoundLoadingMethod.Stream"/> that is looped.
	/// </summary>
	public TimeSpan Cursor
	{
		get => TimeSpan.FromSeconds(1.0 * CursorPcmFrames / SampleRate);
		set => CursorPcmFrames = (ulong)Math.Floor(value.TotalSeconds * SampleRate);
	}

	/// <summary>
	/// Whether instance is looping. <br/>
	/// A looping instance will never be <see cref="Finished"/> or be automatically released.
	/// </summary>
	public bool Looping
	{
		get => GetPlatform(Platform.FosterSoundGetLooping);
		set => SetPlatform<Platform.FosterBool>(value, Platform.FosterSoundSetLooping);
	}

	/// <summary>
	/// Instance loop begin in PCM frames.
	/// </summary>
	public ulong LoopBeginPcmFrames
	{
		get => GetPlatform(Platform.FosterSoundGetLoopBeginPcmFrames);
		set => SetPlatform(value, Platform.FosterSoundSetLoopBeginPcmFrames);
	}

	/// <summary>
	/// Instance loop begin.
	/// </summary>
	public TimeSpan LoopBegin
	{
		get => TimeSpan.FromSeconds(1.0 * LoopBeginPcmFrames / SampleRate);
		set => LoopBeginPcmFrames = (ulong)Math.Floor(value.TotalSeconds * SampleRate);
	}

	/// <summary>
	/// Instance loop end in PCM frames. <br/>
	/// <see cref="ulong.MaxValue"/> indicates no loop end.
	/// </summary>
	public ulong LoopEndPcmFrames
	{
		get => GetPlatform(Platform.FosterSoundGetLoopEndPcmFrames);
		set => SetPlatform(value, Platform.FosterSoundSetLoopEndPcmFrames);
	}

	/// <summary>
	/// Instance loop end.
	/// </summary>
	public TimeSpan? LoopEnd
	{
		get
		{
			var end = LoopEndPcmFrames;
			if (end == ulong.MaxValue)
			{
				return null; // Special case
			}
			return TimeSpan.FromSeconds(1.0 * end / SampleRate);
		}
		set
		{
			if (value.HasValue)
			{
				LoopEndPcmFrames = (ulong)Math.Floor(value.Value.TotalSeconds * SampleRate);
			}
			else
			{
				LoopEndPcmFrames = ulong.MaxValue;
			}
		}
	}

	/// <summary>
	/// Whether this instance is spatialized.
	/// </summary>
	public bool Spatialized
	{
		get => GetPlatform(Platform.FosterSoundGetSpatialized);
		set => SetPlatform<Platform.FosterBool>(value, Platform.FosterSoundSetSpatialized);
	}

	/// <summary>
	/// Instance position. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public Vector3 Position
	{
		get => GetPlatform(Platform.FosterSoundGetPosition);
		set => SetPlatform(value, Platform.FosterSoundSetPosition);
	}

	/// <summary>
	/// Instance velocity. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public Vector3 Velocity
	{
		get => GetPlatform(Platform.FosterSoundGetVelocity);
		set => SetPlatform(value, Platform.FosterSoundSetVelocity);
	}

	/// <summary>
	/// Instance direction. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public Vector3 Direction
	{
		get => GetPlatform(Platform.FosterSoundGetDirection);
		set => SetPlatform(value, Platform.FosterSoundSetDirection);
	}

	/// <summary>
	/// Instance positioning. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public SoundPositioning Positioning
	{
		get => GetPlatform(Platform.FosterSoundGetPositioning);
		set => SetPlatform(value, Platform.FosterSoundSetPositioning);
	}

	/// <summary>
	/// Pinned <see cref="AudioListener"/> index. <br/>
	/// Instance spatial audio effects will be relative to the specified <see cref="AudioListener"/>. <br/>
	/// -1 is used to specify closest <see cref="AudioListener"/>. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public int PinnedListenerIndex
	{
		get => GetPlatform(Platform.FosterSoundGetPinnedListenerIndex);
		set => SetPlatform(value, Platform.FosterSoundSetPinnedListenerIndex);
	}

	/// <summary>
	/// Instance attenuation model. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public SoundAttenuationModel AttenuationModel
	{
		get => GetPlatform(Platform.FosterSoundGetAttenuationModel);
		set => SetPlatform(value, Platform.FosterSoundSetAttenuationModel);
	}

	/// <summary>
	/// Instance rolloff. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public float Rolloff
	{
		get => GetPlatform(Platform.FosterSoundGetRolloff);
		set => SetPlatform(value, Platform.FosterSoundSetRolloff);
	}

	/// <summary>
	/// Instance minimum gain. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public float MinGain
	{
		get => GetPlatform(Platform.FosterSoundGetMinGain);
		set => SetPlatform(value, Platform.FosterSoundSetMinGain);
	}

	/// <summary>
	/// Instance maximum gain. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public float MaxGain
	{
		get => GetPlatform(Platform.FosterSoundGetMaxGain);
		set => SetPlatform(value, Platform.FosterSoundSetMaxGain);
	}

	/// <summary>
	/// Instance minimum distance. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public float MinDistance
	{
		get => GetPlatform(Platform.FosterSoundGetMinDistance);
		set => SetPlatform(value, Platform.FosterSoundSetMinDistance);
	}

	/// <summary>
	/// Instance maximum distance. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public float MaxDistance
	{
		get => GetPlatform(Platform.FosterSoundGetMaxDistance);
		set => SetPlatform(value, Platform.FosterSoundSetMaxDistance);
	}

	/// <summary>
	/// Instance cone. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public SoundCone Cone
	{
		get => GetPlatform(Platform.FosterSoundGetCone);
		set => SetPlatform(value, Platform.FosterSoundSetCone);
	}

	/// <summary>
	/// Instance directional attenuation factor. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public float DirectionalAttenuationFactor
	{
		get => GetPlatform(Platform.FosterSoundGetDirectionalAttenuationFactor);
		set => SetPlatform(value, Platform.FosterSoundSetDirectionalAttenuationFactor);
	}

	/// <summary>
	/// Instance doppler factor. <br/>
	/// Has no effect if <see cref="Spatialized"/> is false.
	/// </summary>
	public float DopplerFactor
	{
		get => GetPlatform(Platform.FosterSoundGetDopplerFactor);
		set => SetPlatform(value, Platform.FosterSoundSetDopplerFactor);
	}

	/// <summary>
	/// When true, this instance will not be automatically released once <see cref="Finished"/> is true or when <see cref="Stop"/> is called.
	/// </summary>
	public bool Protected
	{
		get => ActiveState?.Protected ?? false;
		set
		{
			if(Active)
			{
				state!.Protected = value;
			}
		}
	}

	/// <summary>
	/// Instance sound. <br/>
	/// Returns null if <see cref="Active"/> is false.
	/// </summary>
	public Sound? Sound => ActiveState?.Sound;

	/// <summary>
	/// Instance sound group. <br/>
	/// Returns null if the instance was not created with a sound group or <see cref="Active"/> is false.
	/// </summary>
	public SoundGroup? Group => ActiveState?.Group;

	/// <summary>
	/// Whether the instance is active.
	/// </summary>
	public bool Active => state?.Id == Id;

	private State? ActiveState => state?.Id == Id ? state : null;

	private readonly long Id;
	private readonly State? state;

	private static long nextInstanceId = 1;
	private static readonly Stack<State> statePool = new();

	internal SoundInstance(Sound sound, SoundGroup? group, bool spatialized)
	{
		if (sound is null)
		{
			throw new ArgumentNullException(nameof(sound));
		}

		Id = 0;
		state = null;

		// Ensure max sound threshold hasn't been reached
		if (sound.ActiveInstances >= sound.MaxActiveInstances || Audio.ActiveInstances >= Audio.MaxActiveInstances)
		{
			return;
		}

		// Set some initial flags
		Platform.FosterSoundFlags fosterFlags = 0;

		if (sound.LoadingMethod == SoundLoadingMethod.Stream)
		{
			fosterFlags |= Platform.FosterSoundFlags.STREAM;
		}
		else if (sound.LoadingMethod is SoundLoadingMethod.PreloadDecoded or SoundLoadingMethod.LoadOnDemandDecoded)
		{
			fosterFlags |= Platform.FosterSoundFlags.DECODE;
		}

		if (!spatialized)
		{
			fosterFlags |= Platform.FosterSoundFlags.NO_SPATIALIZATION;
		}

		// Attempt to create the sound
		var ptr = Platform.FosterSoundCreate(sound.Path, fosterFlags, group?.Ptr ?? IntPtr.Zero);

		// Ensure sound was actually created
		if (ptr == IntPtr.Zero)
		{
			return;
		}
		
		lock(statePool)
		{
			// Get new id while we have a lock
			Id = nextInstanceId++;

			// Acquire and set state
			if (!statePool.TryPop(out state))
			{
				state = new State();
			}

			state.Id = Id;
			state.Ptr = ptr;
			state.Sound = sound;
			state.Group = group;
			state.Protected = false;

			// Increment counts while we have a lock
			Audio.ActiveInstances++;
			sound.ActiveInstances++;
		}

		Audio.OnSoundInstanceCreate(this);
	}

	/// <summary>
	/// Releases this instance, making it inactive and freeing memory in the process.
	/// </summary>
	public void Release()
	{
		if (Active && state != null)
		{
			var ptr = state.Ptr;
			
			lock (statePool)
			{
				// Decrement counts while we have a lock
				Audio.ActiveInstances--;
				state.Sound.ActiveInstances--;

				// Clear and free state
				state.Clear();
				statePool.Push(state);
			}

			// Attempt to destroy the sound
			Platform.FosterSoundDestroy(ptr);
		}
	}

	/// <summary>
	/// Plays the instance.
	/// </summary>
	public void Play()
	{
		if (Active)
		{
			Platform.FosterSoundPlay(state!.Ptr);
		}
	}

	/// <summary>
	/// Pauses the instance.
	/// </summary>
	public void Pause()
	{
		if (Active)
		{
			Platform.FosterSoundStop(state!.Ptr);
		}
	}

	/// <summary>
	/// If the instance is <see cref="Protected"/>, stops the instance and seeks back to the beginning.
	/// Otherwise, releases the instance.
	/// </summary>
	public void Stop()
	{
		if (Active)
		{
			if (state!.Protected)
			{
				Platform.FosterSoundStop(state!.Ptr);
				CursorPcmFrames = 0;
			}
			else
			{
				Release();
			}
		}
	}

	//public void Fade(...) { }

	//public void SchedulePlay(...) { }

	//public void ScheduleStop(...) { }

	//public void ScheduleFade(...) { }

	private T GetPlatform<T>(Func<IntPtr, T> getter)
	{
		if (Active)
		{
			return getter(state!.Ptr);
		}
		return default!;
	}

	private void SetPlatform<T>(T value, Action<IntPtr, T> setter)
	{
		if (Active)
		{
			setter(state!.Ptr, value);
		}
	}

	private void GetDataFormat()
	{
		if (Active && state!.Format == AudioFormat.Unknown)
		{
			Platform.FosterSoundGetDataFormat(state.Ptr, out var format, out var channels, out var sampleRate);
			state.Format = format;
			state.Channels = channels;
			state.SampleRate = sampleRate;
		}
	}

	private class State
	{
		public long Id { get; set; }
		public IntPtr Ptr { get; set; }
		public Sound Sound { get; set; } = null!;
		public SoundGroup? Group { get; set; }
		public bool Protected { get; set; }
		public AudioFormat Format { get; set; }
		public int Channels { get; set; }
		public int SampleRate { get; set; }

		public void Clear()
		{
			Id = -1;
			Ptr = default;
			Sound = default!;
			Group = default;
			Protected = default;
			Format = default;
			Channels = default;
			SampleRate = default;
		}
	}
}
