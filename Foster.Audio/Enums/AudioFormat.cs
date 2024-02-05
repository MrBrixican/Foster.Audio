namespace Foster.Audio;

public enum AudioFormat
{
	Unknown = 0,
	U8 = 1,
	S16 = 2,
	S24 = 3,
	S32 = 4,
	F32 = 5
}

public static class AudioFormatExtensions
{
	public static int GetSampleSize(this AudioFormat format)
	{
		return format switch
		{
			AudioFormat.Unknown => 0,
			AudioFormat.U8 => 1,
			AudioFormat.S16 => 2,
			AudioFormat.S24 => 3,
			AudioFormat.S32 => 4,
			AudioFormat.F32 => 4,
			_ => 0
		};
	}
}
