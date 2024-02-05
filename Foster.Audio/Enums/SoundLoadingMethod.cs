namespace Foster.Audio;

public enum SoundLoadingMethod
{
	/// <summary>
	/// Loads encoded data into memory
	/// </summary>
	Preload,
	/// <summary>
	/// Loads decoded data into memory
	/// </summary>
	PreloadDecoded,
	/// <summary>
	/// While at least one instance is active, loads encoded data into memory
	/// </summary>
	LoadOnDemand,
	/// <summary>
	/// While at least one instance is active, loads decoded data into memory
	/// </summary>
	LoadOnDemandDecoded,
	/// <summary>
	/// Streams data from file system <br/>
	/// Limitation: ogg files do not work properly in respect to <see cref="SoundInstance.Length"/>, <see cref="SoundInstance.LengthPcmFrames"/>, <see cref="SoundInstance.Cursor"/>, and <see cref="SoundInstance.CursorPcmFrames"/>
	/// </summary>
	Stream
}
