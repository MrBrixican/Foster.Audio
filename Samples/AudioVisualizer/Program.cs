using Foster.Audio;
using Foster.Framework;
using System.Numerics;
using System.Runtime.InteropServices;

namespace AudioVisualizer;

class Program
{
	public static void Main()
	{
		App.Register<AudioModule>();
		App.Register<Game>();
		App.Run("Audio Visualizer", 1280, 720);
	}
}

class AudioModule : Module
{
	public override void Startup() => Audio.Startup();
	public override void Update() => Audio.Update();
	public override void Shutdown() => Audio.Shutdown();
}

class Game : Module
{
	private const int N = 4096; // Must be a power of 2, higher = more CPU time and higher fidelity results
	private const float LineThickness = 2;
	private const float LineScale = .5f;
	private static readonly int[] FreqBin = new[] { 20, 60, 250, 500, 1000 };

	// Color gradient for visualizer
	private static readonly Color[] colors = Enumerable.Range(0, 256 * 3)
		.Select(m => new Color(
			(byte)Math.Clamp(m, 0, 255),
			(byte)Math.Clamp(m - 256, 0, 255),
			(byte)Math.Clamp(m - 512, 0, 255),
			255))
		.ToArray();

	private readonly Batcher batch = new();
	private readonly SpriteFont font = new SpriteFont(Path.Join("Assets", "monogram.ttf"), 32);

	private Sound? sound;
	private SoundInstance instance;
	private double instanceLength;
	private int channels;

	private byte[]? data;
	private Complex[] fft = new Complex[N];
	private List<float> peakMax = new List<float>();

	public override void Startup()
	{
		var encodedData = File.ReadAllBytes(Path.Join("Assets", "shortcuts.ogg"));
		var format = AudioFormat.F32;
		channels = 0; // Determine automatically
		var sampleRate = Audio.SampleRate;

		if (Sound.TryDecode(encodedData, ref format, ref channels, ref sampleRate, out var frameCount, out data))
		{
			sound = new Sound(data!, format, channels, sampleRate, frameCount);
			instance = sound.CreateInstance();
			instance.Protected = true;
			instance.Looping = true;
			instance.Volume = 0.1f;
			instanceLength = instance.Length.TotalSeconds;
			instance.Play();
		}
		else
		{
			throw new Exception("Couldn't parse encoded data");
		}
	}

	public override void Shutdown()
	{
		sound?.Dispose();
	}

	public override void Update()
	{
		App.Title = $"Audio Visualizer {App.Width}x{App.Height} : {App.WidthInPixels}x{App.HeightInPixels}";

		if (Input.Keyboard.Pressed(Keys.Left))
		{
			instance.Cursor = TimeSpan.FromSeconds(Math.Clamp(instance.Cursor.TotalSeconds - 10, 0, instanceLength));
		}

		if (Input.Keyboard.Pressed(Keys.Right))
		{
			instance.Cursor = TimeSpan.FromSeconds(Math.Clamp(instance.Cursor.TotalSeconds + 10, 0, instanceLength));
		}

		if (Input.Keyboard.Pressed(Keys.Up))
		{
			instance.Pitch = Math.Clamp(instance.Pitch + .25f, 0, 4);
		}

		if (Input.Keyboard.Pressed(Keys.Down))
		{
			instance.Pitch = Math.Clamp(instance.Pitch - .25f, 0, 4);
		}

		if (Input.Keyboard.Pressed(Keys.Space))
		{
			if (instance.Playing)
			{
				instance.Pause();
			}
			else
			{
				instance.Play();
			}
		}
	}

	public override void Render()
	{
		Graphics.Clear(Color.Black);

		RenderCircle(batch, App.WidthInPixels / 2, App.HeightInPixels / 2, 50);

		var playBarSize = new Vector2(App.WidthInPixels, 30);

		batch.Rect(new Rect(Vector2.Zero, playBarSize), Color.Gray * .75f);
		batch.Rect(new Rect((float)(playBarSize.X * instance.Cursor.TotalSeconds / instanceLength), playBarSize.Y), Color.Red * .75f);
		batch.Text(font, $"{instance.Cursor:mm':'ss}/{TimeSpan.FromSeconds(instanceLength):mm':'ss}", new(playBarSize.X / 2, playBarSize.Y / 2), new(0.5f, 0.5f), Color.White);
		batch.Text(font, $"x{instance.Pitch}", new(playBarSize.X / 2, playBarSize.Y + 5), new(0.5f, 0f), Color.White);

		batch.Render();
		batch.Clear();
	}

