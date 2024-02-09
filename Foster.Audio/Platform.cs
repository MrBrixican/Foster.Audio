using System.Numerics;
using System.Runtime.InteropServices;

namespace Foster.Audio;

internal static class Platform
{
	public const string DLL = "FosterAudioPlatform";

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	public delegate void FosterLogFn(IntPtr msg);

	[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
	public struct FosterDesc
	{
		public FosterLogFn onLogInfo;
		public FosterLogFn onLogWarn;
		public FosterLogFn onLogError;
		public int logging;
	}

	public struct FosterBool
	{
		byte value;

		public FosterBool(byte v)
		{
			value = v;
		}

		public static implicit operator bool(FosterBool b) => b.value != 0;
		public static implicit operator FosterBool(bool b) => new(b ? (byte)1 : (byte)0);
	}

	[Flags]
	public enum FosterSoundFlags
	{
		STREAM = 0x00000001,
		DECODE = 0x00000002,
		NO_SPATIALIZATION = 0x00004000
	}

	[DllImport(DLL)]
	public static extern void FosterAudioStartup(FosterDesc desc);
	[DllImport(DLL)]
	public static extern void FosterAudioShutdown();
	[DllImport(DLL)]
	public static extern float FosterAudioGetVolume();
	[DllImport(DLL)]
	public static extern void FosterAudioSetVolume(float value);
	[DllImport(DLL)]
	public static extern int FosterAudioGetChannels();
	[DllImport(DLL)]
	public static extern int FosterAudioGetSampleRate();
	[DllImport(DLL)]
	public static extern ulong FosterAudioGetTimePcmFrames();
	[DllImport(DLL)]
	public static extern void FosterAudioSetTimePcmFrames(ulong value);
	[DllImport(DLL)]
	public static extern int FosterAudioGetListenerCount();
	[DllImport(DLL)]
	public static extern IntPtr FosterAudioDecode(IntPtr data, int length, ref AudioFormat format, ref int channels, ref int sampleRate, out ulong decodedFrameCount);
	[DllImport(DLL)]
	public static extern void FosterAudioFree(IntPtr data);
	[DllImport(DLL)]
	public static extern void FosterAudioRegisterEncodedData(string name, IntPtr data, int length);
	[DllImport(DLL)]
	public static extern void FosterAudioRegisterDecodedData(string name, IntPtr data, ulong frameCount, AudioFormat format, int channels, int sampleRate);
	[DllImport(DLL)]
	public static extern void FosterAudioUnregisterData(string name);

	[DllImport(DLL)]
	public static extern FosterBool FosterAudioListenerGetEnabled(int index);
	[DllImport(DLL)]
	public static extern void FosterAudioListenerSetEnabled(int index, FosterBool value);
	[DllImport(DLL)]
	public static extern Vector3 FosterAudioListenerGetPosition(int index);
	[DllImport(DLL)]
	public static extern void FosterAudioListenerSetPosition(int index, Vector3 value);
	[DllImport(DLL)]
	public static extern Vector3 FosterAudioListenerGetVelocity(int index);
	[DllImport(DLL)]
	public static extern void FosterAudioListenerSetVelocity(int index, Vector3 value);
	[DllImport(DLL)]
	public static extern Vector3 FosterAudioListenerGetDirection(int index);
	[DllImport(DLL)]
	public static extern void FosterAudioListenerSetDirection(int index, Vector3 value);
	[DllImport(DLL)]
	public static extern SoundCone FosterAudioListenerGetCone(int index);
	[DllImport(DLL)]
	public static extern void FosterAudioListenerSetCone(int index, SoundCone value);
	[DllImport(DLL)]
	public static extern Vector3 FosterAudioListenerGetWorldUp(int index);
	[DllImport(DLL)]
	public static extern void FosterAudioListenerSetWorldUp(int index, Vector3 value);

	[DllImport(DLL)]
	public static extern IntPtr FosterSoundCreate(string path, FosterSoundFlags flags, IntPtr soundGroup);
	[DllImport(DLL)]
	public static extern void FosterSoundPlay(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundStop(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundDestroy(IntPtr sound);
	[DllImport(DLL)]
	public static extern float FosterSoundGetVolume(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetVolume(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern float FosterSoundGetPitch(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetPitch(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern float FosterSoundGetPan(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetPan(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern FosterBool FosterSoundGetPlaying(IntPtr sound);
	[DllImport(DLL)]
	public static extern FosterBool FosterSoundGetFinished(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundGetDataFormat(IntPtr sound, out AudioFormat format, out int channels, out int sampleRate);
	[DllImport(DLL)]
	public static extern ulong FosterSoundGetLengthPcmFrames(IntPtr sound);
	[DllImport(DLL)]
	public static extern ulong FosterSoundGetCursorPcmFrames(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetCursorPcmFrames(IntPtr sound, ulong value);
	[DllImport(DLL)]
	public static extern FosterBool FosterSoundGetLooping(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetLooping(IntPtr sound, FosterBool value);
	[DllImport(DLL)]
	public static extern ulong FosterSoundGetLoopBeginPcmFrames(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetLoopBeginPcmFrames(IntPtr sound, ulong value);
	[DllImport(DLL)]
	public static extern ulong FosterSoundGetLoopEndPcmFrames(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetLoopEndPcmFrames(IntPtr sound, ulong value);
	[DllImport(DLL)]
	public static extern FosterBool FosterSoundGetSpatialized(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetSpatialized(IntPtr sound, FosterBool value);
	[DllImport(DLL)]
	public static extern Vector3 FosterSoundGetPosition(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetPosition(IntPtr sound, Vector3 value);
	[DllImport(DLL)]
	public static extern Vector3 FosterSoundGetVelocity(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetVelocity(IntPtr sound, Vector3 value);
	[DllImport(DLL)]
	public static extern Vector3 FosterSoundGetDirection(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetDirection(IntPtr sound, Vector3 value);
	[DllImport(DLL)]
	public static extern SoundPositioning FosterSoundGetPositioning(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetPositioning(IntPtr sound, SoundPositioning value);
	[DllImport(DLL)]
	public static extern int FosterSoundGetPinnedListenerIndex(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetPinnedListenerIndex(IntPtr sound, int value);
	[DllImport(DLL)]
	public static extern SoundAttenuationModel FosterSoundGetAttenuationModel(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetAttenuationModel(IntPtr sound, SoundAttenuationModel value);
	[DllImport(DLL)]
	public static extern float FosterSoundGetRolloff(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetRolloff(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern float FosterSoundGetMinGain(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetMinGain(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern float FosterSoundGetMaxGain(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetMaxGain(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern float FosterSoundGetMinDistance(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetMinDistance(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern float FosterSoundGetMaxDistance(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetMaxDistance(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern SoundCone FosterSoundGetCone(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetCone(IntPtr sound, SoundCone value);
	[DllImport(DLL)]
	public static extern float FosterSoundGetDirectionalAttenuationFactor(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetDirectionalAttenuationFactor(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern float FosterSoundGetDopplerFactor(IntPtr sound);
	[DllImport(DLL)]
	public static extern void FosterSoundSetDopplerFactor(IntPtr sound, float value);
	[DllImport(DLL)]
	public static extern IntPtr FosterSoundGroupCreate(IntPtr parent);
	[DllImport(DLL)]
	public static extern void FosterSoundGroupDestroy(IntPtr soundGroup);
	[DllImport(DLL)]
	public static extern float FosterSoundGroupGetVolume(IntPtr soundGroup);
	[DllImport(DLL)]
	public static extern void FosterSoundGroupSetVolume(IntPtr soundGroup, float value);
	[DllImport(DLL)]
	public static extern float FosterSoundGroupGetPitch(IntPtr soundGroup);
	[DllImport(DLL)]
	public static extern void FosterSoundGroupSetPitch(IntPtr soundGroup, float value);
}
