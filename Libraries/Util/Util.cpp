#include "Util.h"
#include "Arduino.h"

bool boolAnalogRead(byte pin)
{
	return analogRead(pin) > ANALOG_HIGH;
}

float floatAnalogRead(byte pin)
{
	return analogRead(pin) / 1023.0f;
}

void floatAnalogWrite(byte pin, float value)
{
	analogWrite(pin, (int) round(value * 255));
}

int medianAnalogRead(byte pin)
{
#ifdef _DEBUG
	unsigned long t = micros();
#endif

	struct Bucket 
	{
		int Value;
		byte Count;
	};

	static const byte SampleCount = 32;
	static const byte AveragedBucketCount = 3;
	static Bucket buckets[16];

	byte bucketCount = 0;
	byte mostHits = 0;
	byte mostHitBuckets[AveragedBucketCount];

	memset(&buckets, 0, sizeof buckets);
	memset(&mostHitBuckets, 0, sizeof mostHitBuckets);

	for (byte i = 0; i < SampleCount; i++)
	{
		int sample = analogRead(pin);

		bool bucketFound = false;
		for (byte j = 0; j < bucketCount; j++)
			if (buckets[j].Value == sample)
			{
				bucketFound = true;
				buckets[j].Count++;
				if (mostHits < buckets[j].Count)
				{
					for (byte k = 1; k < AveragedBucketCount; k++)
						mostHitBuckets[k] = mostHitBuckets[k - 1];

					mostHitBuckets[0] = j;
					mostHits = buckets[j].Count;
				}
			}

		if (!bucketFound)
		{
			bucketCount++;
			buckets[bucketCount].Value = sample;
			buckets[bucketCount].Count = 1;
			// skip "most hit" test/set for count == 1
		}
	}

	float accum = 0;
	float weight = 0;
	for (byte i = 0; i < AveragedBucketCount; i++)
	{
		float thisWeight = 1.0f / (i + 1);
		accum += (float) buckets[mostHitBuckets[i]].Value * thisWeight;
		weight += thisWeight;
	}
	int value = (int) round(accum / weight);

#ifdef _DEBUG
	t = micros() - t;
#endif
	trace(P("sampled %i from %hhu buckets in %u ms"), value, bucketCount, (unsigned int) (t / 1000));

	return value;
}
