namespace Foster.Audio;

public struct SoundCone
{
	public readonly float InnerAngleInRadians;
	public readonly float OuterAngleInRadians;
	public readonly float OuterGain;

	public SoundCone(float innerAngleInRadians, float outerAngleInRadians, float outerGain)
	{
		InnerAngleInRadians = innerAngleInRadians;
		OuterAngleInRadians = outerAngleInRadians;
		OuterGain = outerGain;
	}
}