	public void RenderCircle(Batcher batch, float centerX, float centerY, float radius)
	{
		if (!instance.Active || data == null)
		{
			return;
		}

		CalcPeakMax();

		// Take the average and add it to the radius of the circle
		float aprox = 0;
		for (int i = 0; i < peakMax.Count; i++)
		{
			aprox += peakMax[i];

			// Scale
			peakMax[i] *= LineScale;
		}

		aprox /= peakMax.Count;
		aprox += 0.0001f; // Avoid dividing by 0 later on

		radius += aprox;

		var angle = MathF.PI / (peakMax.Count);
		for (int j = 0; j < 2; j++)
		{
			for (int i = 0; i < peakMax.Count; i++)
			{
				// Angle in circle
				var a = i * angle + angle * .5f;
				if (j == 1)
				{
					a *= j * -1; // Mirror
				}
				a += MathF.PI / 2; // Rotate by 90 degrees

				// Circle points
				var x = centerX + radius * MathF.Cos(a);
				var y = centerY + radius * MathF.Sin(a);

				// Color
				var c = colors[(int)Math.Clamp((peakMax[i] / aprox + .2f) * colors.Length, 0, colors.Length - 1)];

				// Translate to the circle center then translate to each point and rotate it
				batch.PushMatrix(Matrix3x2.CreateRotation(a) * Matrix3x2.CreateTranslation(x, y));
				batch.Line(new(0, 0), new(peakMax[i], 0), LineThickness, c);
				batch.PopMatrix();
			}
		}
	}

	// Heavily based on https://github.com/miha53cevic/AudioVisualizerJS/tree/master
	private void CalcPeakMax()
	{
		var cursor = (int)instance.CursorPcmFrames;

		var dataF = MemoryMarshal.Cast<byte, float>(data);

		// Fill in input
		for (int i = 0; i < N; i++)
		{
			var index = (i + cursor - N / 2) * channels;

			// Hamming window the input for smoother input values
			var sample = index < dataF!.Length && index >= 0 ?
				dataF[index] * (0.54f - (0.46f * MathF.Cos(2.0f * MathF.PI * (i / ((N - 1) * 1.0f)))))
				: 0;

			// Windowed sample or signal
			fft[i] = sample;
		}

		// Calculate fft
		CalcFFT(fft, false);

		peakMax.Clear();

		// Calculate the magnitudes
		// Only half of the data is useful
		for (int i = 0; i < (N / 2) + 1; i++)
		{
			var freq = 1f * i * Audio.SampleRate / N;
			var magnitude = fft[i].Magnitude;

			// Extract the peaks from defined frequency ranges
			for (int j = 0; j < FreqBin.Length - 1; j++)
			{
				if ((freq > FreqBin[j]) && (freq <= FreqBin[j + 1]))
				{
					peakMax.Add((float)magnitude);
				}
			}
		}
	}

	/// <summary>
	/// FFT <br/>
	/// Modified version of this implementation: https://cp-algorithms.com/algebra/fft.html
	/// </summary>
	/// <param name="a"></param>
	/// <param name="invert"></param>
	private static void CalcFFT(Complex[] a, bool invert)
	{
		int n = a.Length;

		for (int i = 1, j = 0; i < n; i++)
		{
			int bit = n >> 1;
			for (; (j & bit) > 0; bit >>= 1)
				j ^= bit;
			j ^= bit;

			if (i < j)
				(a[i], a[j]) = (a[j], a[i]);
		}

		for (int len = 2; len <= n; len <<= 1)
		{
			double ang = 2 * Math.PI / len * (invert ? -1 : 1);
			var wlen = new Complex(Math.Cos(ang), Math.Sin(ang));
			for (int i = 0; i < n; i += len)
			{
				var w = new Complex(1, 0);
				for (int j = 0; j < len / 2; j++)
				{
					Complex u = a[i + j], v = a[i + j + len / 2] * w;
					a[i + j] = u + v;
					a[i + j + len / 2] = u - v;
					w *= wlen;
				}
			}
		}

		if (invert)
		{
			for (int i = 0; i < a.Length; i++)
				a[i] /= n;
		}
	}
}
