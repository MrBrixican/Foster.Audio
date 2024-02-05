using System.Numerics;
using System.Runtime.InteropServices;

namespace Foster.Audio;

/// <summary>
/// Sound data representation used to create and play <see cref="SoundInstance"/>s
/// </summary>
public class Sound : IDisposable
{
	/// <summary>
	/// The maximum number of simultaneously active <see cref="SoundInstance"/>s allowed for this <see cref="Sound"/>.
	/// Instances created after this threshold are inactive and do not play any audio.
	/// </summary>
	public int MaxActiveInstances { get; set; } = int.MaxValue;

	/// <summary>
	/// The number of currently active <see cref="SoundInstance"/>s for this <see cref="Sound"/>
	/// </summary>
	public int ActiveInstances { get; internal set; }

	/// <summary>
	/// Loading method used for this <see cref="Sound"/>
	/// </summary>
	public SoundLoadingMethod LoadingMethod { get; private set; }

	/// <summary>
	/// Path to uniquely identify the backing audio data
	/// </summary>
	internal string Path { get; private set; } = Guid.NewGuid().ToString();

	private GCHandle handle;
	private IntPtr ptr;

	/// <summary>
	/// Loads encoded data from <paramref name="path"/> using <paramref name="loadingMethod"/>
	/// </summary>
	public Sound(string path, SoundLoadingMethod loadingMethod = SoundLoadingMethod.Preload)
	{
		// Since we actually know the path, we use it instead
		// This enables streaming/automatic resource dedupe
		Path = System.IO.Path.GetFullPath(path);
		LoadingMethod = loadingMethod;
		if(loadingMethod is SoundLoadingMethod.Preload or SoundLoadingMethod.PreloadDecoded)
		{
			var data = File.ReadAllBytes(Path);
			LoadEncoded(data, loadingMethod == SoundLoadingMethod.PreloadDecoded);
		}
	}

	/// <summary>
	/// Loads encoded data from <paramref name="stream"/> into memory, optionally decoding
	/// </summary>
	public Sound(Stream stream, bool decode = false)
	{
		var data = new byte[stream.Length - stream.Position];
		stream.Read(data);
		LoadEncoded(data, decode);
	}

	/// <summary>
	/// Loads encoded data from <paramref name="data"/> into memory, optionally decoding
	/// </summary>
	public Sound(byte[] data, bool decode = false)
	{
		LoadEncoded(data, decode);
	}

	/// <summary>
	/// Loads decoded data from <paramref name="data"/> into memory
	/// </summary>
	public Sound(byte[] data, AudioFormat format, int channels, int sampleRate, ulong frameCount)
	{
		LoadDecoded(data, format, channels, sampleRate, frameCount);
	}

	private void LoadEncoded(byte[] data, bool decode = false)
	{
		if (decode)
		{
			var format = AudioFormat.S16; // TODO
			var channels = 0; // Determine automatically
			var sampleRate = Audio.SampleRate;
			if (TryDecode(data, ref format, ref channels, ref sampleRate, out var frameCount, out data!))
			{
				LoadDecoded(data, format, channels, sampleRate, frameCount);
			}
			else
			{
				throw new Exception("Failed to decode Sound");
			}
		}
		else
		{
			LoadingMethod = SoundLoadingMethod.Preload;
			handle = GCHandle.Alloc(data, GCHandleType.Pinned);
			ptr = handle.AddrOfPinnedObject();
			Platform.FosterAudioRegisterEncodedData(Path, ptr, data.Length);
		}
	}

	private void LoadDecoded(byte[] data, AudioFormat format, int channels, int sampleRate, ulong frameCount)
	{
		LoadingMethod = SoundLoadingMethod.PreloadDecoded;
		handle = GCHandle.Alloc(data, GCHandleType.Pinned);
		ptr = handle.AddrOfPinnedObject();
		Platform.FosterAudioRegisterDecodedData(Path, ptr, frameCount, format, channels, sampleRate);
	}

	/// <summary>
	/// Creates a new <see cref="SoundInstance"/>
	/// </summary>
	public SoundInstance CreateInstance(SoundGroup? group = null)
	{
		var instance = new SoundInstance(this, group, false);
		return instance;
	}

	/// <summary>
	/// Creates a new spatialized <see cref="SoundInstance"/> at <paramref name="position"/>
	/// </summary>
	public SoundInstance CreateInstance3d(Vector3 position, SoundGroup? group = null)
	{
		var instance = new SoundInstance(this, group, true);
		instance.Position = position;
		return instance;
	}

	/// <summary>
	/// Creates and plays a new <see cref="SoundInstance"/>
	/// </summary>
	public SoundInstance Play(SoundGroup? group = null)
	{
		var instance = CreateInstance(group);
		instance.Play();
		return instance;
	}

	/// <summary>
	/// Creates and plays a new spatialized <see cref="SoundInstance"/> at <paramref name="position"/>
	/// </summary>
	public SoundInstance Play3d(Vector3 position, SoundGroup? group = null)
	{
		var instance = CreateInstance3d(position, group);
		instance.Play();
		return instance;
	}

	public void PlayAll() => ApplyAll(m => m.Play());

	public void PauseAll() => ApplyAll(m => m.Pause());

	public void StopAll() => ApplyAll(m => m.Stop());

	public void ReleaseAll() => ApplyAll(m => m.Release());

	public void ApplyAll(Action<SoundInstance> action)
	{
		foreach (var instance in Audio.Instances)
		{
			if (instance.Active && instance.Sound == this)
			{
				action(instance);
			}
		}
	}

	~Sound() => Dispose();

	/// <summary>
	/// Completely dispose of the <see cref="Sound"/> and releases every associated <see cref="SoundInstance"/>
	/// </summary>
	public void Dispose()
	{
		ReleaseAll();

		if (handle.IsAllocated)
		{
			handle.Free();
			Platform.FosterAudioUnregisterData(Path);
		}

		handle = default;
		ptr = default;
	}

	/// <summary>
	/// Attempts to decode <paramref name="data"/> using specified parameters (or determines parameters from encoded data)
	/// </summary>
	/// <param name="data">encoded data</param>
	/// <param name="format">sample format, use <see cref="AudioFormat.Unknown"/> to use and return data defined format</param>
	/// <param name="channels">channels, use 0 to use and return data defined channels</param>
	/// <param name="sampleRate">sample rate, use 0 to use and return data defined sample rate</param>
	/// <param name="frameCount">decoded frame count</param>
	/// <param name="decodedData">decoded data</param>
	/// <returns>Whether the data was successfully decoded</returns>
	public static bool TryDecode(byte[] data, ref AudioFormat format, ref int channels, ref int sampleRate, out ulong frameCount, out byte[]? decodedData)
	{
		frameCount = 0;
		decodedData = null;

		unsafe
		{
			fixed (byte* pData = data)
			{
				var pDecoded = Platform.FosterAudioDecode(new IntPtr(pData), data.Length, ref format, ref channels, ref sampleRate, out frameCount);
				if(pDecoded == IntPtr.Zero)
				{
					return false;
				}
				else
				{
					var length = format.GetSampleSize() * channels * (int)frameCount;
					decodedData = new byte[length];
					new Span<byte>(pDecoded.ToPointer(), length).CopyTo(decodedData);
					Platform.FosterAudioFree(pDecoded);
					return true;
				}
			}
		}
	}
}
