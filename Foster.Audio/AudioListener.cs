using System.Numerics;

namespace Foster.Audio;

public class AudioListener
{
	public int Index { get; }

	public bool Enabled
	{
		get => Platform.FosterAudioListenerGetEnabled(Index);
		set => Platform.FosterAudioListenerSetEnabled(Index, value);
	}

	public Vector3 Position
	{
		get => Platform.FosterAudioListenerGetPosition(Index);
		set => Platform.FosterAudioListenerSetPosition(Index, value);
	}

	public Vector3 Velocity
	{
		get => Platform.FosterAudioListenerGetVelocity(Index);
		set => Platform.FosterAudioListenerSetVelocity(Index, value);
	}

	public Vector3 Direction
	{
		get => Platform.FosterAudioListenerGetDirection(Index);
		set => Platform.FosterAudioListenerSetDirection(Index, value);
	}

	public SoundCone Cone
	{
		get => Platform.FosterAudioListenerGetCone(Index);
		set => Platform.FosterAudioListenerSetCone(Index, value);
	}

	public Vector3 WorldUp
	{
		get => Platform.FosterAudioListenerGetWorldUp(Index);
		set => Platform.FosterAudioListenerSetWorldUp(Index, value);
	}

	internal AudioListener(int index)
	{
		Index = index;
	}
}
